#!/bin/bash 

set -e

if ([ "$TRAVIS_BRANCH" = "master" ] || [ "${TRAVIS_BRANCH}" = "prerelease" ]) &&
    [ "$TRAVIS_PULL_REQUEST" = "false" ] &&
   ([ "$TRAVIS_EVENT_TYPE" = "push" ] || [ "$TRAVIS_EVENT_TYPE" = "api" ]); then
    export UPLOADTOOL_SUFFIX=${TRAVIS_BRANCH}
    find appdir -executable -type f -exec ldd {} \; | grep " => /usr" | cut -d " " -f 2-3 | sort | uniq
    wget -c https://github.com/probonopd/uploadtool/raw/master/upload.sh
    bash upload.sh Ripes*.AppImage*
fi