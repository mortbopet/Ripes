#!/bin/bash

set -e
if ([ "${GITHUB_REF##*/}" = "master" ]) then
    export RIPES_VERSION=continuous
else
    # get tag name and convert '/' to '-'
    export RIPES_VERSION=$(echo ${GITHUB_REF#refs/*/} | sed -e 's/\//-/g')
fi
