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

cp .config_dir400 .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir400
cd ../../../
cp ar531x/src/router/mips-uclibc/dir400-firmware.bin ~/GruppenLW/releases/$DATE/dlink-dir400
cp ar531x/src/router/mips-uclibc/vmlinux.dir400 ~/GruppenLW/releases/$DATE/dlink-dir400/linux.bin

