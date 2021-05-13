#!/bin/bash

# Fail on failing QTest execution
set -e

find . -executable -type f -name "tst_*" | while read line; do
    echo "Running unit test: "
    echo "${line}"
    ($line)
done