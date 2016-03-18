#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n laguna/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-arm_mpcore+vfp_gcc-5.2.0_musl-1.1.11_eabi/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_v6k_gcc-4.7-linaro_uClibc-0.9.33.2_eabi-7/bin:$OLDPATH
cd laguna/src/router
[ -n "$DO_UPDATE" ] && svn update
cat .config_laguna-small | grep -v "^CONFIG_FREERADIUS\|^CONFIG_SAMBA\|^CONFIG_MINIDLNA" >.config
echo "CONFIG_PHP=y" >>.config
echo "CONFIG_LIGHTTPD=y" >>.config
#cp .config_laguna .config
make -f Makefile.laguna kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/gateworks_gw2388-16M-chris
cd ../../../
cp laguna/src/router/arm-uclibc/laguna-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks_gw2388-16M-chris
cp laguna/src/router/arm-uclibc/laguna-firmware.raw2 ~/GruppenLW/releases/$DATE/gateworks_gw2388-16M-chris/uImage.bin

cp notes/laguna/* ~/GruppenLW/releases/$DATE/gateworks_gw2388-16M-chris

#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/pronghorn-SBC/pronghorn-firmware.bin
#cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/pronghorn-SBC/linux.bin


