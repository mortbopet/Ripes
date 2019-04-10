#!/bin/bash

if [ "$TRAVIS_BRANCH" = "master" ] && [ "$TRAVIS_PULL_REQUEST" = "false" ] && [ "$TRAVIS_EVENT_TYPE" = "push" ]; then
    echo "Uploading to git"
    wget -c https://github.com/probonopd/uploadtool/raw/master/upload.sh
    bash upload.sh Ripes*.exe
fi