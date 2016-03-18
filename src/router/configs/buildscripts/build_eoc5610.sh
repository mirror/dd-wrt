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

cp .config_eoc5610 .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/senao-eoc5610
mkdir -p ~/GruppenLW/releases/$DATE/senao-eoc5510
mkdir -p ~/GruppenLW/releases/$DATE/senao-eoc5611
cd ../../../
cp ar531x/src/router/mips-uclibc/vmlinux.eoc5610 ~/GruppenLW/releases/$DATE/senao-eoc5610/linux.bin
cp ar531x/src/router/mips-uclibc/eoc5610-firmware.bin ~/GruppenLW/releases/$DATE/senao-eoc5610/eoc5610-firmware.bin

cd ar531x/src/router
cp .config_eoc5610 .config
echo "CONFIG_EOC5611=y" >> .config
make -f Makefile.ar531x libutils-clean libutils install
cd ../../../
cp ar531x/src/router/mips-uclibc/vmlinux.eoc5610 ~/GruppenLW/releases/$DATE/senao-eoc5611/linux.bin
cp ar531x/src/router/mips-uclibc/eoc5610-firmware.bin ~/GruppenLW/releases/$DATE/senao-eoc5611/eoc5611-firmware.bin


cd ar531x/src/router
cp .config_eoc5610 .config
echo "CONFIG_EOC5510=y" >> .config
make -f Makefile.ar531x libutils-clean libutils install
cd ../../../
cp ar531x/src/router/mips-uclibc/vmlinux.eoc5610 ~/GruppenLW/releases/$DATE/senao-eoc5510/linux.bin
cp ar531x/src/router/mips-uclibc/eoc5610-firmware.bin ~/GruppenLW/releases/$DATE/senao-eoc5510/eoc5510-firmware.bin



