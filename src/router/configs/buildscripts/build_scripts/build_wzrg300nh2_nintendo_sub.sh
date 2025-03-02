#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)

REV=$(svnversion -n pb42/src/router/httpd)
REV+="-"
REV+=$(date +%Y-%m-%d)

export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-12.2.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.1.2-uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_wzrg300nh2_nintendo .config
echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
echo "CONFIG_DEFAULT_COUNTRYCODE=$1" >> .config
echo "$2" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/nintendo/buffalo_wzr_hp_g300nh2
cd ../../../
#cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr_hp_ag300h/wzr-hp-ag300h-dd-wrt-webupgrade-$1.bin
cp pb42/src/router/mips-uclibc/wzrg300nh2-firmware_$1.enc ~/GruppenLW/releases/CUSTOMER/$DATE/nintendo/buffalo_wzr_hp_g300nh2/wzr_hp_g300nh2-$1.bin
cp pb42/src/router/mips-uclibc/wzrg300nh2-firmware.tftp ~/GruppenLW/releases/CUSTOMER/$DATE/nintendo/buffalo_wzr_hp_g300nh2/wzr_hp_g300nh2-$1.tftp
cp customer/buffalo/changelog.txt ~/GruppenLW/releases/CUSTOMER/$DATE/nintendo/buffalo_wzr_hp_g300nh2/
cd /GruppenLW/releases/CUSTOMER/$DATE/nintendo && zip -9 -u $REV-buffalo_wzr_hp_g300nh2.zip buffalo_wzr_hp_g300nh2/*.bin buffalo_wzr_hp_g300nh2/*.txt
cd /home/seg/DEV
