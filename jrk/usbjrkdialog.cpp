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
#include <QTimer>
#include <QDateTime>
#include <QFile>
#include <QSettings>
#include <QFileDialog>
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include "usbjrkdialog.h"
#include "ui_usbjrkdialog.h"
#include "tusb.h"
#include "usbdevice.h"
#include "jrkusb.h"
#include "jrkplotdialog.h"
#include "jrklut.h"
#include "jrklutdialog.h"
#include "osdialog.h"

#include "utils.h"


//---------------------------------------------------------------------------
#define WF_INIT               1
#define WF_NO_UPDATE          2
#define WF_CALIBRATING        4

#define WF_CALIB_MIN_FB      64
#define WF_CALIB_MAX_FB     128
#define WF_VELOCITY         256
#define WF_FORWARD          512

static const int TIMER_INTERVAL_MS = 100;

#ifndef RINT
#   define RINT(x) (floor(x) + 0.5)
#endif
//---------------------------------------------------------------------------
USBJrkDialog::USBJrkDialog(QString _ini, OSDialog *compass_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::USBJrkDialog)
{
    ui->setupUi(this);
    setLayout(ui->mainLayout);

    compass = compass_; // compass window must be open...
    ini = _ini;

    out_fp = NULL;
    jrk = NULL;
    wFlags = 0;

    memset(&pid_vars, 0, sizeof(jrk_pid_variables_t));

    iobuffer = (unsigned char *) malloc(JRK_IO_BUF_SIZE);
    usb = new TUSB();
    jrkusb = new TJrkUSB;
    jrkPlot = new JrkPlotDialog(&jrkusb->vars, &pid_vars, this);

    jrk_timer = new QTimer(this);
    jrk_timer->setInterval(TIMER_INTERVAL_MS);

    connect(jrk_timer, SIGNAL(timeout()), this, SLOT(onJrkReadWrite()));
    connect(this, SIGNAL(finished(int)), this, SLOT(onJrkDialog_finished(int)));

    jrk_timer->stop();
}

//---------------------------------------------------------------------------
USBJrkDialog::~USBJrkDialog()
{
    delete jrkusb;

    delete usb;
    free(iobuffer);

    delete jrk_timer;

    if(jrkPlot->isVisible())
        jrkPlot->close();

    delete jrkPlot;

    vector<JrkLUT *>::iterator i = jrklut.begin();
    for(; i < jrklut.end(); i++)
        delete *i;
    jrklut.clear();

    if(out_fp)
        fclose(out_fp);

    delete ui;
}

//---------------------------------------------------------------------------
void USBJrkDialog::onJrkDialog_finished(int)
{
    jrk_timer->stop();

    if(jrk) {
        on_stopBtn_clicked();

        writeSettings();

        if(out_fp)
            on_recordLUTButton_clicked();

    }
}

//---------------------------------------------------------------------------
void USBJrkDialog::init(void)
{
    this->show();

    on_refreshBtn_clicked();
}

//---------------------------------------------------------------------------
void USBJrkDialog::readSettings(void)
{
    QFile file(ini);

    if(!file.exists(ini))
        return;

    QSettings reg(ini, QSettings::IniFormat);
    jrkusb->readSettings(&reg);

    ui->mindegSb->setValue(jrkusb->minDegrees());
    ui->maxdegSb->setValue(jrkusb->maxDegrees());
    ui->targetTodegreesLUTcb->setChecked(jrkusb->isFlagOn(JRK_USE_LUT));

}

//---------------------------------------------------------------------------
void USBJrkDialog::writeSettings(void)
{
    QSettings reg(ini, QSettings::IniFormat);
    jrkusb->writeSettings(&reg);

    qDebug("INI: %s", ini.toStdString().c_str());
}

//---------------------------------------------------------------------------
void USBJrkDialog::onJrkReadWrite(void)
{
    if(!this->isVisible() || !jrk || (wFlags & (WF_INIT | WF_NO_UPDATE)))
        return;

    QDateTime dt;
    QString str;
    double v, sfb_deg, target_deg;
    int i;

    if(!jrkusb->readVariables())
        return;

    dt = QDateTime::currentDateTime();

    toggleErrors();
    timer_loop++;

    v = pid_vars.error;
    pid_vars.error = jrkusb->vars.scaledFeedback - jrkusb->vars.target;
    pid_vars.integral = jrkusb->vars.errorSum;
    pid_vars.derivative = pid_vars.error - v;


    // realtime Jrk labels
    str.sprintf("%d", jrkusb->vars.feedback);
    ui->feedbackLabel->setText(str);
    str.sprintf("%d", jrkusb->vars.target);
    ui->targetLabel->setText(str);
    str.sprintf("%d", jrkusb->vars.scaledFeedback);
    ui->scaledfbLabel->setText(str);
    str.sprintf("%.0f", pid_vars.error);
    ui->pidErrorLabel->setText(str);


    if(!(wFlags & WF_CALIBRATING)) {
        sfb_deg    = jrkusb->toDegrees(jrkusb->vars.scaledFeedback, 8);
        target_deg = jrkusb->toDegrees(jrkusb->vars.target, 8);

        str.sprintf("%.3f", target_deg);
        ui->targetDegreesLabel->setText(str);

        str.sprintf("%.3f", sfb_deg);
        ui->scaledfbDegreesLabel->setText(str);

        str.sprintf("%.3f", sfb_deg - target_deg);
        ui->degErrorLabel->setText(str);

        if(out_fp) {
            i = abs(startdt.time().secsTo(dt.time()));
            if(i >= (ui->recDelaySb->value())) {
                startdt = QDateTime::currentDateTime();

#if 1

                v = getComapssDegrees();
                qDebug("compass degrees: %d:%f", ui->targetSlider->value(), v);

                fprintf(out_fp, "Target-%04d=%d\n", ui->targetSlider->value(), jrkusb->vars.target);
                fprintf(out_fp, "Degrees-%04d=%f\n", ui->targetSlider->value(), v);

#else

                //fprintf(out_fp, "Target\tFeedback\tJrk Degrees\tTrue degrees\n");
                fprintf(out_fp, "%d\t%d\t%.3f\t%.3f\n",
                        jrkusb->vars.target,
                        jrkusb->vars.feedback,
                        jrkusb->toDegrees(jrkusb->vars.target),
                        compass->getPitch());

#endif

                if(ui->targetSlider->value() == 4095) {
                    on_stopBtn_clicked();

                    return; // done
                }

                i = ui->targetSlider->value() + ui->recTargetStepSb->value();
                ui->targetSlider->setValue(i);


                rec_count++;
            }


        }
    }

    if(wFlags & WF_CALIB_MIN_FB) {
        ui->feedbackMin->setValue(jrkusb->vars.feedback);
        ui->feedbackDisconnectMin->setValue(jrkusb->vars.feedback - 20);

        wFlags &= ~WF_CALIB_MIN_FB;
    }

    if(wFlags & WF_CALIB_MAX_FB) {
        ui->feedbackMax->setValue(jrkusb->vars.feedback);
        ui->feedbackDisconnectMax->setValue(jrkusb->vars.feedback + 20); // (4095 - jrkusb->vars.scaledFeedback) / 2);

        wFlags &= ~WF_CALIB_MAX_FB;
    }


}

//---------------------------------------------------------------------------
double USBJrkDialog::getComapssDegrees(void)
{
    double v = 0;

    if(!compass || !compass->isOpen())
        return v;

    switch(ui->osAxisCb->currentIndex()) {
    case 0:
    {
        v = compass->getPitch();
        // adjust it to be from 0 to 180 degrees and not -90 to +90 degrees
        if(v < 0)
            v = 90.0 + v;
        else
            v = v + 90.0;

        break;
    }

    case 1: v = compass->getHeading(); break;
    case 2: v = compass->getRoll(); break;

    default:
        v = 0;
    }

    return v;
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_devicesCB_currentIndexChanged(int index)
{
    if(wFlags & WF_INIT)
        return;

    jrk_timer->stop();
    timer_loop = 0;
    wFlags = WF_INIT;

    jrk = usb->get_device(index);
    jrkusb->setDevice(jrk);

    readParameters();

    wFlags = 0;

    if(jrk) {
        readSettings();
        jrk_timer->start();
    }
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_refreshBtn_clicked()
{    
    jrk_timer->stop();

    timer_loop = 0;
    wFlags = WF_INIT;

    ui->devicesCB->clear();

    if(usb->list_devices(JRK_VENDOR)) {
        usb->conf_devices(-1);
        usb->open_devices();
        ui->devicesCB->addItems(usb->device_names());
    }

    jrk = usb->get_device(ui->devicesCB->currentIndex());
    jrkusb->setDevice(jrk);

    readSettings();
    readParameters();

    wFlags = 0;
    if(jrk)
        jrk_timer->start();
}

//---------------------------------------------------------------------------
void USBJrkDialog::readParameters(void)
{

    if(!jrk)
        return;

    wFlags |= WF_INIT;

    unsigned char u8, u8_2;
    unsigned short u16, u16_2;

    // read feedback parameters
    qDebug("read feedback parameters");

    u8 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_MODE);
    ui->feedbackTypeCb->setCurrentIndex(u8);
    u8 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_POWER_WITH_AUX);
    ui->auxdisconnectCb->setChecked(u8 & 1 ? true:false);
    // 1 byte unsigned value, 0-8 - averages together 4 * 2^x samples
    u8 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_ANALOG_SAMPLES_EXPONENT);
    ui->adcSamplesCb->setCurrentIndex(u8);

    qDebug("adc samples: %d", u8);

    u8 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_INVERT);
    ui->feedbackInvertedCb->setChecked(u8 & 1 ? true:false);
    u16 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_MINIMUM);
    ui->feedbackMin->setValue(u16);
    u16 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_MAXIMUM);
    ui->feedbackMax->setValue(u16);
    u16 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_DISCONNECT_MINIMUM);
    ui->feedbackDisconnectMin->setValue(u16);
    u16 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_DISCONNECT_MAXIMUM);
    ui->feedbackDisconnectMax->setValue(u16);

    // read motor parameters
    qDebug("read motor parameters");

    u8 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_SERIAL_DEVICE_NUMBER);
    ui->motorIdSb->setValue(u8);
    u8 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_MOTOR_PWM_FREQUENCY);
    ui->pwmCB->setCurrentIndex(u8);
    u8 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_MOTOR_INVERT);
    ui->motorInvertCb->setChecked(u8 & 1 ? true:false);

    u16 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_MOTOR_MAX_DUTY_CYCLE_FORWARD);
    ui->motorMaxDutyCycleSb->setValue(u16);
    u16 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_MOTOR_MAX_ACCELERATION_FORWARD);
    ui->motorMaxAccelerationSb->setValue(u16);
    u8 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_MOTOR_BRAKE_DURATION_FORWARD);
    ui->motorBrakeDurationSb->setValue(u8 * 5);

    u8 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_MOTOR_MAX_CURRENT_FORWARD);
    u8_2 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_MOTOR_CURRENT_CALIBRATION_FORWARD);
    ui->motorMaxCurrentSb->setValue(u8*u8_2 / 1000.0);
    ui->motorCurrentCalibSb->setValue(u8_2);

    // read PID parameters
    qDebug("read PID parameters");

    u16 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_PROPORTIONAL_MULTIPLIER);
    ui->pidPropMultSb->setValue(u16);
    u8 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_PROPORTIONAL_EXPONENT);
    ui->pidPropExpSb->setValue(u8);

    u16 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_INTEGRAL_MULTIPLIER);
    ui->pidIntMultSb->setValue(u16);
    u8 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_INTEGRAL_EXPONENT);
    ui->pidIntExpSb->setValue(u8);

    u16 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_DERIVATIVE_MULTIPLIER);
    ui->pidDerMultSb->setValue(u16);
    u8 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_DERIVATIVE_EXPONENT);
    ui->pidDerExpSb->setValue(u8);

    u16 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_PID_PERIOD);
    ui->pidPeriodSb->setValue(u16);
    u16 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_PID_INTEGRAL_LIMIT);
    ui->pidIntegralLimitSb->setValue(u16);
    u8 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_PID_RESET_INTEGRAL);
    ui->pidResetIntegralCb->setChecked(u8 & 1 ? true:false);
    u8 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_DEAD_ZONE);
    ui->feedbackDeadZoneSb->setValue(u8);

    // error settings
    u16 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_ERROR_ENABLE);
    u16_2 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_ERROR_LATCH);
    setErrorSettings(u16, u16_2);


    // set target slider etc
    jrkusb->readVariables();

    if(ui->targetSlider->value() == jrkusb->vars.target)
        on_targetSlider_valueChanged(jrkusb->vars.target);

    ui->targetSlider->setValue(jrkusb->vars.target);

    wFlags &= ~WF_INIT;

    setResolution();
    calcProportional();
    calcIntegral();
    calcDerivative();
}

//---------------------------------------------------------------------------
void USBJrkDialog::setErrorSettings(quint16 error_enable, quint16 error_latch)
{
    quint16 enable, latch;

    latch  = error_latch & JRK_ERROR(JRK_ERROR_NO_POWER);
    ui->err_powerCb->setCurrentIndex(latch ? 1:0);

    latch  = error_latch & JRK_ERROR(JRK_ERROR_MOTOR_DRIVER);
    ui->err_motordrvCb->setCurrentIndex(latch ? 1:0);

    latch  = error_latch & JRK_ERROR(JRK_ERROR_INPUT_INVALID);
    ui->err_inputinvalidCb->setCurrentIndex(latch ? 1:0);

    enable = error_enable & JRK_ERROR(JRK_ERROR_INPUT_DISCONNECT);
    latch  = error_latch & JRK_ERROR(JRK_ERROR_INPUT_DISCONNECT);
    ui->err_inputdisconnectCb->setCurrentIndex(to3itemIndex(enable, latch));

    enable = error_enable & JRK_ERROR(JRK_ERROR_FEEDBACK_DISCONNECT);
    latch  = error_latch & JRK_ERROR(JRK_ERROR_FEEDBACK_DISCONNECT);
    ui->err_fbdisconnectCb->setCurrentIndex(to3itemIndex(enable, latch));

    enable = error_enable & JRK_ERROR(JRK_ERROR_MAXIMUM_CURRENT_EXCEEDED);
    latch  = error_latch & JRK_ERROR(JRK_ERROR_MAXIMUM_CURRENT_EXCEEDED);
    ui->err_currentCb->setCurrentIndex(to3itemIndex(enable, latch));
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_writeErrorsButton_clicked()
{
    if(!jrk)
        return;

    quint16 enabled = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_ERROR_ENABLE);
    quint16 latched = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_ERROR_LATCH);

    twoItemsToError(&latched, JRK_ERROR(JRK_ERROR_NO_POWER), ui->err_powerCb->currentIndex());
    twoItemsToError(&latched, JRK_ERROR(JRK_ERROR_MOTOR_DRIVER), ui->err_motordrvCb->currentIndex());
    twoItemsToError(&latched, JRK_ERROR(JRK_ERROR_INPUT_INVALID), ui->err_inputinvalidCb->currentIndex());


    threeItemsToError(&enabled, &latched, JRK_ERROR(JRK_ERROR_INPUT_DISCONNECT), ui->err_inputdisconnectCb->currentIndex());
    threeItemsToError(&enabled, &latched, JRK_ERROR(JRK_ERROR_FEEDBACK_DISCONNECT), ui->err_fbdisconnectCb->currentIndex());
    threeItemsToError(&enabled, &latched, JRK_ERROR(JRK_ERROR_MAXIMUM_CURRENT_EXCEEDED), ui->err_currentCb->currentIndex());

    enabled |= JRK_ERRORS_ALWAYS_ENABLED;
    latched |= JRK_ERRORS_ALWAYS_LATCHED;

    set_parameter_u16(JRK_PARAMETER_ERROR_ENABLE, enabled);
    set_parameter_u16(JRK_PARAMETER_ERROR_LATCH, latched);
}


//---------------------------------------------------------------------------
int USBJrkDialog::to3itemIndex(bool enable, bool latch)
{
    /*
      i = 1 = disable
      i = 2 = enable
      i = 3 = enable and latch
      */

    int i = 0;

    if(!enable && !latch)
        i = 0;
    else {
        if(enable)
            i = 1;
        if(latch)
            i = 2;
    }

    return i;
}

//---------------------------------------------------------------------------
void USBJrkDialog::threeItemsToError(quint16 *error_enable, quint16 *error_latch, quint16 value, int index)
{
    *error_enable &= ~value;
    *error_latch &= ~value;

    /*
      index
      1 = disable
      2 = enable
      3 = enable and latch
      */

    if(index == 1 || index == 2) {
        *error_enable |= value;
        if(index == 2)
            *error_latch |= value;
    }
}

//---------------------------------------------------------------------------
void USBJrkDialog::twoItemsToError(quint16 *error_latch, quint16 value, int index)
{
    *error_latch &= ~value;
    /*
      index
      0 = enable
      1 = enable and latch
      */
    *error_latch |= index == 1 ? value:0;

}

//---------------------------------------------------------------------------
void USBJrkDialog::on_targetSlider_valueChanged(int value)
{
    if(!jrk || value < 0 || value > 4095)
        return;

    QString str;
    str.sprintf("%d", value);
    ui->targetSlider->setToolTip(str);

    wFlags |= WF_NO_UPDATE;

    if(!(wFlags & WF_INIT))
        jrkusb->setTarget(value);

    wFlags &= ~WF_NO_UPDATE;
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_applyFeedbackBtn_clicked()
{
    if(!jrk)
        return;

    on_stopBtn_clicked();

    wFlags |= WF_NO_UPDATE;

    set_parameter_u8(JRK_PARAMETER_FEEDBACK_INVERT, ui->feedbackInvertedCb->isChecked() ? 1:0);
    set_parameter_u8(JRK_PARAMETER_FEEDBACK_ANALOG_SAMPLES_EXPONENT, ui->adcSamplesCb->currentIndex());

    set_parameter_u16(JRK_PARAMETER_FEEDBACK_MINIMUM, ui->feedbackMin->value());
    set_parameter_u16(JRK_PARAMETER_FEEDBACK_MAXIMUM, ui->feedbackMax->value());
    set_parameter_u16(JRK_PARAMETER_FEEDBACK_DISCONNECT_MINIMUM, ui->feedbackDisconnectMin->value());
    set_parameter_u16(JRK_PARAMETER_FEEDBACK_DISCONNECT_MAXIMUM, ui->feedbackDisconnectMax->value());

    jrk->control_write(JRK_REQUEST_SET_TYPE, JRK_REQUEST_REINITIALIZE, 0, 0);

    jrkusb->minFeedback(ui->feedbackMin->value());
    jrkusb->maxFeedback(ui->feedbackMax->value());

    setResolution();

    wFlags &= ~WF_NO_UPDATE;
}

//---------------------------------------------------------------------------
void USBJrkDialog::set_parameter_u8(unsigned char id, unsigned char value)
{
    unsigned short paramid = id + (1 << 8);

    jrk->control_write(JRK_REQUEST_SET_TYPE,
                       JRK_REQUEST_SET_PARAMETER,
                       value,
                       paramid);
}

//---------------------------------------------------------------------------
void USBJrkDialog::set_parameter_u16(unsigned char id, unsigned short value)
{
    unsigned short paramid = id + (2 << 8);

    jrk->control_write(JRK_REQUEST_SET_TYPE,
                       JRK_REQUEST_SET_PARAMETER,
                       value,
                       paramid);
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_stopBtn_clicked()
{
    if(!jrk)
        return;

    wFlags |= WF_NO_UPDATE;

    jrk->control_write(JRK_REQUEST_SET_TYPE, JRK_REQUEST_MOTOR_OFF, 0, 0);

    if(out_fp)
        on_recordLUTButton_clicked();

    wFlags &= ~WF_NO_UPDATE;
    wFlags = 0;
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_clrerrorsBtn_clicked()
{
    if(!jrk)
        return;

    wFlags |= WF_NO_UPDATE;

    jrk->control_write(JRK_REQUEST_SET_TYPE, JRK_REQUEST_CLEAR_ERRORS, 0, 0);

    wFlags &= ~WF_NO_UPDATE;
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_applyDegBtn_clicked()
{
    wFlags |= WF_NO_UPDATE;

    wFlags &= ~WF_CALIBRATING;

    ui->gotoDegSb->setMinimum(ui->mindegSb->value());
    ui->gotoDegSb->setMaximum(ui->maxdegSb->value());

    jrkusb->minDegrees(ui->mindegSb->value());
    jrkusb->maxDegrees(ui->maxdegSb->value());
    jrkusb->calcOffsetDegrees();

    writeSettings();

    if(jrkusb->loadLUT()) {
        ui->mindegSb->setValue(jrkusb->minDegrees());
        ui->maxdegSb->setValue(jrkusb->maxDegrees());
    }

    setResolution();

    wFlags &= ~WF_NO_UPDATE;
}

//---------------------------------------------------------------------------
void USBJrkDialog::setResolution(void)
{
    QString str;
    double delta_fb = ui->feedbackMax->value() - ui->feedbackMin->value();
    double delta_deg = ui->maxdegSb->value() - ui->mindegSb->value();

    if(delta_deg >= 0 && delta_deg <= 360)
        str.sprintf("%.3f", delta_deg / delta_fb);
    else
        str = "?";

    ui->degResolutionLabel->setText(str);
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_gotoMinBtn_clicked()
{
    wFlags |= WF_NO_UPDATE;

    ui->targetSlider->setValue(0);

    wFlags &= ~WF_NO_UPDATE;
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_gotoMaxBtn_clicked()
{
    wFlags |= WF_NO_UPDATE;

    ui->targetSlider->setValue(4095);

    wFlags &= ~WF_NO_UPDATE;
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_gotodegBtn_clicked()
{
    if(jrk) {
        unsigned short t = jrkusb->toValue(ui->gotoDegSb->value(), 1);

        ui->targetSlider->setValue(t);
        //qDebug("goto target: %d", t);
    }
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_calibrateMinFeedbackBtn_clicked()
{
    if(jrk)
        wFlags |= WF_CALIB_MIN_FB;
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_calibrateMaxFeedbackBtn_clicked()
{
    if(jrk)
        wFlags |= WF_CALIB_MAX_FB;
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_motorCurrentCalibSb_valueChanged(int arg1)
{
    if(wFlags & WF_INIT)
        return;

    int maxA = RINT(ui->motorMaxCurrentSb->value() * 1000.0 / arg1);
    double A = maxA * arg1 / 1000.0;

    ui->motorMaxCurrentSb->setValue(A);
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_applyMotorBtn_clicked()
{
    if(!jrk)
        return;

    on_stopBtn_clicked();

    wFlags |= WF_NO_UPDATE;

    unsigned char u8;

    set_parameter_u8(JRK_PARAMETER_SERIAL_DEVICE_NUMBER, ui->motorIdSb->value());
    set_parameter_u8(JRK_PARAMETER_MOTOR_PWM_FREQUENCY, ui->pwmCB->currentIndex());
    set_parameter_u8(JRK_PARAMETER_MOTOR_INVERT, ui->motorInvertCb->isChecked() ? 1:0);

    set_parameter_u16(JRK_PARAMETER_MOTOR_MAX_DUTY_CYCLE_FORWARD, ui->motorMaxDutyCycleSb->value());
    set_parameter_u16(JRK_PARAMETER_MOTOR_MAX_DUTY_CYCLE_REVERSE, ui->motorMaxDutyCycleSb->value());

    set_parameter_u16(JRK_PARAMETER_MOTOR_MAX_ACCELERATION_FORWARD, ui->motorMaxAccelerationSb->value());
    set_parameter_u16(JRK_PARAMETER_MOTOR_MAX_ACCELERATION_REVERSE, ui->motorMaxAccelerationSb->value());

    set_parameter_u8(JRK_PARAMETER_MOTOR_BRAKE_DURATION_FORWARD, ui->motorBrakeDurationSb->value() / 5);
    set_parameter_u8(JRK_PARAMETER_MOTOR_BRAKE_DURATION_REVERSE, ui->motorBrakeDurationSb->value() / 5);

    u8 = (unsigned char) RINT(ui->motorMaxCurrentSb->value() * 1000.0 / ui->motorCurrentCalibSb->value());
    set_parameter_u8(JRK_PARAMETER_MOTOR_MAX_CURRENT_FORWARD, u8);
    set_parameter_u8(JRK_PARAMETER_MOTOR_MAX_CURRENT_REVERSE, u8);

    set_parameter_u8(JRK_PARAMETER_MOTOR_CURRENT_CALIBRATION_FORWARD, ui->motorCurrentCalibSb->value());
    set_parameter_u8(JRK_PARAMETER_MOTOR_CURRENT_CALIBRATION_REVERSE, ui->motorCurrentCalibSb->value());

    ui->motorMaxCurrentSb->setValue(u8 * ui->motorCurrentCalibSb->value() / 1000.0);

    jrk->control_write(JRK_REQUEST_SET_TYPE, JRK_REQUEST_REINITIALIZE, 0, 0);

    wFlags &= ~WF_NO_UPDATE;
}

//---------------------------------------------------------------------------
// see http://en.wikipedia.org/wiki/PID_controller
void USBJrkDialog::on_applyPIDBtn_clicked()
{
    if(!jrk)
        return;

    on_stopBtn_clicked();

    wFlags |= WF_NO_UPDATE;

    set_parameter_u16(JRK_PARAMETER_PROPORTIONAL_MULTIPLIER, ui->pidPropMultSb->value());
    set_parameter_u8(JRK_PARAMETER_PROPORTIONAL_EXPONENT, ui->pidPropExpSb->value());

    set_parameter_u16(JRK_PARAMETER_INTEGRAL_MULTIPLIER, ui->pidIntMultSb->value());
    set_parameter_u8(JRK_PARAMETER_INTEGRAL_EXPONENT, ui->pidIntExpSb->value());

    set_parameter_u16(JRK_PARAMETER_DERIVATIVE_MULTIPLIER, ui->pidDerMultSb->value());
    set_parameter_u8(JRK_PARAMETER_DERIVATIVE_EXPONENT, ui->pidDerExpSb->value());

    set_parameter_u16(JRK_PARAMETER_PID_PERIOD, ui->pidPeriodSb->value());
    set_parameter_u16(JRK_PARAMETER_PID_INTEGRAL_LIMIT, ui->pidIntegralLimitSb->value());

    set_parameter_u8(JRK_PARAMETER_PID_RESET_INTEGRAL, ui->pidResetIntegralCb->isChecked() ? 1:0);
    set_parameter_u8(JRK_PARAMETER_FEEDBACK_DEAD_ZONE, ui->feedbackDeadZoneSb->value());

    jrk->control_write(JRK_REQUEST_SET_TYPE, JRK_REQUEST_REINITIALIZE, 0, 0);

    wFlags &= ~WF_NO_UPDATE;
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_pidPropMultSb_valueChanged(int /* arg1 */)
{
    calcProportional();
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_pidPropExpSb_valueChanged(int /* arg1 */)
{
    calcProportional();
}

//---------------------------------------------------------------------------
double USBJrkDialog::calcProportional(void)
{
    pid_vars.cP = 0;

    if(!jrk || (wFlags & WF_INIT))
        return pid_vars.cP;

    QString str;

    pid_vars.cP = ((double) ui->pidPropMultSb->value()) / ((double) (1 << ui->pidPropExpSb->value()));
    str.sprintf("%.5f", pid_vars.cP);
    ui->pidPropLabel->setText(str);

    return pid_vars.cP;
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_pidIntMultSb_valueChanged(int /* arg1 */)
{
    calcIntegral();
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_pidIntExpSb_valueChanged(int /* arg1 */)
{
    calcIntegral();
}

//---------------------------------------------------------------------------
double USBJrkDialog::calcIntegral(void)
{
    pid_vars.cI = 0;

    if(!jrk || (wFlags & WF_INIT))
        return pid_vars.cI;

    QString str;

    pid_vars.cI = ((double) ui->pidIntMultSb->value()) / ((double) (1 << ui->pidIntExpSb->value()));
    str.sprintf("%.5f", pid_vars.cI);
    ui->pidIntLabel->setText(str);

    return pid_vars.cI;
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_pidDerMultSb_valueChanged(int /* arg1 */)
{
    calcDerivative();
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_pidDerExpSb_valueChanged(int /* arg1 */)
{
    calcDerivative();
}

//---------------------------------------------------------------------------
double USBJrkDialog::calcDerivative(void)
{
    pid_vars.cD = 0;

    if(!jrk || (wFlags & WF_INIT))
        return pid_vars.cD;

    QString str;

    pid_vars.cD = ((double) ui->pidDerMultSb->value()) / ((double) (1 << ui->pidDerExpSb->value()));
    str.sprintf("%.5f", pid_vars.cD);
    ui->pidDerLabel->setText(str);

    return pid_vars.cD;
}

//---------------------------------------------------------------------------
void USBJrkDialog::toggleErrors(void)
{
    unsigned short err;

    err = jrkusb->vars.errorOccurredBits | jrkusb->vars.errorFlagBits;

    QString str;

    str.sprintf("Error: 0x%04x", err);
    ui->errorLabel->setText(str);

    ui->errWaitingCb->setChecked(err & 0x0001);
    ui->errPowerCb->setChecked(err & 0x0002);
    ui->errMotorDriverCb->setChecked(err & 0x0004);
    ui->errInputInvalidCb->setChecked(err & 0x0008);
    ui->errInputDisconnectCb->setChecked(err & 0x0010);
    ui->errFeedbackDisconnectCb->setChecked(err & 0x0020);
    ui->errMaxCurrentCb->setChecked(err & 0x0040);
    ui->errSerialSigCb->setChecked(err & 0x0080);
    ui->errSerialOverrunCb->setChecked(err & 0x0100);
    ui->errSerialRXbuffCb->setChecked(err & 0x0200);
    ui->errCRCCb->setChecked(err & 0x0400);
    ui->errSerialProtocolCb->setChecked(err & 0x0800);
    ui->errTimeoutCb->setChecked(err & 0x1000);
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_velocityBtn_clicked()
{
    if(!jrk)
        return;

#if 0 // todo
    if(wFlags & WF_VELOCITY) {
        wFlags &= ~(WF_VELOCITY | WF_FORWARD);

        on_stopBtn_clicked();
    }
    else {
        start_deg = jrkusb->readPos(1);
        qDebug("Start pos: %g", start_deg);

        wFlags |= vars.target < 2048 ? WF_FORWARD:0;

        startdt = QDateTime::currentDateTime();
        ui->targetSlider->setValue(vars.target > 2048 ? 0:4095);

        timer_loop = 0;
        wFlags |= WF_VELOCITY;
    }

    ui->velocityBtn->setText(wFlags & WF_VELOCITY ? "Stop":"Start");
#endif
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_startJrkButton_clicked()
{
    if(!jrk || !ui->errWaitingCb->isChecked())
        return;

    on_clrerrorsBtn_clicked();
    jrkusb->setTarget(jrkusb->vars.target);
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_plotButton_clicked()
{        
    if(/*jrk && */ !jrkPlot->isVisible()) {
        jrkPlot->show();

        if(jrk)
            jrkPlot->run();
    }
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_editLUTbtn_clicked()
{
    JrkLUTDialog dlg(jrkusb->lutFile(), ui->mindegSb->value(), ui->maxdegSb->value(), this);

    if(dlg.exec() == QDialog::Accepted) {
        jrkusb->lutFile(dlg.getFilename());
    }

}

//---------------------------------------------------------------------------
void USBJrkDialog::on_targetTodegreesLUTcb_clicked()
{
    jrkusb->setFlag(JRK_USE_LUT, ui->targetTodegreesLUTcb->isChecked());
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_recordLUTButton_clicked()
{
    if(out_fp) {
        fclose(out_fp);
        out_fp = NULL;

        ui->recordLUTButton->setText("Record");

        ui->mindegSb->setValue(jrkusb->minDegrees());
        ui->maxdegSb->setValue(jrkusb->maxDegrees());

        return;
    }

    if(!jrk || !compass || !compass->isOpen()) {
        qDebug("FATAL Error: on_recordLUTButton_clicked.");
        return;
    }

    if(ui->targetSlider->value() != 0) {
        qDebug("Error: Set target slider should be at 0 (zero)!");
        return;
    }

    QString inifile = QFileDialog::getSaveFileName(this, tr("Save LUT File"), jrkusb->lutFile(), "INI files (*.ini);;All files (*.*)", 0);

    if(inifile.isEmpty())
        return;

    jrkusb->lutFile(inifile);

    jrkusb->setTarget(0); // power on

    rec_count = 0;
    startdt = QDateTime::currentDateTime();

    out_fp = fopen(inifile.toStdString().c_str(), "w");
    if(!out_fp) {
        qDebug("FATAL Error: failed to write to file!");
        return;
    }


#if 1
    fprintf(out_fp, "[JrkTargetToDegrees]\n");
#else
    fprintf(out_fp, "Target\tFeedback\tJrk Degrees\tTrue degrees\n");
#endif


    ui->recordLUTButton->setText("Stop");
}

//---------------------------------------------------------------------------


