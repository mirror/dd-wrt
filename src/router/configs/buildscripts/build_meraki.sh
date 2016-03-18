#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531x/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_mips32_gcc-5.2.0_musl-1.1.12/bin:$OLDPATH
cd ar531x/src/router
svn cleanup
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
svn cleanup
[ -n "$DO_UPDATE" ] && svn update
cd ../../../

cp .config_meraki .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/meraki
cd ../../../
cp ar531x/src/router/mips-uclibc/lzma_vmlinus.meraki ~/GruppenLW/releases/$DATE/meraki/linux.bin
cp ar531x/src/router/mips-uclibc/meraki-firmware.bin ~/GruppenLW/releases/$DATE/meraki



