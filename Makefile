#############################################################################
# Makefile for building: sensor-benchmark
# Generated by qmake (2.01a) (Qt 4.7.3) on: Thu Sep 15 16:31:03 2011
# Project:  sensor-benchmark.pro
# Template: app
# Command: /home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/qmake -spec ../../QtSDK/Desktop/Qt/473/gcc/mkspecs/linux-g++ CONFIG+=debug QMLJSDEBUGGER_PATH=/home/patrik/QtSDK/QtCreator/share/qtcreator/qml/qmljsdebugger -o Makefile sensor-benchmark.pro
#############################################################################

####### Compiler, tools and options

CC            = gcc
CXX           = g++
DEFINES       = -D_TTY_LINUX_ -DQT_GUI_LIB -DQT_CORE_LIB
CFLAGS        = -pipe -g -Wall -W -D_REENTRANT $(DEFINES)
CXXFLAGS      = -pipe -g -Wall -W -D_REENTRANT $(DEFINES)
INCPATH       = -I../../QtSDK/Desktop/Qt/473/gcc/mkspecs/linux-g++ -I. -I../../QtSDK/Desktop/Qt/473/gcc/include/QtCore -I../../QtSDK/Desktop/Qt/473/gcc/include/QtGui -I../../QtSDK/Desktop/Qt/473/gcc/include -Igauge -Iqextserialport -Iusb -Igps -Ijrk -Iinclinometer -Ioceanserver -Iqbox -I/usr/include -I. -I.
LINK          = g++
LFLAGS        = -Wl,-rpath,/home/patrik/QtSDK/Desktop/Qt/473/gcc/lib
LIBS          = $(SUBLIBS)  -L/home/patrik/QtSDK/Desktop/Qt/473/gcc/lib -lusb -lQtGui -lQtCore -lpthread 
AR            = ar cqs
RANLIB        = 
QMAKE         = /home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/qmake
TAR           = tar -cf
COMPRESS      = gzip -9f
COPY          = cp -f
SED           = sed
COPY_FILE     = $(COPY)
COPY_DIR      = $(COPY) -r
STRIP         = strip
INSTALL_FILE  = install -m 644 -p
INSTALL_DIR   = $(COPY_DIR)
INSTALL_PROGRAM = install -m 755 -p
DEL_FILE      = rm -f
SYMLINK       = ln -f -s
DEL_DIR       = rmdir
MOVE          = mv -f
CHK_DIR_EXISTS= test -d
MKDIR         = mkdir -p

####### Output directory

OBJECTS_DIR   = ./

####### Files

SOURCES       = main.cpp \
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
		qextserialport/posix_qextserialport.cpp moc_mainwindow.cpp \
		moc_qextserialport.cpp \
		moc_gauge.cpp \
		moc_gpsdialog.cpp \
		moc_gps.cpp \
		moc_inclinometerdialog.cpp \
		moc_jrkdialog.cpp \
		moc_qboxdialog.cpp \
		moc_osdialog.cpp \
		moc_si21xxdialog.cpp \
		moc_si21xxthread.cpp \
		moc_usbjrkdialog.cpp \
		qrc_resource.cpp
OBJECTS       = main.o \
		mainwindow.o \
		gauge.o \
		gpsdialog.o \
		gps.o \
		inclinometerdialog.o \
		jrkdialog.o \
		jrk.o \
		qextserialport.o \
		qboxdialog.o \
		osdialog.o \
		utils.o \
		si21xxdialog.o \
		si21xx.o \
		si21xxthread.o \
		tusb.o \
		usbdevice.o \
		usbjrkdialog.o \
		jrkusb.o \
		posix_qextserialport.o \
		moc_mainwindow.o \
		moc_qextserialport.o \
		moc_gauge.o \
		moc_gpsdialog.o \
		moc_gps.o \
		moc_inclinometerdialog.o \
		moc_jrkdialog.o \
		moc_qboxdialog.o \
		moc_osdialog.o \
		moc_si21xxdialog.o \
		moc_si21xxthread.o \
		moc_usbjrkdialog.o \
		qrc_resource.o
DIST          = ../../QtSDK/Desktop/Qt/473/gcc/mkspecs/common/g++.conf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/common/unix.conf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/common/linux.conf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/qconfig.pri \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/modules/qt_webkit_version.pri \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/qt_functions.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/qt_config.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/exclusive_builds.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/default_pre.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/debug.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/default_post.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/warn_on.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/qt.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/unix/thread.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/moc.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/resources.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/uic.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/yacc.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/lex.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/include_source_dir.prf \
		sensor-benchmark.pro
QMAKE_TARGET  = sensor-benchmark
DESTDIR       = 
TARGET        = sensor-benchmark

first: all
####### Implicit rules

.SUFFIXES: .o .c .cpp .cc .cxx .C

.cpp.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cc.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.cxx.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.C.o:
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o "$@" "$<"

.c.o:
	$(CC) -c $(CFLAGS) $(INCPATH) -o "$@" "$<"

####### Build rules

all: Makefile $(TARGET)

$(TARGET): ui_mainwindow.h ui_gpsdialog.h ui_inclinometerdialog.h ui_jrkdialog.h ui_qboxdialog.h ui_osdialog.h ui_si21xxdialog.h ui_usbjrkdialog.h $(OBJECTS)  
	$(LINK) $(LFLAGS) -o $(TARGET) $(OBJECTS) $(OBJCOMP) $(LIBS)

Makefile: sensor-benchmark.pro  ../../QtSDK/Desktop/Qt/473/gcc/mkspecs/linux-g++/qmake.conf ../../QtSDK/Desktop/Qt/473/gcc/mkspecs/common/g++.conf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/common/unix.conf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/common/linux.conf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/qconfig.pri \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/modules/qt_webkit_version.pri \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/qt_functions.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/qt_config.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/exclusive_builds.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/default_pre.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/debug.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/default_post.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/warn_on.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/qt.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/unix/thread.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/moc.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/resources.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/uic.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/yacc.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/lex.prf \
		../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/include_source_dir.prf
	$(QMAKE) -spec ../../QtSDK/Desktop/Qt/473/gcc/mkspecs/linux-g++ CONFIG+=debug QMLJSDEBUGGER_PATH=/home/patrik/QtSDK/QtCreator/share/qtcreator/qml/qmljsdebugger -o Makefile sensor-benchmark.pro
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/common/g++.conf:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/common/unix.conf:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/common/linux.conf:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/qconfig.pri:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/modules/qt_webkit_version.pri:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/qt_functions.prf:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/qt_config.prf:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/exclusive_builds.prf:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/default_pre.prf:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/debug.prf:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/default_post.prf:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/warn_on.prf:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/qt.prf:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/unix/thread.prf:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/moc.prf:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/resources.prf:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/uic.prf:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/yacc.prf:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/lex.prf:
../../QtSDK/Desktop/Qt/473/gcc/mkspecs/features/include_source_dir.prf:
qmake:  FORCE
	@$(QMAKE) -spec ../../QtSDK/Desktop/Qt/473/gcc/mkspecs/linux-g++ CONFIG+=debug QMLJSDEBUGGER_PATH=/home/patrik/QtSDK/QtCreator/share/qtcreator/qml/qmljsdebugger -o Makefile sensor-benchmark.pro

dist: 
	@$(CHK_DIR_EXISTS) .tmp/sensor-benchmark1.0.0 || $(MKDIR) .tmp/sensor-benchmark1.0.0 
	$(COPY_FILE) --parents $(SOURCES) $(DIST) .tmp/sensor-benchmark1.0.0/ && $(COPY_FILE) --parents mainwindow.h qextserialport/qextserialport.h gauge/gauge.h gps/gpsdialog.h gps/gps.h inclinometer/inclinometerdialog.h jrk/jrkdialog.h jrk/jrk.h qbox/qboxdialog.h oceanserver/osdialog.h utils.h qbox/si21xxdialog.h qbox/si21xx.h qbox/si21xxthread.h usb/tusb.h usb/usbdevice.h jrk/usbjrkdialog.h jrk/jrk_protocol.h jrk/jrkusb.h .tmp/sensor-benchmark1.0.0/ && $(COPY_FILE) --parents resource.qrc .tmp/sensor-benchmark1.0.0/ && $(COPY_FILE) --parents main.cpp mainwindow.cpp gauge/gauge.cpp gps/gpsdialog.cpp gps/gps.cpp inclinometer/inclinometerdialog.cpp jrk/jrkdialog.cpp jrk/jrk.cpp qextserialport/qextserialport.cpp qbox/qboxdialog.cpp oceanserver/osdialog.cpp utils.cpp qbox/si21xxdialog.cpp qbox/si21xx.cpp qbox/si21xxthread.cpp usb/tusb.cpp usb/usbdevice.cpp jrk/usbjrkdialog.cpp jrk/jrkusb.cpp qextserialport/posix_qextserialport.cpp .tmp/sensor-benchmark1.0.0/ && $(COPY_FILE) --parents mainwindow.ui gps/gpsdialog.ui inclinometer/inclinometerdialog.ui jrk/jrkdialog.ui qbox/qboxdialog.ui oceanserver/osdialog.ui qbox/si21xxdialog.ui jrk/usbjrkdialog.ui .tmp/sensor-benchmark1.0.0/ && (cd `dirname .tmp/sensor-benchmark1.0.0` && $(TAR) sensor-benchmark1.0.0.tar sensor-benchmark1.0.0 && $(COMPRESS) sensor-benchmark1.0.0.tar) && $(MOVE) `dirname .tmp/sensor-benchmark1.0.0`/sensor-benchmark1.0.0.tar.gz . && $(DEL_FILE) -r .tmp/sensor-benchmark1.0.0


clean:compiler_clean 
	-$(DEL_FILE) $(OBJECTS)
	-$(DEL_FILE) *~ core *.core


####### Sub-libraries

distclean: clean
	-$(DEL_FILE) $(TARGET) 
	-$(DEL_FILE) Makefile


check: first

mocclean: compiler_moc_header_clean compiler_moc_source_clean

mocables: compiler_moc_header_make_all compiler_moc_source_make_all

compiler_moc_header_make_all: moc_mainwindow.cpp moc_qextserialport.cpp moc_gauge.cpp moc_gpsdialog.cpp moc_gps.cpp moc_inclinometerdialog.cpp moc_jrkdialog.cpp moc_qboxdialog.cpp moc_osdialog.cpp moc_si21xxdialog.cpp moc_si21xxthread.cpp moc_usbjrkdialog.cpp
compiler_moc_header_clean:
	-$(DEL_FILE) moc_mainwindow.cpp moc_qextserialport.cpp moc_gauge.cpp moc_gpsdialog.cpp moc_gps.cpp moc_inclinometerdialog.cpp moc_jrkdialog.cpp moc_qboxdialog.cpp moc_osdialog.cpp moc_si21xxdialog.cpp moc_si21xxthread.cpp moc_usbjrkdialog.cpp
moc_mainwindow.cpp: mainwindow.h
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/moc $(DEFINES) $(INCPATH) mainwindow.h -o moc_mainwindow.cpp

moc_qextserialport.cpp: qextserialport/qextserialport.h
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/moc $(DEFINES) $(INCPATH) qextserialport/qextserialport.h -o moc_qextserialport.cpp

moc_gauge.cpp: gauge/gauge.h
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/moc $(DEFINES) $(INCPATH) gauge/gauge.h -o moc_gauge.cpp

moc_gpsdialog.cpp: gps/gpsdialog.h
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/moc $(DEFINES) $(INCPATH) gps/gpsdialog.h -o moc_gpsdialog.cpp

moc_gps.cpp: gps/gps.h
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/moc $(DEFINES) $(INCPATH) gps/gps.h -o moc_gps.cpp

moc_inclinometerdialog.cpp: inclinometer/inclinometerdialog.h
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/moc $(DEFINES) $(INCPATH) inclinometer/inclinometerdialog.h -o moc_inclinometerdialog.cpp

moc_jrkdialog.cpp: jrk/jrkdialog.h
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/moc $(DEFINES) $(INCPATH) jrk/jrkdialog.h -o moc_jrkdialog.cpp

moc_qboxdialog.cpp: qbox/qboxdialog.h
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/moc $(DEFINES) $(INCPATH) qbox/qboxdialog.h -o moc_qboxdialog.cpp

moc_osdialog.cpp: oceanserver/osdialog.h
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/moc $(DEFINES) $(INCPATH) oceanserver/osdialog.h -o moc_osdialog.cpp

moc_si21xxdialog.cpp: qbox/si21xxdialog.h
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/moc $(DEFINES) $(INCPATH) qbox/si21xxdialog.h -o moc_si21xxdialog.cpp

moc_si21xxthread.cpp: qbox/si21xxthread.h
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/moc $(DEFINES) $(INCPATH) qbox/si21xxthread.h -o moc_si21xxthread.cpp

moc_usbjrkdialog.cpp: jrk/jrk_protocol.h \
		jrk/usbjrkdialog.h
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/moc $(DEFINES) $(INCPATH) jrk/usbjrkdialog.h -o moc_usbjrkdialog.cpp

compiler_rcc_make_all: qrc_resource.cpp
compiler_rcc_clean:
	-$(DEL_FILE) qrc_resource.cpp
qrc_resource.cpp: resource.qrc \
		images/test-sw.jpg \
		images/Usb-64.png \
		images/gauge-1.png \
		images/compass-1.png
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/rcc -name resource resource.qrc -o qrc_resource.cpp

compiler_image_collection_make_all: qmake_image_collection.cpp
compiler_image_collection_clean:
	-$(DEL_FILE) qmake_image_collection.cpp
compiler_moc_source_make_all:
compiler_moc_source_clean:
compiler_uic_make_all: ui_mainwindow.h ui_gpsdialog.h ui_inclinometerdialog.h ui_jrkdialog.h ui_qboxdialog.h ui_osdialog.h ui_si21xxdialog.h ui_usbjrkdialog.h
compiler_uic_clean:
	-$(DEL_FILE) ui_mainwindow.h ui_gpsdialog.h ui_inclinometerdialog.h ui_jrkdialog.h ui_qboxdialog.h ui_osdialog.h ui_si21xxdialog.h ui_usbjrkdialog.h
ui_mainwindow.h: mainwindow.ui
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/uic mainwindow.ui -o ui_mainwindow.h

ui_gpsdialog.h: gps/gpsdialog.ui
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/uic gps/gpsdialog.ui -o ui_gpsdialog.h

ui_inclinometerdialog.h: inclinometer/inclinometerdialog.ui
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/uic inclinometer/inclinometerdialog.ui -o ui_inclinometerdialog.h

ui_jrkdialog.h: jrk/jrkdialog.ui
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/uic jrk/jrkdialog.ui -o ui_jrkdialog.h

ui_qboxdialog.h: qbox/qboxdialog.ui
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/uic qbox/qboxdialog.ui -o ui_qboxdialog.h

ui_osdialog.h: oceanserver/osdialog.ui
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/uic oceanserver/osdialog.ui -o ui_osdialog.h

ui_si21xxdialog.h: qbox/si21xxdialog.ui
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/uic qbox/si21xxdialog.ui -o ui_si21xxdialog.h

ui_usbjrkdialog.h: jrk/usbjrkdialog.ui
	/home/patrik/QtSDK/Desktop/Qt/473/gcc/bin/uic jrk/usbjrkdialog.ui -o ui_usbjrkdialog.h

compiler_yacc_decl_make_all:
compiler_yacc_decl_clean:
compiler_yacc_impl_make_all:
compiler_yacc_impl_clean:
compiler_lex_make_all:
compiler_lex_clean:
compiler_clean: compiler_moc_header_clean compiler_rcc_clean compiler_uic_clean 

####### Compile

main.o: main.cpp mainwindow.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o main.o main.cpp

mainwindow.o: mainwindow.cpp mainwindow.h \
		ui_mainwindow.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o mainwindow.o mainwindow.cpp

gauge.o: gauge/gauge.cpp gauge/gauge.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o gauge.o gauge/gauge.cpp

gpsdialog.o: gps/gpsdialog.cpp gps/gpsdialog.h \
		ui_gpsdialog.h \
		gps/gps.h \
		mainwindow.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o gpsdialog.o gps/gpsdialog.cpp

gps.o: gps/gps.cpp gps/gps.h \
		utils.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o gps.o gps/gps.cpp

inclinometerdialog.o: inclinometer/inclinometerdialog.cpp inclinometer/inclinometerdialog.h \
		ui_inclinometerdialog.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o inclinometerdialog.o inclinometer/inclinometerdialog.cpp

jrkdialog.o: jrk/jrkdialog.cpp jrk/jrkdialog.h \
		ui_jrkdialog.h \
		jrk/jrk.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o jrkdialog.o jrk/jrkdialog.cpp

jrk.o: jrk/jrk.cpp jrk/jrk.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o jrk.o jrk/jrk.cpp

qextserialport.o: qextserialport/qextserialport.cpp qextserialport/qextserialport.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o qextserialport.o qextserialport/qextserialport.cpp

qboxdialog.o: qbox/qboxdialog.cpp qbox/qboxdialog.h \
		ui_qboxdialog.h \
		utils.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o qboxdialog.o qbox/qboxdialog.cpp

osdialog.o: oceanserver/osdialog.cpp oceanserver/osdialog.h \
		ui_osdialog.h \
		utils.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o osdialog.o oceanserver/osdialog.cpp

utils.o: utils.cpp utils.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o utils.o utils.cpp

si21xxdialog.o: qbox/si21xxdialog.cpp qbox/si21xxdialog.h \
		ui_si21xxdialog.h \
		qbox/si21xx.h \
		utils.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o si21xxdialog.o qbox/si21xxdialog.cpp

si21xx.o: qbox/si21xx.cpp qbox/si21xx.h \
		utils.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o si21xx.o qbox/si21xx.cpp

si21xxthread.o: qbox/si21xxthread.cpp qbox/si21xxthread.h \
		qbox/si21xx.h \
		qbox/si21xxdialog.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o si21xxthread.o qbox/si21xxthread.cpp

tusb.o: usb/tusb.cpp usb/tusb.h \
		usb/usbdevice.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o tusb.o usb/tusb.cpp

usbdevice.o: usb/usbdevice.cpp usb/usbdevice.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o usbdevice.o usb/usbdevice.cpp

usbjrkdialog.o: jrk/usbjrkdialog.cpp jrk/usbjrkdialog.h \
		jrk/jrk_protocol.h \
		ui_usbjrkdialog.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o usbjrkdialog.o jrk/usbjrkdialog.cpp

jrkusb.o: jrk/jrkusb.cpp jrk/jrkusb.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o jrkusb.o jrk/jrkusb.cpp

posix_qextserialport.o: qextserialport/posix_qextserialport.cpp qextserialport/qextserialport.h
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o posix_qextserialport.o qextserialport/posix_qextserialport.cpp

moc_mainwindow.o: moc_mainwindow.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_mainwindow.o moc_mainwindow.cpp

moc_qextserialport.o: moc_qextserialport.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_qextserialport.o moc_qextserialport.cpp

moc_gauge.o: moc_gauge.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_gauge.o moc_gauge.cpp

moc_gpsdialog.o: moc_gpsdialog.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_gpsdialog.o moc_gpsdialog.cpp

moc_gps.o: moc_gps.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_gps.o moc_gps.cpp

moc_inclinometerdialog.o: moc_inclinometerdialog.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_inclinometerdialog.o moc_inclinometerdialog.cpp

moc_jrkdialog.o: moc_jrkdialog.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_jrkdialog.o moc_jrkdialog.cpp

moc_qboxdialog.o: moc_qboxdialog.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_qboxdialog.o moc_qboxdialog.cpp

moc_osdialog.o: moc_osdialog.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_osdialog.o moc_osdialog.cpp

moc_si21xxdialog.o: moc_si21xxdialog.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_si21xxdialog.o moc_si21xxdialog.cpp

moc_si21xxthread.o: moc_si21xxthread.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_si21xxthread.o moc_si21xxthread.cpp

moc_usbjrkdialog.o: moc_usbjrkdialog.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o moc_usbjrkdialog.o moc_usbjrkdialog.cpp

qrc_resource.o: qrc_resource.cpp 
	$(CXX) -c $(CXXFLAGS) $(INCPATH) -o qrc_resource.o qrc_resource.cpp

####### Install

install:   FORCE

uninstall:   FORCE

FORCE:

