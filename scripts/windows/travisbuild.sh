#!/bin/bash
echo $MXE_TARGET
MXE_DIR=/usr/lib/mxe

export PATH=$MXE_DIR/usr/bin:$PATH

echo $PATH

export RIPES_CFG="release"

echo "Building..."
${MXE_DIR}/usr/${MXE_TARGET}/qt5/bin/qmake CONFIG+=${RIPES_CFG} CONFIG+=mxe_cc CONFIG+=c++14 QMAKE_LFLAGS+=-static-libgcc
make -j$(nproc)
echo "Done."


# Start bundling the .zip
APPNAME=Ripes-continuous-win-x86_64
sudo mv app/${RIPES_CFG}/Ripes.exe $APPNAME.exe
