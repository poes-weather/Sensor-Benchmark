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
#ifndef JRKLUTDIALOG_H
#define JRKLUTDIALOG_H

#include <QDialog>
#include <QString>

class QTableWidgetItem;
namespace Ui {
    class JrkLUTDialog;
}

//---------------------------------------------------------------------------
class JrkLUTDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JrkLUTDialog(QString ini, double mindeg_, double maxdeg_, QWidget *parent = 0);
    ~JrkLUTDialog();

    QString getFilename(void) { return lutFile; }

protected:
    void load(QString inifile);
    void save(QString inifile);
    void saveChanges(void);


private slots:
    void itemChanged(QTableWidgetItem *item);
    void on_addRowTb_clicked();
    void on_deleteRowTb_clicked();
    void on_defaultTb_clicked();
    void on_loadTb_clicked();
    void on_saveTb_clicked();
    void on_buttonBox_accepted();

private:
    Ui::JrkLUTDialog *ui;
    QString lutFile;
    double mindeg, maxdeg;

    unsigned int wFlags;
};

#endif // JRKLUTDIALOG_H
