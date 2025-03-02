#!/bin/sh

OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531xr2/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_4kec_gcc-11.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_4kec_gcc-10.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin:$OLDPATH
cd ar531xr2/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../

cp .config_fonera .config
#echo "CONFIG_KERNELLTO=y" >> .config
#echo "CONFIG_KERNELLTO_CP_CLONE=y" >> .config
#echo CONFIG_LTO=y >> .config
make -f Makefile.ar531x build_date
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/fonera
cd ../../../
#cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/fonera
cp ar531xr2/src/router/mips-uclibc/vmlinux.fonera ~/GruppenLW/releases/$DATE/fonera/linux.bin
cp ar531xr2/src/router/mips-uclibc/fonera-firmware.bin ~/GruppenLW/releases/$DATE/fonera


