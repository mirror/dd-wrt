#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531x/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_mips32_gcc-5.2.0_musl-1.1.12/bin:$OLDPATH
cd ar531x/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_ls5 .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_ls5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_ws5
cd ../../../
#cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/ubnt_ls5
#cp ar531x/src/router/mips-uclibc/vmlinux.bin.l7 ~/GruppenLW/releases/$DATE/ubnt_ls5/kernel
cp ar531x/src/router/mips-uclibc/ls5-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_ls5/ubnt_ls5-firmware.bin
cp ar531x/src/router/mips-uclibc/LS5.dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_ls5
#cp ar531x/src/router/mips-uclibc/ls5-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_ms5/ubnt_ms5-firmware.bin
#cp ar531x/src/router/mips-uclibc/MS5.dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_ms5
cp ar531x/src/router/mips-uclibc/ls5-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_ws5/ubnt_ws5-firmware.bin
cp ar531x/src/router/mips-uclibc/WS5.dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_ws5


