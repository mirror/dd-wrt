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

cp .config_wgt624 .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/netgear_wgt624v4
cd ../../../
cp ar531x/src/router/mips-uclibc/vmlinux.mr3202a ~/GruppenLW/releases/$DATE/netgear_wgt624v4/linux.bin
cp ar531x/src/router/mips-uclibc/mr3202a-firmware.bin ~/GruppenLW/releases/$DATE/netgear_wgt624v4/wgt624v4-firmware.bin

