#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-5.2.0_musl-1.1.11/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_mi424wr .config
make -f Makefile.armbe kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/actiontec-mi424wr
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/actiontec-mi424wr/actiontec-mi424wr-firmware.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/actiontec-mi424wr/linux.bin
