#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_pb44 .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/atheros_pb44_rf1_phy
cd ../../../
cp pb42/src/router/mips-uclibc/rs-firmware.bin ~/GruppenLW/releases/$DATE/atheros_pb44_rf1_phy/pb44-firmware.bin
cp pb42/src/router/mips-uclibc/vmlinux.lsx ~/GruppenLW/releases/$DATE/atheros_pb44_rf1_phy/linux.bin


