#!/bin/sh
ps aux | grep -i -e build -e make
printf "really build?(y/n)"
read y
[ "$y" != "y" ] && exit 1

OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
SMALL=""
#working small
cat .config_lima | grep -v "^CONFIG_WPE72_SIZE\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|CONFIG_ELEGANT\|CONFIG_BLUE\|CONFIG_YELLOW\|CONFIG_CYAN\|CONFIG_GREEN\|CONFIG_SPUTNIK_APD\|CONFIG_SWCONFIG=y\|CONFIG_SUPERCHANNEL\|CONFIG_FTP\|CONFIG_NTFS3G\|CONFIG_SAMBA\|CONFIG_SAMBA3\|CONFIG_RTPPROXY\|CONFIG_OPENSER\|CONFIG_MILKFISH\|CONFIG_MINIDLNA\|CONFIG_FREERADIUS\|CONFIG_NAS\|CONFIG_HOTSPOT\|CONFIG_PARENTAL_CONTROL\|CONFIG_SOFTETHER\|CONFIG_TOR\|CONFIG_PRIVOXY\|CONFIG_USBIP\|CONFIG_NOCAT\|CONFIG_SCHEDULER\|CONFIG_PPTPD\|CONFIG_USB\|CONFIG_LIGHTTPD\|CONFIG_MACTELNET\|CONFIG_RFLOW\|CONFIG_MIKROTIK_BTEST\|HEARTBEAT_SUPPORT\|CONFIG_3G\|CONFIG_HEARTBEAT\|CONFIG_WEBSERVER\|CONFIG_STATUS_GPIO" > .config
BDIR=antaira

cat<<EOF >> .config
CONFIG_BRANDING=y
CONFIG_ANTAIRA=y
CONFIG_UQMI=y
# CONFIG_3G is not set
# CONFIG_COMGT=n
CONFIG_USB=y
# CONFIG_USB_ADVANCED is not set
# CONFIG_LIGHTTPD is not set
# CONFIG_OPENVPN is not set
CONFIG_EHCI=y
# CONFIG_WIREGUARD is not set
CONFIG_I2C=y
CONFIG_PERU=y
CONFIG_I2C_GPIO_CUSTOM=y
CONFIG_HWCLOCK=y
CONFIG_GPIOWATCHER=y
CONFIG_WPA3=y
CONFIG_UBOOTENV=y
CONFIG_AP135=y
CONFIG_AP143=y
CONFIG_ANTAIRA_MINI=y
CONFIG_ANTAIRA_AGENT=n
EOF

make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/antaira/peru-mini
cd ../../../

cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/antaira/peru-mini/factory-to-ddwrt-mini.bin
cp pb42/src/router/mips-uclibc/ap93-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/antaira/peru-mini/peru-webflash-mini.bin
