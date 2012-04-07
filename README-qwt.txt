Qwt - Qt Widgets for Technical Applications qwt.sourceforge.net/

create a file /etc/ld.so.conf.d/qwt.conf
add to qwt.conf the library path, example: /usr/local/qwt-6.0.2-svn/lib

tell Qt where it is in the ~./bashrc
example:

QWTBASE=/usr/local/qwt-6.0.2-svn
export QT_PLUGIN_PATH=$QT_PLUGIN_PATH:$QWTBASE/plugins
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$QWTBASE/lib
export QMAKEFEATURES=$QWTBASE/features

reboot (logout/in), you should now see the qwt plugins in creator

Add to .pro example:

CONFIG += qwt
INCLUDEPATH += /usr/local/qwt-6.0.2-svn/include
