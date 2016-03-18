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



#special

cd ar531x/src/router
cp .config_dir300_special .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir300_special
cd ../../../
cp ar531x/src/router/mips-uclibc/vmlinux.dir300 ~/GruppenLW/releases/$DATE/dlink-dir300_special/linux.bin
cp ar531x/src/router/mips-uclibc/dir300-firmware.bin ~/GruppenLW/releases/$DATE/dlink-dir300_special/dir300-special-firmware.bin

