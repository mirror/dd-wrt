#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531x/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin:$OLDPATH
cd ar531x/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_ms2 .config
make -f Makefile.ar531x build_date
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-ms2
cd ../../../
#cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/ubnt-ls2
#cp ar531x/src/router/mips-uclibc/vmlinux.bin.l7 ~/GruppenLW/releases/$DATE/ubnt-ls2
cp ar531x/src/router/mips-uclibc/ls2-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-ms2/ubnt-ms2-firmware.bin
cp ar531x/src/router/mips-uclibc/MS2.dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-ms2


