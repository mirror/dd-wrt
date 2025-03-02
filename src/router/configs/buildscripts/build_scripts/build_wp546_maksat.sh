#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-12.2.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
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
cp .config_wp546 .config
echo "CONFIG_SAMBA3=y" >>.config
echo "CONFIG_MAKSAT=y" >>.config
echo "CONFIG_MAKSAT_CORAL=y" >>.config
echo "CONFIG_BRANDING=y" >>.config
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
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/maksat/compex_wp546${BEXTRA}
cd ../../../
cp pb42/src/router/mips-uclibc/lsx-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/maksat/compex_wp546${BEXTRA}/wp546-firmware.bin
cp pb42/src/router/mips-uclibc/wp546.img ~/GruppenLW/releases/CUSTOMER/$DATE/maksat/compex_wp546${BEXTRA}/wp546-image.tftp
