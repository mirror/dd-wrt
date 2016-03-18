#!/bin/sh -x
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-5.2.0_musl-1.1.11/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-armeb_v5te_gcc-4.7-linaro_uClibc-0.9.33.2-7/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cat .config_xscale_cambria | grep -v "^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_ROUTERSTYLE\|^CONFIG_BLUE\|^CONFIG_YELLOW\|^CONFIG_CYAN\|^CONFIG_RED\|^CONFIG_GREEN" > .config
echo "CONFIG_HOBBIT=y" >>.config
#echo "CONFIG_NOTRIAL=y" >>.config
echo "CONFIG_BRANDING=y" >>.config
#echo "CONFIG_GPSD=y" >>.config
make -f Makefile.armbe kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/hobb-it/cambria
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/hobb-it/cambria/cambria-firmware-squashfs.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/CUSTOMER/$DATE/hobb-it/cambria/linux.bin

