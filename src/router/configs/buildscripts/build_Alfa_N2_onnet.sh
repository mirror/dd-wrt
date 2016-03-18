#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_r2_gcc-4.7-linaro_uClibc-0.9.33.2/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_r2_gcc-linaro_uClibc-0.9.32/bin:$OLDPATH
cd pb42/src/router
if [ x$1 = xBLANK ]
then
	BEXTRA=$2
	BLANK=1
else
	BEXTRA=$1
fi
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_ac622_onnet .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/Alfa_N2
cd ../../../
cp pb42/src/router/mips-uclibc/ap93-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/Alfa_N2/N2-firmware.bin
cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/Alfa_N2/ubootflash.bin
