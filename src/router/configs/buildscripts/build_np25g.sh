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

cp .config_np25g .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/compex_np25g
cd ../../../
cp ar531x/src/router/mips-uclibc/compex-firmware-np25g.bin ~/GruppenLW/releases/$DATE/compex_np25g/compex-firmware-np25g.tftp
cp ar531x/src/router/mips-uclibc/np25g-firmware.bin ~/GruppenLW/releases/$DATE/compex_np25g/compex-np25g-firmware.bin

