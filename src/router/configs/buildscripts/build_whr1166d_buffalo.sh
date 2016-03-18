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
#cp .config_whr1166_buffalo .config
cat .config_whr1166 | grep -v "^CONFIG_ROUTERSTYLE\|^CONFIG_ELEGANT\|^CONFIG_BLUE\|^CONFIG_YELLOW\|^CONFIG_CYAN\|^CONFIG_RED\|^CONFIG_GREEN\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER" > .config
echo "CONFIG_WHR1166D=y" >> .config
echo "CONFIG_BUFFALO=y" >> .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_IAS=y" >> .config
make -f Makefile.rt2880 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_1166d
cd ../../../

cp rt2880/src/router/mipsel-uclibc/mt7620-webflash.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_1166d/buffalo_whr_1166d-webflash.bin
cp rt2880/src/router/mipsel-uclibc/aligned-mt7620.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_1166d/firmware.uimage
cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-1166-jp ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_1166d/firmware-jp.bin
cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-1166-eu ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_1166d/firmware-eu.bin
cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-1166-us ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_1166d/firmware-us.bin

#cp rt2880/src/router/mipsel-uclibc/whrg300n-firmware.tftp ~/GruppenLW/releases/$DATE/WHR-G300N/whrg300n-firmware.tftp


