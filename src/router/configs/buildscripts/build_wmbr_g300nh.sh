#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n danube/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_r2_gcc-4.7-linaro_uClibc-0.9.33.2/bin:$OLDPATH
cd danube/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_wmbr_g300nh .config
make -f Makefile.danube kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wbmr_g300h
cd ../../../
cp danube/src/router/mips-uclibc/danube-webflash.bin ~/GruppenLW/releases/$DATE/buffalo_wbmr_g300h/wbmr_g300h-webflash-firmware.bin
cp danube/src/router/mips-uclibc/wmbr-firmware_MULTI.enc ~/GruppenLW/releases/$DATE/buffalo_wbmr_g300h/buffalo_to_dd-wrt_MULTI.enc
cp danube/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/buffalo_wbmr_g300h/uImage.bin
