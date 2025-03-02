#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n laguna/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-arm_mpcore+vfp_gcc-12.1.0_musl_eabi/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_mpcore+vfp_gcc-4.8-linaro_musl-1.1.4_eabi/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_v6k_gcc-4.7-linaro_uClibc-0.9.33.2_eabi-7/bin:$OLDPATH
cd laguna/src/router
[ -n "$DO_UPDATE" ] && svn update
cat .config_laguna-newkernel | grep -v "^CONFIG_QUAGGA_STABLE\|^CONFIG_SNORT\|^CONFIG_MC\|^CONFIG_PHP\|^CONFIG_TOR\|^CONFIG_TRANSMISSION\|^CONFIG_ASTERISK\|CONFIG_PRIVOXY\|CONFIG_MINIDLNA\|CONFIG_UNBOUND\|CONFIG_USBIP\|CONFIG_FREERADIUS\|CONFIG_POUND\|CONFIG_SAMBA3\|CONFIG_IPTRAF\|CONFIG_NCURSES\|CONFIG_BATMANADV\|CONFIG_IPVS\|CONFIG_LIGHTTPD\|CONFIG_NOOPT\|CONFIG_VNCREPEATER\|CONFIG_NTFS3G\|CONFIG_FTP\|CONFIG_PWC\|CONFIG_ATH10K\|CONFIG_AIRCRACK\|CONFIG_WIFIDOG\|CONFIG_NSTX\|CONFIG_SPUTNIK_APD\|CONFIG_RTPPROXY\|CONFIG_OPENSER\|CONFIG_MILKFISH\|CONFIG_SFTPSERVER" >.config

echo "CONFIG_NEXTMEDIA=y" >> .config
echo "CONFIG_RAIEXTRA=y" >> .config
echo "CONFIG_BRANDING=y" >> .config
#echo "CONFIG_MIITOOL=y" >> .config
echo "CONFIG_IFL=y" >> .config
echo "CONFIG_SOFTFLOWD=y" >> .config
echo "CONFIG_ATH5K=y" >>.config
if [ x$1 = xsmall ]
then
	ADD=-SMALL-FOR-DEV-ONLY
else
	echo "CONFIG_ARPALERT=y" >>.config
	echo "CONFIG_SNMP-UTILS=y" >>.config
	echo "CONFIG_LINKS=y" >> .config
	echo "CONFIG_NRPE=y" >> .config
	echo "CONFIG_NMAP=y" >>.config
	echo "CONFIG_PYTHON=y" >>.config
fi
#echo "CONFIG_NEXTMEDIAEXTRA=y" >> .config
####
#echo "CONFIG_ATH5K=y" >> .config
#echo "CONFIG_ATH5K_PCI=y" >> .config
#echo "CONFIG_MAC80211_MESH=y" >> .config
####
####
#cp .config_laguna .config
echo "CONFIG_SMP=y" >> .config
make -f Makefile.laguna kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/nextmedia/gateworks_gw2388-32M${ADD}
cd ../../../
cp laguna/src/router/arm-uclibc/laguna-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/nextmedia/gateworks_gw2388-32M${ADD}
cp laguna/src/router/arm-uclibc/laguna-firmware.raw2 ~/GruppenLW/releases/CUSTOMER/$DATE/nextmedia/gateworks_gw2388-32M${ADD}/uImage.bin
#dd if=laguna/src/router/arm-uclibc/laguna-firmware.raw2 of=~/GruppenLW/releases/CUSTOMER/$DATE/nextmedia/gateworks_gw2388-32M/uImage_part1_bs131072.bin bs=131072 count=100 
#dd if=laguna/src/router/arm-uclibc/laguna-firmware.raw2 of=~/GruppenLW/releases/CUSTOMER/$DATE/nextmedia/gateworks_gw2388-32M/uImage_part2_bs131072.bin bs=131072 skip=100

cp notes/laguna/* ~/GruppenLW/releases/CUSTOMER/$DATE/nextmedia/gateworks_gw2388-32M${ADD}
#cat >~/GruppenLW/releases/CUSTOMER/$DATE/nextmedia/gateworks_gw2388-32M/laguna-big-flashing-instructions.txt << EOF
#tftpboot 0x100000 uImage_part1_bs131072.bin
#erase 0x10060000 +\$(filesize)
#cp.b 0x100000 0x10060000 \$(filesize)

#tftpboot 0x100000 uImage_part2_bs131072.bin
#erase 0x10CE0000 +\$(filesize)
#cp.b 0x100000 0x10CE0000 \$(filesize)
#EOF
