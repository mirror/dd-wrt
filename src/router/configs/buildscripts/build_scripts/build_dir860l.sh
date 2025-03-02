#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n mt7621/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mipsel_24kc_gcc-12.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mipsel_74kc_gcc-9.2.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mipsel_mips32_gcc-8.2.0_musl/bin:$OLDPATH
cd mt7621/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_dir860 .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_CAKE=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_MT7662=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
make -f Makefile.mt7621 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir860l-b1
cd ../../../

cp mt7621/src/router/mipsel-uclibc/dir860l-webflash.bin ~/GruppenLW/releases/$DATE/dlink-dir860l-b1/dlink-dir860lb1-webflash.bin
cp mt7621/src/router/mipsel-uclibc/web-dir860.img ~/GruppenLW/releases/$DATE/dlink-dir860l-b1/factory-to-ddwrt.bin

#scp mt7621/src/router/mipsel-uclibc/dir860l-webflash.bin 10.88.193.80:/tmp/firmware.bin