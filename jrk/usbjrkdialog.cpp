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

//---------------------------------------------------------------------------
#define WF_INIT               1
#define WF_NO_UPDATE          2
#define WF_CALIBRATING        4
#define WF_GOTO_MIN           8
#define WF_GOTO_MAX          16
#define WF_GOTO_DEG          32
#define WF_CALIB_MIN_FB      64
#define WF_CALIB_MAX_FB     128
#define WF_VELOCITY         256
#define WF_FORWARD          512

static const int TIMER_INTERVAL_MS = 50;

#define RINT(x) (floor(x) + 0.5)
//---------------------------------------------------------------------------
USBJrkDialog::USBJrkDialog(QString _ini, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::USBJrkDialog)
{
    ui->setupUi(this);
    setLayout(ui->mainLayout);
    ini = _ini;

#if 0
    ui->targetLabel->setText("");
    ui->degreesLabel->setText("");
    ui->feedbackLabel->setText("");
    ui->scaleddegreesLabel->setText("");
#endif

    ui->degSb->setVisible(false);
    ui->pushButton->setVisible(true);

    minfb = ui->feedbackMinDegSb->value();
    maxfb = ui->feedbackMaxDegSb->value();
    mindeg = ui->mindegSb->value();
    maxdeg = ui->maxdegSb->value();

    ui->accCb->setCurrentIndex(1);

    jrk = NULL;
    wFlags = 0;

    memset(&vars, 0, sizeof(jrk_variables_t));
    iobuffer = (unsigned char *) malloc(JRK_IO_BUF_SIZE);
    usb = new TUSB();
    jrkusb = new TJrkUSB;
    jrkPlot = new JrkPlotDialog(&vars, this);

    jrk_timer = new QTimer(this);
    jrk_timer->setInterval(TIMER_INTERVAL_MS); //10;
    connect(jrk_timer, SIGNAL(timeout()), this, SLOT(onJrkReadWrite()));
    connect(this, SIGNAL(finished(int)), this, SLOT(onJrkDialog_finished(int)));

    on_refreshBtn_clicked();
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

    delete ui;
}

//---------------------------------------------------------------------------
void USBJrkDialog::onJrkDialog_finished(int)
{
    jrk_timer->stop();

    if(jrk) {
        on_stopBtn_clicked();
        usleep(10 * 1000);

        writeSettings();
    }
}

//---------------------------------------------------------------------------
void USBJrkDialog::readSettings(void)
{
    QFile file(ini);

    if(!file.exists(ini))
        return;

    QSettings reg(ini, QSettings::IniFormat);
    jrkusb->readSettings(&reg);

    ui->feedbackMinDegSb->setValue(jrkusb->minFeedback());
    ui->feedbackMaxDegSb->setValue(jrkusb->maxFeedback());
    ui->mindegSb->setValue(jrkusb->minPos());
    ui->maxdegSb->setValue(jrkusb->maxPos());

    minfb = jrkusb->minFeedback();
    maxfb = jrkusb->maxFeedback();
    mindeg = jrkusb->minPos();
    maxdeg = jrkusb->maxPos();
}

//---------------------------------------------------------------------------
void USBJrkDialog::writeSettings(void)
{
#if 0
    QFile file(ini);

    if(file.exists(ini))
        file.remove();
#endif

    QSettings reg(ini, QSettings::IniFormat);
    jrkusb->writeSettings(&reg);

    qDebug("INI: %s", ini.toStdString().c_str());
}

//---------------------------------------------------------------------------
void USBJrkDialog::onJrkReadWrite(void)
{
    if(!jrk || (wFlags & (WF_INIT | WF_NO_UPDATE)))
        return;

    QDateTime dt;
    QSpinBox *sb;
    QString str;
    double degrees, d_fb, d_deg, sfb;
    double delta, delta2;
    int target;

    pid_vars.period = ui->pidPeriodSb->value();
    pid_vars.integral = ui->pidIntegralLimitSb->value();



    if(!jrk->control_transfer(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_VARIABLES, 0, 0, (char *)iobuffer, sizeof(jrk_variables_t)))
        return;

    vars = *(jrk_variables *) iobuffer;

    toggleErrors();

    timer_loop++;

    str.sprintf("%d", vars.feedback);
    ui->feedbackLabel->setText(str);
    str.sprintf("%d", vars.scaledFeedback);
    ui->scaledfbLabel->setText(str);

    dt = QDateTime::currentDateTime();

    if(wFlags & (WF_GOTO_MIN | WF_GOTO_MAX)) {
        sb = wFlags & WF_GOTO_MIN ? ui->feedbackMinDegSb:ui->feedbackMaxDegSb;
        sb->setValue(vars.feedback);
    }

    if(!(wFlags & WF_CALIBRATING)) {
        d_deg = maxdeg - mindeg;
        current_deg = 0;

        d_fb  = maxfb - minfb;
        if(d_fb > 0 && d_deg > 0) {
            sfb = vars.feedback - minfb;
            degrees = mindeg + (d_deg / d_fb) * sfb;
            current_deg = degrees;

            str.sprintf("%.3f", degrees);
            ui->degreesLabel->setText(str);
        }

        if(wFlags & WF_CALIB_MIN_FB) {
            ui->feedbackMin->setValue(vars.scaledFeedback);
            ui->feedbackDisconnectMin->setValue(vars.scaledFeedback / 2);

            wFlags &= ~WF_CALIB_MIN_FB;
        }

        if(wFlags & WF_CALIB_MAX_FB) {
            ui->feedbackMax->setValue(vars.scaledFeedback);
            ui->feedbackDisconnectMax->setValue(vars.scaledFeedback + (4095 - vars.scaledFeedback) / 2);

            wFlags &= ~WF_CALIB_MAX_FB;
        }

        if(wFlags & WF_GOTO_DEG) {
            // compute needed accuracy let Qt do that...
            ui->degSb->setValue(current_deg);

            delta = ui->degSb->value() - ui->gotoDegSb->value();

            if(delta == 0)
                return;


            target = vars.target;

            if(delta > 0)
                target -= 1;
            else if(delta < 0)
                target += 1;

            target = target < 0 ? 0:target > 4095 ? 4095:target;
            ui->targetSlider->setValue(target);

            usleep(10 * 1000);
        }

        if(wFlags & WF_VELOCITY && (timer_loop % 20) == 0) {
            delta = fabs(current_deg - start_deg);
            delta2 = startdt.msecsTo(dt);

            if(delta2 != 0)
                ui->velocitySb->setValue(delta2 / delta);

            if(delta2 > 5000) {
                // can we stop?
                if(wFlags & WF_FORWARD) {
                    if(vars.feedback >= (maxfb - 3))
                        on_velocityBtn_clicked();
                }
                else {
                    if(vars.feedback >= (minfb + 3))
                        on_velocityBtn_clicked();
                }
            }


        }



    }

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
    unsigned short u16;

    // read feedback parameters
    qDebug("read feedback parameters");

    u8 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_MODE);
    ui->feedbackTypeCb->setCurrentIndex(u8);
    u8 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_POWER_WITH_AUX);
    ui->auxdisconnectCb->setChecked(u8 & 1 ? true:false);
    // 1 byte unsigned value, 0-8 - averages together 4 * 2^x samples
    u8 = jrk->control_read_u8(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_ANALOG_SAMPLES_EXPONENT);
    ui->adcSamplesCb->setCurrentIndex(u8);
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

    // set target slider etc
    jrk->control_transfer(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_VARIABLES, 0, 0, (char *)iobuffer, sizeof(jrk_variables_t));
    vars = *(jrk_variables *) iobuffer;

    if(ui->targetSlider->value() == vars.target)
        on_targetSlider_valueChanged(vars.target);

    ui->targetSlider->setValue(vars.target);

    wFlags &= ~WF_INIT;

    calcProportional();
    calcIntegral();
    calcDerivative();
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
        jrk->control_write(JRK_REQUEST_SET_TYPE, JRK_REQUEST_SET_TARGET, value, 0);

    wFlags &= ~WF_NO_UPDATE;
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_applyFeedbackBtn_clicked()
{
    if(!jrk)
        return;

    wFlags |= WF_NO_UPDATE;

    set_parameter_u8(JRK_PARAMETER_FEEDBACK_INVERT, ui->feedbackInvertedCb->isChecked() ? 1:0);
    set_parameter_u16(JRK_PARAMETER_FEEDBACK_MINIMUM, ui->feedbackMin->value());
    set_parameter_u16(JRK_PARAMETER_FEEDBACK_MAXIMUM, ui->feedbackMax->value());
    set_parameter_u16(JRK_PARAMETER_FEEDBACK_DISCONNECT_MINIMUM, ui->feedbackDisconnectMin->value());
    set_parameter_u16(JRK_PARAMETER_FEEDBACK_DISCONNECT_MAXIMUM, ui->feedbackDisconnectMax->value());

    jrk->control_write(JRK_REQUEST_SET_TYPE, JRK_REQUEST_REINITIALIZE, 0, 0);

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

    wFlags &= ~(WF_CALIBRATING | WF_GOTO_MAX | WF_GOTO_MIN);

    minfb = ui->feedbackMinDegSb->value();
    maxfb = ui->feedbackMaxDegSb->value();
    mindeg = ui->mindegSb->value();
    maxdeg = ui->maxdegSb->value();

    ui->gotoDegSb->setMinimum(mindeg);
    ui->gotoDegSb->setMaximum(maxdeg);

    jrkusb->minFeedback(minfb);
    jrkusb->maxFeedback(maxfb);
    jrkusb->minPos(mindeg);
    jrkusb->maxPos(maxdeg);

    writeSettings();

    wFlags &= ~WF_NO_UPDATE;    
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_gotoMinBtn_clicked()
{
    wFlags |= WF_NO_UPDATE;

    wFlags &= ~WF_GOTO_MAX;
    wFlags |= WF_GOTO_MIN | WF_CALIBRATING;

    ui->targetSlider->setValue(ui->targetSlider->minimum());

    wFlags &= ~WF_NO_UPDATE;
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_gotoMaxBtn_clicked()
{
    wFlags |= WF_NO_UPDATE;

    wFlags &= ~WF_GOTO_MIN;
    wFlags |= WF_GOTO_MAX | WF_CALIBRATING;

    ui->targetSlider->setValue(ui->targetSlider->maximum());

    wFlags &= ~WF_NO_UPDATE;
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_pushButton_clicked()
{
#if 1

    if(!jrk)
        return;

    FILE *fp;
    jrk_variables v;
    QString str;
    int i;
    double degrees, d_fb, d_deg, sfb;

    fp = fopen("jrk-lut-test.txt", "w");
    if(!fp)
        return;

    ui->pushButton->setEnabled(false);
    ui->pushButton->setText("Recording...");

    fprintf(fp, "Target\tScaled\tFeedback\tDegrees\n");

    d_fb  = maxfb - minfb;
    d_deg = maxdeg - mindeg;

    i = 0;

    while(i <= ui->targetSlider->maximum()) {
        ui->targetSlider->setValue(i);

        //usleep(10);

        jrk->control_transfer(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_VARIABLES, 0, 0, (char *)iobuffer, sizeof(jrk_variables_t));
        v = *(jrk_variables *) iobuffer;

        if(i != v.target)
            continue;

        i++;

        sfb = v.feedback - minfb;
        degrees = (mindeg + (d_deg / d_fb) * sfb);

        fprintf(fp, "%6d\t%6d\t%8d\t%f\n",
                v.target,
                v.scaledFeedback,
                v.feedback,
                degrees);
    }

    fclose(fp);

    on_stopBtn_clicked();

    ui->pushButton->setEnabled(true);
    ui->pushButton->setText("Record LUT");

#endif
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_gotodegBtn_clicked()
{
    if(!jrk)
        return;

    if(wFlags & WF_GOTO_DEG) {
        wFlags &= ~WF_GOTO_DEG;
        ui->gotodegBtn->setText("Start");
        ui->gotoDegSb->setDecimals(ui->accCb->count());
        on_stopBtn_clicked();
    }
    else {
        gotodeg = ui->gotoDegSb->value();

        wFlags |= WF_GOTO_DEG;
        ui->gotodegBtn->setText("Stop");

        ui->gotoDegSb->setDecimals(ui->accCb->currentIndex());
        ui->degSb->setDecimals(ui->accCb->currentIndex());
    }

    ui->targetSlider->setEnabled(wFlags & WF_GOTO_DEG ? false:true);
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
void USBJrkDialog::calcProportional(void)
{
    if(!jrk || (wFlags & WF_INIT))
        return;

    QString str;

    str.sprintf("%.5f", ((double) ui->pidPropMultSb->value()) / ((double) (1 << ui->pidPropExpSb->value())));
    ui->pidPropLabel->setText(str);
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
void USBJrkDialog::calcIntegral(void)
{
    if(!jrk || (wFlags & WF_INIT))
        return;

    QString str;

    str.sprintf("%.5f", ((double) ui->pidIntMultSb->value()) / ((double) (1 << ui->pidIntExpSb->value())));
    ui->pidIntLabel->setText(str);
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
void USBJrkDialog::calcDerivative(void)
{
    if(!jrk || (wFlags & WF_INIT))
        return;

    QString str;

    str.sprintf("%.5f", ((double) ui->pidDerMultSb->value()) / ((double) (1 << ui->pidDerExpSb->value())));
    ui->pidDerLabel->setText(str);
}

//---------------------------------------------------------------------------
void USBJrkDialog::toggleErrors(void)
{
    unsigned short err;

    err = vars.errorOccurredBits | vars.errorFlagBits;

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
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_plotButton_clicked()
{        
    if(/*jrk && */ !jrkPlot->isVisible()) {
        jrkPlot->show();
        //jrkPlot->run();
    }
}

//---------------------------------------------------------------------------
