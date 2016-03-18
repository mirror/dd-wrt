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
cp .config_whr_hp_gn .config
echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
echo "CONFIG_DEFAULT_COUNTRYCODE=$1" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_whr_hp_g300n
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_whr_hp_gn
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_whr_g300nv2
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wlae_ag300n
cd ../../../
cp pb42/src/router/mips-uclibc/ap93-firmware.bin ~/GruppenLW/releases/$DATE/buffalo_whr_hp_gn/whr-hp-gn-firmware-$1.bin
cp pb42/src/router/mips-uclibc/whr-hp-gn-firmware_$1.enc ~/GruppenLW/releases/$DATE/buffalo_whr_hp_gn/buffalo-to-dd-wrt_webflash-$1.bin
cd pb42/src/router

cp .config_whr_hp_gn .config
echo "CONFIG_WHRHPG300N=y" >> .config
echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
echo "CONFIG_DEFAULT_COUNTRYCODE=$1" >> .config
make -f Makefile.pb42 services-clean libutils-clean libutils install
cd ../../../
cp pb42/src/router/mips-uclibc/ap93-firmware.bin ~/GruppenLW/releases/$DATE/buffalo_whr_hp_g300n/whr-hp-g300n-firmware-$1.bin
cp pb42/src/router/mips-uclibc/whr-hp-g300n-firmware_$1.enc ~/GruppenLW/releases/$DATE/buffalo_whr_hp_g300n/buffalo-to-dd-wrt_webflash-$1.bin
cd pb42/src/router

cp .config_whr_hp_gn .config
echo "CONFIG_WHRG300NV2=y" >> .config
echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
echo "CONFIG_DEFAULT_COUNTRYCODE=$1" >> .config
make -f Makefile.pb42 services-clean libutils-clean libutils install
cd ../../../
cp pb42/src/router/mips-uclibc/ap93-firmware.bin ~/GruppenLW/releases/$DATE/buffalo_whr_g300nv2/whr-g300nv2-firmware-$1.bin
cp pb42/src/router/mips-uclibc/whr-g300n-firmware_$1.enc ~/GruppenLW/releases/$DATE/buffalo_whr_g300nv2/buffalo-to-dd-wrt_webflash-$1.bin

#cp pb42/src/router/mips-uclibc/whr-hp-gn-firmware.tftp ~/GruppenLW/releases/$DATE/buffalo_whr_g300nv2/whr-g300nv2-firmware-$1.tftp
cd pb42/src/router


cp .config_whr_hp_gn .config
echo "CONFIG_WLAEAG300N=y" >> .config
echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
echo "CONFIG_DEFAULT_COUNTRYCODE=$1" >> .config
make -f Makefile.pb42 services-clean libutils-clean libutils install
cd ../../../

cp pb42/src/router/mips-uclibc/ap93-firmware.bin ~/GruppenLW/releases/$DATE/buffalo_wlae_ag300n/wla-ag300-firmware-$1.bin
cp pb42/src/router/mips-uclibc/wlae-ag300n-firmware_$1.enc ~/GruppenLW/releases/$DATE/buffalo_wlae_ag300n/buffalo-to-dd-wrt_webflash-$1.bin
#cp pb42/src/router/mips-uclibc/whr-hp-gn-firmware.tftp ~/GruppenLW/releases/$DATE/buffalo_whr_g300nv2/whr-g300nv2-firmware-$1.tftp
cd pb42/src/router
