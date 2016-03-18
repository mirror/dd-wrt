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

cp .config_mr3202a_openvpn .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/mr3202a
cd ../../../
cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/mr3202a/root.fs-openvpn
cp ar531x/src/router/mips-uclibc/vmlinux.bin.l7 ~/GruppenLW/releases/$DATE/mr3202a
cp ar531x/src/router/mips-uclibc/fonera-firmware.bin ~/GruppenLW/releases/$DATE/mr3202a/mr3202a-openvpn-firmware.bin


