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
cp .config_hame_no-ip .config
make -f Makefile.rt2880 kernel clean all install
mkdir -p /GruppenLW/releases/CUSTOMER/$DATE/no-ip/hame_a15
cd ../../../
#cp rt2880/src/router/mipsel-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/WHR-G300N/firmware.bin
cp rt2880/src/router/mipsel-uclibc/rt2880-webflash.bin /GruppenLW/releases/CUSTOMER/$DATE/no-ip/hame_a15/hame_a15-webflash.bin
cp rt2880/src/router/mipsel-uclibc/aligned.uimage /GruppenLW/releases/CUSTOMER/$DATE/no-ip/hame_a15/firmware.tftp
#cp rt2880/src/router/mipsel-uclibc/whrg300n-firmware.tftp ~/GruppenLW/releases/$DATE/WHR-G300N/whrg300n-firmware.tftp
echo /GruppenLW/releases/CUSTOMER/$DATE/no-ip/hame_a15/

