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
#cp .config_mms344 .config
cat .config_mms344 | grep -v "^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_ROUTERSTYLE\|^CONFIG_BLUE\|^CONFIG_YELLOW\|^CONFIG_CYAN\|^CONFIG_RED\|^CONFIG_GREEN\|^CONFIG_NOTRIAL\|^CONFIG_NOCAT\|^CONFIG_MC\|^CONFIG_SNORT\|^CONFIG_ASTERISK\|^CONFIG_FREERADIUS\|^CONFIG_UQMI\|^CONFIG_OPENDPI\|^CONFIG_OPENVPN\|^CONFIG_3G\|^CONFIG_COMGT\|^CONFIG_EOP_TUNNEL\|^CONFIG_OVERCLOCKING\|^CONFIG_CHILLILOCAL\|^CONFIG_SPUTNIK_APD\|^CONFIG_SPUTNIK_PRO\|^CONFIG_NOCAT\|^CONFIG_HOTSPOT\|^CONFIG_OPENSER\|^CONFIG_MILKFISH\|^CONFIG_CHILLISPOT\|^CONFIG_QUAGGA" > .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_ONNET=y" >> .config
echo "CONFIG_IPERF=n" >> .config
echo "CONFIG_SUPERCHANNEL=y" >> .config
echo "CONFIG_IPERF=y" >> .config
echo "CONFIG_WPA3=n" >> .config
echo "CONFIG_MAC80211_MESH=n" >> .config
echo "CONFIG_ATH10K_CT=y" >> .config
#echo "CONFIG_MAC80211_MESH=y" >> .config
#echo "CONFIG_TCPDUMP=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/compex-mms344
cp mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/compex-mms344/mms344-firmware.bin
cp mips-uclibc/aligned.uimage  ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/compex-mms344/mms344-uimage.bin
#cp pb42/src/router/mips-uclibc/dir825c1-uimage.bin ~/GruppenLW/releases/$DATE/dlink-dir825-c1/factory-to-ddwrt_NA.bin
cd ../../../


