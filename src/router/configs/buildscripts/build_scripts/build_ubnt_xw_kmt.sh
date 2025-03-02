#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-12.2.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.1.2-uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../

NBM5=~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt_NanoBeam_M5-XW
NBM5M400=~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt_NanoBeam_M5-M400-XW
PBM5M400=~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt_PowerBeam_M5-M400-XW
NSM5=~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt_NanoStation_M5-XW
NSM2=~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt_NanoStation_M2-XW
AGM5=~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt_Airgrid_M5-XW
LM5=~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt_Loco_M5-XW
UAPLRXW=~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt_UAP_LR_v2-XW

mkdir -p $NBM5
mkdir -p $NSM5
mkdir -p $NSM2
mkdir -p $NBM5M400
mkdir -p $PBM5M400
mkdir -p $AGM5
mkdir -p $LM5
mkdir -p $UAPLRXW

##cp .config_ubntm .config
#cat .config_ubntm | grep -v "^CONFIG_JFFS2\|^CONFIG_SQUID\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_NOCAT\|CONFIG_MC\|CONFIG_SNORT\|^CONFIG_WPA3" > .config
#echo "CONFIG_UBNTXW=y" >> .config
#echo "CONFIG_TMK=y" >> .config
#echo "CONFIG_BRANDING=y" >> .config
#echo "CONFIG_WPA3=y" >> .config
#echo "CONFIG_DEBUG_SYSLOG=y" >> .config
#echo "HOSTAPDVERSION=2015-03-25-neu" >> .config
#
#
#make -f Makefile.pb42 kernel clean all install
#cd ../../../
##cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBeam_M2-XW/ubnt_NanoBeam_M2-XW-webflash-firmware.bin
##cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBeam_M2-XW/XW-DD-WRT.bin
#
#
#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin $NSM5/ubnt_NanoStation_M5-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin $NSM5/XW-to-KMT-und-tftp.bin
#
#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin $NSM2/ubnt_NanoStation_M2-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin $NSM2/XW-to-KMT-und-tftp.bin
#
#cd pb42/src/router
#echo "CONFIG_LOCOXW=y" >> .config
#make -f Makefile.pb42 kernel clean all install
#cd ../../../
#
#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin $NBM5/ubnt_NanoBeam_M5-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin $NBM5/XW-to-KMT-und-tftp.bin
#touch $NBM5/LOCOXW
#
#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin $AGM5/ubnt_Airgrid_M5-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin $AGM5/XW-to-KMT-und-tftp.bin
#touch $AGM5/LOCOXW
#
#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin $LM5/ubnt_Loco_M5-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin $LM5/XW-to-KMT-und-tftp.bin
#touch $LM5/LOCOXW
#
#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin $UAPLRXW/UniFiAPv2-webflash-firmware.bin
#cp pb42/src/router/ubnt_mtd/mtd $UAPLRXW/flash-upgrade.bin
#cp notes/unifi/install_uapv2* $UAPLRXW
#touch $UAPLRXW/LOCOXW
#
#exit

cd pb42/src/router
cat .config_ubntm | grep -v "^CONFIG_JFFS2\|^CONFIG_SQUID\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_NOCAT\|CONFIG_MC\|CONFIG_SNORT" > .config
echo "CONFIG_UBNTXW=y" >> .config
echo "CONFIG_TMK=y" >> .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_M400XW=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin $NBM5M400/ubnt_NanoBeam_M5-M400-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin $NBM5M400/XW-to-KMT-und-tftp.bin
cp pb42/src/router/ubnt_mtd/mtd2 $NBM5M400/flash-upgrade.bin
touch $NBM5/LOCOXW

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin $PBM5M400/ubnt_PowerBeam_M5-M400-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin $PBM5M400/XW-to-KMT-und-tftp.bin
cp pb42/src/router/ubnt_mtd/mtd2 $PBM5M400/flash-upgrade.bin
touch $NBM5/LOCOXW


#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M2-Titanium-XW/ubnt_Rocket_M2-Titanium-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M2-Titanium-XW/XW-DD-WRT.bin

#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-Titanium-XW/ubnt_Rocket_M5-Titanium-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-Titanium-XW/XW-DD-WRT.bin

#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-X3-XW/ubnt_Rocket_M5-X3-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-X3-XW/XW-DD-WRT.bin

#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Airgrid_M5-XW/ubnt_Airgrid_M5-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Airgrid_M5-XW/XW-DD-WRT.bin

#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M5-XW/ubnt_Loco_M5-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M5-XW/XW-DD-WRT.bin

