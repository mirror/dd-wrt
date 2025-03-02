#!/bin/sh

OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/libutils)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-13.1.0_musl/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_xscale_cambria_gps .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_ATH5K=y" >> .config
echo "CONFIG_CAKE=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_SMBD=y" >> .config
make -f Makefile.armbe kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/gateworks-cambria_gps
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks-cambria_gps/cambria-firmware-squashfs.bin
#cp xscale/src/router/armeb-uclibc/gateworks-firmware-jffs.bin ~/GruppenLW/releases/$DATE/cambria/cambria-firmware-jffs2.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/gateworks-cambria_gps/linux.bin

