#!/bin/sh
ps aux | grep -i -e build -e make
printf "really build?(y/n)"
read y
[ "$y" != "y" ] && exit 1

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
SMALL=""
#SMALL="\|^CONFIG_FREERADIUS\|^CONFIG_NAS\|^CONFIG_EOP_TUNNEL\|^CONFIG_NOCAT\|^CONFIG_WIFIDOG\|^CONFIG_OVERCLOCKING\|^CONFIG_CHILLILOCAL\|^CONFIG_USBIP\|^CONFIG_EOP_TUNNEL\|^HEARTBEAT_SUPPORT\|^CONFIG_HEARTBEAT\|^CONFIG_L2TP\|^PARENTAL_CONTROL_SUPPORT\|^CONFIG_PPPOESERVER\|^CONFIG_HOTSPOT\|^CONFIG_PRIVOXY\|^CONFIG_PPPOERELAY\|^CONFIG_RFLOW\|^CONFIG_TOR\|^CONFIG_RFLOW\|^CONFIG_PARENTAL_CONTROL\|^CONFIG_WOL\|^CONFIG_QUAGGA"

#cat .config_lima | grep -v "^CONFIG_WPE72_SIZE\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|CONFIG_ELEGANT\|CONFIG_BLUE\|CONFIG_YELLOW\|CONFIG_CYAN\|CONFIG_GREEN\|CONFIG_SPUTNIK_APD\|CONFIG_SWCONFIG=y\|CONFIG_SUPERCHANNEL\|CONFIG_FTP\|CONFIG_NTFS3G\|CONFIG_SAMBA\|CONFIG_SAMBA3\|CONFIG_RTPPROXY\|CONFIG_OPENSER\|CONFIG_MILKFISH\|CONFIG_MINIDLNA${SMALL}" > .config
#working small
cat .config_lima | grep -v "^CONFIG_WPE72_SIZE\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|CONFIG_ELEGANT\|CONFIG_BLUE\|CONFIG_YELLOW\|CONFIG_CYAN\|CONFIG_GREEN\|CONFIG_SPUTNIK_APD\|CONFIG_SWCONFIG=y\|CONFIG_SUPERCHANNEL\|CONFIG_FTP\|CONFIG_NTFS3G\|CONFIG_SAMBA\|CONFIG_SAMBA3\|CONFIG_RTPPROXY\|CONFIG_OPENSER\|CONFIG_MILKFISH\|CONFIG_MINIDLNA\|CONFIG_STATUS_GPIO\|^CONFIG_NOMESSAGE" > .config
#cat .config_lima > .config
echo "CONFIG_NOMESSAGE=y" >> .config
if [ x$1 = xggew ]
then
        BDIR=ggew
        echo "CONFIG_ANTAIRA=y" >>.config
        echo "CONFIG_POUND=y" >>.config
        echo "CONFIG_OPENSSL=y" >>.config
	echo "CONFIG_LIBMBIM=y" >>.config
#	echo "CONFIG_WPE72_SIZE=0x7d0000" >>.config
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
#	echo "CONFIG_WPE72_SIZE=0x7d0000" >>.config
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
	echo "# CONFIG_LIGHTTPD is not set" >>.config
	echo "CONFIG_OPENVPN=y" >>.config
#echo "CONFIG_STRONGSWAN=y" >>.config
	echo "CONFIG_EHCI=y" >>.config
#	echo "CONFIG_WPE72_SIZE=0x7d0000" >>.config
	echo "CONFIG_WIREGUARD=y" >>.config
	echo "CONFIG_I2C=y" >>.config
	echo "CONFIG_PERU=y" >>.config
	echo "CONFIG_I2C_GPIO_CUSTOM=y" >>.config
	echo "CONFIG_HWCLOCK=y" >>.config
	echo "CONFIG_GPIOWATCHER=y" >>.config
	echo "CONFIG_WPA3=y" >>.config
	echo "CONFIG_UBOOTENV=y" >>.config
	echo "CONFIG_CHRONY=y" >> .config
	echo "CONFIG_I2CTOOLS=y" >> .config
	echo "CONFIG_ANTAIRA_AGENT=y" >> .config
if [ "x$2" != "xnombim" ] 
then
	echo "CONFIG_LIBMBIM=y" >>.config
fi
	#06-12-2019 - lima missing gps
        echo "CONFIG_GPSD=y" >>.config
        echo "CONFIG_GPSI=y" >>.config
	echo "CONFIG_BUSYBOX_UDHCPC=y" >>.config
fi

echo CONFIG_AP135=y >> .config
echo CONFIG_AP143=y >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_WPA3=y" >> .config
#make -f Makefile.pb42 kernel clean glib20-configure glib20 libmbim-configure libmbim all install
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/antaira/peru
cd ../../../

cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/antaira/peru/factory-to-ddwrt.bin
cp pb42/src/router/mips-uclibc/ap93-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/antaira/peru/peru-webflash.bin
