#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-5.2.0_musl-1.1.11/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_xscale_16M .config
echo "KERNELVERSION=4.4" >> .config
make -f Makefile.armbe kernel clean all install
#mkdir -p ~/GruppenLW/releases/$DATE/gateworks_16M
#mkdir -p ~/GruppenLW/releases/$DATE/pronghorn-SBC
#mkdir -p ~/GruppenLW/releases/$DATE/compex_wp188
cd ../../../
#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks_16M
#cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/gateworks_16M/linux.bin
#cp xscale/src/router/armeb-uclibc/firmware.compex ~/GruppenLW/releases/$DATE/compex_wp188/firmware.tftp
#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/compex_wp188/compex-firmware-webflash.bin

#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/pronghorn-SBC/pronghorn-firmware.bin
#cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/pronghorn-SBC/linux.bin


