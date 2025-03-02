#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-10.0.1_musl/bin:$OLDPATH

cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
if [ x$1 = xBLANK ]
then
	BEXTRA=$2
	BLANK=1
else
	BEXTRA=$1
fi
cp .config_ixpmaksat${BEXTRA} .config
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
#make -f Makefile.armbe clean
#make -f Makefile.armbe configure
#exit
make -f Makefile.armbe kernel 
cd wavesat
./build_avila.sh
cd ../
make -f Makefile.armbe clean all install
mkdir -p ~/GruppenLW/releases/$DATE/gateworks-maksat${BEXTRA}
cd ../../../
#cp xscale/src/router/armeb-uclibc/gateworks-firmware-jffs.bin ~/GruppenLW/releases/$DATE/gateworks-maksat${BEXTRA}/maksat-gateworks-firmware-jffs2.bin
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks-maksat${BEXTRA}/maksat-gateworks-firmware.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/gateworks-maksat${BEXTRA}/maksat-linux.bin
