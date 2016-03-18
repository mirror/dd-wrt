#!/bin/sh
cd broadcom/src
#svn cleanup
[ -n "$DO_UPDATE" ] && svn update
cd ../opt/
[ -n "$DO_UPDATE" ] && svn update
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ../src/router/httpd)
echo $DATE
export PATH=/xfs/4.1.1/bin:$OLDPATH
./install_dev.sjh
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ../src/router/httpd)
mkdir -p ~/GruppenLW/releases/$DATE/broadcom
mkdir -p ~/GruppenLW/releases/$DATE/broadcom/special
cd ../../
mkdir -p ~/GruppenLW/releases/$DATE/broadcom/buffalo
cp -v ~/GruppenLW/*WHR-HP* ~/GruppenLW/releases/$DATE/broadcom/buffalo
cp -v ~/GruppenLW/*AS-A100* ~/GruppenLW/releases/$DATE/broadcom/buffalo
cp -v ~/GruppenLW/dd-wrt.v24*special* ~/GruppenLW/releases/$DATE/broadcom/special
cp -v ~/GruppenLW/dd-wrt.v24_* ~/GruppenLW/releases/$DATE/broadcom
