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
#include <QDateTime>
#include <QSettings>
#include <QFile>
#include <QMessageBox>

#include "gpsdialog.h"
#include "ui_gpsdialog.h"
#include "gps.h"
#include "mainwindow.h"
//#include "station.h"

#ifdef Q_OS_WIN
#  include <windows.h>
#endif

//---------------------------------------------------------------------------
GPSDialog::GPSDialog(QString ini, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GPSDialog)
{
    ui->setupUi(this);
    setLayout(ui->mainLayout);

    mw = (MainWindow *) parent;

    gps = new TGPS(ui->gpswidget);

    iniFile = ini;
    readSettings();

    connect(gps, SIGNAL(NMEAParsed()), this, SLOT(gpsDataAvailable()));
    connect(this, SIGNAL(finished(int)), this, SLOT(onGPSDialog_finished(int)));

    gpsDataAvailable();
}

//---------------------------------------------------------------------------
GPSDialog::~GPSDialog()
{
    // qDebug("destructor");
    writeSettings();

    gps->close();
    delete ui;
    delete gps;
}

//---------------------------------------------------------------------------
void GPSDialog::onGPSDialog_finished(int /*result*/)
{
    //qDebug("finished");

    //writeSettings();
    gps->close();
}

//---------------------------------------------------------------------------
void GPSDialog::writeSettings(void)
{
    QFile file(iniFile);

    if(file.exists(iniFile))
        file.remove();

    QSettings reg(iniFile, QSettings::IniFormat);

    reg.beginGroup("GPS");

      reg.setValue("DeviceId", ui->gpsPortEd->text());
      reg.setValue("Baudrate", ui->baudrateCb->currentIndex());
      reg.setValue("Flowcontrol", ui->flowControlCb->currentIndex());
      reg.setValue("UTCTime", ui->utcTimeCb->isChecked());
      reg.setValue("DMS", ui->dmsCb->isChecked());

    reg.endGroup();
}

//---------------------------------------------------------------------------
void GPSDialog::readSettings(void)
{
    QFile file(iniFile);

    if(!file.exists(iniFile)) {
        ui->gpsPortEd->setText(gps->deviceName());
        return;
    }

    QSettings reg(iniFile, QSettings::IniFormat);

    reg.beginGroup("GPS");

      ui->gpsPortEd->setText(reg.value("DeviceId", "").toString());
      ui->baudrateCb->setCurrentIndex(reg.value("Baudrate", 1).toInt());
      ui->flowControlCb->setCurrentIndex(reg.value("Flowcontrol", 0).toInt());
      ui->utcTimeCb->setChecked(reg.value("UTCTime", 0).toBool());
      ui->dmsCb->setChecked(reg.value("DMS", 0).toBool());

    reg.endGroup();
}

//---------------------------------------------------------------------------
void GPSDialog::on_startStopButton_clicked()
{
    if(gps->isOpen()) {
        gps->close();
    }
    else {
        gps->deviceName(ui->gpsPortEd->text());
        gps->baudRate(baudrate());
        gps->flowControl(flowtype());

        if(!gps->open())
            QMessageBox::critical(this, "Failed to open port!", gps->ioError());
    }

    ui->startStopButton->setText(gps->isOpen() ? "Stop":"Start");
}

//---------------------------------------------------------------------------
BaudRateType GPSDialog::baudrate(void)
{
    switch(ui->baudrateCb->currentIndex())
    {
    case 0: return BAUD2400;
    case 1: return BAUD4800;
    case 2: return BAUD9600;
    case 3: return BAUD19200;
    case 4: return BAUD38400;
    case 5: return BAUD57600;
    case 6: return BAUD115200;
    default:return BAUD4800;
    }
}

//---------------------------------------------------------------------------
FlowType GPSDialog::flowtype(void)
{
    switch(ui->flowControlCb->currentIndex())
    {
    case 0: return FLOW_OFF;
    case 1: return FLOW_HARDWARE;
    case 2: return FLOW_XONXOFF;
    default:return FLOW_OFF;
    }
}

//---------------------------------------------------------------------------
void GPSDialog::gpsDataAvailable(void)
{
    if(gps->isValid()) {
        if(ui->setTimeCb->isChecked()) {
            ui->setTimeCb->setChecked(false);

            bool rc = false;

#ifdef Q_OS_WIN

            SYSTEMTIME st;
            QDateTime  rx_dt = gps->getrxtime(true);
            QDate rx_date    = rx_dt.date();
            QTime rx_time    = rx_dt.time();

            st.wYear         = rx_date.year();
            st.wMonth        = rx_date.month();
            st.wDayOfWeek    = rx_date.dayOfWeek(); // ignored
            st.wDay          = rx_date.day();

            st.wHour         = rx_time.hour();
            st.wMinute       = rx_time.minute();
            st.wSecond       = rx_time.second();
            st.wMilliseconds = rx_time.msec();

            rc = SetSystemTime(&st);

#else // unix

            const time_t tt = gps->rxtime_t();

            if(stime(&tt) == 0)
                rc = true;

#endif // #ifdef Q_OS_WIN

#if defined(DEBUG_GPS)
            if(rc == false)
                qDebug("** Failed to set system time [%s:%d]\n", __FILE__, __LINE__);
            else
                qDebug("** New system time set to GPS time [%s:%d]\n", __FILE__, __LINE__);
#endif

        }

        if(ui->setQTHCB->isChecked()) {
#if 0 // fixme
           ui->setQTHCB->setChecked(false);

           TStation *qth = mw->getQTH();

           qth->lat(gps->latitude_d());
           qth->lon(gps->longitude_d());
           qth->alt(gps->altitude_d());

           mw->updateQTH();
#endif
        }

    } // if(gps->isValid())

    ui->qualityLabel->setText(gps->quality());
    ui->timeLabel->setText(gps->time(!ui->utcTimeCb->isChecked()));
    ui->diffLabel->setText(gps->timediff());
    ui->satsLabel->setText(gps->sats_in_view());
    ui->lonLabel->setText(gps->longitude(ui->dmsCb->isChecked()));
    ui->altLabel->setText(gps->altitude());
    ui->latLabel->setText(gps->latitude(ui->dmsCb->isChecked()));
    ui->speedLabel->setText(gps->speedStr());
    ui->courceLabel->setText(gps->cource());
    ui->geoHeightLabel->setText(gps->mean_sea_level());
    ui->magvarLabel->setText(gps->magnetic());


}

//---------------------------------------------------------------------------
