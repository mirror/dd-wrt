#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
#cp .config_dir859 .config
cat .config_dir859 | grep -v "^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_ROUTERSTYLE\|^CONFIG_BLUE\|^CONFIG_YELLOW\|^CONFIG_CYAN\|^CONFIG_RED\|^CONFIG_GREEN\|^CONFIG_NOTRIAL|^CONFIG_NOCAT\|CONFIG_MC\|CONFIG_SNORT\|CONFIG_ASTERISK\|CONFIG_FREERADIUS\|^CONFIG_UQMI\|^CONFIG_OPENDPI\|^CONFIG_OPENVPN\|^CONFIG_3G\|^CONFIG_COMGT\|^CONFIG_EOP_TUNNEL\|^CONFIG_OVERCLOCKING\|^CONFIG_CHILLILOCAL\|^CONFIG_SPUTNIK_APD\|^CONFIG_SPUTNIK_PRO\|^CONFIG_NOCAT\|^CONFIG_HOTSPOT\|^CONFIG_OPENSER\|^CONFIG_MILKFISH\|^CONFIG_CHILLISPOT\|^CONFIG_QUAGGA\|^CONFIG_ATH10K\|^CONFIG_MINIDLNA\|^CONFIG_IPETH\|^CONFIG_NMBD\|^CONFIG_NTFS3G\|^CONFIG_USBIP\|^CONFIG_VPNC\|^CONFIG_SAMBA3\|^CONFIG_TCPDUMP\|^CONFIG_SFTPSERVER\|^CONFIG_IPERF\|^CONFIG_VNCREPEATER\|^CONFIG_OLSRD\|^CONFIG_SAMBA\|^CONFIG_RADVD" > .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_ONNET=y" >> .config
echo "CONFIG_SUPERCHANNEL=y" >> .config
echo "CONFIG_REGISTER=y" >> .config
echo "CONFIG_DIR859=y" >> .config
#echo "CONFIG_WR615N=y" >> .config
echo "CONFIG_XD9531=y" >> .config
echo "CONFIG_REGISTER=y" >> .config
sed -i 's/CONFIG_ATH10K=y/# CONFIG_ATH10K is not set/g' .config
sed -i 's/CONFIG_CPUTEMP=y/# CONFIG_CPUTEMP is not set/g' .config

make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/xd9531
echo "~/GruppenLW/releases/CUSTOMER/$DATE/onnet/xd9531"
cd ../../../
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/xd9531/xd9531-webflash.bin
cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/xd9531/firmware.bin
