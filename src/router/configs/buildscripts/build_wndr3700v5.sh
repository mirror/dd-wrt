#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n mt7621/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mipsel_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mipsel_mips32_gcc-8.2.0_musl/bin:$OLDPATH
cd mt7621/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_wndr3700v5 .config
echo "CONFIG_DIR882=y" >> .config
echo "CONFIG_MT7615=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_CAKE=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
#echo "CONFIG_MRP=y" >> .config
#echo "CONFIG_CFM=y" >> .config
#echo "CONFIG_HTOP=y" >> .config
echo "CONFIG_NTFS3=y" >> .config
echo "CONFIG_MDNS=y" >> .config
#echo "CONFIG_SPEEDTEST_CLI=y" >> .config
make -f Makefile.mt7621 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/netgear-wndr3700v5
cd ../../../

cp mt7621/src/router/mipsel-uclibc/3700-webflash.bin ~/GruppenLW/releases/$DATE/netgear-wndr3700v5/wndr3700v5-webflash.bin
cp mt7621/src/router/mipsel-uclibc/3700.img ~/GruppenLW/releases/$DATE/netgear-wndr3700v5/factory-to-ddwrt.img



#scp mt7621/src/router/mipsel-uclibc/dir878-a1-webflash.bin 10.88.193.93:/tmp/firmware.bin