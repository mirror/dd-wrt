#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-5.2.0_musl-1.1.11/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_usr8200 .config
make -f Makefile.armbe kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/usr8200
#mkdir -p ~/GruppenLW/releases/$DATE/pronghorn-SBC
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/usr8200/usr8200-firmware.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/usr8200/linux.bin



