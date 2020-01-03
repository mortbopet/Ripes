#!/bin/bash

set -e

sudo apt-key adv --keyserver keyserver.ubuntu.com --recv-keys 86B72ED9
sudo add-apt-repository 'deb [arch=amd64] http://mirror.mxe.cc/repos/apt trusty main'

sudo apt-get update

export MXE_TARGET=i686-w64-mingw32.static #x86-64-w64-mingw32.static

MXE2_TARGET=$(echo "$MXE_TARGET" | sed 's/_/-/g')
sudo apt-get --yes install \
    mxe-${MXE2_TARGET} || true
