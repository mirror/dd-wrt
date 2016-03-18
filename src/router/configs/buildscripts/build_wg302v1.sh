#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-5.2.0_musl-1.1.11/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_xscale_wg302v1 .config
make -f Makefile.armbe kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/netgear-wg302v1
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/netgear-wg302v1/wg302v1-webflash-firmware.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/netgear-wg302v1/linux.bin
xscale/src/router/tools/checksum 0846 xscale/src/router/armeb-uclibc/wg302.rmt
cp xscale/src/router/armeb-uclibc/wg302.rmt ~/GruppenLW/releases/$DATE/netgear-wg302v1/WG302.rmt
