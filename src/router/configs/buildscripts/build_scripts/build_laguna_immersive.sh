#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-arm_mpcore+vfp_gcc-12.1.0_musl_eabi/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_mpcore+vfp_gcc-4.8-linaro_musl-1.1.4_eabi/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_v6k_gcc-4.7-linaro_uClibc-0.9.33.2_eabi-7/bin:$OLDPATH
cd laguna/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_laguna_immersive .config
echo "CONFIG_SMP=y" >> .config
make -f Makefile.laguna kernel clean all install

mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/immersive/gateworks_gw2388-32M
cd ../../../

cp laguna/src/router/arm-uclibc/laguna-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/immersive/gateworks_gw2388-32M
cp laguna/src/router/arm-uclibc/laguna-firmware.raw2 ~/GruppenLW/releases/CUSTOMER/$DATE/immersive/gateworks_gw2388-32M/uImage.bin
