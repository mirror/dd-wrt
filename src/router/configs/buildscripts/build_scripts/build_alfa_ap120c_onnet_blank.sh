#!/bin/sh -x
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-12.2.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cat .config_ap120c | grep -v "^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_ROUTERSTYLE\|^CONFIG_BLUE\|^CONFIG_YELLOW\|^CONFIG_CYAN\|^CONFIG_RED\|^CONFIG_GREEN\|^CONFIG_NOTRIAL\|^CONFIG_REGISTER" > .config
echo "CONFIG_AP120C=y" >> .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_REGISTER=y" >> .config
echo "CONFIG_ONNET=y" >> .config
echo "CONFIG_ONNET_BLANK=y" >> .config
echo "CONFIG_IPERF=n" >> .config
echo "CONFIG_SUPERCHANNEL=y" >> .config
#cp .config_ap120c .config
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/onnet_blank/alfa-ap120c
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/onnet_blank/alfa-ap120c/alfa-ap120c-firmware.bin
cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/onnet_blank/alfa-ap120c/factory-to-ddwrt.bin

