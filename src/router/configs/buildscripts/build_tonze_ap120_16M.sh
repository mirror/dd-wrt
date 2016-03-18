#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n adm5120/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mipsel_gcc4.1.2/bin:$OLDPATH
cd adm5120/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_ap120_16m .config
make -f Makefile.adm5120 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/Tonze-AP120
cd ../../../
cp adm5120/src/router/mipsel-uclibc/vmlinus.gz ~/GruppenLW/releases/$DATE/Tonze-AP120/image.bin
cp adm5120/src/router/mipsel-uclibc/adm5120-webflash.bin ~/GruppenLW/releases/$DATE/Tonze-AP120/AP120-webflash.bin

