#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_archerc25 .config
echo CONFIG_ATH10K=y >> .config
echo CONFIG_CPUTEMP=y >> .config
echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_CAKE=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
echo "CONFIG_IPERF=y" >> .config
echo "CONFIG_SPEEDTEST_CLI=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/tplink-archer-c25-v1
cd ../../../

#cp pb42/src/router/mips-uclibc/WR841NDv9-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv9/tl-wr841nd-webflash.bin
#cp pb42/src/router/mips-uclibc/tplink-WR841NDv9-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv9/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/ARCHER-C25-firmware.bin ~/GruppenLW/releases/$DATE/tplink-archer-c25-v1/tplink-archer-c25.bin
cp pb42/src/router/mips-uclibc/tplink-ARCHER-C25-firmware.bin ~/GruppenLW/releases/$DATE/tplink-archer-c25-v1/factory-to-ddwrt.bin
#scp pb42/src/router/mips-uclibc/ARCHER-C25-firmware.bin 172.29.0.93:/tmp/firmware.bin
