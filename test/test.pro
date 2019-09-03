TARGET = RipesTest

QT += testlib widgets

CONFIG += c++14

INCLUDEPATH += ../src/
SOURCES += test.cpp
RESOURCES += tests.qrc

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

win32:CONFIG(release, debug|release): PRE_TARGETDEPS += $${PWD}/../src/release/$${WIN_LIBPREFIX}RipesLib.$${WIN_LIBEXT}
else:win32:CONFIG(debug, debug|release): PRE_TARGETDEPS += $${PWD}/../src/debug/$${WIN_LIBPREFIX}RipesLib.$${WIN_LIBEXT}
else:unix: PRE_TARGETDEPS += $${OUT_PWD}/../src/libRipesLib.a
