#!/bin/bash 
echo "Uploading to git"
wget -c https://github.com/probonopd/uploadtool/raw/master/upload.sh
bash upload.sh Ripes*.exe
