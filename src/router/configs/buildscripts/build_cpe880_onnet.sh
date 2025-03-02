#!/bin/sh -x
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
#cp .config_mms344 .config
cat .config_mms344 | grep -v "^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_ROUTERSTYLE\|^CONFIG_BLUE\|^CONFIG_YELLOW\|^CONFIG_CYAN\|^CONFIG_RED\|^CONFIG_GREEN\|^CONFIG_NOTRIAL\|^CONFIG_ATH10K" > .config
echo "CONFIG_CPE880=y" >> .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_ONNET=y" >> .config
echo "CONFIG_IPERF=y" >> .config
echo "CONFIG_SUPERCHANNEL=y" >> .config
#echo "CONFIG_STRACE=y" >> .config
make -f Makefile.pb42 clean kernel clean all install
cd ../../../
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/yuncore-cpe880
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/yuncore-cpe880/cpe880-firmware.bin
cp pb42/src/router/mips-uclibc/aligned.uimage  ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/yuncore-cpe880/cpe880-uimage.bin

cd pb42/src/router
echo "CONFIG_ONNET_STATION=y" >> .config
make -f Makefile.pb42 clean kernel clean all install
cd ../../../
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/yuncore-cpe880_station
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/yuncore-cpe880_station/cpe880-firmware.bin
cp pb42/src/router/mips-uclibc/aligned.uimage  ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/yuncore-cpe880_station/cpe880-uimage.bin


