#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../

mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr1043nd-v4
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr1043n-v5

cp .config_archerc7v2 .config

echo "CONFIG_ARCHERC7V4=y" >> .config
echo "CONFIG_WR1043V4=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_CAKE=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
echo "CONFIG_MDNS=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/WR1043v4-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr1043nd-v4/tplink-wr1043nd-v4.bin
cp pb42/src/router/mips-uclibc/tplink-WR1043v4-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr1043nd-v4/factory-to-ddwrt.bin


cd pb42/src/router
cp .config_wr1043v5 .config


echo "CONFIG_ARCHERC7V4=y" >> .config
echo "CONFIG_WR1043V4=y" >> .config
echo "CONFIG_WR1043V5=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_CAKE=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
echo "CONFIG_MDNS=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/WR1043v5-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr1043n-v5/tplink-wr1043n-v5.bin
cp pb42/src/router/mips-uclibc/tplink-WR1043v5-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr1043n-v5/factory-to-ddwrt.bin


