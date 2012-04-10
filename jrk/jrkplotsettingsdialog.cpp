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

#include "jrkplotsettingsdialog.h"
#include "ui_jrkplotsettingsdialog.h"
#include "jrkplotdialog.h"

#include <QColorDialog>
#include <qwt_plot_curve.h>

//---------------------------------------------------------------------------
JrkPlotSettingsDialog::JrkPlotSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JrkPlotSettingsDialog)
{

    ui->setupUi(this);
    setLayout(ui->mainLayout);

    plotdlg = (JrkPlotDialog *) parent;

    ui->historySb->setValue(plotdlg->history());
    ui->intervalSb->setValue(plotdlg->interval());

    curveProperties(true);
}

//---------------------------------------------------------------------------
JrkPlotSettingsDialog::~JrkPlotSettingsDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------
void JrkPlotSettingsDialog::curveProperties(bool set)
{
    QToolButton *tb;
    QSpinBox    *sb;
    JrkPlotData *data;
    QPen        pen;
    int         i;

    for(i=0; i<(signed) plotdlg->jrkdata.size(); i++) {
        data = (JrkPlotData *)plotdlg->jrkdata.at(i);

        switch(i) {
        case JrkPlotDialog::curve_input:
            tb = ui->inputTb;
            sb = ui->inputSb;
            break;
        case JrkPlotDialog::curve_target:
            tb = ui->targetTb;
            sb = ui->targetSb;
            break;
        case JrkPlotDialog::curve_feedback:
            tb = ui->feedbackTb;
            sb = ui->feedbackSb;
            break;
        case JrkPlotDialog::curve_scaled_feedback:
            tb = ui->scaledfeedbackTb;
            sb = ui->scaledfeedbackSb;
            break;
        case JrkPlotDialog::curve_error:
            tb = ui->errorTb;
            sb = ui->errorSb;
            break;
        case JrkPlotDialog::curve_integral:
            tb = ui->integralTb;
            sb = ui->integralSb;
            break;
        case JrkPlotDialog::curve_derivative:
            tb = ui->derivativeTb;
            sb = ui->derivativeSb;
            break;
        case JrkPlotDialog::curve_duty_cycle:
            tb = ui->dutycycletTb;
            sb = ui->dutycycleSb;
            break;
        case JrkPlotDialog::curve_duty_cycle_target:
            tb = ui->dutycycletargetTb;
            sb = ui->dutycycletargetSb;
            break;
        case JrkPlotDialog::curve_current:
            tb = ui->currentTb;
            sb = ui->currentSb;
            break;

        default:
            return;
        }


        if(set) {
            sb->setValue(data->scale);
            setButtonColor(tb, data->curve->pen().color(), false);
        }
        else {
            data->scale = sb->value();

            pen = data->curve->pen();
            pen.setColor(buttonColor(tb));
            data->curve->setPen(pen);
        }
    }

}

//---------------------------------------------------------------------------
void JrkPlotSettingsDialog::setButtonColor(QToolButton *tb, QColor cl, bool select)
{
    if(select) {
        cl = QColorDialog::getColor(cl, this);
        if(!cl.isValid())
            return;
    }

    QString str;

    str.sprintf("QToolButton#%s { background-color:rgb(%d,%d,%d); color:rgb(%d,%d,%d); }",
                tb->objectName().toStdString().c_str(),
                cl.red(), cl.green(), cl.blue(),
                cl.red(), cl.green(), cl.blue());
    tb->setStyleSheet(str);
}

//---------------------------------------------------------------------------
QColor JrkPlotSettingsDialog::buttonColor(QToolButton *tb)
{
    QPalette pal = tb->palette();

    return pal.color(QPalette::Window);
}

//---------------------------------------------------------------------------
void JrkPlotSettingsDialog::on_inputTb_clicked()
{
    setButtonColor(ui->inputTb, buttonColor(ui->inputTb), true);
}

//---------------------------------------------------------------------------
void JrkPlotSettingsDialog::on_targetTb_clicked()
{
    setButtonColor(ui->targetTb, buttonColor(ui->targetTb), true);
}

//---------------------------------------------------------------------------
void JrkPlotSettingsDialog::on_feedbackTb_clicked()
{
    setButtonColor(ui->feedbackTb, buttonColor(ui->feedbackTb), true);
}

//---------------------------------------------------------------------------
void JrkPlotSettingsDialog::on_scaledfeedbackTb_clicked()
{
    setButtonColor(ui->scaledfeedbackTb, buttonColor(ui->scaledfeedbackTb), true);
}

//---------------------------------------------------------------------------
void JrkPlotSettingsDialog::on_errorTb_clicked()
{
    setButtonColor(ui->errorTb, buttonColor(ui->errorTb), true);
}

//---------------------------------------------------------------------------
void JrkPlotSettingsDialog::on_integralTb_clicked()
{
    setButtonColor(ui->integralTb, buttonColor(ui->integralTb), true);
}

//---------------------------------------------------------------------------
void JrkPlotSettingsDialog::on_derivativeTb_clicked()
{
    setButtonColor(ui->derivativeTb, buttonColor(ui->derivativeTb), true);
}

//---------------------------------------------------------------------------
void JrkPlotSettingsDialog::on_dutycycletargetTb_clicked()
{
    setButtonColor(ui->dutycycletargetTb, buttonColor(ui->dutycycletargetTb), true);
}

//---------------------------------------------------------------------------
void JrkPlotSettingsDialog::on_dutycycletTb_clicked()
{
    setButtonColor(ui->dutycycletTb, buttonColor(ui->dutycycletTb), true);
}

//---------------------------------------------------------------------------
void JrkPlotSettingsDialog::on_currentTb_clicked()
{
    setButtonColor(ui->currentTb, buttonColor(ui->currentTb), true);
}

//---------------------------------------------------------------------------
void JrkPlotSettingsDialog::on_buttonBox_accepted()
{
    JrkPlotData *data;
    int         i;

    curveProperties(false);

    // reallocate vectors
    if(plotdlg->history() != ui->historySb->value() ||
       plotdlg->interval() != ui->intervalSb->value())
    {
        plotdlg->history(ui->historySb->value());
        plotdlg->interval(ui->intervalSb->value());
        plotdlg->setSamples();

        free(plotdlg->timeData);
        plotdlg->timeData = (double *) malloc(plotdlg->samples() * sizeof(double));

        for(i=0; i<(signed) plotdlg->jrkdata.size(); i++) {
            data = (JrkPlotData *)plotdlg->jrkdata.at(i);

            free(data->data);
            data->data = (double *) malloc(plotdlg->samples() * sizeof(double));
        }
    }

}

//---------------------------------------------------------------------------
