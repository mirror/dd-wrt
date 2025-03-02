#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
if [ x$1 = xBLANK ]
then
	BEXTRA=$2
	BLANK=1
else
	BEXTRA=$1
fi
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_9331_maksat .config
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
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/maksat/9331${BEXTRA}
cd ../../../
cp pb42/src/router/mips-uclibc/ap93-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/maksat/9331${BEXTRA}/hornet-firmware.bin
cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/maksat/9331${BEXTRA}/hornet.uimage


