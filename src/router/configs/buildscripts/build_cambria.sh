#!/bin/sh

OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-5.2.0_musl-1.1.11/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_xscale_cambria .config
echo "KERNELVERSION=4.4" >> .config
make -f Makefile.armbe kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/gateworks_cambria
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks_cambria/cambria-firmware-squashfs.bin
#cp xscale/src/router/armeb-uclibc/gateworks-firmware-jffs.bin ~/GruppenLW/releases/$DATE/cambria/cambria-firmware-jffs2.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/gateworks_cambria/linux.bin

