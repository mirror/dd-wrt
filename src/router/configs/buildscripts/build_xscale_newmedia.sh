#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-13.1.0_musl/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_xscale_newmedia .config
make -f Makefile.armbe kernel clean all install
mkdir -p /GruppenLW/releases/$DATE/gateworks
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin /GruppenLW/releases/$DATE/gateworks/newmedia-gateworks-firmware.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/gateworks-sputnik/newmedia-linux.bin

