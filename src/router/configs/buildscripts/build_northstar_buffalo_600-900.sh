#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n northstar/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-arm_cortex-a9_gcc-5.3.0_musl-1.1.14_eabi/bin:$OLDPATH
cd northstar/src/router

cp .config_northstar_buffalo .config

echo "CONFIG_NORTHSTAR_NOSMP=y" >> .config
#echo "CONFIG_BRANDING=y" >> .config
#echo "CONFIG_BUFFALO=y" >> .config
#echo "CONFIG_IAS=y" >> .config

make -f Makefile.northstar kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-900dhp
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-600dhp2
cd ../../../

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-900dhp/buffalo-wzr-900dhp-webflash.bin
cp northstar/src/router/arm-uclibc/buffalo-900.enc ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-900dhp/factory-to-dd-wrt.bin
cp northstar/src/router/arm-uclibc/buffalo-900.encold ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-900dhp/factory-to-dd-wrtcrcold.bin

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-600dhp2/buffalo-wzr-600dhp2-webflash.bin
cp northstar/src/router/arm-uclibc/buffalo-600.enc ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-600dhp2/factory-to-dd-wrt.bin
cp northstar/src/router/arm-uclibc/buffalo-600.encold ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-600dhp2/factory-to-dd-wrtcrcold.bin
