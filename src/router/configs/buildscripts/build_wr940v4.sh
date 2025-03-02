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
cp .config_wr841v8 .config
echo CONFIG_AP135=y >> .config
echo CONFIG_AP143=y >> .config
echo CONFIG_WR841V9=y >> .config
#make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv9
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv10
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv11
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv12
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr941ndv6
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr940ndv3
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr940ndv4
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr940ndv6
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr940ndv5
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wa901ndv4
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wa901ndv5
cd ../../../

cp pb42/src/router/mips-uclibc/WR841NDv9-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv9/tl-wr841nd-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR841NDv9-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv9/factory-to-ddwrt.bin

cd pb42/src/router
echo CONFIG_WR841V10=y >> .config
#make -f Makefile.pb42 libutils-clean libutils install
cd ../../../

cp pb42/src/router/mips-uclibc/WR841NDv10-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv10/tl-wr841nd-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR841NDv10-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv10/factory-to-ddwrt.bin


cd pb42/src/router
echo CONFIG_WR841V11=y >> .config
#make -f Makefile.pb42 libutils-clean libutils install
cd ../../../

cp pb42/src/router/mips-uclibc/WR841NDv11-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv11/tl-wr841nd-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR841NDv11-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv11/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/WR841NDv11-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv11/tl-wr841nd-webflash-US.bin
cp pb42/src/router/mips-uclibc/tplink-WR841NDv11-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv11/factory-to-ddwrt-US.bin

cd pb42/src/router
echo CONFIG_WR841V12=y >> .config
#make -f Makefile.pb42 libutils-clean libutils install
cd ../../../

cp pb42/src/router/mips-uclibc/WR841NDv12-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv12/tl-wr841nd-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR841NDv12-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv12/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/WR841NDv12-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv12/tl-wr841nd-webflash-US.bin
cp pb42/src/router/mips-uclibc/tplink-WR841NDv12-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv12/factory-to-ddwrt-US.bin



cd pb42/src/router
echo CONFIG_WR941V6=y >> .config
#make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/WR941NDv6-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr941ndv6/tl-wr941ndv6-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR941NDv6-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr941ndv6/factory-to-ddwrt.bin

#cp pb42/src/router/mips-uclibc/WR941NDv6-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr941ndv6/tl-wr941ndv6-webflash-US.bin
#cp pb42/src/router/mips-uclibc/tplink-WR941NDv6-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr941ndv6/factory-to-ddwrt-US.bin



cp pb42/src/router/mips-uclibc/WR941NDv6-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr940ndv3/tl-wr940ndv3-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR941NDv6-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr940ndv3/factory-to-ddwrt.bin

#cp pb42/src/router/mips-uclibc/WR941NDv6-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr940ndv3/tl-wr940ndv3-webflash-US.bin
#cp pb42/src/router/mips-uclibc/tplink-WR941NDv6-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr940ndv3/factory-to-ddwrt-US.bin

cd pb42/src/router
echo CONFIG_WR940V4=y >> .config
#make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/WR940NDv4-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr940ndv4/tl-wr940ndv4-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR940NDv4-firmware-eu.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr940ndv4/factory-to-ddwrt-eu.bin
cp pb42/src/router/mips-uclibc/tplink-WR940NDv4-firmware-us.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr940ndv4/factory-to-ddwrt-us.bin

cp pb42/src/router/mips-uclibc/WR940NDv4-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr940ndv5/tl-wr940ndv5-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR940NDv4-firmware-eu.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr940ndv5/factory-to-ddwrt-eu.bin
cp pb42/src/router/mips-uclibc/tplink-WR940NDv4-firmware-us.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr940ndv5/factory-to-ddwrt-us.bin

cd pb42/src/router
echo CONFIG_WR940V4=y >> .config
echo CONFIG_WR940V6=y >> .config
#make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/WR940NDv6-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr940ndv6/tl-wr940ndv6-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR940NDv6-firmware-eu.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr940ndv6/factory-to-ddwrt-eu.bin
cp pb42/src/router/mips-uclibc/tplink-WR940NDv6-firmware-us.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr940ndv6/factory-to-ddwrt-us.bin




cd pb42/src/router
echo CONFIG_WA901V5=y >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/WA901NDv5-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wa901ndv5/tl-wa901ndv5-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WA901NDv5-firmware-eu.bin ~/GruppenLW/releases/$DATE/tplink-tl-wa901ndv5/factory-to-ddwrt.bin

cd pb42/src/router
echo CONFIG_WA901V4=y >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/WA901NDv4-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wa901ndv4/tl-wa901ndv4-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WA901NDv4-firmware-eu.bin ~/GruppenLW/releases/$DATE/tplink-tl-wa901ndv4/factory-to-ddwrt.bin




