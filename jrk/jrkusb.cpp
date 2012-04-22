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
#include <QSettings>
#include <QFile>
#include <stdlib.h>
#include <math.h>

#include "jrkusb.h"
#include "usbdevice.h"
#include "jrklut.h"
#include "utils.h"

//---------------------------------------------------------------------------
TJrkUSB::TJrkUSB(void)
{
    jrk = NULL;

    jrk_flags = 0;

    min_deg = 0;
    max_deg = 360;

    iobuff = (unsigned char *) malloc(JRK_IO_BUF_SIZE + 1);
    memset(&vars, 0, sizeof(jrk_variables_t));
}

//---------------------------------------------------------------------------
TJrkUSB::~TJrkUSB(void)
{
    free(iobuff);
    flushLUT();
}

//---------------------------------------------------------------------------
void TJrkUSB::setFlag(unsigned int flag, bool on)
{
    if(on)
        jrk_flags |= flag;
    else
        jrk_flags &= ~flag;
}

//---------------------------------------------------------------------------
bool TJrkUSB::isFlagOn(unsigned int flag)
{
    return (jrk_flags & flag) ? true:false;
}

//---------------------------------------------------------------------------
bool TJrkUSB::setDevice(TUSBDevice *usbdev)
{
    jrk = usbdev;
    sn_ = "";

    if(!isOpen() || !readVariables())
        return false;

    sn_ = jrk->serialnumber();

    return true;
}

//---------------------------------------------------------------------------
bool TJrkUSB::isOpen(void)
{
    return (jrk && jrk->isOpen()) ? true:false;
}

//---------------------------------------------------------------------------
void TJrkUSB::readSettings(QSettings *reg)
{
    if(!isOpen() || sn_.isEmpty())
        return;

    reg->beginGroup(sn_);

    jrk_flags = reg->value("Flags", "0").toInt();
    lutFile_ = reg->value("LUTFile", "").toString();

    max_deg = reg->value("MaxDegrees", "360").toDouble();
    min_deg = reg->value("MinDegrees", "0").toDouble();

    reg->endGroup();

    loadLUT();
}

//---------------------------------------------------------------------------
void TJrkUSB::writeSettings(QSettings *reg)
{
    if(!isOpen() || sn_.isEmpty())
        return;

    reg->beginGroup(sn_);

    reg->value("Flags", jrk_flags);
    reg->value("LUTFile", lutFile_);

    reg->setValue("MaxDegrees", max_deg);
    reg->setValue("MinDegrees", min_deg);

    reg->endGroup();
}

//---------------------------------------------------------------------------
bool TJrkUSB::readVariables(void)
{
    bool rc = false;

    if(isOpen())
        if(jrk->control_transfer(JRK_REQUEST_GET_TYPE, JRK_REQUEST_GET_VARIABLES, 0, 0, (char *)iobuff, sizeof(jrk_variables_t)))
            rc = true;

    if(rc)
        vars = *(jrk_variables *) iobuff;

    return rc;
}

//---------------------------------------------------------------------------
void TJrkUSB::stop(void)
{
    if(isOpen())
        jrk->control_write(JRK_REQUEST_SET_TYPE, JRK_REQUEST_MOTOR_OFF, 0, 0);
}

//---------------------------------------------------------------------------
void TJrkUSB::clearErrors(void)
{
    if(isOpen())
        jrk->control_write(JRK_REQUEST_SET_TYPE, JRK_REQUEST_CLEAR_ERRORS, 0, 0);
}

//---------------------------------------------------------------------------
void TJrkUSB::errorStr(QStringList *sl)
{
    sl->clear();

    if(!readVariables()) {
        sl->append("Fatal Error: Unable to read variables");
        return;
    }

    unsigned short err = vars.errorFlagBits | vars.errorOccurredBits;

    if(err & 0x0001)
        sl->append("Awaiting command");
    if(err & 0x0002)
        sl->append("No power");
    if(err & 0x0004)
        sl->append("Motor driver error");
    if(err & 0x0008)
        sl->append("Input invalid (Pulse Width Input Mode only)");
    if(err & 0x0010)
        sl->append("Input disconnect");
    if(err & 0x0020)
        sl->append("Feedback disconnect");
    if(err & 0x0040)
        sl->append("Maximum current exceeded");
    if(err & 0x0080)
        sl->append("Serial signal error");
    if(err & 0x0100)
        sl->append("Serial overrun");
    if(err & 0x0200)
        sl->append("Serial RX buffer full");
    if(err & 0x0400)
        sl->append("Serial CRC error");
    if(err & 0x0800)
        sl->append("Serial protocol error");
    if(err & 0x1000)
        sl->append("Serial timeout error");
}


//---------------------------------------------------------------------------
bool TJrkUSB::setTarget(unsigned short target)
{
    if(isOpen())
        if(jrk->control_write(JRK_REQUEST_SET_TYPE,
                              JRK_REQUEST_SET_TARGET,
                              (target & 0x0fff), 0))
            return true;

    return false;
}

//---------------------------------------------------------------------------
// mode & 1 = read variables
unsigned short TJrkUSB::target(int mode)
{
    if(mode & 1)
        readVariables();

    return vars.target;
}

//---------------------------------------------------------------------------
// mode & 1 = read variables
unsigned short TJrkUSB::scaledfeedback(int mode)
{
    if(mode & 1)
        readVariables();

    return vars.scaledFeedback;
}

//---------------------------------------------------------------------------
// mode & 1 = read variables
unsigned short TJrkUSB::feedback(int mode)
{
    if(mode & 1)
        readVariables();

    return vars.feedback;
}

//---------------------------------------------------------------------------
// mode & 1 = read variables
double TJrkUSB::degrees(int mode)
{
    if(mode & 1)
        readVariables();

    return targetTodegrees(vars.target);
}

//---------------------------------------------------------------------------
// mode & 1 = read variables
void TJrkUSB::moveToDegrees(double deg, int mode)
{
    if(!isOpen())
        return;

    if(mode & 1)
        if(!readVariables())
            return;

    deg = deg < min_deg ? min_deg:(deg > max_deg ? max_deg:deg);


    //setTarget(RINT(dt));
}

//---------------------------------------------------------------------------
// t can be either target or scaledfeedback, max 4095
double TJrkUSB::targetTodegrees(unsigned short t)
{
    if(isFlagOn(JRK_USE_LUT))
        return lutTargetToDegrees(t);
    else
        return valueTodegrees(t);
}

//---------------------------------------------------------------------------
// t can be either target or scaled feedback
double TJrkUSB::valueTodegrees(unsigned short t)
{
    double deg;

    t &= 0x0fff;
    deg = min_deg + (max_deg - min_deg) * ((double) t) / 4095.0;

    return deg;
}

//---------------------------------------------------------------------------
unsigned short TJrkUSB::degreesTovalue(double deg)
{
    double dt, d_deg = max_deg - min_deg;
    unsigned short t;

    deg = deg < min_deg ? min_deg:(deg > max_deg ? max_deg:deg);

    if(d_deg <= 0)
        dt = 0;
    else
        dt = (deg - min_deg) * 4095.0 / d_deg;

    t = ((unsigned short) RINT(dt)) & 0x0fff;

    return t;
}


//---------------------------------------------------------------------------
double TJrkUSB::lutTargetToDegrees(unsigned short t)
{
    JrkLUT *l, *l_min = NULL, *l_max = NULL;
    double d_min = min_deg, d_max = max_deg;
    double t_min = 0, t_max = 4095;
    double deg, delta_t, delta_d;

    size_t i;

    t &= 0x0fff;
    for(i=0; i<jrklut.size(); i++) {
        l = (JrkLUT *) jrklut.at(i);

        if(l->target() <= t)
            l_min = l;
        else if(l->target() > t)
            l_max = l;

        if(l_min && l_max)
            break;
    }


    if(l_min) {
        d_min = l_min->degrees();
        t_min = l_min->target();
    }

    if(l_max) {
        d_max = l_max->degrees();
        t_max = l_max->target();
    }

    delta_t = t_max - t_min;
    delta_d = d_max - d_min;

    if(delta_t <= 0)
        deg = valueTodegrees(t); // fatal LUT input error...
    else
        deg = d_min + ((((double) t) - t_min) * delta_d) / delta_t;

    return deg;
}

//---------------------------------------------------------------------------
void TJrkUSB::flushLUT(void)
{
    vector<JrkLUT *>::iterator i = jrklut.begin();

    for(; i < jrklut.end(); i++)
        delete *i;

    jrklut.clear();
}

//---------------------------------------------------------------------------
void TJrkUSB::loadLUT(void)
{
    bool error = true;

    flushLUT();

    if(isFlagOn(JRK_USE_LUT)) {
        if(!lutFile_.isEmpty()) {
            QFile file(lutFile_);
            if(file.exists())
                error = false;
            else
                lutFile_ = "";
        }
    }

    if(error) {
        setFlag(JRK_USE_LUT, false);

        return;
    }

    //
    // load the lookup table ini file
    //

    QSettings reg(lutFile_, QSettings::IniFormat);

    QString d_str, t_str;
    double  d;
    int     t, i = 0;

    reg.beginGroup("JrkTargetToDegrees");

    while(true) {
        t_str.sprintf("Target-%04d", i);
        d_str.sprintf("Degrees-%04d", i);

        t = reg.value(t_str, -999).toInt();
        d = reg.value(d_str, -999).toDouble();

        if(t < 0 || t > 4095 || d < 0 || d > 360)
            break;

        jrklut.push_back(new JrkLUT(t, d));
        i++;
    }

    reg.endGroup();

    if(jrklut.size() == 0)
        setFlag(JRK_USE_LUT, false);
}
