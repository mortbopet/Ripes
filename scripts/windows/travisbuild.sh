#!/bin/bash
echo $MXE_TARGET
MXE_DIR=/usr/lib/mxe

export PATH=$MXE_DIR/usr/bin:$PATH

echo $PATH

echo "Running QMake"
${MXE_DIR}/usr/${MXE_TARGET}/qt5/bin/qmake CONFIG+=release CONFIG+=c++14 QMAKE_LFLAGS+=-static-libgcc


echo "Running make"
make -j 2
echo "Done."

# Start bundling the .zip
APPNAME=Ripes-continuous-win-x86_64

cd release
sudo mv Ripes.exe $APPNAME.exe