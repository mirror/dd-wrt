#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-13.1.0_musl/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
#cp .config_xscale_cambria_kmt .config


#REMOVE="\|CONFIG_MINIDLNA"

cat .config_xscale_cambria | grep -v "^CONFIG_NOMESSAGE\|^CONFIG_SQUID\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_NOCAT\|CONFIG_MC\|CONFIG_SNORT\|CONFIG_LIGHTTPD\|CONFIG_ASTERISK\|CONFIG_AIRCRACK\|CONFIG_MINIDLNA\|CONFIG_WIFIDOG\|CONFIG_PHP\|CONFIG_PHPCGI\|CONFIG_TOR\|CONFIG_TRANSMISSION\|CONFIG_WEBSERVER\|CONFIG_NMAP" > .config

echo "KERNELVERSION=4.9" >> .config

echo "CONFIG_TMK=y" >>.config
echo "CONFIG_BRANDING=y" >>.config

echo "CONFIG_SERCD=y" >>.config
#echo "CONFIG_NPROBE=y" >>.config
#echo "CONFIG_NTPD=y" >>.config # compile fehler
#echo "CONFIG_PHP=y" >>.config
echo "CONFIG_GPSD=y" >>.config
echo "CONFIG_VNCREPEATER=y" >>.config
echo "CONFIG_GPSI=y" >>.config
echo "CONFIG_GPIOWATCHER=y" >>.config
echo "CONFIG_STATUS_GPIO=y" >>.config
echo "CONFIG_ATH5K=y" >>.config
echo "CONFIG_ATH5K_PCI=y" >>.config
echo "CONFIG_IBSS_RSN=y" >>.config
echo "CONFIG_DEBUG_SYSLOG=y" >>.config
echo "CONFIG_MIITOOL=y" >>.config
echo "CONFIG_NLD=y" >>.config
echo "CONFIG_NSMD=y" >>.config
echo "CONFIG_ETHPERF=y" >>.config
echo "CONFIG_ARPALERT=y" >>.config
echo "CONFIG_LSM=y" >>.config
echo "CONFIG_DRIVER_WIRED=y" >>.config
if [ "x$1" != "xnombim" ]
then
	echo "CONFIG_LIBMBIM=y" >>.config
	echo "CONFIG_NOMESSAGE=y" >> .config
fi
echo "CONFIG_MBEDTLS=y" >>.config
echo "CONFIG_PICOCOM=y" >>.config
echo "CONFIG_SCREEN=y" >>.config
echo "CONFIG_CHRONY=y" >>.config
make -f Makefile.armbe kernel clean httpd-clean libutils-clean services-clean all install
IDIR=~/GruppenLW/releases/CUSTOMER/$DATE/kmt/cambria_kmt
mkdir -p ${IDIR}
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ${IDIR}/cambria_kmt-firmware-squashfs.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ${IDIR}/linux_kmt.bin

echo build in ${IDIR}
