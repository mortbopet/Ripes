#!/bin/bash

set -e

export CMAKE_PREFIX_PATH=/c/Qt/${QT_VERSION}/${QT_PREFIX}:${CMAKE_PREFIX_PATH}
export PATH=/c/Qt/${QT_VERSION}/${QT_PREFIX}/bin:${PATH}

cmake . -G "${CMAKE_GENERATOR}"

find /c/Qt/${QT_VERSION}/${QT_PREFIX}/bin

cmake --build .         \
    -j$(nproc)          \
    --config Release

# Copy dependencies using windeployqt
pushd Release

if [ "${TRAVIS_BRANCH}" = "prerelease" ]; then
    APPNAME=Ripes-continuous-win-x86_64.zip
else 
    APPNAME=Ripes-${TRAVIS_BRANCH}-win-x86_64.zip
fi

windeployqt.exe -svg --release Ripes.exe

# Clean up some unused stuff
rm -rf translations

# Bundle
zip -r ${APPNAME} .
popd
mv Release/${APPNAME} .