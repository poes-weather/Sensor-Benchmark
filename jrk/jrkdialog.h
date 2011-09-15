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

#ifndef JRKDIALOG_H
#define JRKDIALOG_H

#include <QDialog>
#include "qextserialport.h"

namespace Ui {
    class JrkDialog;
}

class TJRK;
//---------------------------------------------------------------------------
class JrkDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JrkDialog(QString ini, QWidget *parent = 0);
    ~JrkDialog();

protected:
    void addStrings(QStringList *sl);
    void readErrors(bool occured=true);
    void readAll(void);

private:
    Ui::JrkDialog *ui;

    int     flags;
    QString iniFile;
    TJRK    *jrk;
    QTimer  *jrk_timer;

private slots:
    void on_clearErrButton_clicked();
    void onJrkDialog_finished(int result);
    void onJrkReadWrite(void);
    void on_stopBtn_clicked();
    void on_magnitudedial_dialPressed();
    void on_targetdial_dialPressed();
    void on_magnitudedial_sliderReleased();
    void on_readBtn_clicked();
    void on_targetdial_sliderReleased();
    void on_magnitudedial_valueChanged(int value);
    void on_targetdial_valueChanged(int value);
    void on_readErrorsBtn_clicked();
    void on_openButton_clicked();
};

#endif // JRKDIALOG_H
