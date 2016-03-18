#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-5.2.0_musl-1.1.11/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_xscale .config
make -f Makefile.armbe kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/gateworks
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks
cp xscale/src/router/armeb-uclibc/root.fs ~/GruppenLW/releases/$DATE/gateworks
cp xscale/src/linux/xscale/linux-2.6.20/arch/arm/boot/zImage ~/GruppenLW/releases/$DATE/gateworks
