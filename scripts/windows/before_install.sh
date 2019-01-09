#!/bin/bash

sudo apt-get update

#echo "deb http://pkg.mxe.cc/repos/apt/debian wheezy main" \
#    | sudo tee /etc/apt/sources.list.d/mxeapt.list
#sudo apt-key adv --keyserver x-hkp://keys.gnupg.net \
#    --recv-keys D43A795B73B16ABE9643FE1AFD8FFF16DB45C6AB
sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 86B72ED9
sudo add-apt-repository 'deb [arch=amd64] http://mirror.mxe.cc/repos/apt trusty main'

sudo apt-get update

# sudo apt-get --yes install upx-ucl

export MXE_TARGET=i686-w64-mingw32.static #x86-64-w64-mingw32.static

MXE2_TARGET=$(echo "$MXE_TARGET" | sed 's/_/-/g')
sudo apt-get --yes install \
    mxe-${MXE2_TARGET} || true
