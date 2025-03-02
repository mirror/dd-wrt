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
cp .config_dw02_412h .config
echo CONFIG_AP135=y >> .config
echo CONFIG_AP143=y >> .config
echo "CONFIG_CAKE=y" >> .config
echo CONFIG_MAC80211_MESH=y >> .config
echo CONFIG_WPA3=y >> .config
echo CONFIG_SAMBA4=y >> .config
echo "CONFIG_SMBD=y" >> .config
echo "CONFIG_MDNS=y" >> .config
echo "CONFIG_MDNS_UTILS=y" >> .config
echo "CONFIG_REGISTER=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/Dongwong-dw02_412h
cd ../../../

#cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/dw02_412h/factory-to-ddwrt.bin
cp pb42/src/router/mips-uclibc/tftp/image.tftp ~/GruppenLW/releases/$DATE/Dongwong-dw02_412h/factory-to-ddwrt.tftp
#cp pb42/src/router/mips-uclibc/dw02_412h-firmware.trx ~/GruppenLW/releases/$DATE/dw02_412h/dw02_412h-webflash.bin
cd pb42/src/router

cp .config_dw02_412h_full .config
echo CONFIG_AP135=y >> .config
echo CONFIG_AP143=y >> .config
echo "CONFIG_CAKE=y" >> .config
echo CONFIG_MAC80211_MESH=y >> .config
echo CONFIG_WPA3=y >> .config
echo CONFIG_SAMBA4=y >> .config
echo "CONFIG_SMBD=y" >> .config
echo "CONFIG_MDNS=y" >> .config
echo "CONFIG_MDNS_UTILS=y" >> .config
echo "CONFIG_REGISTER=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/Dongwong-dw02_412h
cd ../../../

#cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/dw02_412h/factory-to-ddwrt.bin
cp pb42/src/router/mips-uclibc/dw02_412h-firmware.trx ~/GruppenLW/releases/$DATE/Dongwong-dw02_412h/dw02_412h-webflash.bin
#scp -P 993 pb42/src/router/mips-uclibc/aligned.uimage 10.20.30.105:/root/tftpd
