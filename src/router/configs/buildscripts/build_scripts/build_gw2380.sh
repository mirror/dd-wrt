#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n laguna/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-arm_mpcore+vfp_gcc-12.1.0_musl_eabi/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_mpcore+vfp_gcc-12.1.0_musl_eabi/bin:$OLDPATH
cd laguna/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_gw2380 .config
echo "KERNELVERSION=4.9" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_ATH5K=y" >> .config
echo "CONFIG_CAKE=y" >> .config
echo "CONFIG_MRP=y" >> .config
echo "CONFIG_CFM=y" >> .config
echo "CONFIG_NTFS3=y" >> .config
#cp .config_laguna .config
make -f Makefile.laguna kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/gateworks-laguna-gw2380
mkdir -p ~/GruppenLW/releases/$DATE/gateworks-laguna-gw2382
mkdir -p ~/GruppenLW/releases/$DATE/gateworks-laguna-gw2391
cd ../../../
cp laguna/src/router/arm-uclibc/laguna-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks-laguna-gw2380
cp laguna/src/router/arm-uclibc/laguna-firmware.raw2 ~/GruppenLW/releases/$DATE/gateworks-laguna-gw2380/uImage.bin

cp laguna/src/router/arm-uclibc/laguna-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks-laguna-gw2382
cp laguna/src/router/arm-uclibc/laguna-firmware.raw2 ~/GruppenLW/releases/$DATE/gateworks-laguna-gw2382/uImage.bin

cp laguna/src/router/arm-uclibc/laguna-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks-laguna-gw2391
cp laguna/src/router/arm-uclibc/laguna-firmware.raw2 ~/GruppenLW/releases/$DATE/gateworks-laguna-gw2391/uImage.bin

cp notes/gw2380/* ~/GruppenLW/releases/$DATE/gateworks-laguna-gw2380
cp notes/gw2380/* ~/GruppenLW/releases/$DATE/gateworks-laguna-gw2382
cp notes/laguna/* ~/GruppenLW/releases/$DATE/gateworks-laguna-gw2391

#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/pronghorn-SBC/pronghorn-firmware.bin
#cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/pronghorn-SBC/linux.bin


