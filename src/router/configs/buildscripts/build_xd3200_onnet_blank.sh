#!/bin/sh -x
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

# Yuncore XD3200
cat .config_dir869 | grep -v "^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_ROUTERSTYLE\|^CONFIG_BLUE\|^CONFIG_YELLOW\|^CONFIG_CYAN\|^CONFIG_RED\|^CONFIG_GREEN\|^CONFIG_NOTRIAL|^CONFIG_NOCAT\|CONFIG_MC\|CONFIG_SNORT\|CONFIG_ASTERISK\|CONFIG_FREERADIUS" > .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_ONNET=y" >> .config
echo "CONFIG_ONNET_BLANK=y" >> .config
echo "CONFIG_IPERF=n" >> .config
echo "CONFIG_SUPERCHANNEL=y" >> .config
echo "CONFIG_DIR859=y" >> .config
echo "CONFIG_ATH10K=y" >> .config
echo "CONFIG_XD3200=y" >> .config
echo "CONFIG_REGISTER=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/onnet_blank/yuncore-xd3200
#cd ../../../
cp mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/onnet_blank/yuncore-xd3200/yuncore-xd3200-webflash.bin
cp mips-uclibc/aligned.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/onnet_blank/yuncore-xd3200/factory-to-ddwrt.bin

# Yuncore SR3200
cat .config_dir869 | grep -v "^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_ROUTERSTYLE\|^CONFIG_BLUE\|^CONFIG_YELLOW\|^CONFIG_CYAN\|^CONFIG_RED\|^CONFIG_GREEN\|^CONFIG_NOTRIAL|^CONFIG_NOCAT\|CONFIG_MC\|CONFIG_SNORT\|CONFIG_ASTERISK\|CONFIG_FREERADIUS" > .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_ONNET=y" >> .config
echo "CONFIG_ONNET_BLANK=y" >> .config
echo "CONFIG_IPERF=n" >> .config
echo "CONFIG_SUPERCHANNEL=y" >> .config
echo "CONFIG_DIR859=y" >> .config
echo "CONFIG_XD3200=y" >> .config
echo "CONFIG_SR3200=y" >> .config
echo "CONFIG_ATH10K=y" >> .config
echo "CONFIG_USB=y" >> .config
echo "CONFIG_USB_ADVANCED=y" >> .config
echo "CONFIG_SAMBA=y" >> .config
echo "CONFIG_SAMBA3=y" >> .config
echo "CONFIG_REGISTER=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/onnet_blank/yuncore-sr3200
cp mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/onnet_blank/yuncore-sr3200/yuncore-sr3200-webflash.bin
cp mips-uclibc/aligned.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/onnet_blank/yuncore-sr3200/factory-to-ddwrt.bin

# Yuncore CPE890
cat .config_dir869 | grep -v "^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_ROUTERSTYLE\|^CONFIG_BLUE\|^CONFIG_YELLOW\|^CONFIG_CYAN\|^CONFIG_RED\|^CONFIG_GREEN\|^CONFIG_NOTRIAL|^CONFIG_NOCAT\|CONFIG_MC\|CONFIG_SNORT\|CONFIG_ASTERISK\|CONFIG_FREERADIUS" > .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_ONNET=y" >> .config
echo "CONFIG_ONNET_BLANK=y" >> .config
echo "CONFIG_IPERF=n" >> .config
echo "CONFIG_SUPERCHANNEL=y" >> .config
echo "CONFIG_DIR859=y" >> .config
echo "CONFIG_XD3200=y" >> .config
echo "CONFIG_CPE890=y" >> .config
echo "CONFIG_ATH10K=y" >> .config
echo "CONFIG_REGISTER=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/onnet/yuncore-cpe890
cp mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/onnet_blank/yuncore-cpe890/yuncore-cpe890-webflash.bin
cp mips-uclibc/aligned.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/onnet_blank/yuncore-cpe890/factory-to-ddwrt.bin

cd ../../..
