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
cp .config_ca8 .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/wla-5000ap
cd ../../../
cp ar531x/src/router/mips-uclibc/zImage ~/GruppenLW/releases/$DATE/wla-5000ap/original-to-ddwrt.bin
cp ar531x/src/router/mips-uclibc/ca8-firmware.bin ~/GruppenLW/releases/$DATE/wla-5000ap/wla-5000ap-firmware.bin

cd ar531x/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../

cp .config_ca8 .config
echo "CONFIG_WHA5500CPE=y" >> .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/wha-5500cpe
cd ../../../
cp ar531x/src/router/mips-uclibc/zImage ~/GruppenLW/releases/$DATE/wha-5500cpe/original-to-ddwrt.bin
cp ar531x/src/router/mips-uclibc/ca8-firmware.bin ~/GruppenLW/releases/$DATE/wha-5500cpe/wha-5500cpe-firmware.bin


cd ar531x/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../

#cp .config_airmax5 .config
cp .config_ca8 .config
echo "CONFIG_AIRMAX5=y" >> .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/airmax5
cd ../../../
cp ar531x/src/router/mips-uclibc/zImage ~/GruppenLW/releases/$DATE/airmax5/original-to-ddwrt.bin
cp ar531x/src/router/mips-uclibc/ca8-firmware.bin ~/GruppenLW/releases/$DATE/airmax5/airmax5-firmware.bin

