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
cp .config_r6800 .config
echo "CONFIG_DIR882=y" >> .config
echo "CONFIG_MT7615=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_CAKE=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
echo "CONFIG_MRP=y" >> .config
echo "CONFIG_CFM=y" >> .config
echo "CONFIG_HTOP=y" >> .config
echo "CONFIG_NTFS3=y" >> .config
echo "CONFIG_APFS=y" >> .config
echo "CONFIG_SPEEDTEST_CLI=y" >> .config
echo "CONFIG_MDNS=y" >> .config
echo "CONFIG_BTOP=y" >> .config
echo "CONFIG_POWERTOP=y" >> .config
echo "CONFIG_HTOP=y" >> .config
#echo "CONFIG_MDNS_UTILS=y" >> .config
#echo "CONFIG_LOCKDEBUG=y" >> .config
make -f Makefile.mt7621 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6800
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6900v2
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6700v2
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r7200
mkdir -p ~/GruppenLW/releases/$DATE/netgear-nighthawk-ac2100
mkdir -p ~/GruppenLW/releases/$DATE/netgear-nighthawk-ac2400
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r7450
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6850
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6260
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6350
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6220
mkdir -p ~/GruppenLW/releases/$DATE/netgear-wac124
cd ../../../

cp mt7621/src/router/mipsel-uclibc/r6800-webflash.bin ~/GruppenLW/releases/$DATE/netgear-r6800/r6800-webflash.bin
cp mt7621/src/router/mipsel-uclibc/r6800.img ~/GruppenLW/releases/$DATE/netgear-r6800/factory-to-ddwrt.img

cp mt7621/src/router/mipsel-uclibc/r6800-webflash.bin ~/GruppenLW/releases/$DATE/netgear-nighthawk-ac2100/ac2100-webflash.bin
cp mt7621/src/router/mipsel-uclibc/r6800.img ~/GruppenLW/releases/$DATE/netgear-nighthawk-ac2100/factory-to-ddwrt.img

cp mt7621/src/router/mipsel-uclibc/r6800-webflash.bin ~/GruppenLW/releases/$DATE/netgear-nighthawk-ac2400/ac2400-webflash.bin
cp mt7621/src/router/mipsel-uclibc/r6800.img ~/GruppenLW/releases/$DATE/netgear-nighthawk-ac2400/factory-to-ddwrt.img

cp mt7621/src/router/mipsel-uclibc/r6800-webflash.bin ~/GruppenLW/releases/$DATE/netgear-r6900v2/r6900v2-webflash.bin
cp mt7621/src/router/mipsel-uclibc/r6800.img ~/GruppenLW/releases/$DATE/netgear-r6900v2/factory-to-ddwrt.img

cp mt7621/src/router/mipsel-uclibc/r6800-webflash.bin ~/GruppenLW/releases/$DATE/netgear-r7450/r7450-webflash.bin
cp mt7621/src/router/mipsel-uclibc/r6800.img ~/GruppenLW/releases/$DATE/netgear-r7450/factory-to-ddwrt.img

cp mt7621/src/router/mipsel-uclibc/r6800-webflash.bin ~/GruppenLW/releases/$DATE/netgear-r6700v2/r6700v2-webflash.bin
cp mt7621/src/router/mipsel-uclibc/r6800.img ~/GruppenLW/releases/$DATE/netgear-r6700v2/factory-to-ddwrt.img

cp mt7621/src/router/mipsel-uclibc/r6800-webflash.bin ~/GruppenLW/releases/$DATE/netgear-r7200/r7200-webflash.bin
cp mt7621/src/router/mipsel-uclibc/r6800.img ~/GruppenLW/releases/$DATE/netgear-r7200/factory-to-ddwrt.img

cp mt7621/src/router/mipsel-uclibc/r6850-webflash.bin ~/GruppenLW/releases/$DATE/netgear-r6850/r6850-webflash.bin
cp mt7621/src/router/mipsel-uclibc/r6850.img ~/GruppenLW/releases/$DATE/netgear-r6850/factory-to-ddwrt.img

cp mt7621/src/router/mipsel-uclibc/r6260-webflash.bin ~/GruppenLW/releases/$DATE/netgear-r6260/r6260-webflash.bin
cp mt7621/src/router/mipsel-uclibc/r6260.img ~/GruppenLW/releases/$DATE/netgear-r6260/factory-to-ddwrt.img

cp mt7621/src/router/mipsel-uclibc/r6350-webflash.bin ~/GruppenLW/releases/$DATE/netgear-r6350/r6350-webflash.bin
cp mt7621/src/router/mipsel-uclibc/r6350.img ~/GruppenLW/releases/$DATE/netgear-r6350/factory-to-ddwrt.img

cp mt7621/src/router/mipsel-uclibc/wac124-webflash.bin ~/GruppenLW/releases/$DATE/netgear-wac124/wac124-webflash.bin
cp mt7621/src/router/mipsel-uclibc/wac124.img ~/GruppenLW/releases/$DATE/netgear-wac124/factory-to-ddwrt.img

cd mt7621/src/router
cp .config_r6220 .config
echo "CONFIG_MDNS=y" >> .config
#echo "CONFIG_MDNS_UTILS=y" >> .config
#echo "CONFIG_LOCKDEBUG=y" >> .config
make -f Makefile.mt7621 kernel clean all install
cd ../../../

cp mt7621/src/router/mipsel-uclibc/r6220-webflash.bin ~/GruppenLW/releases/$DATE/netgear-r6220/r6220-webflash.bin
cp mt7621/src/router/mipsel-uclibc/r6220.img ~/GruppenLW/releases/$DATE/netgear-r6220/factory-to-ddwrt.img


#scp mt7621/src/router/mipsel-uclibc/dir878-a1-webflash.bin 10.88.193.93:/tmp/firmware.bin