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
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ac56u
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ac68u
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r7000
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6300v2
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6250
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wzr-1750dhp
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wzr-1166dhp
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wzr-900dhp
mkdir -p ~/GruppenLW/releases/$DATE/buffalo_wzr-600dhp2
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir868l
mkdir -p ~/GruppenLW/releases/$DATE/linksys-ea6900
mkdir -p ~/GruppenLW/releases/$DATE/linksys-ea6700
mkdir -p ~/GruppenLW/releases/$DATE/linksys-ea6500v2
mkdir -p ~/GruppenLW/releases/$DATE/trendnet-811DRU
mkdir -p ~/GruppenLW/releases/$DATE/trendnet-812DRUv2
mkdir -p ~/GruppenLW/releases/$DATE/trendnet-818DRU



cp .config_northstar .config
echo "CONFIG_SMP=y" >> .config
make -f Makefile.northstar kernel clean all install
#mkdir -p ~/GruppenLW/releases/$DATE/northstar
cd ../../../
#cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/northstar
cp northstar/src/router/arm-uclibc/asus_rt-ac56u-firmware.trx ~/GruppenLW/releases/$DATE/asus-rt-ac56u
cp northstar/src/router/arm-uclibc/asus_rt-ac68u-firmware.trx ~/GruppenLW/releases/$DATE/asus-rt-ac68u

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/buffalo_wzr-1750dhp/buffalo-wzr-1750dhp-webflash.bin
cp northstar/src/router/arm-uclibc/buffalo-1750.encold ~/GruppenLW/releases/$DATE/buffalo_wzr-1750dhp/factory-to-dd-wrt.bin

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/buffalo_wzr-1166dhp/buffalo-wzr-1166dhp-webflash.bin
cp northstar/src/router/arm-uclibc/buffalo-1166.encold ~/GruppenLW/releases/$DATE/buffalo_wzr-1166dhp/factory-to-dd-wrt.bin


cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/netgear-r7000/netgear-r7000-webflash.bin
cp northstar/src/router/arm-uclibc/K3_R7000.chk ~/GruppenLW/releases/$DATE/netgear-r7000/factory-to-dd-wrt.chk

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/netgear-r6300v2/netgear-r6300v2-webflash.bin
cp northstar/src/router/arm-uclibc/K3_R6300V2.chk ~/GruppenLW/releases/$DATE/netgear-r6300v2/factory-to-dd-wrt.chk

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/netgear-r6250/netgear-r6250-webflash.bin
cp northstar/src/router/arm-uclibc/K3_R6250.chk ~/GruppenLW/releases/$DATE/netgear-r6250/factory-to-dd-wrt.chk

cp northstar/src/router/arm-uclibc/webflash-dir868.trx ~/GruppenLW/releases/$DATE/dlink-dir868l/dir868-webflash.bin

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/linksys-ea6900/linksys-ea6900-webflash.bin
cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/linksys-ea6500v2/linksys-ea6500v2-webflash.bin
cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/linksys-ea6700/linksys-ea6700-webflash.bin


