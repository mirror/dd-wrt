#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n laguna/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-arm_mpcore+vfp_gcc-5.2.0_musl-1.1.11_eabi/bin:$OLDPATH
cd laguna/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_gw2380 .config
cat .config_gw2380 | grep -v "^CONFIG_SQUID\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_NOCAT\|CONFIG_MC\|CONFIG_SNORT${REMOVE}" > .config

echo "CONFIG_SRS=y" >>.config
echo "CONFIG_NOTRIAL=y" >>.config
echo "CONFIG_BRANDING=y" >>.config
echo "CONFIG_BUSYBOX_INETD=y" >>.config
echo "CONFIG_NOLOGO=y" >>.config

make -f Makefile.laguna kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/gateworks_gw2380
mkdir -p ~/GruppenLW/releases/$DATE/gateworks_gw2382
mkdir -p ~/GruppenLW/releases/$DATE/gateworks_gw2391
cd ../../../
cp laguna/src/router/arm-uclibc/laguna-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks_gw2380
cp laguna/src/router/arm-uclibc/laguna-firmware.raw2 ~/GruppenLW/releases/$DATE/gateworks_gw2380/uImage.bin

cp laguna/src/router/arm-uclibc/laguna-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks_gw2382
cp laguna/src/router/arm-uclibc/laguna-firmware.raw2 ~/GruppenLW/releases/$DATE/gateworks_gw2382/uImage.bin

cp laguna/src/router/arm-uclibc/laguna-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks_gw2391
cp laguna/src/router/arm-uclibc/laguna-firmware.raw2 ~/GruppenLW/releases/$DATE/gateworks_gw2391/uImage.bin

cp notes/gw2380/* ~/GruppenLW/releases/$DATE/gateworks_gw2380
cp notes/gw2380/* ~/GruppenLW/releases/$DATE/gateworks_gw2382
cp notes/laguna/* ~/GruppenLW/releases/$DATE/gateworks_gw2391

#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/pronghorn-SBC/pronghorn-firmware.bin
#cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/pronghorn-SBC/linux.bin


