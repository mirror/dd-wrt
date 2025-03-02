#!/bin/sh -x
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n rt2880/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mipsel_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mipsel_mips32_gcc-8.2.0_musl/bin:$OLDPATH
cd rt2880/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_whr300hp2 .config
make -f Makefile.rt2880 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/buffalo-whr_300hp2
mkdir -p ~/GruppenLW/releases/$DATE/buffalo-whr_600d
cd ../../../
#cp rt2880/src/router/mipsel-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/WHR-G300N/firmware.bin
cp rt2880/src/router/mipsel-uclibc/mt7620-webflash.bin ~/GruppenLW/releases/$DATE/buffalo-whr_300hp2/buffalo-whr_300hp2-webflash.bin
cp rt2880/src/router/mipsel-uclibc/aligned-mt7620.uimage ~/GruppenLW/releases/$DATE/buffalo-whr_300hp2/firmware.uimage

cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-300-jp ~/GruppenLW/releases/$DATE/buffalo-whr_300hp2/firmware-jp.bin
cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-300-eu ~/GruppenLW/releases/$DATE/buffalo-whr_300hp2/firmware-eu.bin
cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-300-us ~/GruppenLW/releases/$DATE/buffalo-whr_300hp2/firmware-us.bin

cd rt2880/src/router
cp .config_whr600d .config
echo "CONFIG_WHR600D=y" >> .config
make -f Makefile.rt2880 kernel clean all install
cd ../../../

cp rt2880/src/router/mipsel-uclibc/aligned-mt7620.uimage ~/GruppenLW/releases/$DATE/buffalo-whr_600d/firmware.uimage
cp rt2880/src/router/mipsel-uclibc/mt7620-webflash.bin ~/GruppenLW/releases/$DATE/buffalo-whr_600d/buffalo-whr_600d-webflash.bin
cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-600-jp ~/GruppenLW/releases/$DATE/buffalo-whr_600d/firmware-jp.bin
cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-600-eu ~/GruppenLW/releases/$DATE/buffalo-whr_600d/firmware-eu.bin
cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-600-us ~/GruppenLW/releases/$DATE/buffalo-whr_600d/firmware-us.bin
#cp rt2880/src/router/mipsel-uclibc/whrg300n-firmware.tftp ~/GruppenLW/releases/$DATE/WHR-G300N/whrg300n-firmware.tftp
#scp rt2880/src/router/mipsel-uclibc/mt7620-webflash.bin 10.20.30.102:/tmp/firmware.bin


