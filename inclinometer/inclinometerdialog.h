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

#ifndef INCLINOMETERDIALOG_H
#define INCLINOMETERDIALOG_H

#include <QDialog>

class QTimer;
class TGauge;

namespace Ui {
    class InclinometerDialog;
}

//---------------------------------------------------------------------------
class InclinometerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit InclinometerDialog(QWidget *parent = 0);
    ~InclinometerDialog();

private slots:
    void on_pushButton_clicked();
    void onInclinometerDialog_finished(int result);
    void onDataAvailable(void);


private:
    Ui::InclinometerDialog *ui;

    TGauge *gauge;
    QTimer *oak_timer;

    double value;
    int flags;
};

#endif // INCLINOMETERDIALOG_H
