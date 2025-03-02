#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-12.2.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.1.2-uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_wzrg300nh_nintendo .config
echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
echo "CONFIG_DEFAULT_COUNTRYCODE=$1" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/nintendo/buffalo_wzr-hp-g300nh
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/nintendo/buffalo_wzr-hp-g301nh
cd ../../../
cp pb42/src/router/mips-uclibc/ap83-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/nintendo/buffalo_wzr-hp-g300nh/wzr-hp-g300nh-dd-wrt-webupgrade-$1.bin
cp pb42/src/router/mips-uclibc/wzrg300nh-firmware_$1.enc ~/GruppenLW/releases/CUSTOMER/$DATE/nintendo/buffalo_wzr-hp-g300nh/buffalo_to_ddwrt_webflash-$1.bin
cp pb42/src/router/mips-uclibc/ap83-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/nintendo/buffalo_wzr-hp-g301nh/wzr-hp-g301nh-dd-wrt-webupgrade-$1.bin
cp pb42/src/router/mips-uclibc/wzrg301nh-firmware_$1.enc ~/GruppenLW/releases/CUSTOMER/$DATE/nintendo/buffalo_wzr-hp-g301nh/buffalo_to_ddwrt_webflash-$1.bin

