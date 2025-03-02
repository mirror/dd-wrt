#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
cd pb42/src/router
#[ -n "$DO_UPDATE" ] && svn update
#cd opt/etc/config
#[ -n "$DO_UPDATE" ] && svn update
#cd ../../../
cp .config_wzr450hp2_buffalo .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo-wzr_450hp2
cd ../../../
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo-wzr_450hp2/buffalo-wzr_450hp2-firmware.bin
cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo-wzr_450hp2/uimage.bin
