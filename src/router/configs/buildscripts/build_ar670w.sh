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
cp .config_ar670w .config
make -f Makefile.rt2880 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/airlink101-ar670w
cd ../../../
#cp rt2880/src/router/mipsel-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/WHR-G300N/firmware.bin
cp rt2880/src/router/mipsel-uclibc/raw-webflash.bin ~/GruppenLW/releases/$DATE/airlink101-ar670w/ar670w-ddwrt-webflash.bin
cp rt2880/src/router/mipsel-uclibc/ar670w-web.bin ~/GruppenLW/releases/$DATE/airlink101-ar670w/airlink-factory-webflash.bin
#cp rt2880/src/router/mipsel-uclibc/whrg300n-firmware.tftp ~/GruppenLW/releases/$DATE/WHR-G300N/whrg300n-firmware.tftp


