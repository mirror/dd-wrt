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
cp .config_archerc7v2 .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_CAKE=y" >> .config
#echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_WPA3=y" >> .config
#echo "CONFIG_TCPDUMP=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
echo "CONFIG_MDNS=y" >> .config
echo "CONFIG_SPEEDTEST_CLI=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/tplink-archer-c7-v2
mkdir -p ~/GruppenLW/releases/$DATE/tplink-archer-c7-v3
mkdir -p ~/GruppenLW/releases/$DATE/tplink-archer-c5-v1
cd ../../../
cp pb42/src/router/mips-uclibc/ARCHER-C7v2-firmware.bin ~/GruppenLW/releases/$DATE/tplink-archer-c7-v2/tplink-archer-c7-v2.bin
cp pb42/src/router/mips-uclibc/tplink-ARCHER-C7v2-firmware.bin ~/GruppenLW/releases/$DATE/tplink-archer-c7-v2/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/ARCHER-C7v2-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink-archer-c7-v2/tplink-archer-c7-v2-US.bin
cp pb42/src/router/mips-uclibc/tplink-ARCHER-C7v2-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink-archer-c7-v2/factory-to-ddwrt-US.bin

cp pb42/src/router/mips-uclibc/ARCHER-C7v2-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink-archer-c7-v3/tplink-archer-c7-v3.bin
cp pb42/src/router/mips-uclibc/tplink-ARCHER-C7v2-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink-archer-c7-v3/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/ARCHER-C7v2-firmwareIL.bin ~/GruppenLW/releases/$DATE/tplink-archer-c7-v2/tplink-archer-c7-v2-IL.bin
cp pb42/src/router/mips-uclibc/tplink-ARCHER-C7v2-firmwareIL.bin ~/GruppenLW/releases/$DATE/tplink-archer-c7-v2/factory-to-ddwrt-IL.bin
cd pb42/src/router
echo "CONFIG_ARCHERC5=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
#echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_CAKE=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
echo "CONFIG_MDNS=y" >> .config
echo "CONFIG_SPEEDTEST_CLI=y" >> .config
#echo "CONFIG_TCPDUMP=y" >> .config
make -f Makefile.pb42 libutils-clean libutils install
cd ../../../
cp pb42/src/router/mips-uclibc/ARCHER-C5v1-firmware.bin ~/GruppenLW/releases/$DATE/tplink-archer-c5-v1/tplink-archer-c5-v1.bin
cp pb42/src/router/mips-uclibc/tplink-ARCHER-C5v1-firmware.bin ~/GruppenLW/releases/$DATE/tplink-archer-c5-v1/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/ARCHER-C5v1-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink-archer-c5-v1/tplink-archer-c5-v1-US.bin
cp pb42/src/router/mips-uclibc/tplink-ARCHER-C5v1-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink-archer-c5-v1/factory-to-ddwrt-US.bin

cp pb42/src/router/mips-uclibc/ARCHER-C5v1-firmwareIL.bin ~/GruppenLW/releases/$DATE/tplink-archer-c5-v1/tplink-archer-c5-v1-IL.bin
cp pb42/src/router/mips-uclibc/tplink-ARCHER-C5v1-firmwareIL.bin ~/GruppenLW/releases/$DATE/tplink-archer-c5-v1/factory-to-ddwrt-IL.bin


cd pb42/src/router
cp .config_archerc7v2 .config

echo "CONFIG_ARCHERC7V4=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
#echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_CAKE=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_TCPDUMP=y" >> .config
echo "CONFIG_SPEEDTEST_CLI=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/tplink-archer-c7-v4
cd ../../../
cp pb42/src/router/mips-uclibc/ARCHER-C7v4-firmware.bin ~/GruppenLW/releases/$DATE/tplink-archer-c7-v4/tplink-archer-c7-v4.bin
cp pb42/src/router/mips-uclibc/tplink-ARCHER-C7-v4-firmware.bin ~/GruppenLW/releases/$DATE/tplink-archer-c7-v4/factory-to-ddwrt.bin

cd pb42/src/router
cp .config_archerc7v2 .config

echo "CONFIG_ARCHERC7V4=y" >> .config
echo "CONFIG_ARCHERC7V5=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_CAKE=y" >> .config
#echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
echo "CONFIG_MDNS=y" >> .config
echo "CONFIG_SPEEDTEST_CLI=y" >> .config
#echo "CONFIG_TCPDUMP=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/tplink-archer-c7-v5
cd ../../../
cp pb42/src/router/mips-uclibc/ARCHER-C7v5-firmware.bin ~/GruppenLW/releases/$DATE/tplink-archer-c7-v5/tplink-archer-c7-v5.bin
cp pb42/src/router/mips-uclibc/tplink-ARCHER-C7-v5-firmware.bin ~/GruppenLW/releases/$DATE/tplink-archer-c7-v5/factory-to-ddwrt.bin



cd pb42/src/router
cp .config_archerc7v2 .config

echo "CONFIG_ARCHERC7V4=y" >> .config
echo "CONFIG_ARCHERC7V5=y" >> .config
echo "CONFIG_ARCHERA7V5=y" >> .config
echo "CONFIG_CAKE=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
#echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_MDNS=y" >> .config
echo "CONFIG_SPEEDTEST_CLI=y" >> .config
#echo "CONFIG_TCPDUMP=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/tplink-archer-a7-v5
cd ../../../
cp pb42/src/router/mips-uclibc/ARCHER-A7v5-firmware.bin ~/GruppenLW/releases/$DATE/tplink-archer-a7-v5/tplink-archer-a7-v5.bin
cp pb42/src/router/mips-uclibc/tplink-ARCHER-A7-v5-firmware.bin ~/GruppenLW/releases/$DATE/tplink-archer-a7-v5/factory-to-ddwrt.bin
