#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n northstar/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-arm_cortex-a9_gcc-5.3.0_musl-1.1.14_eabi/bin:$OLDPATH
cd northstar/src/router
[ -n "$DO_UPDATE" ] && svn update
#cp .config_laguna-small .config
cp .config_northstar_buffalo .config
echo "CONFIG_SMP=y" >> .config
#echo "CONFIG_BRANDING=y" >> .config
#echo "CONFIG_BUFFALO=y" >> .config
#echo "CONFIG_IAS=y" >> .config

make -f Makefile.northstar kernel clean all install
#mkdir -p ~/GruppenLW/releases/$DATE/northstar
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wxr-1900dhp
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-1750dhp
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-1166dhp
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-900dhp
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-600dhp2
cd ../../../
#cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/northstar

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wxr-1900dhp/buffalo-wxr-1900dhp-webflash.bin
cp northstar/src/router/arm-uclibc/buffalo-1750.encold ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wxr-1900dhp/factory-to-dd-wrt.bin
#cp northstar/src/router/arm-uclibc/buffalo-1750.enc ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-1750dhp/factory-to-dd-wrt-new.bin
#cp northstar/src/router/arm-uclibc/buffalo-1750.encold ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-1750dhp/factory-to-dd-wrt-crcold.bin

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-1750dhp/buffalo-wzr-1750dhp-webflash.bin
cp northstar/src/router/arm-uclibc/buffalo-1750.encold ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-1750dhp/factory-to-dd-wrt.bin
#cp northstar/src/router/arm-uclibc/buffalo-1750.enc ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-1750dhp/factory-to-dd-wrt-new.bin
#cp northstar/src/router/arm-uclibc/buffalo-1750.encold ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-1750dhp/factory-to-dd-wrt-crcold.bin

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-1166dhp/buffalo-wzr-1166dhp-webflash.bin
cp northstar/src/router/arm-uclibc/buffalo-1166.encold ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-1166dhp/factory-to-dd-wrt.bin
#cp northstar/src/router/arm-uclibc/buffalo-1166.enc ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-1166dhp/factory-to-dd-wrt-new.bin
#cp northstar/src/router/arm-uclibc/buffalo-1166.encold ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-1166dhp/factory-to-dd-wrt-crcold.bin
#exit

cd northstar/src/router
exit

cp .config_northstar_buffalo .config

echo "CONFIG_NORTHSTAR_NOSMP=y" >> .config
#echo "CONFIG_BRANDING=y" >> .config
#echo "CONFIG_BUFFALO=y" >> .config
#echo "CONFIG_IAS=y" >> .config

make -f Makefile.northstar kernel clean all install
cd ../../../

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-900dhp/buffalo-wzr-900dhp-webflash.bin
cp northstar/src/router/arm-uclibc/buffalo-900.encold ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-900dhp/factory-to-dd-wrt.bin
#cp northstar/src/router/arm-uclibc/buffalo-900.enc ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-900dhp/factory-to-dd-wrt-new.bin
#cp northstar/src/router/arm-uclibc/buffalo-900.encold ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-900dhp/factory-to-dd-wrtcrcold.bin

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-600dhp2/buffalo-wzr-600dhp2-webflash.bin
cp northstar/src/router/arm-uclibc/buffalo-600.encold ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-600dhp2/factory-to-dd-wrt.bin
#cp northstar/src/router/arm-uclibc/buffalo-600.enc ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-600dhp2/factory-to-dd-wrt-new.bin
#cp northstar/src/router/arm-uclibc/buffalo-600.encold ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr-600dhp2/factory-to-dd-wrtcrcold.bin
