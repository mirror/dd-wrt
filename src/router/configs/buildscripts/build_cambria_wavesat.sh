#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-5.2.0_musl-1.1.11/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_xscale_cambria_wimax .config
make -f Makefile.armbe kernel 
cd wavesat
./build_cambria.sh
cd ../
make -f Makefile.armbe clean all install
mkdir -p ~/GruppenLW/releases/$DATE/cambria_wavesat
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/cambria_wavesat/cambria-firmware-squashfs.bin
cp xscale/src/router/armeb-uclibc/gateworks-firmware-jffs.bin ~/GruppenLW/releases/$DATE/cambria_wavesat/cambria-firmware-jffs2.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/cambria_wavesat/linux.bin

