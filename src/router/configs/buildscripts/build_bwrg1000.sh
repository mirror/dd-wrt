#!/bin/sh

OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531x/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_mips32_gcc-5.2.0_musl-1.1.12/bin:$OLDPATH
cd ar531x/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../

cp .config_bwrg1000 .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/bountiful-bwrg1000
cd ../../../
cp ar531x/src/router/mips-uclibc/bwrg1000-linux.bin ~/GruppenLW/releases/$DATE/bountiful-bwrg1000/linux.bin
cp ar531x/src/router/mips-uclibc/bwrg1000-firmware.bin ~/GruppenLW/releases/$DATE/bountiful-bwrg1000/bwrg1000-firmware.bin

cp notes/BWRG1000/* ~/GruppenLW/releases/$DATE/bountiful-bwrg1000
