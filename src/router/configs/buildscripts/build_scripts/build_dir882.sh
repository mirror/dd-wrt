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
cp .config_dir882_mac80211 .config
echo "CONFIG_DIR882=y" >> .config
#echo "CONFIG_IRQBALANCE=y" >> .config
echo "CONFIG_MT7615=y" >> .config
echo "CONFIG_CAKE=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
#echo "CONFIG_MRP=y" >> .config
echo "CONFIG_SPEEDTEST_CLI=y" >> .config
echo "CONFIG_NTFS3=y" >> .config
#echo "CONFIG_LOCKDEBUG=y" >> .config
make -f Makefile.mt7621 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir882-a1
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir878-a1
cd ../../../

cp mt7621/src/router/mipsel-uclibc/dir882-a1-webflash.bin ~/GruppenLW/releases/$DATE/dlink-dir882-a1/dlink-dir882-a1-webflash.bin
cp mt7621/src/router/mipsel-uclibc/aligned-mt7621_dir882.uimage ~/GruppenLW/releases/$DATE/dlink-dir882-a1/factory-to-ddwrt.bin


cp mt7621/src/router/mipsel-uclibc/dir878-a1-webflash.bin ~/GruppenLW/releases/$DATE/dlink-dir878-a1/dlink-dir878-a1-webflash.bin
cp mt7621/src/router/mipsel-uclibc/aligned-mt7621_dir878.uimage ~/GruppenLW/releases/$DATE/dlink-dir878-a1/factory-to-ddwrt.bin

