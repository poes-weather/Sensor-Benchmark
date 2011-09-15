/*
    POES-Weather Ltd Sensor Benchmark, a software for testing different sensors used to align antennas.
    Copyright (C) 2011 Free Software Foundation, Inc.

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

#include "jrkdialog.h"
#include "ui_jrkdialog.h"
#include "jrk.h"

#define JRK_READ_FB              16
#define JRK_SETTARGET_HIRES      32
#define JRK_SETTARGET_LORES      64
#define JRK_STOP                128
#define JRK_ERRORS              256
#define JRK_READ                512
#define JRK_CLR_ERROR          1024

//---------------------------------------------------------------------------
JrkDialog::JrkDialog(QString ini, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JrkDialog)
{
    ui->setupUi(this);
    setLayout(ui->mainLayout);

    iniFile = ini;
    jrk = new TJRK;

    ui->portEd->setText(jrk->deviceName());
    ui->devId->setValue(jrk->devId());

    jrk_timer = new QTimer(this);
    jrk_timer->setInterval(10);
    connect(jrk_timer, SIGNAL(timeout()), this, SLOT(onJrkReadWrite()));
    connect(this, SIGNAL(finished(int)), this, SLOT(onJrkDialog_finished(int)));

    ui->openButton->setText("Open port");
    flags = 0;
}

//---------------------------------------------------------------------------
JrkDialog::~JrkDialog()
{
    jrk_timer->stop();
    delete jrk_timer;

    delete ui;
    delete jrk;
}

//---------------------------------------------------------------------------
void JrkDialog::onJrkDialog_finished(int /*result*/)
{
    qDebug("finished");

    jrk_timer->stop();

    if(jrk->isOpen())
        jrk->close();
}

//---------------------------------------------------------------------------
void JrkDialog::on_openButton_clicked()
{
    jrk_timer->stop();

    if(jrk->isOpen()) {
        jrk->close();
    }
    else {
        if(!jrk->open(ui->portEd->text(), ui->devId->value(), BAUD115200)) {
            ui->textEdit->append("Fatal Error: Failed to open port!");

            return;
        }
    }

    jrk->delay(100);

    QCoreApplication::processEvents();

    ui->openButton->setText(jrk->isOpen() ? "Close port":"Open port");

    if(!jrk->isOpen())
        return;

    ui->textEdit->clear();

    readAll();
    readErrors();    

#if 0
    int target = 3229;
    qDebug("byte 1: %02x", (unsigned char) (0x40 + (target & 0x001f)));
    qDebug("byte 2: %02x", (unsigned char) ((target >> 5) & 0x007f));
#endif

    flags = 0;

    jrk_timer->start();
}

//---------------------------------------------------------------------------
void JrkDialog::onJrkReadWrite(void)
{
    if(!jrk->isOpen())
        return;

    if(flags & JRK_STOP) {
#if 0
        quint16 data = jrk->clearError(); // jrk->readErrorOccurred();

        if(data & 0x0001)
            jrk->setTargetHiRes(ui->targetdial->value());
        else
            jrk->motorOff();
#else
        jrk->motorOff();
#endif

        flags &= ~JRK_STOP;

        return;
    }

    if(flags & JRK_CLR_ERROR) {
        jrk->motorOff();
        readErrors(false);
        flags &= ~JRK_CLR_ERROR;

        return;
    }

    if(flags & JRK_SETTARGET_HIRES) {
        jrk->setTargetHiRes(ui->targetdial->value());
        flags &= ~JRK_SETTARGET_HIRES;

        return;
    }

    if(flags & JRK_SETTARGET_LORES) {
        jrk->setTargetLowRes(ui->magnitudedial->value());
        flags &= ~JRK_SETTARGET_LORES;

        return;
    }

    if(flags & JRK_ERRORS) {
        readErrors(true);
        flags &= ~JRK_ERRORS;

        return;
    }

    if(flags & JRK_READ) {
        readAll();
        flags &= ~JRK_READ;

        return;
    }

    QString str;
    quint16 data2;

    data2 = jrk->readFeedback();
    ui->feedbackdial->setValue((int) data2);
    str.sprintf("%d", (int) data2); ui->feedbacklabel->setText(str);
}

//---------------------------------------------------------------------------
void JrkDialog::addStrings(QStringList *sl)
{
    for(int i=0; i<sl->count(); i++)
        ui->textEdit->append(sl->at(i));
}

//---------------------------------------------------------------------------
void JrkDialog::on_stopBtn_clicked()
{
    if(!jrk->isOpen()) {
        ui->textEdit->append("Error: Port is not open!");
        return;
    }

    flags |= JRK_STOP;
}

//---------------------------------------------------------------------------
void JrkDialog::on_readErrorsBtn_clicked()
{
    if(!jrk->isOpen()) {
        ui->textEdit->append("Error: Port is not open!");
        return;
    }

    flags |= JRK_ERRORS;
}

//---------------------------------------------------------------------------
void JrkDialog::readErrors(bool occured)
{
    if(!jrk->isOpen()) {
        ui->textEdit->append("Error: Port is not open!");
        return;
    }

    QStringList sl;
    QString str;
    quint16 data2;

    if(occured) {
        data2 = jrk->readErrorOccurred();
        str.sprintf("Error Flags Occurred: 0x%04X", data2);
    }
    else {
        data2 = jrk->clearError();
        str.sprintf("Error Flags Halting: 0x%04X", data2);
    }

    ui->textEdit->append(str);

    jrk->errorStr(&sl, data2);
    addStrings(&sl);
    ui->textEdit->append("");
}

//---------------------------------------------------------------------------
void JrkDialog::readAll()
{
    if(!jrk->isOpen()) {
        ui->textEdit->append("Error: Port is not open!");
        return;
    }

    QString str;
    quint16 data2;
    short   sv;
    int tmp = flags;

    data2 = jrk->readInput();
    str.sprintf("Input: %d [0x%04X]", (int) data2, data2); ui->textEdit->append(str);

    data2 = jrk->readTarget();
    str.sprintf("Target: %d [0x%04X]", (int) data2, data2); ui->textEdit->append(str);
    flags |= 1;
    ui->targetdial->setValue(data2 == 0xffff ? 0:data2);
    on_targetdial_valueChanged(data2 == 0xffff ? 0:data2);
    flags = tmp;

    data2 = jrk->readFeedback();
    ui->feedbackdial->setValue((int) data2);
    str.sprintf("%d", (int) data2); ui->feedbacklabel->setText(str);
    str.sprintf("Feedback: %d [0x%04X]", (int) data2, data2); ui->textEdit->append(str);

    data2 = jrk->readScaledFeedback();
    str.sprintf("Scaled Feedback: %d [0x%04X]", (int) data2, data2); ui->textEdit->append(str);

    sv = jrk->readDutycycle();
    str.sprintf("Duty cycle: %d [0x%04X]", (int) sv, (quint16) (sv & 0xffff)); ui->textEdit->append(str);

    ui->textEdit->append("");
}


//---------------------------------------------------------------------------
void JrkDialog::on_readBtn_clicked()
{
    if(!jrk->isOpen()) {
        ui->textEdit->append("Error: Port is not open!");
        return;
    }

    flags |= JRK_READ;
}

//---------------------------------------------------------------------------
void JrkDialog::on_targetdial_valueChanged(int value)
{
    QString str;
    double  degrees;
    int tmp = flags;

    str.sprintf("%d", value);
    ui->targetlabel->setText(str);

    degrees = jrk->target2degrees(value, 360);
    ui->degdial->setValue((int) degrees);
    str.sprintf("%.3f", degrees); ui->degreelabel->setText(str);

    flags |= 1;
    ui->magnitudedial->setValue(jrk->target2magnitude(value, ui->analogueCb->isChecked()));
    flags = tmp;

    if(!(flags & 1) && jrk->isOpen())
        flags |= JRK_SETTARGET_HIRES;
}

//---------------------------------------------------------------------------
void JrkDialog::on_targetdial_dialPressed()
{
    // flags |= 1;
}

//---------------------------------------------------------------------------
void JrkDialog::on_targetdial_sliderReleased()
{
#if 0
    flags &= ~1;
    on_targetdial_valueChanged(ui->targetdial->value());
    qDebug("on_targetdial_sliderReleased");
#endif
}

//---------------------------------------------------------------------------
void JrkDialog::on_magnitudedial_valueChanged(int value)
{
    QString str;
    int tmp = flags;

    str.sprintf("%d", value);
    ui->magnitudelabel->setText(str);

    flags |= 1;
    ui->targetdial->setValue(jrk->magnitude2target(value, ui->analogueCb->isChecked()));
    flags = tmp;

    if(!(flags & 1) && jrk->isOpen())
        flags |= JRK_SETTARGET_LORES;
}

//---------------------------------------------------------------------------
void JrkDialog::on_magnitudedial_dialPressed()
{
    // flags |= 1;
}

//---------------------------------------------------------------------------
void JrkDialog::on_magnitudedial_sliderReleased()
{
#if 0
    flags &= ~1;
    on_magnitudedial_valueChanged(ui->magnitudedial->value());
#endif
}

//---------------------------------------------------------------------------
void JrkDialog::on_clearErrButton_clicked()
{
    if(jrk->isOpen())
        flags |= JRK_CLR_ERROR;
}
