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
cp .config_wr941 .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr941ndv2
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr941ndv3
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv3
mkdir -p ~/GruppenLW/releases/$DATE/tplink-tl-wa901ndv2
cd ../../../
cp pb42/src/router/mips-uclibc/WR941NDv2-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr941ndv2/tl-wr941nd-webflash.bin
cp pb42/src/router/mips-uclibc/WR941NDv2-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr941ndv3/tl-wr941nd-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR941NDv2-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr941ndv2/factory-to-ddwrt.bin
cp pb42/src/router/mips-uclibc/tplink-WR941NDv2-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr941ndv3/factory-to-ddwrt.bin
cd pb42/src/router

echo "CONFIG_WR841v3=y" >>.config 
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/WR841NDv3-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv3/tl-wr841nd-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR841NDv3-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wr841ndv3/factory-to-ddwrt.bin

cd pb42/src/router
cp .config_wr941 .config
echo "CONFIG_WA901=y" >>.config 
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/WA901NDv2-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wa901ndv2/tl-wa901ndv2-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WA901NDv2-firmware.bin ~/GruppenLW/releases/$DATE/tplink-tl-wa901ndv2/factory-to-ddwrt.bin

