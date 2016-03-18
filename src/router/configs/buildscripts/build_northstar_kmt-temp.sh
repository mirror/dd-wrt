#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n northstar/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH

export PATH=/xfs/toolchains/toolchain-arm_cortex-a9_gcc-5.3.0_musl-1.1.14_eabi/bin:$OLDPATH
cd northstar/src/router
[ -n "$DO_UPDATE" ] && svn update
#cp .config_laguna-small .config
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/asus-rt-ac56u
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/asus-rt-ac68u
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/asus-rt-n18u
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/netgear-r7000
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/netgear-r6300v2
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/netgear-r6250
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/buffalo_wzr-1750dhp
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/buffalo_wxr-1900dhp
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/buffalo_wzr-1166dhp
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/buffalo_wzr-900dhp
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/buffalo_wzr-600dhp2
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/dlink-dir868l
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/linksys-ea6900
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/linksys-ea6700
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/linksys-ea6500v2
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/trendnet-811DRU
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/trendnet-812DRUv2
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/trendnet-818DRU



(cat .config_northstar | grep -v "^CONFIG_SQUID\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_NOCAT\|CONFIG_MC\|CONFIG_SNORT" ; printf "CONFIG_TMK=y\nCONFIG_NOTRIAL=y\nCONFIG_REGISTER=y\n" )>.config
echo "CONFIG_SMP=y" >> .config
make -f Makefile.northstar kernel clean all install
#mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/northstar
cd ../../../
#cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/northstar
cp northstar/src/router/arm-uclibc/asus_rt-ac56u-firmware.trx ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/asus-rt-ac56u
cp northstar/src/router/arm-uclibc/asus_rt-ac68u-firmware.trx ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/asus-rt-ac68u

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/buffalo_wzr-1750dhp/buffalo-wzr-1750dhp-webflash.bin
cp northstar/src/router/arm-uclibc/buffalo-1750.encold ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/buffalo_wzr-1750dhp/factory-to-dd-wrt.bin

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/buffalo_wxr-1900dhp/buffalo-wxr-1900dhp-webflash.bin
cp northstar/src/router/arm-uclibc/buffalo-1900.encold ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/buffalo_wxr-1900dhp/factory-to-dd-wrt.bin

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/buffalo_wzr-1166dhp/buffalo-wzr-1166dhp-webflash.bin
cp northstar/src/router/arm-uclibc/buffalo-1166.encold ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/buffalo_wzr-1166dhp/factory-to-dd-wrt.bin


cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/netgear-r7000/netgear-r7000-webflash.bin
cp northstar/src/router/arm-uclibc/K3_R7000.chk ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/netgear-r7000/factory-to-dd-wrt.chk

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/netgear-r6300v2/netgear-r6300v2-webflash.bin
cp northstar/src/router/arm-uclibc/K3_R6300V2.chk ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/netgear-r6300v2/factory-to-dd-wrt.chk

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/netgear-r6250/netgear-r6250-webflash.bin
cp northstar/src/router/arm-uclibc/K3_R6250.chk ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/netgear-r6250/factory-to-dd-wrt.chk

cp northstar/src/router/arm-uclibc/webflash-dir868.trx ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/dlink-dir868l/dir868-webflash.bin

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/linksys-ea6900/linksys-ea6900-webflash.bin
cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/linksys-ea6500v2/linksys-ea6500v2-webflash.bin
cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/linksys-ea6700/linksys-ea6700-webflash.bin


cd northstar/src/router
(cat .config_northstar_mini | grep -v "^CONFIG_SQUID\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_NOCAT\|CONFIG_MC\|CONFIG_SNORT" ; printf "CONFIG_TMK=y\nCONFIG_NOTRIAL=y\nCONFIG_REGISTER=y\n" )>.config
make -f Makefile.northstar kernel clean all install
cd ../../../
cp northstar/src/router/arm-uclibc/web-dir868.img ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/dlink-dir868l/factory-to-ddwrt.bin
cp northstar/src/router/arm-uclibc/tnet812.trx ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/trendnet-812DRUv2/trendnet-812dru-webflash.bin


cd northstar/src/router
(cat .config_northstar_16m | grep -v "^CONFIG_SQUID\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_NOCAT\|CONFIG_MC\|CONFIG_SNORT" ; printf "CONFIG_TMK=y\nCONFIG_NOTRIAL=y\nCONFIG_REGISTER=y\n" )>.config
make -f Makefile.northstar kernel clean all install
cd ../../../

cp northstar/src/router/arm-uclibc/tnet818.trx ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/trendnet-818DRU/trendnet-818dru-webflash.bin


cd northstar/src/router
(cat .config_northstar_mini | grep -v "^CONFIG_SQUID\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_NOCAT\|CONFIG_MC\|CONFIG_SNORT" ; printf "CONFIG_TMK=y\nCONFIG_NOTRIAL=y\nCONFIG_REGISTER=y\n" )>.config
echo "CONFIG_NORTHSTAR_NOSMP=y" >> .config
make -f Makefile.northstar kernel clean all install
cd ../../../
cp northstar/src/router/arm-uclibc/tnet811.trx ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/trendnet-811DRU/trendnet-811dru-webflash.bin


cd northstar/src/router

(cat .config_northstar | grep -v "^CONFIG_SQUID\|^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_NOCAT\|CONFIG_MC\|CONFIG_SNORT" ; printf "CONFIG_TMK=y\nCONFIG_NOTRIAL=y\nCONFIG_REGISTER=y\n" )>.config

echo "CONFIG_NORTHSTAR_NOSMP=y" >> .config

make -f Makefile.northstar kernel clean all install
cd ../../../


cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/buffalo_wzr-900dhp/buffalo-wzr-900dhp-webflash.bin
cp northstar/src/router/arm-uclibc/buffalo-900.encold ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/buffalo_wzr-900dhp/factory-to-dd-wrt.bin
cp northstar/src/router/arm-uclibc/buffalo-900.enc ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/buffalo_wzr-900dhp/factory-to-dd-wrt-new.bin

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/buffalo_wzr-600dhp2/buffalo-wzr-600dhp2-webflash.bin
cp northstar/src/router/arm-uclibc/buffalo-600.encold ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/buffalo_wzr-600dhp2/factory-to-dd-wrt.bin
cp northstar/src/router/arm-uclibc/buffalo-600.enc ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/buffalo_wzr-600dhp2/factory-to-dd-wrt-new.bin

cp northstar/src/router/arm-uclibc/asus_rt-n18u-firmware.trx ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/asus-rt-n18u
                                



