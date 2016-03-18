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

cat .config_mr3202a_maksat | awk '{gsub(/CONFIG_SUPERCHANNEL=y/, "CONFIG_SUPERCHANNEL=n");print}' | awk '{gsub(/CONFIG_BOESE=y/, "CONFIG_BOESE=n");print}' > .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/mr3202a_maksat-retail
cd ../../../
cp ar531x/src/router/mips-uclibc/vmlinux.mr3202a ~/GruppenLW/releases/$DATE/mr3202a_maksat-retail/linux.bin
cp ar531x/src/router/mips-uclibc/mr3202a-firmware.bin ~/GruppenLW/releases/$DATE/mr3202a_maksat-retail/mr3202a-firmware.bin




