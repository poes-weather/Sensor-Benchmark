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
#include <unistd.h>
#include <time.h>
#include <stdio.h>
#include <math.h>

#include "usbjrkdialog.h"
#include "ui_usbjrkdialog.h"
#include "tusb.h"
#include "usbdevice.h"

//---------------------------------------------------------------------------
#define WF_INIT          1
#define WF_NO_UPDATE     2
#define WF_CALIBRATING   4
#define WF_GOTO_MIN      8
#define WF_GOTO_MAX     16
#define WF_GOTO_DEG     32

//---------------------------------------------------------------------------
USBJrkDialog::USBJrkDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::USBJrkDialog)
{
    ui->setupUi(this);
    setLayout(ui->mainLayout);

    ui->targetLabel->setText("");
    ui->degreesLabel->setText("");
    ui->feedbackLabel->setText("");
    ui->scaleddegreesLabel->setText("");

    ui->degSb->setVisible(false);
    ui->pushButton->setVisible(false);

    minfb = ui->feedbackMinDegSb->value();
    maxfb = ui->feedbackMaxDegSb->value();
    scaledminfb = ui->scaledfeedbackMinDegSb->value();
    scaledmaxfb = ui->scaledfeedbackMaxDegSb->value();

    mindeg = ui->mindegSb->value();
    maxdeg = ui->maxdegSb->value();

    ui->accCb->setCurrentIndex(1);

    jrk_timer = new QTimer(this);
    jrk_timer->setInterval(10);
    connect(jrk_timer, SIGNAL(timeout()), this, SLOT(onJrkReadWrite()));
    connect(this, SIGNAL(finished(int)), this, SLOT(onJrkDialog_finished(int)));

    jrk = NULL;
    wFlags = 0;

    iobuffer = (unsigned char *) malloc(JRK_IO_BUF_SIZE);
    usb = new TUSB();
    on_refreshBtn_clicked();

}

//---------------------------------------------------------------------------
USBJrkDialog::~USBJrkDialog()
{

    delete usb;
    free(iobuffer);

    delete jrk_timer;

    delete ui;
}

//---------------------------------------------------------------------------
void USBJrkDialog::onJrkDialog_finished(int)
{
    jrk_timer->stop();

    if(jrk) {
        on_stopBtn_clicked();
        usleep(50 * 1000);
    }
}

//---------------------------------------------------------------------------
void USBJrkDialog::onJrkReadWrite(void)
{
    if(!jrk || (wFlags & (WF_INIT | WF_NO_UPDATE)))
        return;

    QSpinBox *sb;
    QString str;
    double degrees, d_fb, d_deg, sfb;
    double delta;
    int target;

    jrk->control_transfer(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_VARIABLES, 0, 0, (char *)iobuffer, sizeof(jrk_variables_t));
    vars = *(jrk_variables *) iobuffer;

    str.sprintf("%d", vars.feedback);
    ui->feedbackLabel->setText(str);
    str.sprintf("%d", vars.scaledFeedback);
    ui->scaledfbLabel->setText(str);

    if(wFlags & (WF_GOTO_MIN | WF_GOTO_MAX)) {

        sb = wFlags & WF_GOTO_MIN ? ui->feedbackMinDegSb:ui->feedbackMaxDegSb;
        sb->setValue(vars.feedback);

        sb = wFlags & WF_GOTO_MIN ? ui->scaledfeedbackMinDegSb:ui->scaledfeedbackMaxDegSb;
        sb->setValue(vars.scaledFeedback);
    }

    if(!(wFlags & WF_CALIBRATING)) {
        d_deg = maxdeg - mindeg;
        current_deg = 0;

        d_fb  = maxfb - minfb;
        if(d_fb > 0 && d_deg > 0) {
            sfb = vars.feedback - minfb;
            degrees = mindeg + (d_deg / d_fb) * sfb;

            str.sprintf("%.3f", degrees);
            ui->degreesLabel->setText(str);
        }

        d_fb  = scaledmaxfb - scaledminfb;
        if(d_fb > 0 && d_deg > 0) {
            sfb = vars.scaledFeedback - scaledminfb;
            degrees = mindeg + (d_deg / d_fb) * sfb;

            str.sprintf("%.3f", degrees);
            ui->scaleddegreesLabel->setText(str);

            current_deg = degrees;
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
            on_targetSlider_valueChanged(target);

            usleep(50 * 1000);
        }


    }

}

//---------------------------------------------------------------------------
void USBJrkDialog::on_refreshBtn_clicked()
{
    jrk_timer->stop();

    wFlags = WF_INIT;

    ui->devicesCB->clear();

    if(usb->list_devices(JRK_VENDOR)) {
        usb->conf_devices(-1);
        usb->open_devices();
        ui->devicesCB->addItems(usb->device_names());
    }

    jrk = usb->get_device(ui->devicesCB->currentIndex());
    if(!jrk) {

        return;
    }

    // read feedback parameters
    // controlTransfer(byte RequestType, byte Request, ushort Value, ushort Index, byte[] data)
    QString str;
    unsigned char u8;
    unsigned short u16;

    u8 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_INVERT);
    ui->feedbackInvertedCb->setChecked(u8 & 1 ? true:false);

    u16 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_MINIMUM);
    ui->feedbackMin->setValue(u16);
    u16 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_MAXIMUM);
    ui->feedbackMax->setValue(u16);
    u16 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_DISCONNECT_MINIMUM);
    ui->feedbackDisconnectMin->setValue(u16);
    u16 = jrk->control_read_u16(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_PARAMETER, 0, JRK_PARAMETER_FEEDBACK_DISCONNECT_MAXIMUM);
    ui->feedbackDisconnectMax->setValue(u16);

    jrk->control_transfer(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_VARIABLES, 0, 0, (char *)iobuffer, sizeof(jrk_variables_t));
    vars = *(jrk_variables *) iobuffer;

    if(ui->targetSlider->value() == vars.target)
        on_targetSlider_valueChanged(vars.target);

    ui->targetSlider->setValue(vars.target);

    // qDebug("size: %d", sizeof(jrk_variables_t));

    //jrk->control_write(JRK_REQUEST_GET_TYPE, JRK_REQUEST_MOTOR_OFF, 0, 0);

    wFlags &= ~WF_INIT;

    jrk_timer->start();
}

//---------------------------------------------------------------------------
void USBJrkDialog::on_targetSlider_valueChanged(int value)
{
    if(!jrk || wFlags & WF_INIT)
        return;

    if(value < 0 || value > 4095 || value == vars.target)
        return;

    QString str;
    str.sprintf("%d", value);
    ui->targetLabel->setText(str);

    wFlags |= WF_NO_UPDATE;

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
    scaledminfb = ui->scaledfeedbackMinDegSb->value();
    scaledmaxfb = ui->scaledfeedbackMaxDegSb->value();

    mindeg = ui->mindegSb->value();
    maxdeg = ui->maxdegSb->value();

    ui->gotoDegSb->setMinimum(mindeg);
    ui->gotoDegSb->setMaximum(maxdeg);

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
    if(!jrk)
        return;

    FILE *fp;
    jrk_variables v;
    QString str;
    int i;
    double degrees, d_fb, d_deg, sfb;

    fp = fopen("/home/patrik/tmp/jrk-lut.txt", "w");
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

        usleep(80 * 1000);

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
