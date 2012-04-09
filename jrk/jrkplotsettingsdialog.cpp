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
#include <QColorDialog>

#include "jrkplotsettingsdialog.h"
#include "ui_jrkplotsettingsdialog.h"
#include "jrkplotdialog.h"

#include <qwt_plot_curve.h>

//---------------------------------------------------------------------------
JrkPlotSettingsDialog::JrkPlotSettingsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::JrkPlotSettingsDialog)
{
    int i;

    ui->setupUi(this);

    plotdlg = (JrkPlotDialog *) parent;

    for(i=0; i<(signed) plotdlg->jrkdata.size(); i++)
        setProperties(i);

#if 0
    data = (JrkPlotData *)plotdlg->jrkdata.at(0);
    QColor cl = data->curve->pen().color();

    QString str;
    str.sprintf("QToolButton { background-color: rgb(%d,%d,%d); }", cl.red(), cl.green(), cl.blue());
    ui->toolButton->setStyleSheet(str);
    qDebug(str.toStdString().c_str());

    QPalette pal = ui->toolButton->palette();
    cl = pal.color(QPalette::Window);
    qDebug("r:%d g:%d b:%d", cl.red(), cl.green(), cl.blue());
#endif
}

//---------------------------------------------------------------------------
JrkPlotSettingsDialog::~JrkPlotSettingsDialog()
{
    delete ui;
}

//---------------------------------------------------------------------------
void JrkPlotSettingsDialog::setProperties(int index)
{
    QToolButton *tb;
    QSpinBox    *sb;
    JrkPlotData *data = (JrkPlotData *)plotdlg->jrkdata.at(index);

    switch(index) {
    case JrkPlotDialog::curve_input:
        tb = ui->inputTb;
        sb = ui->inputSb;
        break;

    default:
        return;
    }

    sb->setValue(100.0/data->scale);
    setButtonColor(tb, data->curve->pen().color(), false);
}

//---------------------------------------------------------------------------
void JrkPlotSettingsDialog::setButtonColor(QToolButton *tb, QColor cl, bool select)
{
    if(select) {
        cl = QColorDialog::getColor(cl, this);
        if(!cl.isValid())
            return;
    }

    QString str;

    str.sprintf("QToolButton#%s { background-color:rgb(%d,%d,%d); color:rgb(%d,%d,%d); }",
                tb->objectName().toStdString().c_str(),
                cl.red(), cl.green(), cl.blue(),
                cl.red(), cl.green(), cl.blue());
    tb->setStyleSheet(str);
}

//---------------------------------------------------------------------------
QColor JrkPlotSettingsDialog::buttonColor(QToolButton *tb)
{
    QPalette pal = tb->palette();

    return pal.color(QPalette::Window);
}

//---------------------------------------------------------------------------
void JrkPlotSettingsDialog::on_inputTb_clicked()
{
    setButtonColor(ui->inputTb, buttonColor(ui->inputTb), true);
}
