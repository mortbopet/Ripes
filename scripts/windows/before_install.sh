#!/bin/bash

set -e

export QT_VERSION=5.13.1
export QT_PREFIX=msvc2017_64
export QT_TOOLCHAIN=win64_${QT_PREFIX}
export CMAKE_GENERATOR="Visual Studio 15 2017 Win64"

# Fetch and install Qt using the qbs non-interactive qt installer script
curl -vLO https://code.qt.io/cgit/qbs/qbs.git/plain/scripts/install-qt.sh
bash install-qt.sh \
    --version ${QT_VERSION} \
    --toolchain ${QT_TOOLCHAIN}\
    qtbase qttools svg qtcharts

choco install zip


if [ "${TRAVIS_BRANCH}" = "prerelease" ]; then
    echo "Ripes prerelease build"
fi
