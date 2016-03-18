#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-5.2.0_musl-1.1.11/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_xscale_tonze_wimax .config
make -f Makefile.armbe kernel 
cd wavesat
./build_avila.sh
cd ../
make -f Makefile.armbe clean all install
mkdir -p ~/GruppenLW/releases/$DATE/tonze-ap425
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/tonze-ap425/tonze-ap425-wimax-webflash.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/tonze-ap425/linux-wimax.bin

