#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.1.2-uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBeam_M2-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBeam_M5-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoStation_M5-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Rocket_M2-Titanium-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-Titanium-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-X3-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Airgrid_M5-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Loco_M5-XW

cp .config_ubntm .config
echo "CONFIG_UBNTXW=y" >> .config
echo "CONFIG_LOCOXW=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBeam_M2-XW/ubnt_NanoBeam_M2-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBeam_M2-XW/XW-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBeam_M5-XW/ubnt_NanoBeam_M5-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBeam_M5-XW/XW-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Airgrid_M5-XW/ubnt_Airgrid_M5-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Airgrid_M5-XW/XW-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M5-XW/ubnt_Loco_M5-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M5-XW/XW-DD-WRT.bin

#cd pb42/src/router
#cp .config_ubntm .config
#echo "CONFIG_UBNTXW=y" >> .config
#make -f Makefile.pb42 kernel clean all install
#cd ../../../

#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_M5-XW/ubnt_NanoStation_M5-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_M5-XW/XW-DD-WRT.bin

#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M2-Titanium-XW/ubnt_Rocket_M2-Titanium-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M2-Titanium-XW/XW-DD-WRT.bin

#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-Titanium-XW/ubnt_Rocket_M5-Titanium-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-Titanium-XW/XW-DD-WRT.bin

#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-X3-XW/ubnt_Rocket_M5-X3-XW-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-X3-XW/XW-DD-WRT.bin

