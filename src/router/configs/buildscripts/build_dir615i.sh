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
cp .config_dir615i .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir615i1
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir615i2
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir615i3
cd ../../../
cp pb42/src/router/mips-uclibc/dir615i1-uimage.bin ~/GruppenLW/releases/$DATE/dlink-dir615i1/dir615i1-factory-to-ddwrt-firmware.bin
cp pb42/src/router/mips-uclibc/ap93-firmware.bin ~/GruppenLW/releases/$DATE/dlink-dir615i1/dir615i1-firmware.bin

cp pb42/src/router/mips-uclibc/dir615i3-uimage.bin ~/GruppenLW/releases/$DATE/dlink-dir615i2/dir615i2-factory-to-ddwrt-firmware.bin
cp pb42/src/router/mips-uclibc/ap93-firmware.bin ~/GruppenLW/releases/$DATE/dlink-dir615i2/dir615i2-firmware.bin

cp pb42/src/router/mips-uclibc/dir615i3-uimage.bin ~/GruppenLW/releases/$DATE/dlink-dir615i3/dir615i3-factory-to-ddwrt-firmware.bin
cp pb42/src/router/mips-uclibc/ap93-firmware.bin ~/GruppenLW/releases/$DATE/dlink-dir615i3/dir615i3-firmware.bin
