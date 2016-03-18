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
cp .config_wr841v8 .config
echo CONFIG_AP135=y >> .config
echo CONFIG_AP143=y >> .config
echo CONFIG_WR841V9=y >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv9
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv10
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv11
cd ../../../

cp pb42/src/router/mips-uclibc/WR841NDv9-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv9/tl-wr841nd-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR841NDv9-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv9/factory-to-ddwrt.bin

cd pb42/src/router
echo CONFIG_WR841V10=y >> .config
make -f Makefile.pb42 libutils-clean libutils install
cd ../../../

cp pb42/src/router/mips-uclibc/WR841NDv10-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv10/tl-wr841nd-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR841NDv10-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv10/factory-to-ddwrt.bin


cd pb42/src/router
echo CONFIG_WR841V11=y >> .config
make -f Makefile.pb42 libutils-clean libutils install
cd ../../../

cp pb42/src/router/mips-uclibc/WR841NDv11-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv11/tl-wr841nd-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR841NDv11-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv11/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/WR841NDv11-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv11/tl-wr841nd-webflash-US.bin
cp pb42/src/router/mips-uclibc/tplink-WR841NDv11-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv11/factory-to-ddwrt-US.bin
