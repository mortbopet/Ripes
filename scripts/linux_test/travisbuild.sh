#!/bin/bash 

set -e

# build
cmake -DRIPES_BUILD_TESTS=ON -DRIPES_ENABLE_RISCV_TESTS=ON -DCMAKE_BUILD_TYPE=Release .
make -j $(nproc)

echo "Post build folder contents:"
ls -lh
