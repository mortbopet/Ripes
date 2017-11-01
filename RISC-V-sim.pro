TEMPLATE = app
CONFIG += console c++11
CONFIG -= app_bundle
CONFIG -= qt

SOURCES += src/main.cpp \
    src/parser.cpp \
    src/runner.cpp \
    src/cache.cpp \
    src/dmcache.cpp \
    src/facache.cpp

HEADERS += \
    src/parser.h \
    src/defines.h \
    src/runner.h \
    src/cache.h \
    src/dmcache.h \
    src/facache.h
