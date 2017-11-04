QT += widgets printsupport

CONFIG += c++14

INCLUDEPATH  += $$PWD/src/ \
                $$PWD/external/QCustomPlot/


SOURCES += src/main.cpp \
    src/parser.cpp \
    src/runner.cpp \
    src/dmcache.cpp \
    src/facache.cpp \
    src/mainwindow.cpp \
    src/registerwidget.cpp \
    src/programfiletab.cpp \
    src/processortab.cpp \
    src/memorytab.cpp \
    src/cachetab.cpp \
    src/cachesetupwidget.cpp \
    src/cachebase.cpp \
    src/runnercache.cpp \
    src/cacheinspector.cpp

HEADERS += \
    src/parser.h \
    src/defines.h \
    src/runner.h \
    src/dmcache.h \
    src/facache.h \
    src/mainwindow.h \
    src/registerwidget.h \
    src/programfiletab.h \
    src/processortab.h \
    src/memorytab.h \
    src/cachetab.h \
    src/cachesetupwidget.h \
    src/cachebase.h \
    src/runnercache.h \
    src/cacheinspector.h

FORMS += \
    src/mainwindow.ui \
    src/registerwidget.ui \
    src/programfiletab.ui \
    src/processortab.ui \
    src/memorytab.ui \
    src/cachetab.ui \
    src/cachesetupwidget.ui \
    src/cacheinspector.ui


# External
HEADERS += \
   external/QCustomPlot/qcustomplot.h

SOURCES += \
    external/QCustomPlot/qcustomplot.cpp
