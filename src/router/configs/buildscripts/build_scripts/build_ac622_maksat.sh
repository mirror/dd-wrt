#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-mips_r2_gcc-linaro_uClibc-0.9.32/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-12.2.0_musl/bin:$OLDPATH
cd pb42/src/router
if [ x$1 = xBLANK ]
then
	BEXTRA=$2
	BLANK=1
else
	BEXTRA=$1
fi
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_ac622_maksat${BEXTRA} .config
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
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/ac622_maksat${BEXTRA}
cd ../../../
cp pb42/src/router/mips-uclibc/ap93-firmware.bin ~/GruppenLW/releases/$DATE/ac622_maksat${BEXTRA}/ac622-firmware.bin
cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/ac622_maksat${BEXTRA}/ubootflash.bin
