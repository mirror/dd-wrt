#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n laguna/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-arm_mpcore+vfp_gcc-13.1.0_musl_eabi/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_v6k_gcc-4.7-linaro_uClibc-0.9.33.2_eabi-7/bin:$OLDPATH
cd laguna/src/router
[ -n "$DO_UPDATE" ] && svn update
if [ x$1 = xmaksat ]
then
	DATE=CUSTOMER/${DATE}/maksat
	cat .config_laguna-small | grep -v "CONFIG_UQMI=y\|CONFIG_IPVS=y\|CONFIG_COOVA_CHILLI=y\|CONFIG_NMBD=y\|CONFIG_USBIP=y\|CONFIG_FREERADIUS=y\|CONFIG_FTP=y\|CONFIG_NTFS3G=y\|CONFIG_SAMBA3=y\|CONFIG_WIFIDOG=y" >.config
	echo "CONFIG_MAKSAT=y" >>.config
	echo "CONFIG_MAKSAT_CORAL=y" >>.config
	echo "CONFIG_BRANDING=y" >>.config
	echo "CONFIG_STRACE=y" >>.config
	echo "CONFIG_TCPDUMP=y" >>.config
else
	cat .config_laguna-small >.config
fi
echo "CONFIG_SMP=y" >> .config
echo "KERNELVERSION=4.9" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_ATH5K=y" >> .config
echo "CONFIG_IRQBALANCE=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_CAKE=y" >> .config
#echo "CONFIG_SMBD=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
echo "CONFIG_SPEEDTEST_CLI=y" >> .config
echo "CONFIG_MRP=y" >> .config
echo "CONFIG_CFM=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
#echo "CONFIG_NTFS3=y" >> .config
#echo "CONFIG_HTOP=y" >> .config
#echo "CONFIG_KERNELLTO=y" >> .config
##cp .config_laguna .config
make -f Makefile.laguna kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/gateworks-laguna-gw2388-16M
cd ../../../
cp laguna/src/router/arm-uclibc/laguna-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/gateworks-laguna-gw2388-16M
cp laguna/src/router/arm-uclibc/laguna-firmware.raw2 ~/GruppenLW/releases/$DATE/gateworks-laguna-gw2388-16M/uImage.bin

cp notes/laguna/* ~/GruppenLW/releases/$DATE/gateworks-laguna-gw2388-16M

#cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/pronghorn-SBC/pronghorn-firmware.bin
#cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/pronghorn-SBC/linux.bin


