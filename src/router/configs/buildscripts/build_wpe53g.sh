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

cp .config_wpe53g .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/compex_wpe53g
cd ../../../
cp ar531x/src/router/mips-uclibc/compex-firmware-wpe53g.bin ~/GruppenLW/releases/$DATE/compex_wpe53g/compex-firmware-wpe53g.tftp
cp ar531x/src/router/mips-uclibc/np25g-firmware.bin ~/GruppenLW/releases/$DATE/compex_wpe53g/compex-wpe53g-firmware.bin

