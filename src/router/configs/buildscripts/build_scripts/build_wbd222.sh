#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n storm/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-arm_gcc4.2.3/bin:$OLDPATH
cd storm/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_wbd222 .config
make -f Makefile.storm kernel busybox clean all install
mkdir -p ~/GruppenLW/releases/$DATE/wiligear-wbd222
cd ../../../
cp storm/src/router/arm-uclibc/fwupdate.bin ~/GruppenLW/releases/$DATE/wiligear-wbd222
cp storm/src/router/arm-uclibc/wb111-webflash.bin ~/GruppenLW/releases/$DATE/wiligear-wbd222/wb222-webflash.bin

