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
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wa701ndv2
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr703nv1
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-mr3020

cd pb42/src/router
cp .config_wr703 .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/WR703Nv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr703nv1/tl-wr703v1-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR703Nv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr703nv1/factory-to-ddwrt.bin

cd pb42/src/router
cp .config_wr703 .config

echo "CONFIG_MR3020=y" >>.config 
make -f Makefile.pb42 services-clean services kernel libutils-clean libutils install
cd ../../../
cp pb42/src/router/mips-uclibc/MR3020-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-mr3020/tl-MR3020-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-MR3020-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-mr3020/factory-to-ddwrt.bin




cd pb42/src/router
cp .config_wa701v2 .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/WA701NDv2-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wa701ndv2/tl-wa701ndv2-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WA701NDv2-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wa701ndv2/factory-to-ddwrt.bin


