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
#ifndef USBJRKDIALOG_H
#define USBJRKDIALOG_H

#include <QDialog>
#include <QDateTime>
#include <stdio.h>
#include "jrk_protocol.h"

class QTimer;
class TUSB;
class TUSBDevice;
class TJrkUSB;
class JrkPlotDialog;
class JrkLUT;

class OSDialog;

using namespace std;
namespace Ui {
    class USBJrkDialog;
}

//---------------------------------------------------------------------------
class USBJrkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit USBJrkDialog(QString _ini, OSDialog *compass_, QWidget *parent = 0);
    ~USBJrkDialog();

    void init(void);

private slots:
    void onJrkReadWrite(void);
    void onJrkDialog_finished(int result);
    void on_refreshBtn_clicked();
    void on_targetSlider_valueChanged(int value);
    void on_applyFeedbackBtn_clicked();
    void on_stopBtn_clicked();
    void on_clrerrorsBtn_clicked();
    void on_applyDegBtn_clicked();
    void on_gotoMinBtn_clicked();
    void on_gotoMaxBtn_clicked();
    void on_pushButton_clicked();
    void on_gotodegBtn_clicked();

    void on_calibrateMinFeedbackBtn_clicked();
    void on_calibrateMaxFeedbackBtn_clicked();
    void on_motorCurrentCalibSb_valueChanged(int arg1);
    void on_applyMotorBtn_clicked();
    void on_applyPIDBtn_clicked();
    void on_pidPropMultSb_valueChanged(int arg1);
    void on_pidPropExpSb_valueChanged(int arg1);
    void on_pidIntMultSb_valueChanged(int arg1);
    void on_pidIntExpSb_valueChanged(int arg1);
    void on_pidDerMultSb_valueChanged(int arg1);
    void on_pidDerExpSb_valueChanged(int arg1);
    void on_devicesCB_currentIndexChanged(int index);
    void on_velocityBtn_clicked();
    void on_plotButton_clicked();
    void on_startJrkButton_clicked();
    void on_editLUTbtn_clicked();

    void on_targetTodegreesLUTcb_clicked();
    void on_recordLUTButton_clicked();

protected:
    void readSettings(void);
    void writeSettings(void);

    void readParameters(void);
    void set_parameter_u8(unsigned char id, unsigned char value);
    void set_parameter_u16(unsigned char id, unsigned short value);
    double calcProportional(void);
    double calcIntegral(void);
    double calcDerivative(void);
    void toggleErrors(void);
    void setResolution(void);
    double getComapssDegrees(void);


private:
    Ui::USBJrkDialog *ui;
    QTimer  *jrk_timer;
    int     wFlags;
    QString ini;

    TUSB *usb;
    TUSBDevice    *jrk;
    TJrkUSB       *jrkusb;
    JrkPlotDialog *jrkPlot;

    jrk_pid_variables  pid_vars;
    vector<JrkLUT *>   jrklut;

    OSDialog *compass;
    unsigned char *iobuffer;
    unsigned long timer_loop;
    unsigned short last_err;

    double current_deg;
    //int    gototarget;

    QDateTime startdt;
    double    start_deg;
    int       rec_count;
    FILE      *out_fp;

};

#endif // USBJRKDIALOG_H
