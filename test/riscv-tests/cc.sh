#!/bin/bash

# Usage: bash compile.sh [-d] $program
#   -d : Disassemble

FILE=""

if [ "$1" == "-d" ]; then
    FILE=$2
else
    FILE=$1
fi

riscv64-unknown-elf-gcc -march=rv32im -mabi=ilp32 $FILE -o $FILE.out

if [ "$1" == "-d" ]; then
    riscv64-unknown-elf-objdump --disassemble $FILE.out > $FILE.dis
fi
