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

#ifndef JRK_H
#define JRK_H

#include <QString>
#include <QStringList>
#include "qextserialport.h"

//---------------------------------------------------------------------------
class TJRK
{
public:
    TJRK(void);
    ~TJRK(void);

    bool isOpen(void);
    bool open(QString portname, int devid_, BaudRateType baudrate);
    void close(void);

    QString deviceName(void) const;
    BaudRateType baudRate(void) const;

    int  devId(void) const { return devid; }
    void devId(int devid_) { devid = devid_; }

    void delay(unsigned int msec);

    quint16 readErrorHalting(void);
    quint16 readErrorOccurred(void);
    void    errorStr(QStringList *sl, quint16 err);

    quint16 readInput(void);
    quint16 readTarget(void);
    quint16 readFeedback(void);
    quint16 readScaledFeedback(void);
    short   readDutycycle(void);

    void    motorOff(void);
    quint16 clearError(void);
    bool    setTargetHiRes(int target);
    bool    setTargetLowRes(int magnitude);

    quint16 magnitude2target(int magnitude, bool analog);
    int     target2magnitude(quint16 target, bool analog);

    double target2degrees(quint16 target, double max_degrees);


protected:
    bool send3bytecommand(unsigned char cmd);

    bool read2byte(void);

private:
    QextSerialPort *port;
    int devid;

    quint16 data2;
    unsigned char iobuff[20];

};

#endif // JRK_H
