#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531x/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_mips32_gcc-5.2.0_musl-1.1.12/bin:$OLDPATH
cd ar531x/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_lc2 .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_loco2
cd ../../../
#cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/ubnt_ls5
#cp ar531x/src/router/mips-uclibc/vmlinux.bin.l7 ~/GruppenLW/releases/$DATE/ubnt_ls5/kernel
cp ar531x/src/router/mips-uclibc/ls2-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_loco2/ubnt_loco2-firmware.bin
cp ar531x/src/router/mips-uclibc/LC2.dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_loco2/LC2.dd-wrt.bin


