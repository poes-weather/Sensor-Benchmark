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
#ifndef SI21XXTHREAD_H
#define SI21XXTHREAD_H

#include <QThread>

//---------------------------------------------------------------------------
#define TF_STOP     1

//---------------------------------------------------------------------------
class TSI21xx;
class SI21xxDialog;

//---------------------------------------------------------------------------
class TSI21xxThread : public QThread
{
    Q_OBJECT
public:
    explicit TSI21xxThread(QObject *parent = 0);
    ~TSI21xxThread(void);

    void run();
    void stop();

signals:

public slots:

private:
    TSI21xx      *si21xx;
    SI21xxDialog *owner;

    int flags;
};

#endif // SI21XXTHREAD_H
