#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
#cp .config_wpe72 .config
cat .config_wpe72 | grep -v "^CONFIG_WPE72_SIZE\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|CONFIG_SPUTNIK_APD" > .config
echo "CONFIG_BRANDING=y" >>.config
echo "CONFIG_ANTAIRA=y" >>.config
echo "CONFIG_UQMI=y" >>.config
echo "CONFIG_3G=y" >>.config
echo "CONFIG_COMGT=y" >>.config
echo "CONFIG_USB=y" >>.config
echo "CONFIG_USB_ADVANCED=y" >>.config
echo "CONFIG_OPENVPN=y" >>.config
#echo "CONFIG_STRONGSWAN=y" >>.config
echo "CONFIG_EHCI=y" >>.config
echo "CONFIG_WPE72_SIZE=0x7d0000" >>.config

make -f Makefile.pb42 kernel clean all install
TDIR=~/GruppenLW/releases/CUSTOMER/$DATE/antaira/compex_wpe72
mkdir -p ${TDIR}
cd ../../../
cp pb42/src/router/mips-uclibc/lsx-firmware.bin ${TDIR}/wpe72-firmware.bin
cp pb42/src/router/mips-uclibc/wpe72.img ${TDIR}/wpe72-image.tftp


