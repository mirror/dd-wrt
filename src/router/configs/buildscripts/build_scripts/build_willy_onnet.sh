#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-12.2.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
#cp .config_mms344 .config
cat .config_mms344 | grep -v "^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_ROUTERSTYLE\|^CONFIG_BLUE\|^CONFIG_YELLOW\|^CONFIG_CYAN\|^CONFIG_RED\|^CONFIG_GREEN\|^CONFIG_NOTRIAL" > .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_ONNET=y" >> .config
echo "CONFIG_SUPERCHANNEL=y" >> .config
echo "CONFIG_WILLY=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/willy
cd ../../../
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/willy/willy-firmware.bin
cp pb42/src/router/mips-uclibc/willy.img  ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/willy/willy-factory.img
#cp pb42/src/router/mips-uclibc/dir825c1-uimage.bin ~/GruppenLW/releases/$DATE/dlink-dir825-c1/factory-to-ddwrt_NA.bin


