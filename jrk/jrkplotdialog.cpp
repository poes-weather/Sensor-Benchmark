/*
    POES-USRP, a software for recording and decoding POES high resolution weather satellite images.
    Copyright (C) 2009-2011 Free Software Foundation, Inc.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

    Email: <postmaster@poes-weather.com>
    Web: <http://www.poes-weather.com>
*/
//---------------------------------------------------------------------------
#include "jrkplotdialog.h"
#include "ui_jrkplotdialog.h"
#include <QTimer>

#include <qwt_plot_curve.h>
#include <qwt_plot_layout.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_draw.h>
#include <qwt_scale_widget.h>
#include <qwt_legend.h>
#include <qwt_legend_item.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_grid.h>
#include <qwt_picker_machine.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_plot_renderer.h>
#include <qwt_plot_magnifier.h>
#include <qwt_curve_fitter.h>
#include <qwt_text.h>
#include <qwt_math.h>

#include "jrkplotsettingsdialog.h"


//---------------------------------------------------------------------------
JrkPlotDialog::JrkPlotDialog(jrk_variables *indata, jrk_pid_variables *pid_indata, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JrkPlotDialog)
{
    int i;

    ui->setupUi(this);
    setLayout(ui->mainLayout);

    interval(200);
    history(10);
    setSamples();

    data_ptr = indata;
    pid_data_ptr = pid_indata;

    timeData = (double *) malloc(SAMPLES * sizeof(double));
    reset();

    ui->jrkPlot->setAutoReplot(false);
    ui->jrkPlot->canvas()->setBorderRadius(0);
    ui->jrkPlot->plotLayout()->setAlignCanvasToScales(true);
    ui->jrkPlot->setCanvasBackground(Qt::white);

    QwtLegend *legend = new QwtLegend;
    legend->setItemMode(QwtLegend::CheckableItem);
    ui->jrkPlot->insertLegend(legend, QwtPlot::RightLegend);

    ui->jrkPlot->setAxisTitle(QwtPlot::xBottom, "Seconds");
    ui->jrkPlot->setAxisScale(QwtPlot::xBottom, timeData[0], timeData[SAMPLES - 1]);

//    ui->jrkPlot->setAxisLabelRotation( QwtPlot::xBottom, -50.0 );
    ui->jrkPlot->setAxisLabelAlignment( QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom );
    QwtScaleWidget *scaleWidget = ui->jrkPlot->axisWidget(QwtPlot::xBottom);
    i = QFontMetrics(scaleWidget->font()).height();
    scaleWidget->setMinBorderDist(0, i / 2);

    ui->jrkPlot->setAxisTitle(QwtPlot::yLeft, "%");
    ui->jrkPlot->setAxisScale(QwtPlot::yLeft, -100, 100);

    // picker, panner, zoomer
    plot_picker = new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft,
                                    QwtPlotPicker::CrossRubberBand, QwtPicker::AlwaysOn,
                                    ui->jrkPlot->canvas());
#if 0
    plot_picker->setStateMachine(new QwtPickerDragPointMachine());
    plot_picker->setRubberBandPen(QColor(Qt::black));
    plot_picker->setRubberBand(QwtPicker::CrossRubberBand );
#endif
    plot_picker->setTrackerPen(QColor(Qt::black));

    // panning with the left mouse button
    plot_panner = new QwtPlotPanner(ui->jrkPlot->canvas());
    plot_panner->setUpdatesEnabled(true);

    // zoom in/out with the wheel
    plot_zoomer = new QwtPlotMagnifier(ui->jrkPlot->canvas());

    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin( true );
    grid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    grid->setMinPen(QPen(Qt::black, 0, Qt::DotLine));
    grid->attach(ui->jrkPlot);

    // curves, scale is in %
    createCurve("Input",                Qt::gray,           false,  4095);
    createCurve("Target",               Qt::blue,           false,  4095);
    createCurve("Feedback",             Qt::darkBlue,       false,  4095);
    createCurve("Scaled feedback",      Qt::magenta,        false,  4095);
    createCurve("Error",                Qt::red,            true,   4095);
    createCurve("Integral",             Qt::darkGreen,      true,   1000);
    createCurve("Derivative",           Qt::yellow,         true,   1000);
    createCurve("Duty cycle target",    Qt::darkCyan,       true,   600);
    createCurve("Duty cycle",           Qt::darkRed,        false,  600);
    createCurve("Current",              Qt::black,          true,   50);

    plot_timer = new QTimer(this);
    plot_timer->setInterval(INTERVAL);

    connect(plot_timer, SIGNAL(timeout()), this, SLOT(onUpdateGraph()));
    connect(ui->jrkPlot, SIGNAL(legendChecked(QwtPlotItem *, bool)),
            SLOT(showCurve(QwtPlotItem *, bool)));

#if 0
    connect(plot_picker, SIGNAL(moved(const QPoint &)),
            SLOT(picker_moved(const QPoint &)));
    connect(plot_picker, SIGNAL(selected(const QPolygon &)),
            SLOT(picker_selected(const QPolygon &)));
#endif

    connect(this, SIGNAL(finished(int)), this, SLOT(onFinished(int)));

//    plot_timer->start();
}

//---------------------------------------------------------------------------
JrkPlotDialog::~JrkPlotDialog()
{
    plot_timer->stop();
    delete plot_timer;
    free(timeData);

    vector<JrkPlotData *>::iterator i = jrkdata.begin();
    for(; i < jrkdata.end(); i++)
        delete *i;

    jrkdata.clear();

    delete plot_picker;
    delete plot_panner;
    delete plot_zoomer;

    delete ui;
}

//---------------------------------------------------------------------------
void JrkPlotDialog::onFinished(int)
{
    if(plot_timer->isActive())
        on_stopButton_clicked();

    //qDebug("JrkPlotDialog::onFinished");
}

//---------------------------------------------------------------------------
void JrkPlotDialog::createCurve(QString title, QColor cl, bool on, double scale)
{
    QwtPlotCurve *curve;    
    QPen pen;

    curve = new QwtPlotCurve(title);

    pen.setColor(cl);
    pen.setWidth(2);

    curve->setPen(pen);

    curve->setPaintAttribute(QwtPlotCurve::ClipPolygons, true);
    curve->setRenderHint( QwtPlotCurve::RenderAntialiased, true);

#if 0
    QwtSplineCurveFitter* curveFitter = new QwtSplineCurveFitter();
    curveFitter->setSplineSize(500);
    curve->setCurveFitter(curveFitter);
#endif

    curve->attach(ui->jrkPlot);
    showCurve(curve, on);

    jrkdata.push_back(new JrkPlotData(curve, scale, samples()));
    // qDebug("Scale: %f", scale);
}

//---------------------------------------------------------------------------
void JrkPlotDialog::showCurve(QwtPlotItem *item, bool on)
{
    //qDebug("showCurve");

    item->setVisible(on);

    QwtLegendItem *legendItem =
        qobject_cast<QwtLegendItem *>(ui->jrkPlot->legend()->find(item));

    if(legendItem)
        legendItem->setChecked(on);

    //ui->jrkPlot->replot();
}

//---------------------------------------------------------------------------
void JrkPlotDialog::picker_moved(const QPoint &pos)
{
    QString info;
    info.sprintf( "Freq=%g, Ampl=%g, Phase=%g",
        ui->jrkPlot->invTransform( QwtPlot::xBottom, pos.x() ),
        ui->jrkPlot->invTransform( QwtPlot::yLeft, pos.y() ),
        ui->jrkPlot->invTransform( QwtPlot::yRight, pos.y() )
    );

}

//---------------------------------------------------------------------------
void JrkPlotDialog::picker_selected(const QPolygon &)
{

}

//---------------------------------------------------------------------------
void JrkPlotDialog::onUpdateGraph()
{
    JrkPlotData *curve;
    double v;
    int i, j, sample;

    // move data down one step
    if(dataCount >= SAMPLES) {
        v = INTERVAL/1000.0;

        for(i=0; i<(signed) jrkdata.size(); i++) {
            curve = (JrkPlotData *)jrkdata.at(i);

            for(j=1; j<SAMPLES; j++) {
                curve->data[j-1] = curve->data[j];

                if(i == 0)
                    timeData[j-1] += v;
            }
        }

        timeData[SAMPLES - 1] += v;
        ui->jrkPlot->setAxisScale(QwtPlot::xBottom, timeData[0], timeData[SAMPLES - 1]);
    }

    // qDebug("fb= %d sfb = %d", data_ptr->feedback, data_ptr->scaledFeedback);
    sample = dataCount < SAMPLES ? dataCount:(SAMPLES - 1);

    setCurveData(curve_input,               sample, data_ptr->input);
    setCurveData(curve_target,              sample, data_ptr->target);
    setCurveData(curve_feedback,            sample, data_ptr->feedback);
    setCurveData(curve_scaled_feedback,     sample, data_ptr->scaledFeedback);
    setCurveData(curve_error,               sample, pid_data_ptr->error);
    setCurveData(curve_integral,            sample, pid_data_ptr->integral);
    setCurveData(curve_derivative,          sample, pid_data_ptr->derivative);
    setCurveData(curve_duty_cycle_target,   sample, data_ptr->dutyCycleTarget);
    setCurveData(curve_duty_cycle,          sample, data_ptr->dutyCycle);
    setCurveData(curve_current,             sample, data_ptr->current);


    for(j=0; j<(signed) jrkdata.size(); j++) {
        curve = (JrkPlotData *)jrkdata.at(j);
        curve->curve->setRawSamples(timeData, curve->data, dataCount);
    }

    if(dataCount < SAMPLES)
        dataCount++;

    ui->jrkPlot->replot();
}

//---------------------------------------------------------------------------
void JrkPlotDialog::setCurveData(int curve_id, int data_id, double value)
{
    JrkPlotData *curve;

    curve = (JrkPlotData *)jrkdata.at(curve_id);
    if(curve) {
        curve->setValue(value, data_id);

#if 0
        qDebug("curve id: %d, data %f%%, value: %f", curve_id, curve->data[data_id], value);
#endif
    }
}

//---------------------------------------------------------------------------
// derivative is current error minus the previous error
// current error must be assigned before assigning derivative
// integral is computed as the sum of the error over all PID cycles
void JrkPlotDialog::setCurveDerivative(int data_id)
{
    JrkPlotData *e_curve, *d_curve;
    double v;

    e_curve = (JrkPlotData *)jrkdata.at(curve_error);
    d_curve = (JrkPlotData *)jrkdata.at(curve_derivative);

    if(e_curve && d_curve) {
        if(data_id > 0)
            v =  e_curve->data[data_id] - e_curve->data[data_id -1];
        else
            v = 0;

        d_curve->data[data_id] = v;
    }
}

//---------------------------------------------------------------------------
void JrkPlotDialog::run(void)
{
    if(!plot_timer->isActive())
        on_stopButton_clicked();
}

//---------------------------------------------------------------------------
void JrkPlotDialog::reset(void)
{
    double v;
    int i;

    v = INTERVAL/1000.0;
    for(i=0; i<SAMPLES; i++) {
        timeData[i] = ((double) i) * v;
    }


    dataCount = 0;
    ui->jrkPlot->setAxisScale(QwtPlot::xBottom, timeData[0], timeData[SAMPLES - 1]);
//    ui->jrkPlot->replot();
}

//---------------------------------------------------------------------------
//
//          Data from Jrk
//
//---------------------------------------------------------------------------
JrkPlotData::JrkPlotData(QwtPlotCurve *curve_, double scale_, int samples)
{
    curve = curve_;
    scale = scale_;
    data = (double *) malloc(samples * sizeof(double));
}

//---------------------------------------------------------------------------
JrkPlotData::~JrkPlotData(void)
{
    free(data);
}

//---------------------------------------------------------------------------
void JrkPlotData::setValue(double value, int index)
{
    if(scale != 0)
        data[index] = value * 100.0 / scale;
    else
        data[index] = value;

#if 0
    if(index == 4)
        qDebug("data = %g, value = %f scale = %f", data[index], value, scale);
#endif
}

//---------------------------------------------------------------------------
void JrkPlotDialog::on_stopButton_clicked()
{
    if(plot_timer->isActive()) {
        plot_timer->stop();
        ui->stopButton->setText("Run");
    }
    else {
        reset();
        plot_timer->start();
        ui->stopButton->setText("Stop");
    }

}

//---------------------------------------------------------------------------
void JrkPlotDialog::on_settingsButton_clicked()
{
    bool running = plot_timer->isActive();
    bool changed = false;

    if(running)
        plot_timer->stop();

    JrkPlotSettingsDialog dlg(this);
    if(dlg.exec() == QDialog::Accepted)
        changed = true;

    if(running) {
        if(changed)
            on_stopButton_clicked(); // reset
        else
            plot_timer->start(); // continue
    }
}

//---------------------------------------------------------------------------
