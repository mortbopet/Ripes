#!/bin/bash 

set -e

# build
cmake -DCMAKE_BUILD_TYPE=Release .
make -j $(nproc)

echo "Post build folder contents:"
ls -lh

# Move bin to appdir
mkdir -p appdir/usr/bin/
mv Ripes appdir/usr/bin/

wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH

if [ "${TRAVIS_BRANCH}" = "prerelease" ]; then
    export VERSION=continuous-linux # linuxdeployqt uses this for naming the file
else 
    export VERSION=${TRAVIS_BRANCH}-linux # linuxdeployqt uses this for naming the file
fi

./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -appimage -extra-plugins=iconengines

echo "Post bundle folder contents:"
ls -lh
