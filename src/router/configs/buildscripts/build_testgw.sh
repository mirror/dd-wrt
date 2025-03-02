#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-13.1.0_musl/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_xscale .config
echo "KERNELVERSION=4.9" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_SPEEDCHECKER=y" >> .config
echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_WPA3=y" >> .config
#sed -i 's/CONFIG_QUAGGA=y/CONFIG_FRR=y/g' .config

echo "CONFIG_PRONGHORN=y" >> .config
make -f Makefile.armbe kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/gateworks_test
#mkdir -p ~/GruppenLW/releases/$DATE/pronghorn-SBC
#mkdir -p ~/GruppenLW/releases/$DATE/compex-wp188
cd ../../../
#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks_test
#cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/gateworks_test/linux.bin
#cp xscale/src/router/armeb-uclibc/firmware.compex ~/GruppenLW/releases/$DATE/compex-wp188/firmware.tftp
#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/compex-wp188/compex-firmware-webflash.bin

#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/pronghorn-SBC/pronghorn-firmware.bin
#cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/pronghorn-SBC/linux.bin


