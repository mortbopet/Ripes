#!/bin/bash

set -e
# Get the last tag which is not the top-of-head continuous tag
export RIPES_VERSION=$(git describe --tags --exclude "continuous")
