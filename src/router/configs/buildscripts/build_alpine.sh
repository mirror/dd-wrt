#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n alpine/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-arm_cortex-a15+neon-vfpv4_gcc-8.2.0_musl_eabi/bin:$OLDPATH
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r9000
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r8900
cd alpine/src/router
cp .config_alpine .config
echo  "CONFIG_SPEEDCHECKER=y" >>.config
echo  "CONFIG_DDRESCUE=y" >>.config
echo "CONFIG_LSOF=y" >> .config
echo "CONFIG_MIKROTIK_BTEST=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_ZFS=y" >> .config
echo "CONFIG_SCREEN=y" >> .config
echo "CONFIG_STRACE=y" >>.config
echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_WPA3=y" >> .config
sed -i 's/CONFIG_QUAGGA=y/CONFIG_FRR=y/g' .config

#echo "KERNELVERSION=4.14" >> .config

make -f Makefile.alpine kernel clean all install

cd ../../../
#cp ipq806x/src/router/arm-uclibc/ddwrt-netgear-R7800.bin ~/GruppenLW/releases/$DATE/netgear-r9000/dd-wrt-webupgrade.bin
cp alpine/src/router/arm-uclibc/R9000-V1.0.1.36.img ~/GruppenLW/releases/$DATE/netgear-r9000/factory-to-ddwrt.img
cp alpine/src/router/arm-uclibc/r9000.bin ~/GruppenLW/releases/$DATE/netgear-r9000/r9000-webupgrade.bin


cp alpine/src/router/arm-uclibc/R8900-V1.0.1.36.img ~/GruppenLW/releases/$DATE/netgear-r8900/factory-to-ddwrt.img
cp alpine/src/router/arm-uclibc/r9000.bin ~/GruppenLW/releases/$DATE/netgear-r8900/r8900-webupgrade.bin

