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
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr710nv1
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr710nv2
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr710nv2.1

cd pb42/src/router
cp .config_wr710 .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/WR710Nv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr710nv1/tl-wr710v1-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR710Nv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr710nv1/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/WR710Nv1US-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr710nv1/tl-wr710v1-webflash-US.bin
cp pb42/src/router/mips-uclibc/tplink-WR710Nv1US-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr710nv1/factory-to-ddwrt-US.bin


cd pb42/src/router
cp .config_wr703 .config
echo "CONFIG_WR710=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/WR710Nv2-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr710nv2/tl-wr710v2-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR710Nv2-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr710nv2/factory-to-ddwrt.bin



cd pb42/src/router
cp .config_wr710 .config
echo "CONFIG_WR71021=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/WR710Nv2.1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr710nv2.1/tl-wr710v2.1-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR710Nv2.1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr710nv2.1/factory-to-ddwrt.bin

