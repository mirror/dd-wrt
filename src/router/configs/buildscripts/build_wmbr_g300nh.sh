#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n danube/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mipsel_24kc_gcc-13.1.0_musl/bin:$OLDPATH
cd danube/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_wmbr_g300nh .config
make -f Makefile.danube kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/buffalo-wbmr_g300h
cd ../../../
cp danube/src/router/mips-uclibc/danube-webflash.bin ~/GruppenLW/releases/$DATE/buffalo-wbmr_g300h/wbmr_g300h-webflash-firmware.bin
cp danube/src/router/mips-uclibc/wmbr-firmware_MULTI.enc ~/GruppenLW/releases/$DATE/buffalo-wbmr_g300h/buffalo-to_dd-wrt_MULTI.enc
cp danube/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/buffalo-wbmr_g300h/uImage.bin
