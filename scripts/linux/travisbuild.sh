#!/bin/bash 
qmake --version
 # build
qmake CONFIG+=release PREFIX=/usr
make -j$(nproc)
pushd app
make INSTALL_ROOT=../appdir -j$(nproc) install
popd
echo "Appdir contents:"
find appdir/

wget -c -nv "https://github.com/probonopd/linuxdeployqt/releases/download/continuous/linuxdeployqt-continuous-x86_64.AppImage"
chmod a+x linuxdeployqt-continuous-x86_64.AppImage
unset QTDIR; unset QT_PLUGIN_PATH ; unset LD_LIBRARY_PATH
export VERSION=continuous-linux # linuxdeployqt uses this for naming the file
./linuxdeployqt-continuous-x86_64.AppImage appdir/usr/share/applications/*.desktop -appimage -extra-plugins=iconengines

