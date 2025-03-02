#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-12.2.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_rambutan .config
echo CONFIG_AP135=y >> .config
echo CONFIG_AP143=y >> .config
echo "CONFIG_CAKE=y" >> .config
echo CONFIG_MAC80211_MESH=y >> .config
echo CONFIG_WPA3=y >> .config
echo CONFIG_SAMBA4=y >> .config
echo "CONFIG_SMBD=y" >> .config
echo "CONFIG_MDNS=y" >> .config
echo "CONFIG_MDNS_UTILS=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/8devices-rambutan
cd ../../../

cp pb42/src/router/mips-uclibc/root.squashfs.ubi ~/GruppenLW/releases/$DATE/8devices-rambutan/factory-to-ddwrt.bin
cp pb42/src/router/mips-uclibc/webflash-rambutan.trx ~/GruppenLW/releases/$DATE/8devices-rambutan/8devices-rambutan-webflash.bin
