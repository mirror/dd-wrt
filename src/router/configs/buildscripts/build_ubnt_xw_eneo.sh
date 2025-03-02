#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.1.2-uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
#mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-NanoBeam_M2-XW
#mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-NanoBeam_M5-XW
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-NanoStation_M5-XW
#mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-Rocket_M2-Titanium-XW
#mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-Rocket_M5-Titanium-XW
#mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-Rocket_M5-X3-XW
#mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-Airgrid_M5-XW
#mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-Loco_M5-XW

#cp .config_ubntm .config
cat .config_ubntm | grep -v "^CONFIG_JFFS2\|^CONFIG_SQUID\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_NOCAT\|CONFIG_MC\|CONFIG_SNORT" > .config
echo "CONFIG_GPIOWATCHER=y" >> .config
echo "CONFIG_UBNTXW=y" >> .config
echo "CONFIG_NOTRIAL=y" >>.config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_ENEO=y" >> .config

make -f Makefile.pb42 kernel clean all install
cd ../../../
#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-NanoBeam_M2-XW/ubnt-NanoBeam_M2-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-NanoBeam_M2-XW/XW-DD-WRT.bin
#
#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-NanoBeam_M5-XW/ubnt-NanoBeam_M5-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-NanoBeam_M5-XW/XW-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-NanoStation_M5-XW/ubnt-NanoStation_M5-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-NanoStation_M5-XW/XW-DD-WRT.bin

#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-Rocket_M2-Titanium-XW/ubnt-Rocket_M2-Titanium-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-Rocket_M2-Titanium-XW/XW-DD-WRT.bin
#
#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-Rocket_M5-Titanium-XW/ubnt-Rocket_M5-Titanium-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-Rocket_M5-Titanium-XW/XW-DD-WRT.bin
#
#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-Rocket_M5-X3-XW/ubnt-Rocket_M5-X3-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-Rocket_M5-X3-XW/XW-DD-WRT.bin
#
#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-Airgrid_M5-XW/ubnt-Airgrid_M5-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-Airgrid_M5-XW/XW-DD-WRT.bin
#
#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-Loco_M5-XW/ubnt-Loco_M5-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/CUSTOMER/$DATE/eneo/ubnt-Loco_M5-XW/XW-DD-WRT.bin

