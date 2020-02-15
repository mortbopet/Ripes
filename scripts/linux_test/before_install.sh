#!/bin/bash 

export CC=gcc-8
export CXX=g++-8

sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 90

source /opt/qt*/bin/qt*-env.sh

pip install googledrivedownloader

# Grab prebuilt riscv64-unknown-elf toolchain, required for building RISC-V
# related unit tests
wget --load-cookies /tmp/cookies.txt \
    "https://docs.google.com/uc?export=download&confirm=$(wget --quiet --save-cookies /tmp/cookies.txt --keep-session-cookies --no-check-certificate 'https://docs.google.com/uc?export=download&id=1KHCVeLFw6ef_HF0E4g3sqczCFYG_8EtA' -O- | sed -rn 's/.*confirm=([0-9A-Za-z_]+).*/\1\n/p')&id=1KHCVeLFw6ef_HF0E4g3sqczCFYG_8EtA" \
    -O riscv64-unknown-elf_x86-64.tar.gz && rm -rf /tmp/cookies.txt

echo "Downloaded files:"
ls -lha

# Unpack and add to path
tar -xvf riscv64-unknown-elf_x86-64.tar.gz
export PATH=$(pwd)/riscv64-unknown-elf_x86-64/bin/:${PATH}
