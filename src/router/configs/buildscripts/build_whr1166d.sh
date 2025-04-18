#!/bin/sh
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
cp .config_whr1166 .config
echo "CONFIG_WHR1166D=y" >> .config
make -f Makefile.rt2880 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/buffalo-whr_1166d
cd ../../../

cp rt2880/src/router/mipsel-uclibc/mt7620-webflash.bin ~/GruppenLW/releases/$DATE/buffalo-whr_1166d/buffalo-whr_1166d-webflash.bin
cp rt2880/src/router/mipsel-uclibc/aligned-mt7620.uimage ~/GruppenLW/releases/$DATE/buffalo-whr_1166d/firmware.uimage
cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-1166-jp ~/GruppenLW/releases/$DATE/buffalo-whr_1166d/firmware-jp.bin
cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-1166-eu ~/GruppenLW/releases/$DATE/buffalo-whr_1166d/firmware-eu.bin
cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-1166-us ~/GruppenLW/releases/$DATE/buffalo-whr_1166d/firmware-us.bin

#scp rt2880/src/router/mipsel-uclibc/mt7620-webflash.bin 10.88.193.69:/tmp/firmware.bin
#cp rt2880/src/router/mipsel-uclibc/whrg300n-firmware.tftp ~/GruppenLW/releases/$DATE/WHR-G300N/whrg300n-firmware.tftp


