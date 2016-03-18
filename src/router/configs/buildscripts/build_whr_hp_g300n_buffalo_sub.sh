#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
[ -n "$CONFIG_KERNEL_ELF_CORE" ] && DATE+="-DEBUG"

REV=$(svnversion -n pb42/src/router/httpd)
REV+="-"
REV+=$(date +%Y-%m-%d)

export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.1.2-uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_whr_hp_gn_buffalo .config
echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
echo "CONFIG_DEFAULT_COUNTRYCODE=$1" >> .config
[ -n "$CONFIG_KERNEL_ELF_CORE" ] && echo "CONFIG_KERNEL_ELF_CORE=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_hp_g300n
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_hp_gn
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_g300nv2
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_hp_g300n_SA
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_hp_gn_SA
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_g300nv2_SA
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_whr_g300nv2
cd ../../../
#cp pb42/src/router/mips-uclibc/ap93-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_hp_gn/whr-hp-gn-firmware-$1.bin
cp pb42/src/router/mips-uclibc/whr-hp-gn-firmware_$1.enc ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_hp_gn/whr_hp_gn-$1.bin
cp customer/buffalo/changelog.txt ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_hp_gn/
cd ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo && zip -9 -u $REV-buffalo_whr_hp_gn.zip buffalo_whr_hp_gn/*.bin buffalo_whr_hp_gn/*.txt
#[ -n "$CONFIG_KERNEL_ELF_CORE" ] && cp pb42/src/router/hostapd-wps/wpad ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_hp_gn/
cd /home/seg/DEV
cd pb42/src/router

#cp .config_whr_hp_gn_buffalo .config
#echo "CONFIG_WHRHPG300N=y" >> .config
#echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
#echo "CONFIG_DEFAULT_COUNTRYCODE=$1" >> .config
#make -f Makefile.pb42 services-clean libutils-clean libutils install
#cd ../../../
#cp pb42/src/router/mips-uclibc/ap93-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_hp_g300n/whr-hp-g300n-firmware-$1.bin
#cp pb42/src/router/mips-uclibc/whr-hp-g300n-firmware_$1.enc ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_hp_g300n/whr_hp_g300n-$1.bin
#cp customer/buffalo/changelog.txt ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_hp_g300n/
#cd ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo && zip -9 -u $REV-buffalo_whr_hp_g300n.zip buffalo_whr_hp_g300n/*.bin buffalo_whr_hp_g300n/*.txt
#cd /home/seg/DEV

cd pb42/src/router

#cp .config_whr_hp_gn_buffalo .config
#echo "CONFIG_WHRG300NV2=y" >> .config
#echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
#echo "CONFIG_DEFAULT_COUNTRYCODE=$1" >> .config
#make -f Makefile.pb42 services-clean libutils-clean libutils install
#cd ../../../
#cp pb42/src/router/mips-uclibc/ap93-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_g300nv2/whr-g300nv2-firmware-$1.bin
#cp pb42/src/router/mips-uclibc/whr-g300n-firmware_$1.enc ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_g300nv2/whr_g300nv2-$1.bin
#cp customer/buffalo/changelog.txt ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_g300nv2/
#cd ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo && zip -9 -u $REV-buffalo_whr_g300nv2.zip buffalo_whr_g300nv2/*.bin buffalo_whr_g300nv2/*.txt
cd /home/seg/DEV
cd pb42/src/router

cp .config_whr_hp_gn_buffalo .config
echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
echo "CONFIG_DEFAULT_COUNTRYCODE=$1" >> .config
echo "CONFIG_BUFFALO_SA=y" >> .config
make -f Makefile.pb42 libutils-clean libutils buffalo_flash-clean buffalo_flash upnp-clean upnpc
make -f Makefile.pb42 kernel clean all install
cd ../../../
#cp pb42/src/router/mips-uclibc/whr-hp-gn-firmware_$1.enc ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_hp_gn_SA/whr_hp_gn-$1.bin
/root/firmware/pem/dosign.sh pb42/src/router/mips-uclibc/whr-hp-gn-firmware_$1.enc
cp pb42/src/router/mips-uclibc/whr-hp-gn-firmware_$1.enc.signed ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_hp_gn_SA/whr_hp_gn-$1.bin
cp customer/buffalo/changelog.txt ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_hp_gn_SA/
cd ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo && zip -9 -u $REV-buffalo_whr_hp_gn_SA.zip buffalo_whr_hp_gn_SA/*.bin buffalo_whr_hp_gn_SA/*.txt
cd /home/seg/DEV
cd pb42/src/router

cp .config_whr_hp_gn_buffalo .config
echo "CONFIG_WHRHPG300N=y" >> .config
echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
echo "CONFIG_DEFAULT_COUNTRYCODE=$1" >> .config
echo "CONFIG_BUFFALO_SA=y" >> .config
make -f Makefile.pb42 libutils-clean libutils bcuffalo_flash-clean buffalo_flash upnp-clean upnp
make -f Makefile.pb42 services-clean libutils-clean libutils install
cd ../../../
#cp pb42/src/router/mips-uclibc/whr-hp-g300n-firmware_$1.enc ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_hp_g300n_SA/whr_hp_g300n-$1.bin
/root/firmware/pem/dosign.sh pb42/src/router/mips-uclibc/whr-hp-g300n-firmware_$1.enc
cp pb42/src/router/mips-uclibc/whr-hp-g300n-firmware_$1.enc.signed ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_hp_g300n_SA/whr_hp_g300n-$1.bin
cp customer/buffalo/changelog.txt ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_hp_g300n_SA/
cd ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo && zip -9 -u $REV-buffalo_whr_hp_g300n_SA.zip buffalo_whr_hp_g300n_SA/*.bin buffalo_whr_hp_g300n_SA/*.txt
cd /home/seg/DEV

cd pb42/src/router

cp .config_whr_hp_gn_buffalo .config
echo "CONFIG_WHRG300NV2=y" >> .config
echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
echo "CONFIG_DEFAULT_COUNTRYCODE=$1" >> .config
echo "CONFIG_BUFFALO_SA=y" >> .config
make -f Makefile.pb42 services-clean libutils-clean libutils install
cd ../../../
cp pb42/src/router/mips-uclibc/whr-g300n-firmware_$1.enc ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_g300nv2_SA/whr_g300nv2-$1.bin
cp customer/buffalo/changelog.txt ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_whr_g300nv2_SA/
cd ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo && zip -9 -u $REV-buffalo_whr_g300nv2_SA.zip buffalo_whr_g300nv2_SA/*.bin buffalo_whr_g300nv2_SA/*.txt
cd /home/seg/DEV



