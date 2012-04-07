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

const int INTERVAL = 500; // msec
const int HISTORY = 11;
const int SAMPLES = HISTORY * 1000/INTERVAL;

//---------------------------------------------------------------------------
JrkPlotDialog::JrkPlotDialog(jrk_variables *indata, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JrkPlotDialog)
{
    int i;

    ui->setupUi(this);
    setLayout(ui->mainLayout);

    data_ptr = indata;
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

    ui->jrkPlot->setAxisLabelRotation( QwtPlot::xBottom, -50.0 );
    ui->jrkPlot->setAxisLabelAlignment( QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom );
    QwtScaleWidget *scaleWidget = ui->jrkPlot->axisWidget(QwtPlot::xBottom);
    i = QFontMetrics(scaleWidget->font()).height();
    scaleWidget->setMinBorderDist(0, i / 2);

    // ui->jrkPlot->setAxisScaleDraw( QwtPlot::xBottom, new TimeScaleDraw( cpuStat.upTime() ) );
#if 0
    setAxisTitle(QwtPlot::xBottom, " System Uptime [h:m:s]" );
    setAxisScaleDraw( QwtPlot::xBottom, new TimeScaleDraw( cpuStat.upTime() ) );
    setAxisScale( QwtPlot::xBottom, 0, HISTORY );
    setAxisLabelRotation( QwtPlot::xBottom, -50.0 );
    setAxisLabelAlignment( QwtPlot::xBottom, Qt::AlignLeft | Qt::AlignBottom );

/*
     In situations, when there is a label at the most right position of the
     scale, additional space is needed to display the overlapping part
     of the label would be taken by reducing the width of scale and canvas.
     To avoid this "jumping canvas" effect, we add a permanent margin.
     We don't need to do the same for the left border, because there
     is enough space for the overlapping label below the left scale.
     */

    QwtScaleWidget *scaleWidget = axisWidget( QwtPlot::xBottom );
    const int fmh = QFontMetrics( scaleWidget->font() ).height();
    scaleWidget->setMinBorderDist( 0, fmh / 2 );
#endif

    ui->jrkPlot->setAxisTitle(QwtPlot::yLeft, "%");
    ui->jrkPlot->setAxisScale(QwtPlot::yLeft, -100, 100);

    // grid
    QwtPlotGrid *grid = new QwtPlotGrid;
    grid->enableXMin( true );
    grid->setMajPen(QPen(Qt::black, 0, Qt::DotLine));
    grid->setMinPen(QPen(Qt::black, 0, Qt::DotLine));
    grid->attach(ui->jrkPlot);

    // curves, scale is in %
    createCurve("Input",                Qt::gray,           false,  100.0/4095.0);
    createCurve("Target",               Qt::blue,           true,   100.0/4095.0);
    createCurve("Feedback",             Qt::darkBlue,       true,   100.0/4095.0);
    createCurve("Scaled feedback",      Qt::magenta,        true,   100.0/4095.0);
    createCurve("Error",                Qt::red,            true,   100.0/4095);
    createCurve("Integral",             Qt::darkGreen,      false,  100.0/1000.0);
    createCurve("Duty cycle target",    Qt::darkCyan,       true,   100.0/600.0);
    createCurve("Duty cycle",           Qt::darkRed,        true,   100.0/600.0);
    createCurve("Current",              Qt::black,          true,   100.0/5000.0);

    plot_timer = new QTimer(this);
    plot_timer->setInterval(INTERVAL);

    connect(plot_timer, SIGNAL(timeout()), this, SLOT(onUpdateGraph()));
    connect(ui->jrkPlot, SIGNAL(legendChecked(QwtPlotItem *, bool)),
            SLOT(showCurve(QwtPlotItem *, bool)));
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

    pen.setColor(cl);
    pen.setWidth(2);

    curve = new QwtPlotCurve(title);
    curve->setPen(pen);
    curve->attach(ui->jrkPlot);
    showCurve(curve, on);

    jrkdata.push_back(new JrkPlotData(curve, scale));
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
/*
createCurve("Input", Qt::lightGray, false, 100/4095);
createCurve("Target", Qt::blue, true, 100/4095);
createCurve("Feedback", Qt::darkBlue, false, 100/4095);
createCurve("Scaled feedback", Qt::magenta, false, 100/4095);
createCurve("Error", Qt::red, false, 100/1024);
createCurve("Integral", Qt::darkGreen, false, 100/1000);
createCurve("Duty cycle target", Qt::darkCyan, false, 100/600);
createCurve("Duty cycle", Qt::darkMagenta, false, 100/600);
createCurve("Current", Qt::darkGray, false, 100/5000);
*/

void JrkPlotDialog::onUpdateGraph()
{
    JrkPlotData *curve;
    double v;
    int i, j;

    // move data down one step
    if(dataCount >= SAMPLES) {
        v = INTERVAL/1000.0;
        for(i=1; i<dataCount; i++) {
            for(j=0; j<(signed) jrkdata.size(); j++) {
                curve = (JrkPlotData *)jrkdata.at(j);
                curve->data[i-1] = curve->data[i];
            }

            timeData[i-1] += v;
        }

        timeData[SAMPLES - 1] += v;
        ui->jrkPlot->setAxisScale(QwtPlot::xBottom, timeData[0], timeData[SAMPLES - 1]);
    }

    // qDebug("fb= %d sfb = %d", data_ptr->feedback, data_ptr->scaledFeedback);

    i=0;
    setCurveData(i++, dataCount, data_ptr->input);
    setCurveData(i++, dataCount, data_ptr->target);
    setCurveData(i++, dataCount, data_ptr->feedback);
    setCurveData(i++, dataCount, data_ptr->scaledFeedback);
    setCurveData(i++, dataCount, data_ptr->scaledFeedback - data_ptr->target);
    setCurveData(i++, dataCount, 0); // integral
    setCurveData(i++, dataCount, data_ptr->dutyCycleTarget);
    setCurveData(i++, dataCount, data_ptr->dutyCycle);
    setCurveData(i++, dataCount, data_ptr->current);

    if(dataCount < SAMPLES) //HISTORY)
        dataCount++;

    for(j=0; j<(signed) jrkdata.size(); j++) {
        curve = (JrkPlotData *)jrkdata.at(j);
        curve->curve->setRawSamples(timeData, curve->data, dataCount);
    }

    ui->jrkPlot->replot();
}

//---------------------------------------------------------------------------
void JrkPlotDialog::setCurveData(int curve_id, int data_id, double value)
{
    JrkPlotData *curve;

    curve = (JrkPlotData *)jrkdata.at(curve_id);
    if(curve) {
        curve->setValue(value, data_id);

#if 1
        qDebug("curve id: %d, data %f%%, value: %f", curve_id, curve->data[data_id], value);
#endif
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
    ui->jrkPlot->replot();
}

//---------------------------------------------------------------------------
//
//          Data from Jrk
//
//---------------------------------------------------------------------------
JrkPlotData::JrkPlotData(QwtPlotCurve *curve_, double scale_)
{
    curve = curve_;
    scale = scale_;
    data = (double *) malloc(SAMPLES * sizeof(double));
}

//---------------------------------------------------------------------------
JrkPlotData::~JrkPlotData(void)
{
    if(data)
        free(data);
}

//---------------------------------------------------------------------------
void JrkPlotData::setValue(double value, int index)
{
    if(data != NULL && index >= 0 && index < SAMPLES) {
        data[index] = value * scale ;

#if 0
        if(index == 4)
            qDebug("data = %g, value = %f scale = %f", data[index], value, scale);
#endif
    }
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
