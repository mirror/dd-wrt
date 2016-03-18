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
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wdr4300v1
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wdr4310v1
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wdr3600v1
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wdr3500v1
cp .config_wdr4300 .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/TL-WDR4300-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr4300v1/tl-wdr4300-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-TL-WDR4300-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr4300v1/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/TL-WDR4300_US-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr4300v1/tl-wdr4300_us-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-TL-WDR4300_US-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr4300v1/factory-to-ddwrt-us.bin


cp pb42/src/router/mips-uclibc/TL-WDR4310-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr4310v1/tl-wdr4310-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-TL-WDR4310-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr4310v1/factory-to-ddwrt.bin

cd pb42/src/router
echo "CONFIG_WDR3600=y" >> .config
make -f Makefile.pb42 libutils-clean libutils install
cd ../../../

cp pb42/src/router/mips-uclibc/TL-WDR3600-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr3600v1/tl-wdr3600-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-TL-WDR3600-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr3600v1/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/TL-WDR3600_US-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr3600v1/tl-wdr3600_us-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-TL-WDR3600_US-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr3600v1/factory-to-ddwrt-us.bin

cd pb42/src/router

cp .config_wdr4300 .config
echo "CONFIG_WDR3500=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/TL-WDR3500-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr3500v1/tl-wdr3500-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-TL-WDR3500-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr3500v1/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/TL-WDR3500_US-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr3500v1/tl-wdr3500_us-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-TL-WDR3500_US-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr3500v1/factory-to-ddwrt-us.bin

