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
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ac56u
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ac68u
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-n18u
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r7000
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6300v2
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6250
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wzr-1750dhp
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wxr-1900dhp
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wzr-1166dhp
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wzr-900dhp
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wzr-600dhp2
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir868l
mkdir -p ~/GruppenLW/releases/$DATE/linksys-ea6900
mkdir -p ~/GruppenLW/releases/$DATE/linksys-ea6700
mkdir -p ~/GruppenLW/releases/$DATE/linksys-ea6500v2
mkdir -p ~/GruppenLW/releases/$DATE/trendnet-811DRU
mkdir -p ~/GruppenLW/releases/$DATE/trendnet-812DRUv2
mkdir -p ~/GruppenLW/releases/$DATE/trendnet-818DRU





cp .config_northstar_16m .config
make -f Makefile.northstar kernel clean all install
cd ../../../

cp northstar/src/router/arm-uclibc/tnet818.trx ~/GruppenLW/releases/$DATE/trendnet-818DRU/trendnet-818dru-webflash.bin
