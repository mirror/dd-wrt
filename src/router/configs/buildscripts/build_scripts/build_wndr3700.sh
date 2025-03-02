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
cp .config_wndr3700 .config
echo "CONFIG_SMARTDNS=y" >> .config
echo "CONFIG_MDNS=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/netgear-wndr3700
cd ../../../
cp pb42/src/router/mips-uclibc/wndr3700-webflash.bin ~/GruppenLW/releases/$DATE/netgear-wndr3700
cp pb42/src/router/mips-uclibc/wndr3700-factory_NA.img ~/GruppenLW/releases/$DATE/netgear-wndr3700
cp pb42/src/router/mips-uclibc/wndr3700-factory_WW.img ~/GruppenLW/releases/$DATE/netgear-wndr3700/wndr3700-factory.img


