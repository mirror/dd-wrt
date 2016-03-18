#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n laguna/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH

export PATH=/xfs/toolchains/toolchain-arm_mpcore+vfp_gcc-5.2.0_musl-1.1.11_eabi/bin:$OLDPATH
cd laguna/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_laguna-newkernel .config
echo "CONFIG_SMP=y" >> .config
#cp .config_laguna .config
make -f Makefile.laguna kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/gateworks_gw2388-32M
cd ../../../
cp laguna/src/router/arm-uclibc/laguna-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks_gw2388-32M
cp laguna/src/router/arm-uclibc/laguna-firmware.raw2 ~/GruppenLW/releases/$DATE/gateworks_gw2388-32M/uImage.bin

cp notes/laguna/* ~/GruppenLW/releases/$DATE/gateworks_gw2388-32M

#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/pronghorn-SBC/pronghorn-firmware.bin
#cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/pronghorn-SBC/linux.bin


