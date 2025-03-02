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
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/antaira/ams2111-mini

cd pb42/src/router
#cp .config_carambola .config
#cat .config_carambola | grep -v "^CONFIG_FREERADIUS\|^CONFIG_MINIDLNA\|^CONFIG_MC\|CONFIG_VNCREPEATER\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_NOCAT\|^CONFIG_NOMESSAGE" >.config
cat .config_carambola | grep -v "^CONFIG_WPE72_SIZE\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_ELEGANT\|^CONFIG_BLUE\|^CONFIG_YELLOW\|^CONFIG_CYAN\|^CONFIG_GREEN\|^CONFIG_SPUTNIK_APD\|^CONFIG_SWCONFIG=y\|^CONFIG_SUPERCHANNEL\|^CONFIG_FTP\|^CONFIG_NTFS3G\|^CONFIG_SAMBA\|^CONFIG_SAMBA3\|^CONFIG_RTPPROXY\|^CONFIG_OPENSER\|^CONFIG_MILKFISH\|^CONFIG_MINIDLNA\|^CONFIG_FREERADIUS\|^CONFIG_NAS\|^CONFIG_HOTSPOT\|^CONFIG_PARENTAL_CONTROL\|^CONFIG_SOFTETHER\|^CONFIG_TOR\|^CONFIG_PRIVOXY\|^CONFIG_USBIP\|^CONFIG_NOCAT\|^CONFIG_SCHEDULER\|^CONFIG_PPTPD\|^CONFIG_USB\|^CONFIG_LIGHTTPD\|^CONFIG_MACTELNET\|^CONFIG_RFLOW\|^CONFIG_MIKROTIK_BTEST\|^HEARTBEAT_SUPPORT\|^CONFIG_3G\|^CONFIG_HEARTBEAT\|^CONFIG_WEBSERVER\|^CONFIG_STATUS_GPIO" > .config

echo "CONFIG_FMS2111=y" >> .config
echo "CONFIG_ANTAIRA=y" >> .config
echo "CONFIG_ANTAIRA_MINI=y" >> .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_NOTRIAL=y" >>.config
#echo "CONFIG_SERCD=y" >>.config
#echo "CONFIG_NLD=y" >>.config
echo "CONFIG_GPIOWATCHER=y" >>.config
#echo "CONFIG_STATUS_GPIO=y" >>.config
echo "CONFIG_IBSS_RSN=y" >>.config
echo "CONFIG_AP=y" >>.config
echo "CONFIG_I2C_GPIO_CUSTOM=y" >>.config
echo "CONFIG_HWCLOCK=y" >>.config
 echo "CONFIG_WPA3=y" >> .config
echo "# CONFIG_STATUS_GPIO is not set" >> .config
#echo "HOSTAPDVERSION=20130807" >>.config
#echo "CONFIG_NOMESSAGE=n"

make -f Makefile.pb42 kernel clean all install
cd ../../../
echo "=> /GruppenLW/releases/CUSTOMER/$DATE/antaira/ams2111-mini/"
cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/antaira/ams2111-mini/factory-flash-uimage.bin
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/antaira/ams2111-mini/dd-wrt-webflash.bin

