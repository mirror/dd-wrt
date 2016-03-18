#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE=$DATE-r
DATE=$DATE$(svnversion -n ipq806x/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH

export PATH=/xfs/toolchains/toolchain-arm_cortex-a9+neon-vfpv4_gcc-5.2.0_musl-1.1.11_eabi/bin:$OLDPATH
cd ipq806x/src/router
[ -n "$DO_UPDATE" ] && svn update
#cp .config_laguna-small .config
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r7500v1
#mkdir -p ~/GruppenLW/releases/$DATE/trendnet-tew827
mkdir -p ~/GruppenLW/releases/$DATE/linksys-ea8500

make -f Makefile.ipq806x kernel clean all install
cd ../../../
#cp ipq806x/src/router/arm-uclibc/tew827-upgrade.bin ~/GruppenLW/releases/$DATE/trendnet-tew827/factory-to-ddwrt.bin
#cp ipq806x/src/router/arm-uclibc/ddwrt-trendnet-tew827.bin ~/GruppenLW/releases/$DATE/trendnet-tew827/dd-wrt-webupgrade.bin

cp ipq806x/src/router/arm-uclibc/ddwrt-netgear-R7500v1.bin ~/GruppenLW/releases/$DATE/netgear-r7500v1/dd-wrt-webupgrade.bin
cp ipq806x/src/router/arm-uclibc/R7500v1-factory-to-ddwrt.img ~/GruppenLW/releases/$DATE/netgear-r7500v1/factory-to-ddwrt.img

cp ipq806x/src/router/arm-uclibc/ddwrt-Linksys-EA8500.bin ~/GruppenLW/releases/$DATE/linksys-ea8500/dd-wrt-webupgrade.bin
cp ipq806x/src/router/arm-uclibc/EA8500-factory-to-ddwrt.img ~/GruppenLW/releases/$DATE/linksys-ea8500/factory-to-ddwrt.img
cp ipq806x/src/router/arm-uclibc/EA8500WW-factory-to-ddwrt.img ~/GruppenLW/releases/$DATE/linksys-ea8500/factory-to-ddwrt-ww.img
