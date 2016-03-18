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
cp .config_wzrag300nh .config
echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
echo "CONFIG_DEFAULT_COUNTRYCODE=$1" >> .config
echo "$2" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wzr-hp-ag300h
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wzr-600dhp
cd ../../../
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/$DATE/buffalo_wzr-hp-ag300h/wzr-hp-ag300h-dd-wrt-webupgrade-$1.bin
cp pb42/src/router/mips-uclibc/wzrag300h-firmware_$1.enc ~/GruppenLW/releases/$DATE/buffalo_wzr-hp-ag300h/buffalo_to_ddwrt_webflash-$1.bin

cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/$DATE/buffalo_wzr-600dhp/wzr-600dhp-dd-wrt-webupgrade-$1.bin
cp pb42/src/router/mips-uclibc/wzr_600dhp-firmware_$1.enc ~/GruppenLW/releases/$DATE/buffalo_wzr-600dhp/buffalo_to_ddwrt_webflash-$1.bin
