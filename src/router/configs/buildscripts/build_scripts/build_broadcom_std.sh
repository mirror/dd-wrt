#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n broadcom/src/router/httpd)
export PATH=/xfs/4.1.1/bin:$OLDPATH
cd broadcom/src
svn cleanup
[ -n "$DO_UPDATE" ] && svn update
cd ../opt/
[ -n "$DO_UPDATE" ] && svn update
./install_dev.sjh
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n broadcom/src/router/httpd)
mkdir -p ~/GruppenLW/releases/$DATE/broadcom
mkdir -p ~/GruppenLW/releases/$DATE/broadcom/special
mkdir -p ~/GruppenLW/releases/$DATE/broadcom/whr-g125
mkdir -p ~/GruppenLW/releases/$DATE/broadcom/DIR-320
cd ../../
mv ~/GruppenLW/dd-wrt.v23*special* ~/GruppenLW/releases/$DATE/broadcom/special
mv ~/GruppenLW/dd-wrt.v24*special* ~/GruppenLW/releases/$DATE/broadcom/special
mv ~/GruppenLW/dd-wrt.v24*whr-g125* ~/GruppenLW/releases/$DATE/broadcom/whr-g125
mv ~/GruppenLW/dd-wrt.v24*DIR-320* ~/GruppenLW/releases/$DATE/broadcom/DIR-320
mv ~/GruppenLW/dd-wrt.v23_* ~/GruppenLW/releases/$DATE/broadcom
mv ~/GruppenLW/dd-wrt.v24_* ~/GruppenLW/releases/$DATE/broadcom
mv ~/GruppenLW/dd-wrt_megabytes* ~/GruppenLW/releases/$DATE/broadcom
