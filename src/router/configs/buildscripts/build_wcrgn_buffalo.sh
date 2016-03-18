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
cp .config_wcrgn_buffalo .config
cp /home/seg/peter/pb42/src/router/kromo/dd-wrt/lang_pack/korean.js /home/seg/DEV/rt2880/src/router/kromo/dd-wrt/lang_pack/
make -f Makefile.rt2880 kernel clean all install
rm /home/seg/DEV/rt2880/src/router/kromo/dd-wrt/lang_pack/korean.js
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/wcr-gn
cd ../../../
#cp rt2880/src/router/mipsel-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/WHR-G300N/firmware.bin
cp rt2880/src/router/mipsel-uclibc/rt2880-webflash.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/wcr-gn/WCR-GN-webflash.bin
cp rt2880/src/router/mipsel-uclibc/aligned.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/wcr-gn/firmware.tftp
#cp rt2880/src/router/mipsel-uclibc/whrg300n-firmware.tftp ~/GruppenLW/releases/$DATE/WHR-G300N/whrg300n-firmware.tftp
