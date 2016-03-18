#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n danube/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_r2_gcc-4.7-linaro_uClibc-0.9.33.2/bin:$OLDPATH
cd danube/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_danube .config
make -f Makefile.danube kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/danube
cd ../../../
cp danube/src/router/mips-uclibc/danube-webflash.bin ~/GruppenLW/releases/$DATE/danube/danube-webflash-firmware.bin
cp danube/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/danube/uImage.bin
