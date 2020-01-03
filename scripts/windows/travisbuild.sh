#!/bin/bash

set -e

echo $MXE_TARGET
MXE_DIR=/usr/lib/mxe

export PATH=$MXE_DIR/usr/bin:$PATH
export CMAKE_PREFIX_PATH=${MXE_DIR}/usr/${MXE_TARGET}/qt5/:$CMAKE_PREFIX_PATH

export RIPES_CFG="release"

$MXE_TARGET-cmake .
make -j $(nproc)

echo "Post build folder contents:"
ls -lh

APPNAME=Ripes-continuous-win-x86_64
sudo mv Ripes.exe $APPNAME.exe