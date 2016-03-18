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

cp .config_mr3202a-4mb_maksat .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/mr3202a_maksat
cd ../../../
cp ar531x/src/router/mips-uclibc/vmlinux.mr3202a ~/GruppenLW/releases/$DATE/mr3202a_maksat/linux-4mb.bin
cp ar531x/src/router/mips-uclibc/mr3202a-firmware.bin ~/GruppenLW/releases/$DATE/mr3202a_maksat/mr3202a-firmware-4mb.bin




