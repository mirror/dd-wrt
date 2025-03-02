#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-arm_mpcore+vfp_gcc-13.1.0_musl_eabi/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_mpcore+vfp_gcc-4.8-linaro_musl-1.1.4_eabi/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_v6k_gcc-4.7-linaro_uClibc-0.9.33.2_eabi-7/bin:$OLDPATH
cd laguna/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_laguna .config
echo "CONFIG_SMP=y" >> .config
make -f Makefile.laguna kernel clean all install

mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/laguna-SBC-excellent
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/laguna-SBC-excelmed
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/laguna-SBC-excelmin
cd ../../../

./broadcom/opt/tools/addpattern -4 -p XMAX -v v3.00.0 -i laguna/src/router/arm-uclibc/laguna-firmware-squashfs.bin -o ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/laguna-SBC-excellent/laguna-excellent-firmware.bin -g
#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/pronghorn-SBC-excellent/pronghorn-excellent-firmware.bin
cp laguna/src/router/arm-uclibc/laguna-firmware.raw2 ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/laguna-SBC-excellent/linux-excellent.bin

cd laguna/src/router
cp .config_laguna_wikings .config
echo "CONFIG_SUB3=y" >> .config
echo "CONFIG_SMP=y" >> .config
make -f Makefile.laguna httpd-clean httpd install
cd ../../../
./broadcom/opt/tools/addpattern -4 -p XMED -v v3.00.0 -i laguna/src/router/arm-uclibc/laguna-firmware-squashfs.bin -o ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/laguna-SBC-excelmed/laguna-excelmed-firmware.bin -g
#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/pronghorn-SBC-excelmed/pronghorn-excelmed-firmware.bin
cp laguna/src/router/arm-uclibc/laguna-firmware.raw2 ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/laguna-SBC-excelmed/linux-excelmed.bin

cd laguna/src/router
cp .config_laguna_wikings .config
echo "CONFIG_SUB6=y" >> .config
echo "CONFIG_SMP=y" >> .config
make -f Makefile.laguna httpd-clean httpd install
cd ../../../
./broadcom/opt/tools/addpattern -4 -p XMIN -v v3.00.0 -i laguna/src/router/arm-uclibc/laguna-firmware-squashfs.bin -o ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/laguna-SBC-excelmin/gateworks-excelmin-firmware.bin -g
#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/pronghorn-SBC-excelmin/pronghorn-excelmin-firmware.bin
cp laguna/src/router/arm-uclibc/laguna-firmware.raw2 ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/laguna-SBC-excelmin/linux-excelmin.bin

