#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_tew632brp .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/trendnet-tew632brp
mkdir -p ~/GruppenLW/releases/$DATE/trendnet-tew652brp
cd ../../../
cp pb42/src/router/mips-uclibc/tew632brp-uimage.bin ~/GruppenLW/releases/$DATE/trendnet-tew632brp/tew632brp-factory-to-ddwrt-firmware.bin
cp pb42/src/router/mips-uclibc/ap83-firmware.bin ~/GruppenLW/releases/$DATE/trendnet-tew632brp/tew632brp-firmware.bin

cd pb42/src/router
cp .config_tew632brp .config
echo "CONFIG_TEW652BRP=y" >> .config

make -f Makefile.pb42 libutils-clean libutils install
mkdir -p ~/GruppenLW/releases/$DATE/trendnet-tew632brp
cd ../../../
cp pb42/src/router/mips-uclibc/tew652brp-uimage.bin ~/GruppenLW/releases/$DATE/trendnet-tew652brp/tew652brp-factory-to-ddwrt-firmware.bin
cp pb42/src/router/mips-uclibc/ap83-firmware.bin ~/GruppenLW/releases/$DATE/trendnet-tew652brp/tew652brp-firmware.bin



cd pb42/src/router
cp .config_tew632brp .config
echo "CONFIG_DIR615C1=y" >> .config

make -f Makefile.pb42 libutils-clean libutils install
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir615c1
cd ../../../
cp pb42/src/router/mips-uclibc/dir615c1-uimage.bin ~/GruppenLW/releases/$DATE/dlink-dir615c1/dir615c1-factory-to-ddwrt-firmware.bin
cp pb42/src/router/mips-uclibc/ap83-firmware.bin ~/GruppenLW/releases/$DATE/dlink-dir615c1/dir615c1-firmware.bin

