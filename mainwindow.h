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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class GPSDialog;
class JrkDialog;
class QboxDialog;
class OSDialog;
class SI21xxDialog;

//---------------------------------------------------------------------------
class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);
    bool write_buffer(const char *buf);

private:
    Ui::MainWindow *ui;
    GPSDialog    *gps;
    JrkDialog    *jrkACM;
    QboxDialog   *qbox;
    OSDialog     *os5k;
    SI21xxDialog *si21xx;


private slots:
    void on_actionToradex_Inclinometer_triggered();
    void on_actionOcean_Server_Compass_triggered();
    void on_actionGPS_triggered();
    void on_actionExit_triggered();
    void on_actionSI21xx_triggered();
    void on_actionGeneral_DVB_S_S2_triggered();
    void on_actionACM_Control_triggered();
    void on_actionUSB_Contol_triggered();
};

#endif // MAINWINDOW_H
