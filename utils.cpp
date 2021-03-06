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

#include <QtGlobal>
#include "utils.h"

#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
#    include <unistd.h>
#    include <time.h>
#endif
#if defined(Q_OS_WIN32)
#    include <windows.h>
#endif

//---------------------------------------------------------------------------
void delay(unsigned int msec)
{
#if defined(Q_OS_UNIX) || defined(Q_OS_LINUX)
   usleep(msec * 1000);
#else
#  if defined(Q_OS_WIN32)
      Sleep(msec);
#  endif
#endif
}

//---------------------------------------------------------------------------
void clearGrid(QTableWidget *grid, int setrowcount)
{
 QTableWidgetItem *item;
 int  i, j;

  for(i=0; i<grid->rowCount(); i++)
      for(j=0; j<grid->columnCount(); j++) {
         item = grid->item(i, j);
         if(item)
             delete item;
     }

  grid->setRowCount(setrowcount);
}
