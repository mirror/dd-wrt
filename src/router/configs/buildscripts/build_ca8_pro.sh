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
cp .config_ca8pro .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/ca8-4pro
cd ../../../
cp ar531x/src/router/mips-uclibc/zImage ~/GruppenLW/releases/$DATE/ca8-4pro/original-to-ddwrt.bin
cp ar531x/src/router/mips-uclibc/zImageclient ~/GruppenLW/releases/$DATE/ca8-4pro/original-to-ddwrt\(client\).bin
cp ar531x/src/router/mips-uclibc/ca8pro-firmware.bin ~/GruppenLW/releases/$DATE//ca8-4pro/ca8-4pro-firmware.bin

