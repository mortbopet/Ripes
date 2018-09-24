#!/bin/bash

# Adapted from https://github.com/Subsurface-divelog/subsurface/blob/master/scripts/mac/before_install.sh

set -x

# try to get rid of the insane debug crap
unalias -a
unset -f rvm_debug
unset -f cd
unset -f pushd
unset -f popd

# Travis only pulls shallow repos. But that messes with git describe.
# Sorry Travis, fetching the whole thing and the tags as well...
git fetch --unshallow
git pull --tags
git describe

# prep things so we can build for Mac
# we have a custom built Qt some gives us just what we need
# we should just build and install this into /usr/local/ as well and have
# it all be part of the cache...

pushd ${TRAVIS_BUILD_DIR}

mkdir -p Qt/5.11.1

echo "Get custom Qt build and unpack it"
curl --output Qt-5.11.1-mac.tar.xz \
                https://storage.googleapis.com/travis-cache/Qt-5.11.1-mac.tar.xz
md5 Qt-5.11.1-mac.tar.xz

tar -xJ -C Qt/5.11.1 -f Qt-5.11.1-mac.tar.xz
