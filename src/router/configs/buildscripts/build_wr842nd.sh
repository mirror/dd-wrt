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
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr842ndv1
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr842ndv2
cd pb42/src/router
cp .config_wr842 .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/WR842NDv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr842ndv1/tl-wr842ndv1-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR842NDv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr842ndv1/factory-to-ddwrt.bin

cd pb42/src/router
cp .config_wr842v2 .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/WR842NDv2-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr842ndv2/tl-wr842ndv2-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR842NDv2-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr842ndv2/factory-to-ddwrt.bin

