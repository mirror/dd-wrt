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
cp .config_dir825c1 .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir825-c1
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dhp1565-a1
cd ../../../
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/$DATE/dlink-dir825-c1/dir825c1-firmware.bin
cp pb42/src/router/mips-uclibc/dir825c1-uimage.bin ~/GruppenLW/releases/$DATE/dlink-dir825-c1/factory-to-ddwrt_NA.bin

cd pb42/src/router
echo "CONFIG_DHP1565=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/$DATE/dlink-dhp1565-a1/dhp1565-a1-firmware.bin
cp pb42/src/router/mips-uclibc/dir825c1-uimage.bin ~/GruppenLW/releases/$DATE/dlink-dhp1565-a1/factory-to-ddwrt_NA.bin

