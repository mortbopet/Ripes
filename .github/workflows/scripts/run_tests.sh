#!/bin/bash

# Fail on failing QTest execution
set -e

find . -type f -name "tst_*" -exec test -x {} \; -print | while read line; do
    echo "Running unit test: "
    echo "${line}"
    ($line)
done
