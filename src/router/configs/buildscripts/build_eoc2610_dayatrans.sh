#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531x/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin:$OLDPATH
cd ar531x/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../

cp .config_eoc2610 .config
make -f Makefile.ar531x build_date
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/senao-eoc2610_dayatrans
mkdir -p ~/GruppenLW/releases/$DATE/senao-eoc2611_dayatrans
cd ../../../
#cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/fonera
cp ar531x/src/router/mips-uclibc/vmlinux.fonera ~/GruppenLW/releases/$DATE/senao-eoc2610_dayatrans/linux.bin
cp ar531x/src/router/mips-uclibc/fonera-firmware.bin ~/GruppenLW/releases/$DATE/senao-eoc2610_dayatrans/eoc2610-firmware.bin

cd ar531x/src/router
echo "CONFIG_EOC2611=y" >> .config
make -f Makefile.ar531x build_date
make -f Makefile.ar531x libutils-clean libutils install
cd ../../../
#cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/fonera
cp ar531x/src/router/mips-uclibc/vmlinux.fonera ~/GruppenLW/releases/$DATE/senao-eoc2611/linux.bin
cp ar531x/src/router/mips-uclibc/fonera-firmware.bin ~/GruppenLW/releases/$DATE/senao-eoc2611/eoc2611-firmware.bin
