#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/bkm/carambola2

cd pb42/src/router
#cp .config_carambola .config
cat .config_carambola | grep -v "^CONFIG_FREERADIUS\|^CONFIG_MINIDLNA\|^CONFIG_MC\|CONFIG_VNCREPEATER^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_NOCAT" >.config

echo "CONFIG_BKM=y" >> .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_NOTRIAL=y" >>.config
#echo "CONFIG_SERCD=y" >>.config
echo "CONFIG_NLD=y" >>.config
echo "CONFIG_GPIOWATCHER=y" >>.config
echo "CONFIG_STATUS_GPIO=y" >>.config
echo "CONFIG_IBSS_RSN=y" >>.config
echo "CONFIG_AP=y" >>.config
#echo "HOSTAPDVERSION=20130807" >>.config

make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/bkm/carambola2/factory-flash-uimage.bin
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/bkm/carambola2/dd-wrt-webflash.bin

