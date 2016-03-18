#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_wndr3700v2 .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/netgear-wndr3700v2
mkdir -p ~/GruppenLW/releases/$DATE/netgear-wndr3800
cd ../../../
cp pb42/src/router/mips-uclibc/wndr3700v2-webflash.bin ~/GruppenLW/releases/$DATE/netgear-wndr3700v2/wndr3700v2-webflash.bin
cp pb42/src/router/mips-uclibc/wndr3700v2-factory_NA.img ~/GruppenLW/releases/$DATE/netgear-wndr3700v2
cp pb42/src/router/mips-uclibc/wndr3700v2-factory_WW.img ~/GruppenLW/releases/$DATE/netgear-wndr3700v2/wndr3700v2-factory.img

cp pb42/src/router/mips-uclibc/wndr3800-webflash.bin ~/GruppenLW/releases/$DATE/netgear-wndr3800/wndr3800-webflash.bin
cp pb42/src/router/mips-uclibc/wndr3800-factory_NA.img ~/GruppenLW/releases/$DATE/netgear-wndr3800
cp pb42/src/router/mips-uclibc/wndr3800-factory_WW.img ~/GruppenLW/releases/$DATE/netgear-wndr3800/wndr3800-factory.img


