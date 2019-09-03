TARGET = Ripes

QT += widgets

CONFIG += c++14

INCLUDEPATH += ../src/
SOURCES += main.cpp

# Add RipesLib as a static library and as a PRE_TARGETDEPS.
# PRE_TARGETDEPS ensures that this application is relinked every time the
# target dependency is modified.
win32:CONFIG(release, debug|release): LIBS += -L$${OUT_PWD}/../src/release/ -lRipesLib
else:win32:CONFIG(debug, debug|release): LIBS += -L$${OUT_PWD}/../src/debug/ -lRipesLib
else:unix: LIBS += -L$${OUT_PWD}/../src/ -lRipesLib

win32 {
    # if cross compiling with MXE on unix, windows libraries will still be
    # specified as having unix library extensions.
    # Check whether the MXE_CC variable has been set by the CI setup
    CONFIG(mxe_cc) {
        WIN_LIBEXT = "a"
        WIN_LIBPREFIX = "lib"
    } else {
        WIN_LIBPREFIX = ""
        WIN_LIBEXT = "lib"
    }
}

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $${OUT_PWD}/../src/release/$${WIN_LIBPREFIX}RipesLib.$${WIN_LIBEXT}
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $${OUT_PWD}/../src/debug/$${WIN_LIBPREFIX}RipesLib.$${WIN_LIBEXT}
else:unix: PRE_TARGETDEPS += $${OUT_PWD}/../src/libRipesLib.a

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
