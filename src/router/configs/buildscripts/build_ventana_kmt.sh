#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ventana/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-arm_cortex-a9+neon_gcc-13.1.0_musl_eabi/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_v7-a_gcc-4.8-linaro_uClibc-0.9.33.2-eabi-imx6/bin:$OLDPATH
cd ventana/src/router
[ -n "$DO_UPDATE" ] && svn update
cat .config_ventana | grep -v "^CONFIG_SQUID\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_NOCAT\|CONFIG_MC\|CONFIG_SNORT\|CONFIG_LIGHTTPD\|CONFIG_ASTERISK\|CONFIG_AIRCRACK\|CONFIG_MINIDLNA\|CONFIG_WIFIDOG\|CONFIG_TOR\|CONFIG_TRANSMISSION\|CONFIG_WEBSERVER\|CONFIG_NMAP\|CONFIG_JAVA" > .config
echo "CONFIG_SMP=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_TMK=y" >>.config
echo "CONFIG_BRANDING=y" >>.config

echo "CONFIG_SERCD=y" >>.config
echo "CONFIG_GPSD=y" >>.config
echo "CONFIG_VNCREPEATER=y" >>.config
echo "CONFIG_GPSI=y" >>.config
echo "CONFIG_NLD=y" >>.config
echo "CONFIG_GPIOWATCHER=y" >>.config
echo "CONFIG_STATUS_GPIO=y" >>.config
echo "CONFIG_ATH5K=y" >>.config
echo "CONFIG_ATH5K_PCI=y" >>.config
echo "CONFIG_IBSS_RSN=y" >>.config
echo "CONFIG_DEBUG_SYSLOG=y" >>.config
#echo "CONFIG_MIITOOL=y" >>.config
echo "CONFIG_NSMD=y" >>.config
echo "CONFIG_SMBD=y" >>.config
echo "CONFIG_ETHPERF=y" >>.config
echo "CONFIG_ARPALERT=y" >>.config
echo "CONFIG_LSM=y" >>.config
#echo "CONFIG_DRIVER_WIRED=y" >>.config
echo "CONFIG_LIBMBIM=y" >>.config
echo "CONFIG_MBEDTLS=y" >>.config
echo "CONFIG_PICOCOM=y" >>.config
echo "CONFIG_QCA9984" >> .config
#cp .config_laguna .config
make -f Makefile.ventana kernel clean all install

DATE=CUSTOMER/${DATE}/kmt

mkdir -p ~/GruppenLW/releases/$DATE/gateworks-ventana-gw54xx
#mkdir -p ~/GruppenLW/releases/$DATE/gateworks-ventana-gw53xx
#mkdir -p ~/GruppenLW/releases/$DATE/gateworks-ventana-gw52xx
#mkdir -p ~/GruppenLW/releases/$DATE/gateworks-ventana-gw51xx
cd ../../../
cp ventana/src/router/arm-uclibc/root-aligned.ubi ~/GruppenLW/releases/$DATE/gateworks-ventana-gw54xx/gw54xx.ubi
cp ventana/src/router/arm-uclibc/webflash-ventana.trx ~/GruppenLW/releases/$DATE/gateworks-ventana-gw54xx/ventana-webflash.bin
#scp ventana/src/router/arm-uclibc/webflash-ventana.trx 10.20.30.104:/tmp/firmware.bin

#cp ventana/src/router/arm-uclibc/root-aligned.ubi ~/GruppenLW/releases/$DATE/gateworks-ventana-gw53xx/gw53xx.ubi
#cp ventana/src/router/arm-uclibc/root-aligned.ubi ~/GruppenLW/releases/$DATE/gateworks-ventana-gw52xx/gw52xx.ubi
#cp ventana/src/router/arm-uclibc/root-aligned.ubi ~/GruppenLW/releases/$DATE/gateworks-ventana-gw51xx/gw51xx.ubi

#cp notes/laguna/* ~/GruppenLW/releases/$DATE/gateworks_gw2388-16M

#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/pronghorn-SBC/pronghorn-firmware.bin
#cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/pronghorn-SBC/linux.bin


