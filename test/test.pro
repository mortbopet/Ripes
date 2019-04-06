TARGET = RipesTest

QT += testlib widgets

CONFIG += c++14

INCLUDEPATH += ../src/
SOURCES += test.cpp
RESOURCES += tests.qrc

LIBS += -L$${DESTDIR}../src/ -lRipesLib
