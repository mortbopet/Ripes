#!/bin/bash
set -eo pipefail

git shortlog -sn --no-merges \
| awk '{ \
    printf "<p align=\"center\" style=\"margin: 0\">"; \
    for (i=2; i<NF; i++) printf $i " "; \
    print $NF "</p>" \
  }' \
> gen_contributors.html
