#!/bin/sh
OLDPATH=$PATH
DATE="CUSTOMER/"$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n mvebu/src/router/httpd)
DATE+="/speedchecker"
export PATH=/xfs/toolchains/toolchain-arm_cortex-a9+vfpv3_gcc-8.2.0_musl_eabi/bin:$OLDPATH
cd mvebu/src/router
cat .config_wrt1900ac | grep -v "^CONFIG_MINIDLNA\|CONFIG_ASTERISK" > .config
echo  "CONFIG_SPEEDCHECKER=y" >>.config
echo  "CONFIG_SPEEDCHECKER_DEBUG=y" >>.config
make -f Makefile.mvebu kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/linksys-wrt1200ac
mkdir -p ~/GruppenLW/releases/$DATE/linksys-wrt1200acv2
mkdir -p ~/GruppenLW/releases/$DATE/linksys-wrt1900ac
mkdir -p ~/GruppenLW/releases/$DATE/linksys-wrt1900acs
mkdir -p ~/GruppenLW/releases/$DATE/linksys-wrt1900acsv2
mkdir -p ~/GruppenLW/releases/$DATE/linksys-wrt3200acm
mkdir -p ~/GruppenLW/releases/$DATE/linksys-wrt1900acv2
cd ../../../
cp mvebu/src/router/arm-uclibc/ddwrt-linksys-wrt1900ac.bin ~/GruppenLW/releases/$DATE/linksys-wrt1900ac/ddwrt-linksys-wrt1900ac-webflash.bin
cp mvebu/src/router/arm-uclibc/ddwrt-mvebu-armada-xp-mamba-squashfs-factory.img ~/GruppenLW/releases/$DATE/linksys-wrt1900ac/factory-to-ddwrt.bin


cp mvebu/src/router/arm-uclibc/ddwrt-linksys-wrt1900acs.bin ~/GruppenLW/releases/$DATE/linksys-wrt1900acs/ddwrt-linksys-wrt1900acs-webflash.bin
cp mvebu/src/router/arm-uclibc/ddwrt-mvebu-armada-385-shelby-squashfs-factory.img ~/GruppenLW/releases/$DATE/linksys-wrt1900acs/factory-to-ddwrt.bin

cp mvebu/src/router/arm-uclibc/ddwrt-linksys-wrt1900acs.bin ~/GruppenLW/releases/$DATE/linksys-wrt1900acsv2/ddwrt-linksys-wrt1900acsv2-webflash.bin
cp mvebu/src/router/arm-uclibc/ddwrt-mvebu-armada-385-shelby-squashfs-factory.img ~/GruppenLW/releases/$DATE/linksys-wrt1900acsv2/factory-to-ddwrt.bin


cp mvebu/src/router/arm-uclibc/ddwrt-linksys-wrt1900acv2.bin ~/GruppenLW/releases/$DATE/linksys-wrt1900acv2/ddwrt-linksys-wrt1900acv2-webflash.bin
cp mvebu/src/router/arm-uclibc/ddwrt-mvebu-armada-385-cobra-squashfs-factory.img ~/GruppenLW/releases/$DATE/linksys-wrt1900acv2/factory-to-ddwrt.bin


cp mvebu/src/router/arm-uclibc/ddwrt-linksys-wrt1200ac.bin ~/GruppenLW/releases/$DATE/linksys-wrt1200ac/ddwrt-linksys-wrt1200ac-webflash.bin
cp mvebu/src/router/arm-uclibc/ddwrt-mvebu-armada-385-caiman-squashfs-factory.img ~/GruppenLW/releases/$DATE/linksys-wrt1200ac/factory-to-ddwrt.bin

cp mvebu/src/router/arm-uclibc/ddwrt-linksys-wrt1200ac.bin ~/GruppenLW/releases/$DATE/linksys-wrt1200acv2/ddwrt-linksys-wrt1200acv2-webflash.bin
cp mvebu/src/router/arm-uclibc/ddwrt-mvebu-armada-385-caiman-squashfs-factory.img ~/GruppenLW/releases/$DATE/linksys-wrt1200acv2/factory-to-ddwrt.bin

cp mvebu/src/router/arm-uclibc/ddwrt-linksys-wrt3200acm.bin ~/GruppenLW/releases/$DATE/linksys-wrt3200acm/ddwrt-linksys-wrt3200acm-webflash.bin
cp mvebu/src/router/arm-uclibc/ddwrt-mvebu-armada-385-rango-squashfs-factory.img ~/GruppenLW/releases/$DATE/linksys-wrt3200acm/factory-to-ddwrt.bin

