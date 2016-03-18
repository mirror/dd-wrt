#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE=$DATE-r
DATE=$DATE$(svnversion -n mvebu/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH

export PATH=/xfs/toolchains/toolchain-arm_cortex-a9+vfpv3_gcc-5.2.0_musl-1.1.11_eabi/bin:$OLDPATH
cd mvebu/src/router
[ -n "$DO_UPDATE" ] && svn update
#cp .config_laguna-small .config
mkdir -p ~/GruppenLW/releases/$DATE/linksys-wrt1900ac
mkdir -p ~/GruppenLW/releases/$DATE/linksys-wrt1900acs
mkdir -p ~/GruppenLW/releases/$DATE/linksys-wrt1900acv2
mkdir -p ~/GruppenLW/releases/$DATE/linksys-wrt1200ac

make -f Makefile.mvebu kernel clean all install
cd ../../../
cp mvebu/src/router/arm-uclibc/ddwrt-mvebu-armada-xp-mamba-squashfs-factory.img ~/GruppenLW/releases/$DATE/linksys-wrt1900ac/factory-to-ddwrt.img
cp mvebu/src/router/arm-uclibc/ddwrt-linksys-wrt1900ac.bin ~/GruppenLW/releases/$DATE/linksys-wrt1900ac/ddwrt-linksys-wrt1900ac-webflash.bin

cp mvebu/src/router/arm-uclibc/ddwrt-mvebu-armada-385-caiman-squashfs-factory.img ~/GruppenLW/releases/$DATE/linksys-wrt1200ac/factory-to-ddwrt.img
cp mvebu/src/router/arm-uclibc/ddwrt-linksys-wrt1200ac.bin ~/GruppenLW/releases/$DATE/linksys-wrt1200ac/ddwrt-linksys-wrt1200ac-webflash.bin

cp mvebu/src/router/arm-uclibc/ddwrt-mvebu-armada-385-cobra-squashfs-factory.img ~/GruppenLW/releases/$DATE/linksys-wrt1900acv2/factory-to-ddwrt.img
cp mvebu/src/router/arm-uclibc/ddwrt-linksys-wrt1900acv2.bin ~/GruppenLW/releases/$DATE/linksys-wrt1900acv2/ddwrt-linksys-wrt1900acv2-webflash.bin

cp mvebu/src/router/arm-uclibc/ddwrt-mvebu-armada-385-shelby-squashfs-factory.img ~/GruppenLW/releases/$DATE/linksys-wrt1900acs/factory-to-ddwrt.img
cp mvebu/src/router/arm-uclibc/ddwrt-linksys-wrt1900acs.bin ~/GruppenLW/releases/$DATE/linksys-wrt1900acs/ddwrt-linksys-wrt1900acs-webflash.bin

