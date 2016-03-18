#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n adm5120/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mipsel_gcc4.1.2/bin:$OLDPATH
cd adm5120/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_np28g .config
make -f Makefile.adm5120 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/compex-NP28G
mkdir -p ~/GruppenLW/releases/$DATE/compex-NP27G
cd ../../../
cp adm5120/src/router/mipsel-uclibc/compex-firmware-np28g.bin ~/GruppenLW/releases/$DATE/compex-NP28G/tftp-image-np28g.bin
cp adm5120/src/router/mipsel-uclibc/adm5120-webflash.bin ~/GruppenLW/releases/$DATE/compex-NP28G/NP28G-webflash.bin

cp adm5120/src/router/mipsel-uclibc/compex-firmware-np27g.bin ~/GruppenLW/releases/$DATE/compex-NP27G/tftp-image-np27g.bin
cp adm5120/src/router/mipsel-uclibc/adm5120-webflash.bin ~/GruppenLW/releases/$DATE/compex-NP27G/NP27G-webflash.bin

