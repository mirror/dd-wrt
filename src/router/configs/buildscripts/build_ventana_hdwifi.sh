#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ventana/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-arm_cortex-a9+neon_gcc-13.1.0_musl_eabi/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_cortex-a9+neon_gcc-4.8-linaro_musl-1.1.2_eabi/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_cortex-a9+neon_gcc-4.9-linaro_musl-1.1.1_eabi/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_v7-a_gcc-4.8-linaro_uClibc-0.9.33.2-eabi-imx6/bin:$OLDPATH
cd ventana/src/router
[ -n "$DO_UPDATE" ] && svn update
cat .config_ventana | grep -v "^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\^CONFIG_ROUTERSTYLE\^CONFIG_BLUE\^CONFIG_YELLOW\^CONFIG_CYAN\^CONFIG_RED\^CONFIG_GREEN\^CONFIG_SAMBA3\^CONFIG_REGISTER" > .config
echo "CONFIG_HDWIFI=y" >> .config
echo "CONFIG_NOTRIAL=y" >> .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_SMBD=y" >> .config
echo "CONFIG_SMP=y" >> .config
echo "CONFIG_QCA9984" >> .config
#cp .config_laguna .config
make -f Makefile.ventana kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/hdwifi/gateworks_gw54xx
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/hdwifi/gateworks_gw53xx
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/hdwifi/gateworks_gw52xx
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/hdwifi/gateworks_gw51xx
cd ../../../
cp ventana/src/router/arm-uclibc/root-aligned.ubi ~/GruppenLW/releases/CUSTOMER/$DATE/hdwifi/gateworks_gw54xx/gw54xx.ubi
cp ventana/src/router/arm-uclibc/root-aligned.ubi ~/GruppenLW/releases/CUSTOMER/$DATE/hdwifi/gateworks_gw53xx/gw53xx.ubi
cp ventana/src/router/arm-uclibc/root-aligned.ubi ~/GruppenLW/releases/CUSTOMER/$DATE/hdwifi/gateworks_gw52xx/gw52xx.ubi
cp ventana/src/router/arm-uclibc/root-aligned.ubi ~/GruppenLW/releases/CUSTOMER/$DATE/hdwifi/gateworks_gw51xx/gw51xx.ubi

#cp notes/laguna/* ~/GruppenLW/releases/$DATE/gateworks_gw2388-16M

#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/pronghorn-SBC/pronghorn-firmware.bin
#cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/pronghorn-SBC/linux.bin


