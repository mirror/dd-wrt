#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-12.2.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_dir859 .config
echo "CONFIG_DIR859=y" >> .config
echo "CONFIG_ATH10K=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
#echo "CONFIG_SMBD=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir859
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir869
cd ../../../
cp pb42/src/router/mips-uclibc/webflash-dir859.trx ~/GruppenLW/releases/$DATE/dlink-dir859/dir859-webflash.bin
cp pb42/src/router/mips-uclibc/web-dir859.img ~/GruppenLW/releases/$DATE/dlink-dir859/factory-to-ddwrt.img

cd pb42/src/router
cp .config_dir869 .config
echo "CONFIG_DIR869=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/webflash-dir869.trx ~/GruppenLW/releases/$DATE/dlink-dir869/dir869-webflash.bin
cp pb42/src/router/mips-uclibc/web-dir869.img ~/GruppenLW/releases/$DATE/dlink-dir869/factory-to-ddwrt.img
