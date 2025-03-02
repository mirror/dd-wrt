#!/bin/sh -x
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n laguna/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_v6k_gcc-4.7-linaro_uClibc-0.9.33.2_eabi-7/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-arm_mpcore+vfp_gcc-13.1.0_musl_eabi/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_mpcore+vfp_gcc-4.8-linaro_musl-1.1.4_eabi/bin:$OLDPATH
cd laguna/src/router
[ -n "$DO_UPDATE" ] && svn update
cat .config_laguna-newkernel | grep -v "^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\^CONFIG_ROUTERSTYLE\^CONFIG_BLUE\^CONFIG_YELLOW\^CONFIG_CYAN\^CONFIG_RED\^CONFIG_GREEN" > .config
echo "CONFIG_HDWIFI=y" >> .config
echo "CONFIG_NOTRIAL=y" >> .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_SMP=y" >> .config
#cp .config_laguna-newkernel .config
#cp .config_laguna .config
make -f Makefile.laguna kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/hdwifi/gateworks_gw2388-32M
cd ../../../
cp laguna/src/router/arm-uclibc/laguna-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/hdwifi/gateworks_gw2388-32M
cp laguna/src/router/arm-uclibc/laguna-firmware.raw2 ~/GruppenLW/releases/CUSTOMER/$DATE/hdwifi/gateworks_gw2388-32M/uImage.bin

cp notes/laguna/* ~/GruppenLW/releases/$DATE/gateworks_gw2388-32M

#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/pronghorn-SBC/pronghorn-firmware.bin
#cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/pronghorn-SBC/linux.bin


