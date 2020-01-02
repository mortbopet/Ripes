#!/bin/bash

set -x
set -e

# this gets executed by Travis when building an App for Mac
# it gets started from inside the subsurface directory

export QT_ROOT=${TRAVIS_BUILD_DIR}/Qt/5.13.0/clang_64
export PATH=$QT_ROOT/bin:$PATH # Make sure correct qmake is found on the $PATH
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH

qmake CONFIG+=release CONFIG+=c++14
make -j$(nproc)

# Start bundling the app
pushd app/
macdeployqt Ripes.app

APPNAME=Ripes-continuous-mac-x86_64
sudo mv Ripes.app $APPNAME.app

sudo zip -r ${APPNAME}.zip ${APPNAME}.app/

ln -s . 5.0
popd
