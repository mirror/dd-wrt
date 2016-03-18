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
cp .config_rcaa01_preflash .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/WLA-9000AP
cd ../../../
cp ar531x/src/router/mips-uclibc/zImage ~/GruppenLW/releases/$DATE/WLA-9000AP/original-to-ddwrt.bin

cd ar531x/src/router
cp .config_rcaa01 .config
make -f Makefile.ar531x kernel clean all install
cd ../../../
cp ar531x/src/router/mips-uclibc/rcaa01-firmware.bin ~/GruppenLW/releases/$DATE/WLA-9000AP/WLA9000AP-firmware.bin

cp notes/WLA9000AP/note.txt ~/GruppenLW/releases/$DATE/WLA-9000AP/note.txt
