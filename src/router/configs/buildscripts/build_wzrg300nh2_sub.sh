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
cp .config_wzrg300nh2 .config
echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
echo "CONFIG_DEFAULT_COUNTRYCODE=$1" >> .config
echo "$2" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wzr-hp-g300nh2
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wzr-300hp
cd ../../../
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/$DATE/buffalo_wzr-hp-g300nh2/wzr-hp-g300nh2-dd-wrt-webupgrade-$1.bin
cp pb42/src/router/mips-uclibc/wzrg300nh2-firmware_$1.enc ~/GruppenLW/releases/$DATE/buffalo_wzr-hp-g300nh2/buffalo_to_ddwrt_webflash-$1.bin


cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/$DATE/buffalo_wzr-300hp/wzr-300hp-dd-wrt-webupgrade-$1.bin
cp pb42/src/router/mips-uclibc/wzr_300hp-firmware_$1.enc ~/GruppenLW/releases/$DATE/buffalo_wzr-300hp/buffalo_to_ddwrt_webflash-$1.bin

