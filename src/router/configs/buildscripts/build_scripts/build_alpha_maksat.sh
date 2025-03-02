#!/bin/sh

if [ x$1 = xBLANK ]
then
	BEXTRA=$2
	BLANK=1
else
	BEXTRA=$1
fi

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
cp .config_alpha-maksat${BEXTRA} .config
if [ x$BLANK = x1 ]
then
 echo "CONFIG_MAKSAT_BLANK=y" >> .config
 echo "CONFIG_ROUTERSTYLE=y" >>.config
 echo "CONFIG_ORANGE=y" >>.config
 echo "CONFIG_RED=y" >>.config
 #echo "CONFIG_BLUE=y" >>.config
 #echo "CONFIG_YELLOW=y" >>.config
 #echo "CONFIG_CYAN=y" >>.config
 #echo "CONFIG_RED=y" >>.config
 #echo "CONFIG_GREEN=y" >>.config

 BEXTRA=${2}_blank
fi
mkdir -p ~/GruppenLW/releases/$DATE/alfa-maksat${BEXTRA}
make -f Makefile.ar531x build_date
make -f Makefile.ar531x kernel clean all install
cd ../../../
cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/alfa-maksat${BEXTRA}
cp ar531x/src/router/mips-uclibc/vmlinux.bin.l7 ~/GruppenLW/releases/$DATE/alfa-maksat${BEXTRA}/
cp ar531x/src/router/mips-uclibc/alpha-firmware.bin ~/GruppenLW/releases/$DATE/alfa-maksat${BEXTRA}/alfa-firmware.bin

