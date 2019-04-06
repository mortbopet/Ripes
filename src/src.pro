TARGET = RipesLib
TEMPLATE = lib

QT += widgets svg

CONFIG += c++14 staticlib

INCLUDEPATH  += $$PWD \
                $$PWD/graphics/ \
                $$PWD/../external/fancytabbar/

SOURCES += parser.cpp \
    mainwindow.cpp \
    registerwidget.cpp \
    programfiletab.cpp \
    processortab.cpp \
    memorytab.cpp \
    memorymodel.cpp \
    memorydisplaydelegate.cpp \
    memoryview.cpp \
    gotocombobox.cpp \
    addressdialog.cpp \
    codeeditor.cpp \
    tabbar.cpp \
    graphics/shape.cpp \
    graphics/connection.cpp \
    graphics/pipelinewidget.cpp \
    registercontainerwidget.cpp \
    aboutwidget.cpp \
    graphics/backgrounditems.cpp \
    instructionmodel.cpp \
    pipeline.cpp \
    pipelineobjects.cpp \
    assembler.cpp \
    syntaxhighlighter.cpp \
    pipelinetable.cpp \
    pipelinetablemodel.cpp \
    rwjumpwidget.cpp \
    rwjumpmodel.cpp \
    rundialog.cpp \
    binutils.cpp \

HEADERS += \
    parser.h \
    defines.h \
    mainwindow.h \
    registerwidget.h \
    programfiletab.h \
    processortab.h \
    memorytab.h \
    memorymodel.h \
    memorydisplaydelegate.h \
    memoryview.h \
    gotocombobox.h \
    addressdialog.h \
    addressdialog.h \
    codeeditor.h \
    tabbar.h \
    graphics/shape.h \
    graphics/connection.h \
    graphics/pipelinewidget.h \
    registercontainerwidget.h \
    aboutwidget.h \
    graphics/backgrounditems.h \
    instructionmodel.h \
    binutils.h \
    pipelineobjects.h \
    pipeline.h \
    mainmemory.h \
    assembler.h \
    syntaxhighlighter.h \
    pipelinetable.h \
    pipelinetablemodel.h \
    rwjumpwidget.h \
    rwjumpmodel.h \
    lexerutilities.h \
    graphics/descriptions.h \
    rundialog.h

FORMS += \
    mainwindow.ui \
    registerwidget.ui \
    programfiletab.ui \
    processortab.ui \
    memorytab.ui \
    addressdialog.ui \
    registercontainerwidget.ui \
    aboutwidget.ui \
    pipelinetable.ui \
    rwjumpwidget.ui \
    rundialog.ui

# External
DEFINES += QT_NO_PRINTER

HEADERS += \
    $$PWD/../external/fancytabbar/fancytab.h \
    $$PWD/../external/fancytabbar/fancytabbar.h \


SOURCES += \
    $$PWD/../external/fancytabbar/fancytab.cpp \
    $$PWD/../external/fancytabbar/fancytabbar.cpp \

RESOURCES += \
    $$PWD/../resources/images.qrc \
    $$PWD/../examples/examples.qrc
