/*
    POES-USRP, a software for recording and decoding POES high resolution weather satellite images.
    Copyright (C) 2009-2011 Free Software Foundation, Inc.

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


#ifndef SI21XXDIALOG_H
#define SI21XXDIALOG_H

#include <QDialog>

//---------------------------------------------------------------------------
namespace Ui {
    class SI21xxDialog;
}

class TSI21xx;

//---------------------------------------------------------------------------
class SI21xxDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SI21xxDialog(QWidget *parent = 0);
    ~SI21xxDialog();

    TSI21xx *getSI21xx(void) { return si21xx; }

protected:
    void enablecontrols(bool enb);

private:
    Ui::SI21xxDialog *ui;
    TSI21xx *si21xx;

private slots:
    void onSI21xxDialog_finished(int);
    void on_openDevBtn_clicked();
    void on_tuneBtn_clicked();
};

#endif // SI21XXDIALOG_H
