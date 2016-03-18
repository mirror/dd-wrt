#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-4.8-linaro_musl-1.1.2/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-mips_r2_gcc-4.8-linaro_uClibc-0.9.33.2/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cat .config_mms344 | grep -v "^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|CONFIG_ELEGANT\|CONFIG_BLUE\|CONFIG_YELLOW\|CONFIG_CYAN\|CONFIG_GREEN\|CONFIG_SPUTNIK_APD\|CONFIG_VPNC\|CONFIG_POUND\|CONFIG_NSTX\|CONFIG_OPENSSL\|CONFIG_WIFIDOG\|CONFIG_SPUTNIK_APD\|CONFIG_SPUTNIK_PRO\|CONFIG_MUSL" > .config

echo "CONFIG_BRANDING=y" >>.config
echo "CONFIG_ANTAIRA=y" >>.config
echo "CONFIG_UQMI=y" >>.config
echo "CONFIG_3G=y" >>.config
echo "CONFIG_COMGT=y" >>.config
echo "CONFIG_USB=y" >>.config
echo "CONFIG_USB_ADVANCED=y" >>.config
echo "CONFIG_OPENVPN=y" >>.config
#echo "CONFIG_STRONGSWAN=y" >>.config
echo "CONFIG_CHILLISPOT=y" >>.config
echo "CONFIG_EHCI=y" >>.config
#echo "HOSTAPDVERSION=20131120" >>.config
#echo "CONFIG_WPE72_SIZE=0x7d0000" >>.config
echo "CONFIG_GPSD=y" >>.config
echo "CONFIG_LIBCPP=y" >>.config
echo "CONFIG_GPSI=y" >>.config
echo "CONFIG_NCURSES=y" >>.config
echo "CONFIG_LIBNLTINY=y" >>.config

echo "CONFIG_UBOOTENV=y" >>.config

make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/antaira/compex_wpj344
cd ../../../
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/antaira/compex_wpj344/wpj344-firmware.bin
cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/antaira/compex_wpj344/wpj344-uimage.bin
#cp pb42/src/router/mips-uclibc/dir825c1-uimage.bin ~/GruppenLW/releases/$DATE/dlink-dir825-c1/factory-to-ddwrt_NA.bin


