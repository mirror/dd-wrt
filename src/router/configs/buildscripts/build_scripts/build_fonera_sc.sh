#!/bin/sh

OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531x/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_4kec_gcc-11.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-12.2.0_musl/bin:$OLDPATH
cd ar531xr2/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../

cp .config_fonera_superchannel .config
make -f Makefile.ar531x build_date
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/fonera
cd ../../../
cp ar531xr2/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/fonera/root.fs-superchannel
cp ar531xr2/src/router/mips-uclibc/vmlinux.bin.l7 ~/GruppenLW/releases/$DATE/fonera/
cp ar531xr2/src/router/mips-uclibc/fonera-firmware.bin ~/GruppenLW/releases/$DATE/fonera/fonera-firmware-superchannel.bin


