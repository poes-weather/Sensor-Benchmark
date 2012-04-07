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
#ifndef JRKPLOTDIALOG_H
#define JRKPLOTDIALOG_H

#include <QDialog>
#include <vector>
#include "jrk_protocol.h"

using namespace std;
namespace Ui {
    class JrkPlotDialog;
}

class QTimer;
class QwtPlot;
class QwtPlotCurve;
class QwtPlotItem;
class JrkPlotData;

//---------------------------------------------------------------------------
class JrkPlotDialog : public QDialog
{
    Q_OBJECT

public:
    explicit JrkPlotDialog(jrk_variables *indata, QWidget *parent = 0);
    ~JrkPlotDialog();

    void run(void);

private slots:
    void onUpdateGraph(void);
    void onFinished(int result);


private Q_SLOTS:
    void showCurve(QwtPlotItem *, bool on);

    void on_stopButton_clicked();

private:
    Ui::JrkPlotDialog *ui;
    QTimer *plot_timer;

    jrk_variables *data_ptr;
    vector<JrkPlotData *> jrkdata;

    double *timeData;
    int    dataCount;

    void createCurve(QString title, QColor cl, bool on, double scale);
    void setCurveData(int curve_id, int data_id, double value);
    void reset(void);
};

#if 0
//---------------------------------------------------------------------------
class JrkPlot : public QwtPlot
{
    Q_OBJECT

public:
    JrkPlot(QwtPlot *plot_, jrk_variables *indata);
    ~JrkPlot(void);

    /*
    const QwtPlotCurve *cpuCurve( int id ) const
    {
        return data[id].curve;
    }
    */

protected:
    void timerEvent(QTimerEvent *e);

private Q_SLOTS:
    void showCurve(QwtPlotItem *, bool on);

private:
    jrk_variables *data_ptr;
    vector<JrkPlotData *> jrkdata;

    double *timeData;
    int    dataCount;

    void createCurve(QString title, QColor cl, bool on, double scale);

};
#endif

//---------------------------------------------------------------------------
class JrkPlotData
{
public:
    JrkPlotData(QwtPlotCurve *curve_, double scale_);
    ~JrkPlotData(void);

    void setValue(double value, int index);

    QwtPlotCurve *curve;
    double *data;
    double scale;

};

#endif // JRKPLOTDIALOG_H
