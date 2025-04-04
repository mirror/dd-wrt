#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n rt2880/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin:$OLDPATH
cd rt2880/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_rut500 .config
make -f Makefile.rt2880 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/rut500
cd ../../../
#cp rt2880/src/router/mipsel-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/WHR-G300N/firmware.bin
cp rt2880/src/router/mipsel-uclibc/rt2880-webflash.bin ~/GruppenLW/releases/$DATE/rut500/rut500-webflash.bin
cp rt2880/src/router/mipsel-uclibc/root.uimage ~/GruppenLW/releases/$DATE/rut500/factory-to-ddwrt.bin
#cp rt2880/src/router/mipsel-uclibc/whrg300n-firmware.tftp ~/GruppenLW/releases/$DATE/WHR-G300N/whrg300n-firmware.tftp


scp rt2880/src/router/mipsel-uclibc/rt2880-webflash.bin 10.20.30.201:/tmp