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
#include <QtGui>
#include <QFontMetrics>
#include <math.h>

#include "gauge.h"

//---------------------------------------------------------------------------
TGauge::TGauge(GaugeType type, QWidget *parent, double minValue, double maxValue) :
    QWidget(parent)
{
    //qDebug("w:%d h:%d", parent->width(), parent->height());

    // assume its size is squared
    setFixedSize(parent->width(), parent->height());
    cp.setX(width() / 2.0);
    cp.setY(height() / 2.0);

    if(width() > height()) {
        // left align
        radius = cp.y() - 20;
        cp.setX(radius + 20);
    }
    else
        radius = cp.x() - 20;

    gaugetype = type;

    value = 30;
    roll  = 10;
    pitch = 15;

    pitch_x = radius * 1.5; // pitch gauge x
    gap = 40; // gap between max and min angle in degrees

    font = new QFont(parent->font());

    switch(gaugetype) {
    case Azimuth_GaugeType:
        {
            min_value = 0;
            max_value = 360;
        }
        break;
    case Elevation_GaugeType:
        {
            min_value = 0;
            max_value = 180;
            value = 0;
        }
        break;
    case MinMax_GaugeType:
        {
            min_value = minValue;
            max_value = maxValue;
        }
        break;
    case RollPitch_GaugeType:
        {
            min_value = 0;
            max_value = 360;
            gap = 0;
        }
        break;

    default:
        {
            type = MinMax_GaugeType;
            min_value = 0;
            max_value = 100;
        }
    }

}

//---------------------------------------------------------------------------
TGauge::~TGauge(void)
{
    delete font;
}

//---------------------------------------------------------------------------
void TGauge::setValue(double new_value)
{
    if(value == new_value)
        return;

    if(new_value < min_value)
        value = min_value;
    else if(new_value > max_value)
        value = max_value;
    else
        value = new_value;

    // qDebug("setvalue: %g", value);
    repaint();
}

//---------------------------------------------------------------------------
void TGauge::setRollPitchValue(double _heading, double _roll, double _pitch)
{
    if(value == _heading && roll == _roll && pitch == _pitch)
        return;

    value = _heading;
    roll  = _roll;
    pitch = _pitch;

    repaint();
}

//---------------------------------------------------------------------------
void TGauge::paintEvent(QPaintEvent *event)
{
    QPainter painter;

    // qDebug("paint value: %g", value);

    painter.begin(this);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.translate(cp);

    switch(gaugetype) {
    case Azimuth_GaugeType:
        drawAzimuthType(&painter, event);
        break;

    case Elevation_GaugeType:
    case MinMax_GaugeType:
        drawElevationType(&painter, event);
        break;

    case RollPitch_GaugeType:
        drawRollPitchType(&painter, event);
        break;
    default:
        break;
    }

    painter.end();
}

//---------------------------------------------------------------------------
void TGauge::drawAzimuthType(QPainter *painter, QPaintEvent* /*event*/)
{
    QPointF points[3];
    int w = 9;

    painter->save();
    painter->rotate(value);

    // north left side
    points[0] = QPointF(-w, 0);
    points[1] = QPointF(0, -radius);
    points[2] = QPointF(0, 0);

    painter->setPen(QPen(Qt::transparent));
    painter->setBrush(QBrush(Qt::red));
    painter->drawPolygon(points, 3);

    // north right side
    points[0] = QPointF(w, 0);
    points[1] = QPointF(0, -radius);
    points[2] = QPointF(0, 0);
    painter->setBrush(QBrush(Qt::darkRed));
    painter->drawPolygon(points, 3);

    // south left side
    points[0] = QPointF(-w, 0);
    points[1] = QPointF(0, radius);
    points[2] = QPointF(0, 0);
    painter->setBrush(QBrush(Qt::blue));
    painter->drawPolygon(points, 3);

    // south right side
    points[0] = QPointF(w, 0);
    points[1] = QPointF(0, radius);
    points[2] = QPointF(0, 0);
    painter->setBrush(QBrush(Qt::darkBlue));
    painter->drawPolygon(points, 3);

    painter->restore();
}


//---------------------------------------------------------------------------
void TGauge::drawElevationType(QPainter *painter, QPaintEvent* /*event*/)
{
    double rad;


#if 1


    painter->setFont(*font);
    painter->setPen(QPen(Qt::black));

    drawCircularTicks(painter, true);

    rad = valuetoangle(value);

    painter->setPen(QPen(Qt::black));
    painter->drawLine(QPointF(0, 0), polarto(radius - 10, rad));


    QString str;
    str.sprintf("%g", value);

    QPointF pt(-10, radius*3/4);
    painter->drawText(pt, str);

#endif


}

//---------------------------------------------------------------------------
void TGauge::drawRollPitchType(QPainter *painter, QPaintEvent * /*event*/)
{
    QPointF pt1, pt2;
    QString str;
    double  rad, v;

    // draw captions and values
    painter->setPen(QPen(Qt::black));
    font->setBold(true);
    painter->setFont(*font);
    QFontMetrics fm(*font);

    str = "Pitch";
    painter->drawText(QPointF(pitch_x - fm.width(str), -radius - fm.xHeight()*2), str);

    str.sprintf("Heading: %g", value);
    painter->drawText(QPointF(-radius, -radius - fm.xHeight()*2), str);
    // N, E, S and W outside the gauge
    painter->drawText(QPointF(-fm.width("N")/2, -radius - fm.xHeight()/2), "N");
    painter->drawText(QPointF(radius + fm.xHeight()/2, fm.xHeight()/2), "E");
    painter->drawText(QPointF(-fm.width("S")/2, radius + fm.xHeight()+4), "S");
    painter->drawText(QPointF(-radius - fm.width("W") - fm.xHeight()/2, fm.xHeight()/2), "W");

    str.sprintf("Roll: %g", roll);
    painter->drawText(QPointF(-radius, radius + fm.xHeight()), str);

    font->setBold(false);
    painter->setFont(*font);

    // heading (azimuth) and roll
    drawCircularTicks(painter, false);
    drawArrow(painter, Qt::darkBlue, value + 180.0);

    // pitch (elevation)
    painter->setPen(QPen(Qt::black));
    drawVerticalTicks(painter, true);
    drawVerticalArrow(painter, Qt::darkBlue, pitch);

    // roll
    v = radius * 0.5;

    painter->setPen(QPen(Qt::black));
    painter->drawLine(-v, 3, v, 3);
    painter->drawLine(-v, -3, v, -3);

    painter->setPen(QPen(Qt::darkBlue));
    rad = valuetoangle(roll - 90.0);
    pt1 = polarto(v, rad);
    pt2 = polarto(v, rad + M_PI);
    painter->drawLine(pt1, pt2);

    drawArrow(painter, Qt::darkBlue, roll - 90.0);
    drawArrow(painter, Qt::darkBlue, roll + 90.0);

}

//---------------------------------------------------------------------------
void TGauge::drawCircularTicks(QPainter *painter, bool drawtext)
{
    QFontMetrics fm(*font);
    QRect txtrect;
    QPointF pt1, pt2;
    double a, rad, delta;
    int    i, w;
    QString str;

#if 1
    delta = 2;
    a = min_value;
    i = 0;

    while(a <= max_value) {

        rad = valuetoangle(a);

        w = (i % 5) == 0 ? 10:5;
        pt1 = polarto(radius - w, rad);
        pt2 = polarto(radius, rad);
        painter->drawLine(pt1, pt2);

        if(drawtext && (i % 10) == 0) {
            str.sprintf("%g", a);
            txtrect = fm.boundingRect(str);

            if(pt1.x() < 0)
                pt1 = polarto(radius - w - txtrect.height() / 2.0, rad);

            painter->drawText(pt1, str);

        }


        i += 1;
        a += delta;
    }

#else
    delta = 5;
    i = 0;
    a = gap;

    while(a < (360.0 - gap)) {

        rad = (a + 90.0) * M_PI / 180.0;

        w = (i % 5) == 0 ? 10:5;
        painter->drawLine(polarto(radius - w, rad),
                          polarto(radius, rad));

        i += 1;
        a += delta;
    }
#endif

}

//---------------------------------------------------------------------------
// draw vertical scale to the right (-90...+90)
// pitch -90...+90 (elevation)
void TGauge::drawVerticalTicks(QPainter *painter, bool drawtext)
{
    QFontMetrics fm(*font);
    QString      str;
    QPointF      pt1;

    double  cx = pitch_x;
    double  a, y;
    int     i, w;

    painter->drawLine(cx, -radius, cx, radius);

    a = -90;
    i = 0;
    while(a <= 90) {
        y = radius / 90.0 * a;
        //w = (i % 5) == 0 ? 10:5;
        w = (i % 2) == 0 ? 10:5;

        painter->drawLine(cx, y, cx - w, y);

        //if(drawtext && (i % 10) == 0) {
        if(drawtext && (i % 4) == 0) {
            str.sprintf("%g", -a);
            pt1.setX(cx - w - fm.width(str) - 5);
            pt1.setY(y + fm.xHeight() / 2);
            painter->drawText(pt1, str);
        }

#if 0
        i += 1;
        a += 2;
#else
        i += 1;
        a += 5;
#endif
    }

    //qDebug("x: %g y: %g 2r: %g", topright.x(), topright.y(), radius*2.0);
}

//---------------------------------------------------------------------------
// angle is in degrees
void TGauge::drawArrow(QPainter *painter, QColor cl, double angle)
{
    QPointF points[4];
    double edge = radius - 5;
    double  w;

    painter->setBrush(QBrush(cl));
    painter->setPen(QPen(cl));

    w = 20; // 15 degree edge length

    points[0] = polarto(edge, valuetoangle(angle)); // arrow sharp edge
    points[1] = polarto(w, valuetoangle(angle - 165.0),
                        points[0].x(), points[0].y());
    points[2] = polarto(edge - w / 2.0, valuetoangle(angle));
    points[3] = polarto(w, valuetoangle(angle + 165.0),
                        points[0].x(), points[0].y());

    painter->drawPolygon(points, 4);
    painter->drawLine(points[2],
                      polarto(w, valuetoangle(angle + 180.0),
                                              points[2].x(), points[2].y()));
}

//---------------------------------------------------------------------------
void TGauge::drawVerticalArrow(QPainter *painter, QColor cl, double angle)
{
    QFontMetrics fm(*font);
    QString      str;
    QPointF points[4];
    double  cx = pitch_x - 7;
    double  cy = -radius / 90.0 * angle;

    painter->setBrush(QBrush(cl));
    painter->setPen(QPen(cl));

    // 15 degree edge length = 20 pixels

    points[0] = QPointF(cx, cy);
    points[1] = QPointF(cx - 19, cy + 5);
    points[2] = QPointF(cx - 11, cy);
    points[3] = QPointF(cx - 19, cy - 5);
    painter->drawPolygon(points, 4);

    painter->drawLine(points[2], QPointF(points[2].x()-20, points[2].y()));

    str.sprintf("%g", angle);
    points[0].setX(pitch_x + 5);
    points[0].setY(cy + fm.xHeight() / 2);
    painter->drawText(points[0], str);
}

//---------------------------------------------------------------------------
// angle is in radians
QPointF TGauge::polarto(double length, double angle,
                        double dx, double dy)
{
    qreal x = dx + length * cos(angle);
    qreal y = dy + length * sin(angle);

    return QPointF(x, y);
}

//---------------------------------------------------------------------------
// returns the gauge value converted to position in radians
double TGauge::valuetoangle(double gauge_value)
{
    double v;

    v = (90.0 + gap + ((360.0 - 2.0 * gap) / (max_value - min_value) ) * gauge_value) * M_PI / 180.0;

    return v;
}

//---------------------------------------------------------------------------
