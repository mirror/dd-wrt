#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531x/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_mips32_gcc-5.2.0_musl-1.1.12/bin:$OLDPATH
cd ar531x/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_wrt54gv7 .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/wrt54gv7
cd ../../../
#cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/ubnt_ls2
#cp ar531x/src/router/mips-uclibc/vmlinux.bin.l7 ~/GruppenLW/releases/$DATE/ubnt_ls2

cp ar531x/src/router/mips-uclibc/wrk54g-firmware.bin ~/GruppenLW/releases/$DATE/wrt54gv7/wrt54gv7-firmware.bin
cp ar531x/src/router/mips-uclibc/vmlinux.wrk ~/GruppenLW/releases/$DATE/wrt54gv7/linux.bin

