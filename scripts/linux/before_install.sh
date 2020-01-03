#!/bin/bash 

export CC=gcc-8
export CXX=g++-8

sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 90

source /opt/qt*/bin/qt*-env.sh
