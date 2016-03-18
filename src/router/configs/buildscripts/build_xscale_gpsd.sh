#!/bin/sh
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
cp .config_xscale_gpsd .config
make -f Makefile.armbe kernel clean all install
mkdir -p /GruppenLW/releases/$DATE/gateworks
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin /GruppenLW/releases/$DATE/gateworks/gpsd_gateworx-firmware-squashfs.bin
cp xscale/src/router/armeb-uclibc/root.fs /GruppenLW/releases/$DATE/gateworks/gpsd_root.fs
cp xscale/src/linux/xscale/linux-2.6.22/arch/arm/boot/zImage /GruppenLW/releases/$DATE/gateworks/gpsd_zImage

exit

cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_ixpmaksat .config
make -f Makefile.armbe kernel clean all install
mkdir -p /GruppenLW/releases/$DATE/gateworks
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin /GruppenLW/releases/$DATE/gateworks/maksat-gateworks-firmware.bin
cp xscale/src/router/armeb-uclibc/root.fs /GruppenLW/releases/$DATE/gateworks/maksat-root.fs
cp xscale/src/linux/xscale/linux-2.6.22/arch/arm/boot/zImage /GruppenLW/releases/$DATE/gateworks/maksat-zImage


cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_xscale_boese .config
make -f Makefile.armbe kernel clean all install
mkdir -p /GruppenLW/releases/$DATE/gateworks
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin /GruppenLW/releases/$DATE/gateworks/superchannel-gateworks-firmware.bin
cp xscale/src/router/armeb-uclibc/root.fs /GruppenLW/releases/$DATE/gateworks/superchannel-root.fs
cp xscale/src/linux/xscale/linux-2.6.22/arch/arm/boot/zImage /GruppenLW/releases/$DATE/gateworks/superchannel-zImage

cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_xscale_william .config
make -f Makefile.armbe kernel clean all install
mkdir -p /GruppenLW/releases/$DATE/gateworks
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin /GruppenLW/releases/$DATE/gateworks/william-gateworks-firmware.bin
cp xscale/src/router/armeb-uclibc/root.fs /GruppenLW/releases/$DATE/gateworks/william-root.fs
cp xscale/src/linux/xscale/linux-2.6.22/arch/arm/boot/zImage /GruppenLW/releases/$DATE/gateworks/william-zImage
