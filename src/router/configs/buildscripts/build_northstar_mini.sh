#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n northstar/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-arm_cortex-a9_gcc-5.3.0_musl-1.1.14_eabi/bin:$OLDPATH
cd northstar/src/router
[ -n "$DO_UPDATE" ] && svn update
#cp .config_laguna-small .config
#mkdir -p ~/GruppenLW/releases/$DATE/northstar
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ac56u
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ac67u
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wzr-1750dhp
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wzr-1166dhp
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wzr-900dhp
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wzr-600dhp2
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir868l


cd northstar/src/router
cp .config_northstar_mini .config
make -f Makefile.northstar kernel clean all install
cd ../../../
cp northstar/src/router/arm-uclibc/web-dir868.img ~/GruppenLW/releases/$DATE/dlink-dir868l/factory-to-ddwrt.bin
cp northstar/src/router/arm-uclibc/webflash-dir868.trx ~/GruppenLW/releases/$DATE/dlink-dir868l/dir868-webflash.bin
