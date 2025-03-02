#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n storm/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-arm_gcc4.2.3/bin:$OLDPATH
cd storm/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_storm .config
make -f Makefile.storm kernel busybox clean all install
mkdir -p ~/GruppenLW/releases/$DATE/wiligear-wb111
cd ../../../
cp storm/src/router/arm-uclibc/fwupdate.bin ~/GruppenLW/releases/$DATE/wiligear-wb111
cp storm/src/router/arm-uclibc/wb111-webflash.bin ~/GruppenLW/releases/$DATE/wiligear-wb111

