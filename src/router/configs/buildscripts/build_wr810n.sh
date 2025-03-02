#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr810nv1
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr810nv2
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_wr810n .config
make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/WR810Nv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr810nv1/tl-wr810nv1-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR810Nv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr810nv1/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/WR810Nv2-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr810nv2/tl-wr810nv2-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR810Nv2-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr810nv2/factory-to-ddwrt.bin
