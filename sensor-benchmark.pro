# -------------------------------------------------
# Project created by QtCreator 2010-08-11T08:26:56
# -------------------------------------------------
TARGET = sensor-benchmark
TEMPLATE = app
SOURCES += main.cpp \
    mainwindow.cpp \
    gauge/gauge.cpp \
    gps/gpsdialog.cpp \
    gps/gps.cpp \
    inclinometer/inclinometerdialog.cpp \
    jrk/jrkdialog.cpp \
    jrk/jrk.cpp \
    qextserialport/qextserialport.cpp \
    qbox/qboxdialog.cpp \
    oceanserver/osdialog.cpp \
    utils.cpp \
    qbox/si21xxdialog.cpp \
    qbox/si21xx.cpp \
    qbox/si21xxthread.cpp \
    usb/tusb.cpp \
    usb/usbdevice.cpp \
    jrk/usbjrkdialog.cpp \
    jrk/jrkusb.cpp \
    jrk/jrkplotdialog.cpp \
    jrk/jrkplotsettingsdialog.cpp \
    jrk/jrklut.cpp \
    jrk/jrklutdialog.cpp
HEADERS += mainwindow.h \
    qextserialport/qextserialport.h \
    gauge/gauge.h \
    gps/gpsdialog.h \
    gps/gps.h \
    inclinometer/inclinometerdialog.h \
    jrk/jrkdialog.h \
    jrk/jrk.h \
    qbox/qboxdialog.h \
    oceanserver/osdialog.h \
    utils.h \
    qbox/si21xxdialog.h \
    qbox/si21xx.h \
    qbox/si21xxthread.h \
    usb/tusb.h \
    usb/usbdevice.h \
    jrk/usbjrkdialog.h \
    jrk/jrk_protocol.h \
    jrk/jrkusb.h \
    jrk/jrkplotdialog.h \
    jrk/jrkplotsettingsdialog.h \
    jrk/jrklut.h \
    jrk/jrklutdialog.h
FORMS += mainwindow.ui \
    gps/gpsdialog.ui \
    inclinometer/inclinometerdialog.ui \
    jrk/jrkdialog.ui \
    qbox/qboxdialog.ui \
    oceanserver/osdialog.ui \
    qbox/si21xxdialog.ui \
    jrk/usbjrkdialog.ui \
    jrk/jrkplotdialog.ui \
    jrk/jrkplotsettingsdialog.ui \
    jrk/jrklutdialog.ui

INCLUDEPATH += gauge \
    qextserialport \
    usb \
    gps \
    jrk \
    inclinometer \
    oceanserver \
    qbox

DEFINES += DEBUG_GPS

DEFINES += SENSOR_BENCHMARK
CONFIG += qwt

unix {
   DEFINES += _TTY_LINUX_

   SOURCES += qextserialport/posix_qextserialport.cpp
   INCLUDEPATH += \
                /usr/include \
                /usr/local/qwt-6.0.2-svn/include
   LIBS += -lusb -lqwt
}

win32 {
   SOURCES += qextserialport/win_qextserialport.cpp
}

RESOURCES += resource.qrc











