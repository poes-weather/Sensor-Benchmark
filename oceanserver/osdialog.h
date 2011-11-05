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

/*
    http://www.ngdc.noaa.gov/geomagmodels/struts/calcDeclination
    POES Weather Ltd Magnetic declination 6th April 2011 7.148290452811556
*/

//---------------------------------------------------------------------------
#ifndef OSDIALOG_H
#define OSDIALOG_H

#include <QDialog>
#include "qextserialport.h"

namespace Ui {
    class OSDialog;
}

class TGauge;
//---------------------------------------------------------------------------
class OSDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OSDialog(QString ini, QWidget *parent = 0);
    ~OSDialog();

protected:
    void writeSettings(void);
    void readSettings(void);

    void close(void);
    void version(void);
    void setSentenceFormat(void);

    int  getDisplayBitmap(void);
    int  getDisplayIndex(int prev_index);

    void parse_01(void);
    void parse_02(void);
    void parse_04(void);
    void parse_08(void);
    void parse_10(void);

private:
    Ui::OSDialog *ui;
    QextSerialPort *port;
    QString iniFile;

    char *rxbuff, *txbuff, *tmpbuff;
    int  modes;

    double heading, pitch, roll, temperature;
    double offset_heading, offset_pitch, offset_roll;

    TGauge *gauge;


private slots:
    void on_applyDeclBtn_clicked();
    void on_applyFormatBtn_clicked();
    void on_helpBtn_clicked();
    void on_dumpconfBtn_clicked();
    void on_applyDataFieldsBtn_clicked();
    void on_openBtn_clicked();
    void onDataAvailable(void);
    void onDialog_finished(int);
    void on_applyOffsetBtn_clicked();
};

#endif // OSDIALOG_H
