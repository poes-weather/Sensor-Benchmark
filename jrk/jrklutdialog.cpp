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
#include <QFileDialog>
#include <QSettings>
#include <QMessageBox>
#include <math.h>
#include "jrklutdialog.h"
#include "ui_jrklutdialog.h"
#include "utils.h"

#define WF_MODIFIED 1

//---------------------------------------------------------------------------
JrkLUTDialog::JrkLUTDialog(QString ini, double mindeg_, double maxdeg_, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JrkLUTDialog)
{
    ui->setupUi(this);
    setLayout(ui->mainLayout);

    wFlags = 0;
    lutFile = ini;

    load(lutFile);

    mindeg = mindeg_;
    maxdeg = maxdeg_;

    connect(ui->luttable, SIGNAL(itemChanged(QTableWidgetItem *)), this, SLOT(itemChanged(QTableWidgetItem *)));
}

//---------------------------------------------------------------------------
JrkLUTDialog::~JrkLUTDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------
void JrkLUTDialog::itemChanged(QTableWidgetItem *)
{
    wFlags |= WF_MODIFIED;
    // qDebug("Dirty");
}

//---------------------------------------------------------------------------
void JrkLUTDialog::on_buttonBox_accepted()
{
    saveChanges();
}

//---------------------------------------------------------------------------
void JrkLUTDialog::saveChanges(void)
{
    if(wFlags & WF_MODIFIED) {
        QString text = lutFile.isEmpty() ? "Save changes..?":lutFile;

        int rc = QMessageBox::question(this, "Save changes?", text, QMessageBox::Ok | QMessageBox::Cancel);
        if(rc == QMessageBox::Ok) {
            if(!lutFile.isEmpty())
                save(lutFile);
            else
                on_saveTb_clicked();
        }
    }

    wFlags &= ~WF_MODIFIED;
}

//---------------------------------------------------------------------------
void JrkLUTDialog::load(QString inifile)
{
    if(inifile.isEmpty())
        return;

    clearGrid(ui->luttable);

    QString target, target_i, degrees, degrees_i;
    QSettings reg(inifile, QSettings::IniFormat);
    int i, j = 0;

    reg.beginGroup("JrkTargetToDegrees");

    for(i=0; i <= 4095; i++) {
        target_i.sprintf("Target-%04d", i);
        target = reg.value(target_i, "").toString();
        if(target.isEmpty())
            continue;

        degrees_i.sprintf("Degrees-%04d", i);
        degrees = reg.value(degrees_i, "").toString();

        if(degrees.isEmpty())
            break;

        if(ui->luttable->rowCount() <= j)
            ui->luttable->setRowCount(j+1);

        ui->luttable->setItem(j, 0, new QTableWidgetItem(target));
        ui->luttable->setItem(j, 1, new QTableWidgetItem(degrees));

        j++;
    }
    //qDebug("i: %d", i);

    reg.endGroup();
    lutFile = inifile;

    wFlags &= ~WF_MODIFIED;
}

//---------------------------------------------------------------------------
void JrkLUTDialog::save(QString inifile)
{
    if(inifile.isEmpty())
        return;

    QTableWidgetItem *t, *d;
    QString target_i, degrees_i;
    int i, target;
    double degrees;

    QFile file(inifile);

    if(file.exists(inifile))
        file.remove();

    QSettings reg(inifile, QSettings::IniFormat);

    reg.beginGroup("JrkTargetToDegrees");

    for(i=0; i<ui->luttable->rowCount(); i++) {
        t = ui->luttable->item(i, 0);
        d = ui->luttable->item(i, 1);

        if(!t || !d || t->text().isEmpty() || d->text().isEmpty())
            continue;

        target = atoi(t->text().toStdString().c_str());
        degrees = atof(d->text().toStdString().c_str());

        if(target < 0 || target > 4095 || degrees < 0 || degrees > 360)
            break;

        target_i.sprintf("Target-%04d", i);
        degrees_i.sprintf("Degrees-%04d", i);

        reg.setValue(target_i, target);
        reg.setValue(degrees_i, degrees);
    }

    reg.endGroup();
    lutFile = inifile;

    wFlags &= ~WF_MODIFIED;
}

//---------------------------------------------------------------------------
void JrkLUTDialog::on_loadTb_clicked()
{
    saveChanges();

    QString inifile = QFileDialog::getOpenFileName(this, tr("Open LUT File"), lutFile, "INI files (*.ini);;All files (*.*)");

    if(!inifile.isEmpty())
        load(inifile);
}

//---------------------------------------------------------------------------
void JrkLUTDialog::on_saveTb_clicked()
{
    QString inifile = QFileDialog::getSaveFileName(this, tr("Save LUT File"), "jrk/conf/", "INI files (*.ini);;All files (*.*)", 0);

    if(!inifile.isEmpty()) {
        save(inifile);
    }
}

//---------------------------------------------------------------------------
void JrkLUTDialog::on_addRowTb_clicked()
{
    ui->luttable->insertRow(ui->luttable->currentRow() + 1);
}

//---------------------------------------------------------------------------
void JrkLUTDialog::on_deleteRowTb_clicked()
{
    ui->luttable->removeRow(ui->luttable->currentRow());
}

//---------------------------------------------------------------------------
void JrkLUTDialog::on_defaultTb_clicked()
{
    QString str1, str2;
    double delta = maxdeg - mindeg;
    double deg;
    int    i, j, target_step = 50;

    if(delta <= 0)
        return;

    saveChanges();

    clearGrid(ui->luttable);

    for(i=j=0; i<=4095; i+=target_step, j++) {
        ui->luttable->setRowCount(j+1);

        deg = mindeg + (delta / 4095.0) * ((double) i);

        str1.sprintf("%d", i);
        str2.sprintf("%.3f", deg);

        ui->luttable->setItem(j, 0, new QTableWidgetItem(str1));
        ui->luttable->setItem(j, 1, new QTableWidgetItem(str2));
    }

    if(i != 4095) {
        ui->luttable->setRowCount(j+1);

        str2.sprintf("%.3f", maxdeg);

        ui->luttable->setItem(j, 0, new QTableWidgetItem("4095"));
        ui->luttable->setItem(j, 1, new QTableWidgetItem(str2));
    }

    wFlags |= WF_MODIFIED;
}

//---------------------------------------------------------------------------
