#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv8
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wa901ndv3
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wa850rev1
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wa860rev1
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_wr841v8 .config
make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/WR841NDv8-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv8/tl-wr841nd-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR841NDv8-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv8/factory-to-ddwrt.bin

cd pb42/src/router
cp .config_wr841v8 .config
echo CONFIG_WA901V3=y >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/WA901NDv3-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wa901ndv3/tl-wa901ndv3-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WA901NDv3-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wa901ndv3/factory-to-ddwrt.bin

cd pb42/src/router
cp .config_wr841v8 .config
echo CONFIG_WA901V3=y >> .config
echo CONFIG_WA850RE=y >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/WA850REv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wa850rev1/tl-wa850rev1-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WA850REv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wa850rev1/factory-to-ddwrt.bin

cd pb42/src/router
cp .config_wr841v8 .config
echo CONFIG_WA901V3=y >> .config
echo CONFIG_WA860RE=y >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/WA860REv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wa860rev1/tl-wa860rev1-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WA860REv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wa860rev1/factory-to-ddwrt.bin
