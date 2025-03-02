#!/bin/sh

OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531x/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin:$OLDPATH
cd ar531x/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
mkdir -p ~/GruppenLW/releases/$DATE/alfa-b1
cp .config_alfa_b1 .config
make -f Makefile.ar531x build_date
make -f Makefile.ar531x kernel clean all install
cd ../../../
cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/alfa-b1
cp ar531x/src/router/mips-uclibc/vmlinux.bin.l7 ~/GruppenLW/releases/$DATE/alfa-b1/
cp ar531x/src/router/mips-uclibc/alpha-firmware.bin ~/GruppenLW/releases/$DATE/alfa-b1/alfa-firmware.bin

cd ar531x/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
mkdir -p ~/GruppenLW/releases/$DATE/alfa-b2
cp .config_alfa_b2 .config
make -f Makefile.ar531x build_date
make -f Makefile.ar531x kernel clean all install
cd ../../../
cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/alfa-b2
cp ar531x/src/router/mips-uclibc/vmlinux.bin.l7 ~/GruppenLW/releases/$DATE/alfa-b2/
cp ar531x/src/router/mips-uclibc/alpha-firmware.bin ~/GruppenLW/releases/$DATE/alfa-b2/alfa-firmware.bin

