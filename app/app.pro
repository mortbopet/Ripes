TARGET = Ripes

QT += widgets

CONFIG += c++14

INCLUDEPATH += ../src/
SOURCES += main.cpp

win32 {
    CONFIG(release, debug|release) {
      BUILD_PREFIX = release/
    }
    CONFIG(debug, debug|release) {
      BUILD_PREFIX = debug/
    }
}

LIBS += -L$${PWD}/../src/$${BUILD_PREFIX} -lRipesLib

unix {
    isEmpty(PREFIX) {
        PREFIX = /usr
    }
    target.path = $$PREFIX/bin

    desktop.path = $$PREFIX/share/applications/
    desktop.files += Ripes.desktop

    icon512.path = $$PREFIX/share/icons/hicolor/512x512/apps
    icon512.files += Ripes.png

    icon256.path = $$PREFIX/share/icons/hicolor/256x256/apps
    icon256.files += Ripes.png

    INSTALLS += icon512
    INSTALLS += icon256
    INSTALLS += desktop
    INSTALLS += target
}
