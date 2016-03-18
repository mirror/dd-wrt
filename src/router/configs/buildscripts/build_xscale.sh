#!/bin/sh
#./build_cambria_kmt.sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-5.2.0_musl-1.1.11/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_xscale .config
make -f Makefile.armbe kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/gateworks_8M
mkdir -p ~/GruppenLW/releases/$DATE/gateworks_16M
mkdir -p ~/GruppenLW/releases/$DATE/pronghorn-SBC
mkdir -p ~/GruppenLW/releases/$DATE/compex_wp188
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks_8M
#cp xscale/src/router/armeb-uclibc/gateworks-firmware-jffs.bin ~/GruppenLW/releases/$DATE/gateworks_8M
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/gateworks_8M/linux.bin

#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/pronghorn-SBC/pronghorn-firmware.bin
#cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/pronghorn-SBC/linux.bin

cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/compex_wp188/compex-firmware-webflash.bin
cp xscale/src/router/armeb-uclibc/firmware.compex ~/GruppenLW/releases/$DATE/compex_wp188/firmware.tftp

cd xscale/src/router
echo "CONFIG_PRONGHORN=y" >> .config
make -f Makefile.armbe libutils-clean libutils install
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/pronghorn-SBC/pronghorn-firmware.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/pronghorn-SBC/linux.bin

cd xscale/src/router

cp .config_xscale_16M .config
make -f Makefile.armbe kernel clean all install
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks_16M
#cp xscale/src/router/armeb-uclibc/gateworks-firmware-jffs.bin ~/GruppenLW/releases/$DATE/gateworks_16M
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/gateworks_16M/linux.bin


#./build_wrt300nv2.sh
./build_nop8670.sh
./build_wg302.sh
./build_wg302v1.sh
#./build_xscale_maksat.sh
#cd xscale/src/router
#svn update
#cp .config_ixpmaksat .config
#make -f Makefile.armbe kernel clean all install
#mkdir -p ~/GruppenLW/releases/$DATE/gateworks
#cd ../../../
#cp xscale/src/router/armeb-uclibc/gateworx-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks/maksat-gateworks-firmware.bin
#cp xscale/src/router/armeb-uclibc/root.fs ~/GruppenLW/releases/$DATE/gateworks/maksat-root.fs
#cp xscale/src/linux/xscale/linux-2.6.23/arch/arm/boot/zImage ~/GruppenLW/releases/$DATE/gateworks/maksat-zImage


#cd xscale/src/router
#svn update
#cp .config_xscale_boese .config
#make -f Makefile.armbe kernel clean all install
#mkdir -p ~/GruppenLW/releases/$DATE/gateworks
#cd ../../../
#cp xscale/src/router/armeb-uclibc/gateworx-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks/superchannel-gateworks-firmware.bin
#cp xscale/src/router/armeb-uclibc/root.fs ~/GruppenLW/releases/$DATE/gateworks/superchannel-root.fs
#cp xscale/src/linux/xscale/linux-2.6.22/arch/arm/boot/zImage ~/GruppenLW/releases/$DATE/gateworks/superchannel-zImage

#cd xscale/src/router
#svn update
#cp .config_xscale_william .config
#make -f Makefile.armbe kernel clean all install
#mkdir -p ~/GruppenLW/releases/$DATE/gateworks
#cd ../../../
#cp xscale/src/router/armeb-uclibc/gateworx-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks/william-gateworks-firmware.bin
#cp xscale/src/router/armeb-uclibc/root.fs ~/GruppenLW/releases/$DATE/gateworks/william-root.fs
#cp xscale/src/linux/xscale/linux-2.6.22/arch/arm/boot/zImage ~/GruppenLW/releases/$DATE/gateworks/william-zImage

#cd xscale/src/router
#svn update
#cp .config_xscale_mimo .config
#make -f Makefile.armbe kernel clean all install
#mkdir -p ~/GruppenLW/releases/$DATE/gateworks
#cd ../../../
#cp xscale/src/router/armeb-uclibc/gateworx-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks/gateworks-firmware-mimo.bin
#cp xscale/src/router/armeb-uclibc/root.fs ~/GruppenLW/releases/$DATE/gateworks/root.fs-mimo
#cp xscale/src/linux/xscale/linux-2.6.22/arch/arm/boot/zImage ~/GruppenLW/releases/$DATE/gateworks/zImage-mimo
