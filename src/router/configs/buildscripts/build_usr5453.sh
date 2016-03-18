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
cp .config_usr5453 .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/usr5453
mkdir -p ~/GruppenLW/releases/$DATE/alfa
cd ../../../
cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/usr5453
cp ar531x/src/router/mips-uclibc/lzma_vmlinus ~/GruppenLW/releases/$DATE/usr5453
cp ar531x/src/router/mips-uclibc/usr5453-firmware.bin ~/GruppenLW/releases/$DATE/usr5453/usr5453-firmware.bin

cd ar531x/src/router
cp .config_alpha .config
make -f Makefile.ar531x shared-clean libutils-clean services-clean httpd-clean rc-clean libutils-clean libutils services rc httpd install
cd ../../../
cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/alfa
cp ar531x/src/router/mips-uclibc/vmlinux.bin.l7 ~/GruppenLW/releases/$DATE/alfa
cp ar531x/src/router/mips-uclibc/alpha-firmware.bin ~/GruppenLW/releases/$DATE/alfa/alfa-firmware.bin

