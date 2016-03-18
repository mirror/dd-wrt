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
mkdir -p ~/GruppenLW/releases/$DATE/8devices_carambola2
mkdir -p ~/GruppenLW/releases/$DATE/8devices_carambola2_8mb
cd pb42/src/router
cp .config_carambola .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/8devices_carambola2/factory-flash-uimage.bin
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/$DATE/8devices_carambola2/dd-wrt-webflash.bin

cd pb42/src/router
cp .config_carambola_8m .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/8devices_carambola2_8mb/factory-flash-uimage.bin
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/$DATE/8devices_carambola2_8mb/dd-wrt-webflash.bin



