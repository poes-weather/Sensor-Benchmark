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

#include "si21xxthread.h"
#include "si21xx.h"
#include "si21xxdialog.h"

//---------------------------------------------------------------------------
TSI21xxThread::TSI21xxThread(QObject *parent) :
    QThread(parent)
{
    owner = (SI21xxDialog *) parent;
    si21xx = owner->getSI21xx();

    flags = 0;
}

//---------------------------------------------------------------------------
TSI21xxThread::~TSI21xxThread(void)
{

}

//---------------------------------------------------------------------------
void TSI21xxThread::stop(void)
{
    if(isRunning())
        flags |= TF_STOP;
}

//---------------------------------------------------------------------------
// this thread MUST be stopped before new settings are applied
void TSI21xxThread::run()
{
    flags &= ~TF_STOP;

    while(!(flags & TF_STOP)) {


        if(flags & TF_STOP)
            break;

    }

    flags |= TF_STOP;
}
