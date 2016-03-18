#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n rt2880/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mipsel_gcc4.1.2/bin:$OLDPATH
cd rt2880/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
#cp .config_whr300hp2 .config
cat .config_whr300hp2 | grep -v "^CONFIG_MILKFISH\|^CONFIG_OPENSER\|^CONFIG_EOP_TUNNEL\|^CONFIG_WIFIDOG\|^CONFIG_OVERCLOCKING\|^CONFIG_PROXYWATCHDOG" > .config
#echo "CONFIG_BRANDING=y" >> .config
#echo "CONFIG_BUFFALO=y" >> .config
#echo "CONFIG_IAS=y" >> .config
echo "CONFIG_CLOUD4WI=y" >> .config
make -f Makefile.rt2880 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/cloud4wi/buffalo_whr_300hp2
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/cloud4wi/buffalo_whr_600d
cd ../../../
#cp rt2880/src/router/mipsel-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/WHR-G300N/firmware.bin
cp rt2880/src/router/mipsel-uclibc/mt7620-webflash.bin ~/GruppenLW/releases/CUSTOMER/$DATE/cloud4wi/buffalo_whr_300hp2/buffalo_whr_300hp2-webflash.bin
cp rt2880/src/router/mipsel-uclibc/aligned-mt7620.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/cloud4wi/buffalo_whr_300hp2/firmware.uimage
cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-300-jp ~/GruppenLW/releases/CUSTOMER/$DATE/cloud4wi/buffalo_whr_300hp2/firmware-jp.bin
cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-300-eu ~/GruppenLW/releases/CUSTOMER/$DATE/cloud4wi/buffalo_whr_300hp2/firmware-eu.bin
cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-300-us ~/GruppenLW/releases/CUSTOMER/$DATE/cloud4wi/buffalo_whr_300hp2/firmware-us.bin

cp rt2880/src/router/mipsel-uclibc/mt7620-webflash.bin ~/GruppenLW/releases/CUSTOMER/$DATE/cloud4wi/buffalo_whr_600d/buffalo_whr_600d-webflash.bin
cp rt2880/src/router/mipsel-uclibc/aligned-mt7620.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/cloud4wi/buffalo_whr_600d/firmware.uimage

cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-600-jp ~/GruppenLW/releases/CUSTOMER/$DATE/cloud4wi/buffalo_whr_600d/firmware-jp.bin
cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-600-eu ~/GruppenLW/releases/CUSTOMER/$DATE/cloud4wi/buffalo_whr_600d/firmware-eu.bin
cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-600-us ~/GruppenLW/releases/CUSTOMER/$DATE/cloud4wi/buffalo_whr_600d/firmware-us.bin
#cp rt2880/src/router/mipsel-uclibc/whrg300n-firmware.tftp ~/GruppenLW/releases/$DATE/WHR-G300N/whrg300n-firmware.tftp


