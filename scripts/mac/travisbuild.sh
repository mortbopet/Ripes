#!/bin/bash

set -x
set -e

export QT_ROOT=${TRAVIS_BUILD_DIR}/Qt/5.13.0/clang_64
export PATH=$QT_ROOT/bin:$PATH # Make sure correct qmake is found on the $PATH
export PKG_CONFIG_PATH=/usr/local/lib/pkgconfig:$PKG_CONFIG_PATH

cmake -DCMAKE_BUILD_TYPE=Release .
make -j $(nproc)

echo "Post build folder contents:"
ls -lh

# Bundle the app
macdeployqt Ripes.app

if [ "${TRAVIS_BRANCH}" = "prerelease" ]; then
    APPNAME=Ripes-${TRAVIS_BRANCH}-mac-x86_64
else 
    APPNAME=Ripes-continuous-mac-x86_64
fi

sudo mv Ripes.app $APPNAME.app

sudo zip -r ${APPNAME}.zip ${APPNAME}.app/

ln -s . 5.0

echo "Post bundle folder contents:"
ls -lh
