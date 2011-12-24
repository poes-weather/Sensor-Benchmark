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
#include <QSettings>
#include <QFile>
#include <QMessageBox>

#include "si21xxdialog.h"
#include "ui_si21xxdialog.h"
#include "si21xx.h"
#include "utils.h"

//---------------------------------------------------------------------------
SI21xxDialog::SI21xxDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SI21xxDialog)
{
    ui->setupUi(this);

    si21xx = new TSI21xx;
    si_timer = new QTimer(this);
    si_timer->setInterval(50);

    connect(si_timer, SIGNAL(timeout()), this, SLOT(onSIReadWrite()));
    connect(this, SIGNAL(finished(int)), this, SLOT(onSI21xxDialog_finished(int)));

    enablecontrols(false);
}

//---------------------------------------------------------------------------
SI21xxDialog::~SI21xxDialog()
{
    delete ui;

    delete si21xx;

    delete si_timer;
}

//---------------------------------------------------------------------------
void SI21xxDialog::onSIReadWrite(void)
{
    if(si21xx->isOpen())
        status();
}

//---------------------------------------------------------------------------
void SI21xxDialog::onSI21xxDialog_finished(int)
{
    si_timer->stop();

    si21xx->i2c_close();

    // qDebug("onSI21xxDialog_finished(...)");

    enablecontrols(false);
}

//---------------------------------------------------------------------------
void SI21xxDialog::enablecontrols(bool enb)
{
    ui->openDevBtn->setText(enb ? "Close":"Open");
    ui->tuneBtn->setEnabled(enb);
}

//---------------------------------------------------------------------------
void SI21xxDialog::on_openDevBtn_clicked()
{
    si_timer->stop();

    if(si21xx->isOpen()) {
        si21xx->i2c_close();
        enablecontrols(false);

        return;
    }
    else {
        if(!si21xx->i2c_open(ui->i2c_dev->text().toStdString().c_str())) {
            QMessageBox::critical(this, "Error!", "Failed to open or communicate with i2c device!");
            return;
        }
    }

    int i;

    // General Tab
    ui->si21_id_label->setText(si21xx->getIdStr());
    ui->modulationCB->setCurrentIndex(si21xx->isBPSK() ? 0:1);
    ui->fecCB->setCurrentIndex((int) si21xx->fec());
    ui->descramblerCB->setChecked(si21xx->bypass_descrambler());
    ui->rsCB->setChecked(si21xx->bypass_reed_solomon());
    ui->deintCB->setChecked(si21xx->bypass_deinterleaver());

    // Si2107/9 can not change LNB voltage to 18V only Si2108/10
    ui->lnbCB->setCurrentIndex(si21xx->isLNB_13V() ? 0:1);
    i = si21xx->getId();
    ui->lnbCB->setEnabled((i == 2107 || i == 2109) ? false:true);

    // Tune Tab
    ui->csrCB->setCurrentIndex((int) si21xx->carrier_search_range());
    ui->loSB->setValue(si21xx->lnb_lo());
    ui->freqSB->setValue(si21xx->frequency() + si21xx->lnb_lo());
    ui->symbolrateSB->setValue(si21xx->symbolrate() * 1.0e-3);

    enablecontrols(true);

    si_timer->start();
}

//---------------------------------------------------------------------------
void SI21xxDialog::on_tuneBtn_clicked()
{
    si21xx->setdemodulation(ui->modulationCB->currentIndex() == 0 ? true:false);
    si21xx->fec((fec_t) ui->fecCB->currentIndex());
    si21xx->bypass(ui->descramblerCB->isChecked(), ui->rsCB->isChecked(), ui->deintCB->isChecked());
    si21xx->lnb_voltage(ui->lnbCB->currentIndex() == 0 ? true:false);

    si21xx->carrier_search_range((carrier_search_range_t) ui->csrCB->currentIndex());
    si21xx->lnb_lo(ui->loSB->value());

    if(!si21xx->tune(ui->freqSB->value(), ui->symbolrateSB->value() * 1.0e-3)) {
        QMessageBox::critical(this, "Error!", "Failed to tune! Check input parameters!");
        return;
    }

    unsigned short lock_status, acq_status;
    int i = 0;

    qDebug("\nTuning... [%s:%d]", __FILE__, __LINE__);

    delay(500);

    while(i++ < 100) {
        lock_status = si21xx->getlockstatus();
        acq_status  = si21xx->getacqusitionstatus();

        qDebug("%03d: Lock status: 0x%04x, Acqusiotion errors: 0x%04x", i, lock_status, acq_status);
        delay(100);

        if((lock_status & LOCK_RCV_LOCK) || (acq_status & ACQ_ACQ_FAIL))
            break;
    }

    qDebug("Locked %s", (lock_status & LOCK_RCV_LOCK) ? "YES":"NO");

    status();
}

//---------------------------------------------------------------------------
void SI21xxDialog::status(void)
{
    QString str;
    unsigned short lock_status, acq_status;

    lock_status = si21xx->getlockstatus();
    acq_status  = si21xx->getacqusitionstatus();

    str.sprintf("Lock Status: 0x%04x", lock_status);
    ui->lockstatusLabel->setText(str);
    str.sprintf("Acqusition Errors: 0x%04x", acq_status);
    ui->acqusitionstatusLabel->setText(str);

    ui->agclCb->setChecked(lock_status & LOCK_AGC_DONE ? true:false);
    ui->celCb->setChecked(lock_status & LOCK_CE_DONE ? true:false);
    ui->srlCb->setChecked(lock_status & LOCK_SR_DONE ? true:false);
    ui->stlCb->setChecked(lock_status & LOCK_ST_LOCK ? true:false);
    ui->crlCb->setChecked(lock_status & LOCK_CR_LOCK ? true:false);
    ui->vtlCb->setChecked(lock_status & LOCK_VT_LOCK ? true:false);
    ui->fslCb->setChecked(lock_status & LOCK_FS_LOCK ? true:false);
    ui->rcvlCb->setChecked(lock_status & LOCK_RCV_LOCK ? true:false);
    ui->bsdaCb->setChecked(lock_status & LOCK_BSD_READY ? true:false);
    ui->bsdoCb->setChecked(lock_status & LOCK_BS_DONE ? true:false);

    ui->aqfCb->setChecked(acq_status & ACQ_ACQ_FAIL ? true:false);
    ui->agcfCb->setChecked(acq_status & ACQ_AGC_FAIL ? true:false);
    ui->cefCb->setChecked(acq_status & ACQ_CE_FAIL ? true:false);
    ui->srfCb->setChecked(acq_status & ACQ_SR_FAIL ? true:false);
    ui->stfCb->setChecked(acq_status & ACQ_ST_FAIL ? true:false);
    ui->crfCb->setChecked(acq_status & ACQ_CR_FAIL ? true:false);
    ui->vtfCb->setChecked(acq_status & ACQ_VT_FAIL ? true:false);
    ui->fsfCb->setChecked(acq_status & ACQ_FS_FAIL ? true:false);

    ui->signaPGB->setValue(si21xx->getsignalstrength());
    ui->snrPGB->setValue(si21xx->getsnr());

    str.sprintf("%d", (int) si21xx->getber());
    ui->berLabel->setText(str);
}

//---------------------------------------------------------------------------

