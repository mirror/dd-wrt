#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n northstar/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH

export PATH=/xfs/toolchains/toolchain-arm_cortex-a9_gcc-13.1.0_musl_eabi/bin:$OLDPATH
cd northstar/src/router
[ -n "$DO_UPDATE" ] && svn update
#cp .config_laguna-small .config
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ac56u
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ac68u
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ac1900p
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ac88u
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ac5300
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ac3200
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ac3100
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-n18u
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6700
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6700v3
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r7000
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6300v2
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6250
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6400
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6400v2
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r6400v2otp
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r8000
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r7000P
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r8500
mkdir -p ~/GruppenLW/releases/$DATE/netgear-ex6200
mkdir -p ~/GruppenLW/releases/$DATE/netgear-ac1450
mkdir -p ~/GruppenLW/releases/$DATE/buffalo-wzr-1750dhp
mkdir -p ~/GruppenLW/releases/$DATE/buffalo-wxr-1900dhp
mkdir -p ~/GruppenLW/releases/$DATE/buffalo-wzr-1166dhp
mkdir -p ~/GruppenLW/releases/$DATE/buffalo-wzr-900dhp
mkdir -p ~/GruppenLW/releases/$DATE/buffalo-wzr-600dhp2
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir868l-reva
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir868l-revb
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir868l-revc
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir860l-a1
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir880l
mkdir -p ~/GruppenLW/releases/$DATE/linksys-ea6900
mkdir -p ~/GruppenLW/releases/$DATE/linksys-ea6300
mkdir -p ~/GruppenLW/releases/$DATE/linksys-ea6350
mkdir -p ~/GruppenLW/releases/$DATE/linksys-ea6400
mkdir -p ~/GruppenLW/releases/$DATE/linksys-ea6700
mkdir -p ~/GruppenLW/releases/$DATE/linksys-ea6500v2
mkdir -p ~/GruppenLW/releases/$DATE/trendnet-811DRU
mkdir -p ~/GruppenLW/releases/$DATE/trendnet-812DRUv2
mkdir -p ~/GruppenLW/releases/$DATE/trendnet-818DRU
mkdir -p ~/GruppenLW/releases/$DATE/trendnet-828DRU
mkdir -p ~/GruppenLW/releases/$DATE/tplink-archer-c9v1
mkdir -p ~/GruppenLW/releases/$DATE/tplink-archer-c9v2
mkdir -p ~/GruppenLW/releases/$DATE/tplink-archer-c9v3
mkdir -p ~/GruppenLW/releases/$DATE/tplink-archer-c9v4
mkdir -p ~/GruppenLW/releases/$DATE/tplink-archer-c1900
mkdir -p ~/GruppenLW/releases/$DATE/tplink-archer-c8v1
mkdir -p ~/GruppenLW/releases/$DATE/tplink-archer-c8v2
mkdir -p ~/GruppenLW/releases/$DATE/tplink-archer-c8v3
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir890l
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir895l
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir885l


cp .config_northstar .config
echo "CONFIG_SMP=y" >> .config
echo "CONFIG_SOFTETHER=y" >> .config
echo "CONFIG_SPEEDCHECKER=y" >> .config
echo "CONFIG_IRQBALANCE=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_RAID=y" >> .config
echo "CONFIG_SMBD=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
echo "CONFIG_HTOP=y" >> .config
echo "CONFIG_IPSET=y" >> .config
echo "CONFIG_PROFILING=y" >> .config

make -f Makefile.northstar kernel clean all install
#mkdir -p ~/GruppenLW/releases/$DATE/northstar
cd ../../../
#cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/northstar
cp northstar/src/router/arm-uclibc/asus_rt-ac56u-firmware.trx ~/GruppenLW/releases/$DATE/asus-rt-ac56u

cp northstar/src/router/arm-uclibc/asus_rt-ac68u-firmware.trx ~/GruppenLW/releases/$DATE/asus-rt-ac68u

cp northstar/src/router/arm-uclibc/asus_rt-ac68u-firmware.trx ~/GruppenLW/releases/$DATE/asus-rt-ac1900p/asus_rt-ac1900p-firmware.trx

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/buffalo-wzr-1750dhp/buffalo-wzr-1750dhp-webflash.bin
cp northstar/src/router/arm-uclibc/buffalo-1750.encold ~/GruppenLW/releases/$DATE/buffalo-wzr-1750dhp/factory-to-dd-wrt.bin

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/buffalo-wxr-1900dhp/buffalo-wxr-1900dhp-webflash.bin
cp northstar/src/router/arm-uclibc/buffalo-1900.encold ~/GruppenLW/releases/$DATE/buffalo-wxr-1900dhp/factory-to-dd-wrt.bin

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/buffalo-wzr-1166dhp/buffalo-wzr-1166dhp-webflash.bin
cp northstar/src/router/arm-uclibc/buffalo-1166.encold ~/GruppenLW/releases/$DATE/buffalo-wzr-1166dhp/factory-to-dd-wrt.bin


cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/netgear-r7000/netgear-r7000-webflash.bin
cp northstar/src/router/arm-uclibc/K3_R7000.chk ~/GruppenLW/releases/$DATE/netgear-r7000/factory-to-dd-wrt.chk

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/netgear-r6700/netgear-r6700-webflash.bin
cp northstar/src/router/arm-uclibc/K3_R6700.chk ~/GruppenLW/releases/$DATE/netgear-r6700/factory-to-dd-wrt.chk

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/netgear-r6300v2/netgear-r6300v2-webflash.bin
cp northstar/src/router/arm-uclibc/K3_R6300V2.chk ~/GruppenLW/releases/$DATE/netgear-r6300v2/factory-to-dd-wrt.chk

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/netgear-r6250/netgear-r6250-webflash.bin
cp northstar/src/router/arm-uclibc/K3_R6250.chk ~/GruppenLW/releases/$DATE/netgear-r6250/factory-to-dd-wrt.chk

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/netgear-ac1450/netgear-ac1450-webflash.bin
cp northstar/src/router/arm-uclibc/K3_AC1450.chk ~/GruppenLW/releases/$DATE/netgear-ac1450/factory-to-dd-wrt.chk


cp northstar/src/router/arm-uclibc/webflash-dir868.trx ~/GruppenLW/releases/$DATE/dlink-dir868l-reva/dir868a-webflash.bin
cp northstar/src/router/arm-uclibc/webflash-dir868b.trx ~/GruppenLW/releases/$DATE/dlink-dir868l-revb/dir868b-webflash.bin
cp northstar/src/router/arm-uclibc/webflash-dir868c.trx ~/GruppenLW/releases/$DATE/dlink-dir868l-revc/dir868c-webflash.bin
cp northstar/src/router/arm-uclibc/webflash-dir860.trx ~/GruppenLW/releases/$DATE/dlink-dir860l-a1/dir860-webflash.bin

cp northstar/src/router/arm-uclibc/webflash-dir880.trx ~/GruppenLW/releases/$DATE/dlink-dir880l/dir880-webflash.bin

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/linksys-ea6900/linksys-ea6900-webflash.bin
cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/linksys-ea6300/linksys-ea6300-webflash.bin
cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/linksys-ea6350/linksys-ea6350-webflash.bin
cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/linksys-ea6400/linksys-ea6400-webflash.bin
cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/linksys-ea6500v2/linksys-ea6500v2-webflash.bin
cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/linksys-ea6700/linksys-ea6700-webflash.bin


