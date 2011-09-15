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
#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
#    include <unistd.h>
#    include <time.h>
#endif
#if defined(Q_OS_WIN32)
#    include <windows.h>
#endif

#include <QCoreApplication>
#include "jrk.h"

//---------------------------------------------------------------------------
TJRK::TJRK(void)
{
    port = new QextSerialPort(QextSerialPort::Polling);

    port->setBaudRate(BAUD115200);
    port->setDataBits(DATA_8);
    port->setParity(PAR_NONE);
    port->setStopBits(STOP_1);
    port->setFlowControl(FLOW_OFF);
    port->setTimeout(500);

#if defined(Q_OS_UNIX)
    port->setPortName("/dev/ttyACM0");
#else
    port->setPortName("COM6");
#endif

    devid = 11;
}

//---------------------------------------------------------------------------
TJRK::~TJRK(void)
{
    close();

    delete port;
}

//---------------------------------------------------------------------------
QString TJRK::deviceName(void) const
{
    return port->portName();
}

//---------------------------------------------------------------------------
BaudRateType TJRK::baudRate(void) const
{
    return port->baudRate();
}

//---------------------------------------------------------------------------
bool TJRK::isOpen(void)
{
    return port->isOpen();
}

//---------------------------------------------------------------------------
bool TJRK::open(QString portname, int devid_, BaudRateType baudrate)
{
    bool rc;

    close();

    devid = devid_;

    port->setPortName(portname);
    port->setBaudRate(baudrate);

    rc = port->open(QIODevice::ReadWrite | QIODevice::Unbuffered);

    if(rc) {
        delay(50);
        clearError();
    }

    return rc;
}

//---------------------------------------------------------------------------
void TJRK::close(void)
{
    if(port->isOpen()) {
        motorOff();
        port->close();
    }
}

//---------------------------------------------------------------------------
quint16 TJRK::readErrorHalting(void)
{
    if(!send3bytecommand(0x33))
        return 0xffff;
    else
        return read2byte() ? data2:0xffff;
}

//---------------------------------------------------------------------------
quint16 TJRK::readErrorOccurred(void)
{
    if(!send3bytecommand(0x35))
        return 0xffff;
    else
        return read2byte() ? data2:0xffff;
}

//---------------------------------------------------------------------------
quint16 TJRK::readInput(void)
{
    if(!send3bytecommand(0xa1 & 0x7f))
        return 0xffff;
    else
        return read2byte() ? data2:0xffff;
}

//---------------------------------------------------------------------------
quint16 TJRK::readTarget(void)
{
    if(!send3bytecommand(0xa3 & 0x7f))
        return 0xffff;
    else
        return read2byte() ? (data2 & 0x0fff):0xffff;
}

//---------------------------------------------------------------------------
quint16 TJRK::readFeedback(void)
{
    if(!send3bytecommand(0xa5 & 0x7f))
        return 0xffff;
    else
        return read2byte() ? (data2 & 0x0fff):0xffff;
}

//---------------------------------------------------------------------------
quint16 TJRK::readScaledFeedback(void)
{
    if(!send3bytecommand(0xa9 & 0x7f))
        return 0xffff;
    else
        return read2byte() ? (data2 & 0x0fff):0xffff;
}

//---------------------------------------------------------------------------
short TJRK::readDutycycle(void)
{
    short rc = 0;

    if(!send3bytecommand(0xad & 0x7f))
        return rc;
    else if(read2byte()) {
        if(data2 > 0x258)
            rc = data2 - 0xffff;
        else
            rc = data2;

        // qDebug("readDutycycle data: %d, 0x%04x", (int)data2, data2);

        return rc;
    }
    else
        return rc;
}

//---------------------------------------------------------------------------
void TJRK::motorOff(void)
{
    send3bytecommand(0x7f);
}

//---------------------------------------------------------------------------
quint16 TJRK::clearError(void)
{
    return readErrorHalting();
}

//---------------------------------------------------------------------------
bool TJRK::setTargetHiRes(int target)
{
    if(!port->isOpen() || target < 0 || target > 4095)
        return false;

    iobuff[0] = 0xaa;
    iobuff[1] = (unsigned char) devid;
    iobuff[2] = (unsigned char) (0x40 + (target & 0x1F));
    iobuff[3] = (unsigned char) ((target >> 5) & 0x7F);

    qDebug("set target hi res: 0x%02x%02x%02x%02x", iobuff[0], iobuff[1], iobuff[2], iobuff[3]);
    if(port->write((const char *) iobuff, 4) != 4)
        return false;
    else
        return true;
}

//---------------------------------------------------------------------------
bool TJRK::setTargetLowRes(int magnitude)
{
    if(!port->isOpen() || magnitude < -127 || magnitude > 127)
        return false;

    iobuff[0] = 0xaa;
    iobuff[1] = (unsigned char) devid;
    iobuff[2] = magnitude < 0 ? 0x60:0x61;
    iobuff[3] = (unsigned char) (magnitude < 0 ? -magnitude:magnitude);

    qDebug("set target low res: 0x%02x%02x%02x%02x", iobuff[0], iobuff[1], iobuff[2], iobuff[3]);
    if(port->write((const char *) iobuff, 4) != 4)
        return false;
    else
        return true;
}

//---------------------------------------------------------------------------
quint16 TJRK::magnitude2target(int magnitude, bool analog)
{
    quint16 rc = 2048;

    if(analog)
        rc = 2048 + 16 * magnitude;
    else
        rc = 2048 + (600 / 127) * magnitude;

    return rc;
}

//---------------------------------------------------------------------------
int TJRK::target2magnitude(quint16 target, bool analog)
{
    int rc = 0;

    if(analog)
        rc = (target - 2048) / 16;
    else
        rc = (target - 2048) * (127 / 600);

    return rc;
}

//---------------------------------------------------------------------------
double TJRK::target2degrees(quint16 target, double max_degrees)
{
    double degrees;

    degrees = (((double) target) * max_degrees) / 4095.0;

    // qDebug("target2degrees, target: %d degrees: %f", target, degrees);

    return degrees;
}

//---------------------------------------------------------------------------
void TJRK::errorStr(QStringList *sl, quint16 err)
{
    sl->clear();

    if(err == 0xffff) {
        sl->append("Unknown error (read failed?)");
        return;
    }

    if(err & 0x0001)
        sl->append("Awaiting command");
    if(err & 0x0002)
        sl->append("No power");
    if(err & 0x0004)
        sl->append("Motor driver error");
    if(err & 0x0008)
        sl->append("Input invalid (Pulse Width Input Mode only)");
    if(err & 0x0010)
        sl->append("Input disconnect");
    if(err & 0x0020)
        sl->append("Feedback disconnect");
    if(err & 0x0040)
        sl->append("Maximum current exceeded");
    if(err & 0x0080)
        sl->append("Serial signal error");
    if(err & 0x0100)
        sl->append("Serial overrun");
    if(err & 0x0200)
        sl->append("Serial RX buffer full");
    if(err & 0x0400)
        sl->append("Serial CRC error");
    if(err & 0x0800)
        sl->append("Serial protocol error");
    if(err & 0x1000)
        sl->append("Serial timeout error");
}

//---------------------------------------------------------------------------
bool TJRK::read2byte(void)
{
    int  retrys = 20;
    int  i = 0;

    QCoreApplication::processEvents();

    while(port->bytesAvailable() != 2) {
        i++;
        if(i >= retrys)
            return false;

        delay(50);
        //QCoreApplication::processEvents();
    }

    if(port->read((char *) iobuff, 2) != 2)
        return false;

    data2 = (iobuff[1] << 8) | iobuff[0];

    return true;
}

//---------------------------------------------------------------------------
bool TJRK::send3bytecommand(unsigned char cmd)
{
    if(!port->isOpen())
        return false;

    iobuff[0] = 0xaa;
    iobuff[1] = (unsigned char) devid;
    iobuff[2] = cmd;

    if(port->write((const char *) iobuff, 3) != 3)
        return false;

    return true;
}

//---------------------------------------------------------------------------
void TJRK::delay(unsigned int msec)
{
#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
   usleep(msec * 1000);
#else
#  if defined(Q_OS_WIN32)
      Sleep(msec);
#  endif
#endif
}
