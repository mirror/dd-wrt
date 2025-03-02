#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n northstar/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH

export PATH=/xfs/toolchains/toolchain-arm_cortex-a9_gcc-12.1.0_musl_eabi/bin:$OLDPATH
cd northstar/src/router
[ -n "$DO_UPDATE" ] && svn update
#cp .config_laguna-small .config
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir890l
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir895l
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir885l
mkdir -p ~/GruppenLW/releases/$DATE/trendnet-828DRU
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ac88u
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ac5300
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ac3200
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-ac3100
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r8000
mkdir -p ~/GruppenLW/releases/$DATE/netgear-r7000P



cp .config_northstar .config
echo "CONFIG_SMP=y" >> .config
echo "CONFIG_DHDAP=y" >> .config
echo "CONFIG_SOFTETHER=y" >> .config
echo "CONFIG_IRQBALANCE=y" >> .config
echo "CONFIG_SPEEDCHECKER=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_RAID=y" >> .config
echo "CONFIG_SMBD=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
echo "CONFIG_HTOP=y" >> .config
echo "CONFIG_MDNS=y" >> .config
echo "CONFIG_MDNS_UTILS=y" >> .config
make -f Makefile.northstar kernel clean all install
#mkdir -p ~/GruppenLW/releases/$DATE/northstar
cd ../../../
#cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/northstar

cp northstar/src/router/arm-uclibc/webflash-dir890.trx ~/GruppenLW/releases/$DATE/dlink-dir890l/dir890-webflash.bin
cp northstar/src/router/arm-uclibc/web-dir890.img ~/GruppenLW/releases/$DATE/dlink-dir890l/factory-to-ddwrt.bin

cp northstar/src/router/arm-uclibc/webflash-dir895.trx ~/GruppenLW/releases/$DATE/dlink-dir895l/dir895-webflash.bin
cp northstar/src/router/arm-uclibc/web-dir895.img ~/GruppenLW/releases/$DATE/dlink-dir895l/factory-to-ddwrt.bin

cp northstar/src/router/arm-uclibc/webflash-dir885.trx ~/GruppenLW/releases/$DATE/dlink-dir885l/dir885-webflash.bin
cp northstar/src/router/arm-uclibc/web-dir885.img ~/GruppenLW/releases/$DATE/dlink-dir885l/factory-to-ddwrt.bin

cp northstar/src/router/arm-uclibc/asus_rt-ac88u-firmware.trx ~/GruppenLW/releases/$DATE/asus-rt-ac88u
cp northstar/src/router/arm-uclibc/asus_rt-ac5300-firmware.trx ~/GruppenLW/releases/$DATE/asus-rt-ac5300
cp northstar/src/router/arm-uclibc/asus_rt-ac3200-firmware.trx ~/GruppenLW/releases/$DATE/asus-rt-ac3200
cp northstar/src/router/arm-uclibc/asus_rt-ac3100-firmware.trx ~/GruppenLW/releases/$DATE/asus-rt-ac3100

cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/netgear-r8000/netgear-r8000-webflash.bin
cp northstar/src/router/arm-uclibc/K3_R8000.chk ~/GruppenLW/releases/$DATE/netgear-r8000/factory-to-dd-wrt.chk


cp northstar/src/router/arm-uclibc/northstar-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/netgear-r7000P/netgear-r7000p-webflash.bin
cp northstar/src/router/arm-uclibc/K3_R7000P.chk ~/GruppenLW/releases/$DATE/netgear-r7000P/factory-to-dd-wrt.chk




cd northstar/src/router
cp .config_northstar_16m .config
echo "CONFIG_SMP=y" >> .config
echo "CONFIG_DHDAP=y" >> .config
echo "CONFIG_IRQBALANCE=y" >> .config
echo "CONFIG_SMBD=y" >> .config
sed -i 's/\CONFIG_SNORT=y/# CONFIG_SNORT is not set/g' .config
sed -i 's/\CONFIG_PHP=y/# CONFIG_PHP is not set/g' .config
sed -i 's/\CONFIG_ASTERISK=y/# CONFIG_ASTERISK is not set/g' .config
sed -i 's/\CONFIG_WEBSERVER=y/# CONFIG_WEBSERVER is not set/g' .config
sed -i 's/\CONFIG_PRIVOXY=y/# CONFIG_PRIVOXY is not set/g' .config
sed -i 's/\CONFIG_TRANSMISSION=y/# CONFIG_TRANSMISSION is not set/g' .config
make -f Makefile.northstar kernel clean all install
cd ../../../
cp northstar/src/router/arm-uclibc/tnet828.trx ~/GruppenLW/releases/$DATE/trendnet-828DRU/trendnet-828dru-webflash.bin



#cd northstar/src/router
#cp .config_northstar_mini .config
#echo "CONFIG_SMP=y" >> .config
#echo "CONFIG_DHDAP=y" >> .config
#sed -i 's/\CONFIG_SNORT=y/# CONFIG_SNORT is not set/g' .config
#sed -i 's/\CONFIG_PHP=y/# CONFIG_PHP is not set/g' .config
#sed -i 's/\CONFIG_ASTERISK=y/# CONFIG_ASTERISK is not set/g' .config
#sed -i 's/\CONFIG_WEBSERVER=y/# CONFIG_WEBSERVER is not set/g' .config

#make -f Makefile.northstar kernel clean all install
#cd ../../../
#cp northstar/src/router/arm-uclibc/tnet828.trx ~/GruppenLW/releases/$DATE/trendnet-828DRU/factory-to-ddwrt.bin
