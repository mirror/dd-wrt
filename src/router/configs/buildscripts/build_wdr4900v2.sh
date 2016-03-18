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
cp .config_wr1043v2 .config
echo CONFIG_ARCHERC7=y >> .config
echo CONFIG_WDR4900V2=y >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wdr4900v2
cd ../../../
cp pb42/src/router/mips-uclibc/TL-WDR4900v2-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr4900v2/tl-wdr4900v2-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-TL-WDR4900v2-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr4900v2/factory-to-ddwrt.bin


