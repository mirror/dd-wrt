#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n mpc85xx/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-powerpc_8540_gcc-5.2.0_musl-1.1.11/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-powerpc_gcc-4.6-linaro_uClibc-0.9.33.2/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd mpc85xx/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wdr4900v1
cp .config_wdr4900 .config
make -f Makefile.mpc85xx kernel clean all install
cd ../../../
cp mpc85xx/src/router/powerpc-uclibc/wdr4900-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr4900v1/tl-wdr4900-webflash.bin
cp mpc85xx/src/router/powerpc-uclibc/tplink-wdr4900-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr4900v1/factory-to-ddwrt.bin

