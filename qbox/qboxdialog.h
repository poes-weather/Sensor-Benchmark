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
#ifndef QBOXDIALOG_H
#define QBOXDIALOG_H

#include <QDialog>
#include <linux/dvb/frontend.h>

namespace Ui {
    class QboxDialog;
}

class TSI21XX;
//---------------------------------------------------------------------------
class QboxDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QboxDialog(QString ini, QWidget *parent = 0);
    ~QboxDialog();

protected:
    void readSettings(void);
    void close_frontend(void);
    bool isOpen(void);

    void readStatus(void);

    int i2c_open(const char *dev);
    void i2c_close(void);

    int i2c_read(unsigned short len, unsigned char regaddr);

private:
    Ui::QboxDialog *ui;
    QString iniFile;

    int    frontend_fd;
    struct dvb_frontend_info fe_info;
    struct dvb_frontend_parameters fe_params;

    int i2c_fd;
    unsigned char *i2c_rx_buf, *i2c_tx_buf;

    TSI21XX *si21xx;

private slots:
    void on_openBtn_clicked();
    void onQboxDialog_finished(int);
};

#endif // QBOXDIALOG_H
