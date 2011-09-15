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
#include <QTimer>

#include "inclinometerdialog.h"
#include "ui_inclinometerdialog.h"

#include "gauge.h"

//---------------------------------------------------------------------------
InclinometerDialog::InclinometerDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InclinometerDialog)
{
    ui->setupUi(this);

//    gauge = new TGauge(Elevation_GaugeType, ui->elevWidget);
    gauge = new TGauge(RollPitch_GaugeType, ui->testWidget);

    oak_timer = new QTimer(this);
    connect(oak_timer,  SIGNAL(timeout()), this, SLOT(onDataAvailable()));
    oak_timer->setInterval(250);

    value = 0;
    flags = 0;

    connect(this, SIGNAL(finished(int)), this, SLOT(onInclinometerDialog_finished(int)));

    // oak_timer->start();

}

//---------------------------------------------------------------------------
InclinometerDialog::~InclinometerDialog()
{
    delete ui;

    delete gauge;
    delete oak_timer;
}

//---------------------------------------------------------------------------
void InclinometerDialog::onDataAvailable(void)
{
    if(flags & 1 || !oak_timer->isActive()) {
        oak_timer->stop();
        return;
    }

    gauge->setValue(value);

    value += 1;
//    if(value > 180)
    if(value > 360)
        value = 0;
}

//---------------------------------------------------------------------------
void InclinometerDialog::onInclinometerDialog_finished(int /*result*/)
{
    qDebug("finished");
    flags |= 1;

    //QCoreApplication::processEvents();
    //oak_timer->stop();
    //sleep(1);

}

//---------------------------------------------------------------------------
void InclinometerDialog::on_pushButton_clicked()
{
    if(oak_timer->isActive())
        oak_timer->stop();
    else
        oak_timer->start();

    ui->pushButton->setText(oak_timer->isActive() ? "Stop":"Start");
}
