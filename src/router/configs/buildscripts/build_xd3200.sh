#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_dir869 .config
echo "CONFIG_DIR859=y" >> .config
echo "CONFIG_ATH10K=y" >> .config
echo "CONFIG_XD3200=y" >> .config
echo "CONFIG_REGISTER=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/yuncore-xd3200
cd ../../../
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/$DATE/yuncore-xd3200/yuncore-xd3200-webflash.bin
cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/yuncore-xd3200/factory-to-ddwrt.bin
