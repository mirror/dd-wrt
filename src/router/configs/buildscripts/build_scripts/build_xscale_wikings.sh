#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-10.0.1_musl/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_xscale_wikings .config
echo "CONFIG_PRONGHORN=y" >> .config
make -f Makefile.armbe kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/pronghorn-SBC-excellent
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/pronghorn-SBC-excelmed
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/pronghorn-SBC-excelmin
cd ../../../

./broadcom/opt/tools/addpattern -4 -p XMAX -v v3.00.0 -i xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin -o ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/pronghorn-SBC-excellent/pronghorn-excellent-firmware.bin -g
#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/pronghorn-SBC-excellent/pronghorn-excellent-firmware.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/pronghorn-SBC-excellent/linux-excellent.bin

cd xscale/src/router
cp .config_xscale_wikings .config
echo "CONFIG_PRONGHORN=y" >> .config
echo "CONFIG_SUB3=y" >> .config
make -f Makefile.armbe httpd-clean httpd install
cd ../../../
./broadcom/opt/tools/addpattern -4 -p XMED -v v3.00.0 -i xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin -o ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/pronghorn-SBC-excelmed/pronghorn-excelmed-firmware.bin -g
#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/pronghorn-SBC-excelmed/pronghorn-excelmed-firmware.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/pronghorn-SBC-excelmed/linux-excelmed.bin

cd xscale/src/router
cp .config_xscale_wikings .config
echo "CONFIG_PRONGHORN=y" >> .config
echo "CONFIG_SUB6=y" >> .config
make -f Makefile.armbe httpd-clean httpd install
cd ../../../
./broadcom/opt/tools/addpattern -4 -p XMIN -v v3.00.0 -i xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin -o ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/pronghorn-SBC-excelmin/pronghorn-excelmin-firmware.bin -g
#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/pronghorn-SBC-excelmin/pronghorn-excelmin-firmware.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/CUSTOMER/$DATE/wikings/pronghorn-SBC-excelmin/linux-excelmin.bin

