#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-12.2.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_wpe72 .config
cat .config_wpe72 | grep -v "^CONFIG_WPE72_SIZE\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|CONFIG_ELEGANT\|CONFIG_BLUE\|CONFIG_YELLOW\|CONFIG_CYAN\|CONFIG_GREEN\|CONFIG_SWCONFIG=y\|CONFIG_SUPERCHANNEL" > .config
#cat .config_wpe72 | grep -v "^CONFIG_WPE72_SIZE\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|CONFIG_ELEGANT\|CONFIG_BLUE\|CONFIG_YELLOW\|CONFIG_CYAN\|CONFIG_GREEN\|CONFIG_SPUTNIK_APD\|CONFIG_SWCONFIG=y\|CONFIG_SUPERCHANNEL\|CONFIG_FTP\|CONFIG_NTFS3G\|CONFIG_SAMBA\|CONFIG_SAMBA3\|CONFIG_RTPPROXY\|CONFIG_OPENSER\|CONFIG_MILKFISH" > .config
if [ x$1 = xggew ]
then
        BDIR=ggew
        echo "CONFIG_ANTAIRA=y" >>.config
        echo "CONFIG_POUND=y" >>.config
        echo "CONFIG_OPENSSL=y" >>.config
	echo "CONFIG_LIBMBIM=y" >>.config
	echo "CONFIG_WPE72_SIZE=0x7d0000" >>.config
        echo "CONFIG_IPV6=y" >>.config
        #echo "CONFIG_SAMBA=y" >>.config
elif [ x$1 = xkmt ]
then
        BDIR=kmt
	echo "CONFIG_BRANDING=y" >>.config
        echo "CONFIG_TMK=y" >>.config
        echo "CONFIG_OLSRD=y" >>.config
        echo "CONFIG_GPSD=y" >>.config
        echo "CONFIG_GPSI=y" >>.config
        echo "CONFIG_ELEGANT=y" >>.config
        echo "CONFIG_BLUE=y" >>.config
        echo "CONFIG_YELLOW=y" >>.config
        echo "CONFIG_CYAN=y" >>.config
        echo "CONFIG_GREEN=y" >>.config
        echo "CONFIG_MBEDTLS=y" >>.config
        echo "CONFIG_IPV6=y" >>.config
        echo "CONFIG_SNMP=y" >>.config
	echo "CONFIG_COMGT=y" >>.config
	echo "CONFIG_USB=y" >>.config
	echo "CONFIG_USB_ADVANCED=y" >>.config
	echo "CONFIG_EHCI=y" >>.config
	echo "CONFIG_LIBMBIM=y" >>.config
	echo "CONFIG_WIREGUARD=y" >>.config
	echo "CONFIG_WPE72_SIZE=0x7d0000" >>.config
#       echo "CONFIG_SAMBA=y" >>.config
else
        BDIR=antaira
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
	echo "CONFIG_WIREGUARD=y" >>.config
fi

make -f Makefile.pb42 kernel clean all install
TDIR=~/GruppenLW/releases/CUSTOMER/$DATE/${BDIR}/compex_wpe72
mkdir -p ${TDIR}
cd ../../../
cp pb42/src/router/mips-uclibc/lsx-firmware.bin ${TDIR}/wpe72-firmware.bin
cp pb42/src/router/mips-uclibc/wpe72.img ${TDIR}/wpe72-image.tftp


