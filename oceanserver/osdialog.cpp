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

#include <QSettings>
#include <QFile>
#include <QListWidgetItem>
#include <ctype.h>

#include "osdialog.h"
#include "ui_osdialog.h"
#include "gauge.h"
#include "utils.h"

//---------------------------------------------------------------------------
#define M_CLOSE            1
#define M_READING          2
#define M_READ_CMD         4
#define M_READ_CMD_RC      8

#define RX_BUFF_SIZE 512
#define TX_BUFF_SIZE  64

//---------------------------------------------------------------------------
OSDialog::OSDialog(QString ini, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OSDialog)
{
    ui->setupUi(this);
    setLayout(ui->mainLayout);

    gauge = new TGauge(RollPitch_GaugeType, ui->gaugeWidget);

    port = new QextSerialPort(QextSerialPort::EventDriven);

    iniFile = ini;

    port->setBaudRate(BAUD19200);
    port->setDataBits(DATA_8);
    port->setParity(PAR_NONE);
    port->setStopBits(STOP_1);
    port->setFlowControl(FLOW_OFF);
    port->setTimeout(500);

    connect(port, SIGNAL(readyRead()), this, SLOT(onDataAvailable()));
    connect(this, SIGNAL(finished(int)), this, SLOT(onDialog_finished(int)));

    rxbuff = (char *) malloc(RX_BUFF_SIZE + 1);
    txbuff = (char *) malloc(TX_BUFF_SIZE + 1);
    tmpbuff = (char *) malloc(RX_BUFF_SIZE + 1);

    modes = 0;

    offset_heading = 0;
    offset_pitch = 0;
    offset_roll = 0;

    heading = 0;
    pitch = 0;
    roll = 0;
    temperature = 0;

    readSettings();
}

//---------------------------------------------------------------------------
OSDialog::~OSDialog()
{
    writeSettings();

    close();
    delete port;

    if(rxbuff)
        free(rxbuff);
    if(txbuff)
        free(txbuff);
    if(tmpbuff)
        free(tmpbuff);

    delete gauge;
    delete ui;
}

//---------------------------------------------------------------------------
void OSDialog::onDialog_finished(int /*value*/)
{
    qDebug("OS finished");

    close();
}

//---------------------------------------------------------------------------
void OSDialog::writeSettings(void)
{
    QFile file(iniFile);

    if(file.exists(iniFile))
        file.remove();

    QSettings reg(iniFile, QSettings::IniFormat);

    reg.beginGroup("OS5000");

    reg.setValue("DeviceId", ui->deviceEd->text());
    reg.setValue("Output_Format", ui->outputformatCb->currentIndex());
    reg.setValue("Display_Fields", getDisplayBitmap());
    reg.setValue("Declination", ui->declinationSb->value());
    reg.setValue("Verbose", ui->verboseCb->isChecked());

    reg.setValue("OffsetHeading", offset_heading);
    reg.setValue("OffsetPitch", offset_pitch);
    reg.setValue("OffsetRoll", offset_roll);

    reg.endGroup();
}

//---------------------------------------------------------------------------
void OSDialog::readSettings(void)
{
    QFile file(iniFile);

    if(!file.exists(iniFile))
        return;

    QSettings reg(iniFile, QSettings::IniFormat);
    QListWidgetItem *item;
    int i, bits;

    reg.beginGroup("OS5000");

    ui->deviceEd->setText(reg.value("DeviceId", "/dev/ttyUSB0").toString());
    ui->outputformatCb->setCurrentIndex(reg.value("Output_Format", 1).toInt());
    ui->declinationSb->setValue(reg.value("Declination", 0).toDouble());
    ui->verboseCb->setChecked(reg.value("Verbose", 1).toBool());
    bits = reg.value("Display_Fields", 15).toInt();

    offset_heading = reg.value("OffsetHeading", 0).toDouble();
    offset_pitch   = reg.value("OffsetPitch", 0).toDouble();
    offset_roll    = reg.value("OffsetRoll", 0).toDouble();

    ui->offsetHeadingSb->setValue(offset_heading);
    ui->offsetPitchSb->setValue(offset_pitch);
    ui->offsetRollSb->setValue(offset_roll);

    reg.endGroup();

    // tick output fields
    for(i=0; i<ui->datalistWidget->count(); i++) {
        item = ui->datalistWidget->item(i);
        item->setCheckState( (bits & (1 << i)) ? (Qt::Checked) : (Qt::Unchecked) );

        if(item->checkState() == Qt::Checked)
            bits |= (1 << i);
    }

}

//---------------------------------------------------------------------------
bool OSDialog::isOpen(void)
{
    return port->isOpen();
}

//---------------------------------------------------------------------------
void OSDialog::on_openBtn_clicked()
{
    if(port->isOpen())
        close();
    else {
        modes = 0;

        ui->plainTextEdit->clear();
        port->setPortName(ui->deviceEd->text());
        if(!port->open(QIODevice::ReadWrite))
            ui->plainTextEdit->appendPlainText("Failed to open device " + ui->deviceEd->text());
    }

    ui->openBtn->setText(port->isOpen() ? "Close":"Open");

    if(!port->isOpen())
        return;

    version();
}

//---------------------------------------------------------------------------
void OSDialog::close(void)
{
    if(!port->isOpen())
        return;

    if(!(modes & M_READING)) {
        port->close();

        return;
    }
    else
        modes |= M_CLOSE;

    int i = 0;
    while(port->isOpen()) {
        i++;

        if(i > 10) {
            port->close();
            break;
        }

        delay(200);

        QCoreApplication::processEvents();
    }

    modes = 0;
}

//---------------------------------------------------------------------------
void OSDialog::onDataAvailable(void)
{
    if(!port->isOpen() || !rxbuff)
        return;

    if(modes & M_CLOSE) {
        port->close();
        return;
    }

    modes |= M_READING;

    int avail = port->bytesAvailable();
    int read = 0;
    if(avail > 2) {
        read = port->readLine(rxbuff, RX_BUFF_SIZE) - 2;
        // replace 0x0d 0x0a with NULL
        if(read > 0) {
            rxbuff[read] = '\0';

            if(modes & M_READ_CMD) {
                if(*rxbuff == 'C' && strstr(rxbuff, "CMD")) {
                    modes &= ~M_READ_CMD;
                    modes |= M_READ_CMD_RC;
                }
            }
            else if(modes & M_READ_CMD_RC) {
                if(*rxbuff == '$' || isdigit(*rxbuff)) {
                    modes &= ~M_READ_CMD_RC;
                    ui->plainTextEdit->appendPlainText("");
                }
                else
                    ui->plainTextEdit->appendPlainText(rxbuff);
            }

            if(!(modes & (M_READ_CMD | M_READ_CMD_RC))) {
                if(ui->verboseCb->isChecked())
                    ui->plainTextEdit->appendPlainText(rxbuff);

                // parse output
                switch(ui->outputformatCb->currentIndex()) {
                case 0: parse_01();
                    break;
                case 1: parse_02();
                    break;
                case 2: parse_04();
                    break;
                case 3: parse_08();
                    break;
                case 4: parse_10();
                    break;

                }
            }
        }
    }

    if(modes & M_CLOSE) {
        port->close();

        return;
    }

    modes &= ~M_READING;
}

//---------------------------------------------------------------------------
void OSDialog::version(void)
{
    if(!port->isOpen() || !txbuff)
        return;

    txbuff[0] = 0x1b; // esc
    txbuff[1] = 0x56; // V
    txbuff[2] = 0x20; // space

    if(port->write(txbuff, 3) != 3)
        ui->plainTextEdit->appendPlainText("Failed to write version command");
    else
        modes |= M_READ_CMD;
}

//---------------------------------------------------------------------------
void OSDialog::setSentenceFormat(void)
{
    if(!port->isOpen() || !txbuff)
        return;

    // set output to “$OHPR” sentence format
    // $OHPR value1,value2,value3,value4....*cc
    txbuff[0] = 0x1b; // esc
    txbuff[1] = 0x2a; // *
    txbuff[2] = 0x20; // space

    if(port->write(txbuff, 3) != 3) {
        ui->plainTextEdit->appendPlainText("Failed to change sentence format");
        return;
    }

    delay(200);

    txbuff[0] = '2'; // format
    txbuff[1] = '\r'; // CR, carriage return

    if(port->write(txbuff, 2) != 2)
        ui->plainTextEdit->appendPlainText("Failed to change sentence format");

    delay(200);
}

//---------------------------------------------------------------------------
void OSDialog::on_applyFormatBtn_clicked()
{
    if(!port->isOpen() || !txbuff)
        return;

    // set output to “$OHPR” sentence format
    // $OHPR value1,value2,value3,value4....*cc
    txbuff[0] = 0x1b; // esc
    txbuff[1] = 0x2a; // *
    txbuff[2] = 0x20; // space

    if(port->write(txbuff, 3) != 3) {
        ui->plainTextEdit->appendPlainText("Failed to write sentence format");
        return;
    }

    delay(200);

    sprintf(txbuff, "%d\r", (1 << ui->outputformatCb->currentIndex()));

    if(port->write(txbuff, 2) != 2)
        ui->plainTextEdit->appendPlainText("Failed to change sentence format");

    delay(200);
}

//---------------------------------------------------------------------------
void OSDialog::on_applyDataFieldsBtn_clicked()
{
    if(!port->isOpen() || !txbuff)
        return;

    txbuff[0] = 0x1b; // esc
    txbuff[1] = 0x58; // X
    txbuff[2] = 0x20; // space

    if(port->write(txbuff, 3) != 3) {
        ui->plainTextEdit->appendPlainText("Failed to change output format");
        return;
    }

    delay(200);

    QListWidgetItem *item;
    int i, bits = 0;

    for(i=0; i<ui->datalistWidget->count(); i++) {
        item = ui->datalistWidget->item(i);
        if(item->checkState() == Qt::Checked)
            bits |= (1 << i);
    }

    if(bits == 0)
        bits = 15;

    i = sprintf(txbuff, "%d\r", bits);
    if(port->write(txbuff, i) != i) {
        ui->plainTextEdit->appendPlainText("Failed to write output parameter command");
    }

    delay(200);
}

//---------------------------------------------------------------------------
void OSDialog::on_applyDeclBtn_clicked()
{
    if(!port->isOpen() || !txbuff)
        return;

    bool ve = ui->verboseCb->isChecked();
    ui->verboseCb->setChecked(true);

    txbuff[0] = 0x1b; // esc
    txbuff[1] = 0x51; // Q
    txbuff[2] = 0x20; // space

    if(port->write(txbuff, 3) != 3) {
        ui->plainTextEdit->appendPlainText("Failed to change declination");
        ui->verboseCb->setChecked(ve);
        return;
    }

    delay(200);

    int i = sprintf(txbuff, "%.0f\r", ui->declinationSb->value()*10);

    if(port->write(txbuff, i) != i)
        ui->plainTextEdit->appendPlainText("Failed to write declination command");

    ui->verboseCb->setChecked(ve);
}

//---------------------------------------------------------------------------
int OSDialog::getDisplayBitmap(void)
{
    QListWidgetItem *item;
    int i, bits = 0;

    for(i=0; i<ui->datalistWidget->count(); i++) {
        item = ui->datalistWidget->item(i);
        if(item->checkState() == Qt::Checked)
            bits |= (1 << i);
    }

    return bits == 0 ? 15:bits;
}

//---------------------------------------------------------------------------
int OSDialog::getDisplayIndex(int prev_index)
{
    QListWidgetItem *item;
    int i;

    for(i=prev_index + 1; i<ui->datalistWidget->count(); i++) {
        item = ui->datalistWidget->item(i);
        if(item->checkState() == Qt::Checked)
            return i;
    }

    return -1;
}

//---------------------------------------------------------------------------
void OSDialog::on_dumpconfBtn_clicked()
{
    if(!port->isOpen() || !txbuff)
        return;

    txbuff[0] = 0x1b; // esc
    txbuff[1] = 0x26; // &
    txbuff[2] = 0x20; // space

    if(port->write(txbuff, 3) != 3)
        ui->plainTextEdit->appendPlainText("Failed to write dump command");
    else
        modes |= M_READ_CMD;
}

//---------------------------------------------------------------------------
void OSDialog::on_helpBtn_clicked()
{
    if(!port->isOpen() || !txbuff)
        return;

    txbuff[0] = 0x1b; // esc
    txbuff[1] = 0x48; // H
    txbuff[2] = 0x20; // space

    if(port->write(txbuff, 3) != 3)
        ui->plainTextEdit->appendPlainText("Failed to write help command");
    else
        modes |= M_READ_CMD;
}

//---------------------------------------------------------------------------
// $Chhh.hPpp.pRrr.rTtt.tMx0.000My0.000Mz0.000Ax000.0Ay000.0Az000.0*cc
void OSDialog::parse_01(void)
{
    if(*rxbuff != '$' || !tmpbuff)
        return;

    long i, j, len = strlen(rxbuff) - 3;
    int flags = 0;

    // commaseparate, skip $ and *cc
    for(j=0, i=1; i<len; i++, j++) {
        if(isalpha(rxbuff[i])) {
            flags |= 1;
            if(flags & 2)
                tmpbuff[j++] = ','; // add comma after the last number
        }
        else if(flags & 1) {
            tmpbuff[j++] = ','; // add comma after the last ascii
            flags &= ~1;
            flags |= 2;
        }

        tmpbuff[j] = rxbuff[i];
    }
    tmpbuff[j] = '\0';

    // qDebug("tmpbuff 0x01: %s", tmpbuff);

    QString     value, str(tmpbuff);
    QStringList sl = str.split(',', QString::SkipEmptyParts);
    QLabel      *label;

    for(i=1; i<sl.count(); i+=2) {
        str = sl.at(i-1);
        value = sl.at(i);
        label = NULL;

        if(str == "C") {
            heading = value.toDouble() + offset_heading;
            //label = ui->headinglabel;
        }
        else if(str == "P") {
            pitch = value.toDouble() + offset_pitch;
            //label = ui->pitchlabel;
        }
        else if(str == "R") {
            roll = value.toDouble() + offset_roll;
            //label = ui->rolllabel;
        }
        else if(str == "T") {
            temperature = value.toDouble();
            //label = ui->templabel;
        }
        else {
            label = NULL;
        }
        // TODO: Add more support....

        if(label)
            label->setText(value);
    }

    gauge->setRollPitchValue(heading, roll, pitch);
}

//---------------------------------------------------------------------------
// $OHPR,237.7,-14.3,175.5,30.1*2B
void OSDialog::parse_02(void)
{
    if(*rxbuff != '$' || !tmpbuff)
        return;

    long len = strlen(rxbuff);

    if(len < 6)
        return;

    // remove $OHPR, and *cc
    rxbuff[len - 3] = '\0';
    strcpy(tmpbuff, rxbuff + 6);

    //qDebug("tmpbuff 0x02: %s", tmpbuff);

    QString     value, str(tmpbuff);
    QStringList sl = str.split(',', QString::SkipEmptyParts);
    QLabel      *label;
    int         i, index = -1;

    for(i=0; i<sl.count(); i++) {
        value = sl.at(i);
        index = getDisplayIndex(index);
        label = NULL;

        switch(index) {
        case 0:
            heading = value.toDouble() + offset_heading;
            //label = ui->headinglabel;
            break;
        case 1:
            pitch = value.toDouble() + offset_pitch;
            //label = ui->pitchlabel;
            break;
        case 2:
            roll = value.toDouble() + offset_roll;
            //label = ui->rolllabel;
            break;
        case 3:
            temperature = value.toDouble();
            //label = ui->templabel;

            break;

        // TODO: Add more support....
        default:
            label = NULL;
            qDebug("Unsupported index: %d", index);
        }

        if(label)
            label->setText(value);
    }

    gauge->setRollPitchValue(heading, roll, pitch);

}

//---------------------------------------------------------------------------
// The NMEA0183 standard true heading sentence
// $HCHDT,212.4,T*2C
void OSDialog::parse_04(void)
{
    if(*rxbuff != '$' || !tmpbuff)
        return;

    long len = strlen(rxbuff);

    if(len < 7)
        return;

    // remove $HCHDT, and ,T*cc
    rxbuff[len - 5] = '\0';
    strcpy(tmpbuff, rxbuff + 7);

    // qDebug("tmpbuff 0x04: %s", tmpbuff);

    heading = atof(tmpbuff);
    //ui->headinglabel->setText(tmpbuff);
    //gauge->setRollPitchValue(heading, roll, pitch);
}

//---------------------------------------------------------------------------
// comma delimited
// value1,value2,value3,va,....
void OSDialog::parse_08(void)
{
    //qDebug("tmpbuff 0x08: %s", tmpbuff);

    QString     value, str(rxbuff);
    QStringList sl = str.split(',', QString::SkipEmptyParts);
    QLabel      *label = NULL;
    int         i, index = -1;

    for(i=0; i<sl.count(); i++) {
        value = sl.at(i);
        index = getDisplayIndex(index);
        label = NULL;

        switch(index) {
        case 0:
            heading = value.toDouble() + offset_heading;
            //label = ui->headinglabel;
            break;
        case 1:
            pitch = value.toDouble() + offset_pitch;
            //label = ui->pitchlabel;
            break;
        case 2:
            roll = value.toDouble() + offset_roll;
            //label = ui->rolllabel;
            break;
        case 3:
            temperature = value.toDouble();
            //label = ui->templabel;
            break;

        // TODO: Add more support....
        default:
            label = NULL;
            qDebug("Unsupported index: %d", index);
        }

        if(label)
            label->setText(value);
    }

    gauge->setRollPitchValue(heading, roll, pitch);
}

//---------------------------------------------------------------------------
// Azimuth=0.0, Elevation=0.0....
// NOTE: Outputrate must be set to 2
void OSDialog::parse_10(void)
{

}

//---------------------------------------------------------------------------
void OSDialog::on_applyOffsetBtn_clicked()
{
    offset_heading = ui->offsetHeadingSb->value();
    offset_pitch   = ui->offsetPitchSb->value();
    offset_roll    = ui->offsetRollSb->value();
}

//---------------------------------------------------------------------------
