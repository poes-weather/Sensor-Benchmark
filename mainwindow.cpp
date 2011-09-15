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
#include <QDesktopWidget>
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "gpsdialog.h"
#include "jrkdialog.h"
#include "qboxdialog.h"
#include "osdialog.h"
#include "inclinometerdialog.h"
#include "si21xxdialog.h"
#include "usbjrkdialog.h"


//---------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString ini = qApp->applicationDirPath() + "/gps.ini";
    gps = new GPSDialog(ini, this);

    ini = qApp->applicationDirPath() + "/jrk.ini";
    jrkACM = new JrkDialog(ini, this);

#if defined Q_OS_LINUX
    ini = qApp->applicationDirPath() + "/dvb.ini";
    qbox = new QboxDialog(ini, this);

    si21xx = new SI21xxDialog(this);
#else
    qbox = NULL;
    si21xx  = NULL;
#endif



    ini = qApp->applicationDirPath() + "/os5000.ini";
    os5k = new OSDialog(ini, this);

    QRect rect = QApplication::desktop()->availableGeometry(this);
    move(rect.center() - this->rect().center());
}

//---------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;

    delete gps;
    delete jrkACM;
    delete os5k;

    if(qbox != NULL)
        delete qbox;

    if(si21xx != NULL)
        delete si21xx;
}

//---------------------------------------------------------------------------
void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}

//---------------------------------------------------------------------------
void MainWindow::on_actionExit_triggered()
{
    exit(0);
}

//---------------------------------------------------------------------------
void MainWindow::on_actionGPS_triggered()
{
    gps->show();
}

//---------------------------------------------------------------------------
void MainWindow::on_actionOcean_Server_Compass_triggered()
{
    os5k->show();
}

//---------------------------------------------------------------------------
void MainWindow::on_actionToradex_Inclinometer_triggered()
{
    InclinometerDialog dlg(this);
    dlg.exec();
}

//---------------------------------------------------------------------------
void MainWindow::on_actionGeneral_DVB_S_S2_triggered()
{
    if(qbox)
        qbox->show();
}

//---------------------------------------------------------------------------
void MainWindow::on_actionSI21xx_triggered()
{
    if(si21xx)
        si21xx->show();
}

//---------------------------------------------------------------------------
void MainWindow::on_actionACM_Control_triggered()
{
    jrkACM->show();
}

//---------------------------------------------------------------------------
void MainWindow::on_actionUSB_Contol_triggered()
{
    USBJrkDialog dlg(this);

    dlg.exec();
}
