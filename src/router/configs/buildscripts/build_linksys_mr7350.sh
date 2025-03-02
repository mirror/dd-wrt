#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ipq60xx/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-aarch64_cortex-a53_gcc-14.2.0_musl/bin:$OLDPATH
cd ipq60xx/src/router
[ -n "$DO_UPDATE" ] && svn update
cp configs/ipq60xx/.config_mr7350 .config
#echo "CONFIG_KERNELDEBUG=y" >> .config

make -f Makefile.ipq60xx kernel clean all install

mkdir -p ~/GruppenLW/releases/$DATE/linksys-mr7350
mkdir -p ~/GruppenLW/releases/$DATE/linksys-mr7500
mkdir -p ~/GruppenLW/releases/$DATE/linksys-mx4200-v1
mkdir -p ~/GruppenLW/releases/$DATE/linksys-mx4200-v2
mkdir -p ~/GruppenLW/releases/$DATE/linksys-mx4300
mkdir -p ~/GruppenLW/releases/$DATE/linksys-mx5300
mkdir -p ~/GruppenLW/releases/$DATE/linksys-mx8500
mkdir -p ~/GruppenLW/releases/$DATE/linksys-mx5500
mkdir -p ~/GruppenLW/releases/$DATE/linksys-mr5500
mkdir -p ~/GruppenLW/releases/$DATE/dynalink-dl-wrx36
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ax89x
mkdir -p ~/GruppenLW/releases/$DATE/fortinet-fap-231f
mkdir -p ~/GruppenLW/releases/$DATE/buffalo-wxr-5950ax12
mkdir -p ~/GruppenLW/releases/$DATE/buffalo-wxr-6000ax12s
mkdir -p ~/GruppenLW/releases/$DATE/buffalo-wxr-6000ax12b
mkdir -p ~/GruppenLW/releases/$DATE/buffalo-wxr-6000ax12p

cd ../../../
cp ipq60xx/src/router/aarch64-uclibc/linksys_mr7350-webflash.bin ~/GruppenLW/releases/$DATE/linksys-mr7350/dd-wrt-webflash.bin
cp ipq60xx/src/router/aarch64-uclibc/linksys_mr7500-webflash.bin ~/GruppenLW/releases/$DATE/linksys-mr7500/dd-wrt-webflash.bin
#scp ipq60xx/src/router/aarch64-uclibc/Image-factory-mr7350.bin 10.88.193.134:/srv/tftpboot/mr7350.img
#scp ipq60xx/src/router/aarch64-uclibc/linksys_mr7350-webflash.bin 10.88.193.94:/tmp/firmware.bin

cp ipq60xx/src/router/aarch64-uclibc/dynalink_dlwrx36.ubi ~/GruppenLW/releases/$DATE/dynalink-dl-wrx36/factory-to-ddwrt.ubi
cp ipq60xx/src/router/aarch64-uclibc/dynalink_dlwrx36-webflash.bin ~/GruppenLW/releases/$DATE/dynalink-dl-wrx36/dd-wrt-webflash.bin

cp ipq60xx/src/router/aarch64-uclibc/fortinet_fap231f.ubi ~/GruppenLW/releases/$DATE/fortinet-fap-231f/factory-to-ddwrt.ubi
cp ipq60xx/src/router/aarch64-uclibc/fortinet_fap231f-webflash.bin ~/GruppenLW/releases/$DATE/fortinet-fap-231f/dd-wrt-webflash.bin
cp notes/fap231/flashing.txt ~/GruppenLW/releases/$DATE/fortinet-fap-231f/
#scp ipq60xx/src/router/aarch64-uclibc/linksys_mr7350-webflash.bin 10.88.193.93:/tmp/firmware.bin

cd ipq60xx/src/router

cp configs/ipq60xx/.config_mx4200_preflash .config
#echo "CONFIG_KERNELDEBUG=y" >> .config

make -f Makefile.ipq60xx kernel clean all install

cd ../../../


cp ipq60xx/src/router/aarch64-uclibc/Image-factory-mr5500.bin ~/GruppenLW/releases/$DATE/linksys-mr5500/factory-to-ddwrt.img
cp ipq60xx/src/router/aarch64-uclibc/Image-factory-mx5500.bin ~/GruppenLW/releases/$DATE/linksys-mx5500/factory-to-ddwrt.img
cp ipq60xx/src/router/aarch64-uclibc/Image-factory-mx4300.bin ~/GruppenLW/releases/$DATE/linksys-mx4300/factory-to-ddwrt.img
cp ipq60xx/src/router/aarch64-uclibc/Image-factory-mx4200v1.bin ~/GruppenLW/releases/$DATE/linksys-mx4200-v1/factory-to-ddwrt.img
cp ipq60xx/src/router/aarch64-uclibc/Image-factory-mx4200v2.bin ~/GruppenLW/releases/$DATE/linksys-mx4200-v2/factory-to-ddwrt.img
cp ipq60xx/src/router/aarch64-uclibc/Image-factory-mr7350.bin ~/GruppenLW/releases/$DATE/linksys-mr7350/factory-to-ddwrt.img
cp ipq60xx/src/router/aarch64-uclibc/Image-factory-mr7500.bin ~/GruppenLW/releases/$DATE/linksys-mr7500/factory-to-ddwrt.img
#echo "this is untested, need serial bootlog for finishing" > ~/GruppenLW/releases/$DATE/linksys-mx4300/note.txt

cd ipq60xx/src/router

cp configs/ipq60xx/.config_mx4200 .config
#echo "CONFIG_KERNELDEBUG=y" >> .config


make -f Makefile.ipq60xx kernel clean all install
cd ../../../

cp ipq60xx/src/router/aarch64-uclibc/linksys_mx4200v2-webflash.bin ~/GruppenLW/releases/$DATE/linksys-mx4200-v2/dd-wrt-webflash.bin
cp ipq60xx/src/router/aarch64-uclibc/linksys_mx4200v1-webflash.bin ~/GruppenLW/releases/$DATE/linksys-mx4200-v1/dd-wrt-webflash.bin
cp ipq60xx/src/router/aarch64-uclibc/linksys_mx4300-webflash.bin ~/GruppenLW/releases/$DATE/linksys-mx4300/dd-wrt-webflash.bin

cp ipq60xx/src/router/aarch64-uclibc/linksys_mx5500-webflash.bin ~/GruppenLW/releases/$DATE/linksys-mx5500/dd-wrt-webflash.bin
cp ipq60xx/src/router/aarch64-uclibc/linksys_mr5500-webflash.bin ~/GruppenLW/releases/$DATE/linksys-mr5500/dd-wrt-webflash.bin
cp ipq60xx/src/router/aarch64-uclibc/linksys_mx8500-webflash.bin ~/GruppenLW/releases/$DATE/linksys-mx8500/dd-wrt-webflash.bin
cp ipq60xx/src/router/aarch64-uclibc/Image-factory-mx8500.bin ~/GruppenLW/releases/$DATE/linksys-mx8500/factory-to-ddwrt.img

cp ipq60xx/src/router/aarch64-uclibc/linksys_mx5300-webflash.bin ~/GruppenLW/releases/$DATE/linksys-mx5300/dd-wrt-webflash.bin
cp ipq60xx/src/router/aarch64-uclibc/Image-factory-mx5300.bin ~/GruppenLW/releases/$DATE/linksys-mx5300/factory-to-ddwrt.img

#scp ipq60xx/src/router/aarch64-uclibc/dynalink_dlwrx36-webflash.bin  172.29.0.90:/tmp/firmware.bin


cd ipq60xx/src/router

echo "CONFIG_AX89=y" >> .config
echo "CONFIG_NUT=y" >> .config
make -f Makefile.ipq60xx kernel clean all install
cd ../../../

#cp ipq60xx/src/router/aarch64-uclibc/linksys_mx4200v2-webflash.bin ~/GruppenLW/releases/$DATE/linksys-mx4200-v2/dd-wrt-webflash.bin
#cp ipq60xx/src/router/aarch64-uclibc/linksys_mx4200v1-webflash.bin ~/GruppenLW/releases/$DATE/linksys-mx4200-v1/dd-wrt-webflash.bin

#cp ipq60xx/src/router/aarch64-uclibc/Image-factory-mr5500.bin ~/GruppenLW/factory-to-ddwrt.bin
#scp ipq60xx/src/router/aarch64-uclibc/Image-factory-mr5500.bin 10.88.193.134:/srv/tftpboot/MR5500.img
#scp ipq60xx/src/router/aarch64-uclibc/linksys_mr5500-webflash.bin 10.88.193.94:/tmp/firmware.bin
#cp ipq60xx/src/router/aarch64-uclibc/dynalink_dlwrx36.ubi ~/GruppenLW/releases/$DATE/dynalink-dl-wrx36/factory-to-ddwrt.ubi
#cp ipq60xx/src/router/aarch64-uclibc/dynalink_dlwrx36-webflash.bin ~/GruppenLW/releases/$DATE/dynalink-dl-wrx36/dynalink_dlwrx36-webflash.bin
#scp ipq60xx/src/router/aarch64-uclibc/linksys_mx5500-webflash.bin 172.29.0.188:/tmp/firmware.bin
#scp ipq60xx/src/router/aarch64-uclibc/linksys_mx4200v1-webflash.bin 10.88.193.93:/tmp/firmware.bin
#scp ipq60xx/src/router/aarch64-uclibc/dynalink_dlwrx36-webflash.bin  172.29.0.90:/tmp/firmware.bin

cp ipq60xx/src/router/aarch64-uclibc/asus-rt-ax89x-upgrade.trx ~/GruppenLW/releases/$DATE/asus-rt-ax89x/factory-to-ddwrt.trx
cp ipq60xx/src/router/aarch64-uclibc/asus-rt-ax89x-upgrade.bin ~/GruppenLW/releases/$DATE/asus-rt-ax89x/dd-wrt-webflash.bin



cd ipq60xx/src/router
cp configs/ipq60xx/.config_buffalo_wxr .config
echo "CONFIG_NUT=y" >> .config

make -f Makefile.ipq60xx kernel clean all install
cd ../../../

#cp ipq60xx/src/router/aarch64-uclibc/linksys_mx4200v2-webflash.bin ~/GruppenLW/releases/$DATE/linksys-mx4200-v2/dd-wrt-webflash.bin
#cp ipq60xx/src/router/aarch64-uclibc/linksys_mx4200v1-webflash.bin ~/GruppenLW/releases/$DATE/linksys-mx4200-v1/dd-wrt-webflash.bin

#cp ipq60xx/src/router/aarch64-uclibc/Image-factory-mr5500.bin ~/GruppenLW/factory-to-ddwrt.bin
#scp ipq60xx/src/router/aarch64-uclibc/Image-factory-mr5500.bin 10.88.193.134:/srv/tftpboot/MR5500.img
#scp ipq60xx/src/router/aarch64-uclibc/linksys_mr5500-webflash.bin 10.88.193.94:/tmp/firmware.bin
#cp ipq60xx/src/router/aarch64-uclibc/dynalink_dlwrx36.ubi ~/GruppenLW/releases/$DATE/dynalink-dl-wrx36/factory-to-ddwrt.ubi
#cp ipq60xx/src/router/aarch64-uclibc/dynalink_dlwrx36-webflash.bin ~/GruppenLW/releases/$DATE/dynalink-dl-wrx36/dynalink_dlwrx36-webflash.bin
#scp ipq60xx/src/router/aarch64-uclibc/linksys_mx5500-webflash.bin 172.29.0.188:/tmp/firmware.bin
#scp ipq60xx/src/router/aarch64-uclibc/linksys_mx4200v1-webflash.bin 10.88.193.93:/tmp/firmware.bin
#scp ipq60xx/src/router/aarch64-uclibc/dynalink_dlwrx36-webflash.bin  172.29.0.90:/tmp/firmware.bin

cp ipq60xx/src/router/aarch64-uclibc/buffalo-wxr-5950ax12.ubi ~/GruppenLW/releases/$DATE/buffalo-wxr-5950ax12/factory-to-ddwrt.ubi
cp ipq60xx/src/router/aarch64-uclibc/buffalo-wxr-5950ax12.enc ~/GruppenLW/releases/$DATE/buffalo-wxr-5950ax12/factory-to-ddwrt.enc
cp ipq60xx/src/router/aarch64-uclibc/buffalo-wxr-5950ax12-webflash.bin ~/GruppenLW/releases/$DATE/buffalo-wxr-5950ax12/buffalo-wxr-5950ax12-webflash.bin

cp ipq60xx/src/router/aarch64-uclibc/buffalo-wxr-6000ax12s.ubi ~/GruppenLW/releases/$DATE/buffalo-wxr-6000ax12s/factory-to-ddwrt.ubi
cp ipq60xx/src/router/aarch64-uclibc/buffalo-wxr-6000ax12s.enc ~/GruppenLW/releases/$DATE/buffalo-wxr-6000ax12s/factory-to-ddwrt.enc
cp ipq60xx/src/router/aarch64-uclibc/buffalo-wxr-6000ax12s-webflash.bin ~/GruppenLW/releases/$DATE/buffalo-wxr-6000ax12s/buffalo-wxr-6000ax12s-webflash.bin

cp ipq60xx/src/router/aarch64-uclibc/buffalo-wxr-6000ax12b.ubi ~/GruppenLW/releases/$DATE/buffalo-wxr-6000ax12b/factory-to-ddwrt.ubi
cp ipq60xx/src/router/aarch64-uclibc/buffalo-wxr-6000ax12b.enc ~/GruppenLW/releases/$DATE/buffalo-wxr-6000ax12b/factory-to-ddwrt.enc
cp ipq60xx/src/router/aarch64-uclibc/buffalo-wxr-6000ax12b-webflash.bin ~/GruppenLW/releases/$DATE/buffalo-wxr-6000ax12b/buffalo-wxr-6000ax12b-webflash.bin

cp ipq60xx/src/router/aarch64-uclibc/buffalo-wxr-6000ax12p.ubi ~/GruppenLW/releases/$DATE/buffalo-wxr-6000ax12p/factory-to-ddwrt.ubi
cp ipq60xx/src/router/aarch64-uclibc/buffalo-wxr-6000ax12p.enc ~/GruppenLW/releases/$DATE/buffalo-wxr-6000ax12p/factory-to-ddwrt.enc
cp ipq60xx/src/router/aarch64-uclibc/buffalo-wxr-6000ax12p-webflash.bin ~/GruppenLW/releases/$DATE/buffalo-wxr-6000ax12p/buffalo-wxr-6000ax12p-webflash.bin
