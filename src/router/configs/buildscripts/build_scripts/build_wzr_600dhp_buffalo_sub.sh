#!/bin/sh -x

OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
##DATE+="19438"
DATE+=$(svnversion -n pb42/src/router/httpd)
##[ -n "$CONFIG_KERNEL_ELF_CORE" ] && DATE+="-DEBUG"

REV=$(svnversion -n pb42/src/router/httpd)
##REV="19438"
REV+="-"
REV+=$(date +%Y-%m-%d)

export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-12.2.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mipsel_24kc_gcc-12.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.1.2-uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
#cp .config_wzrag300nh_buffalo .config
#cp .config_wzrag300nh .config
cat .config_wzrag300nh | grep -v "^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_ROUTERSTYLE\|^CONFIG_BLUE\|^CONFIG_YELLOW\|^CONFIG_CYAN\|^CONFIG_RED\|^CONFIG_GREEN" > .config
echo "CONFIG_WZR600DHP=y" >> .config
echo "CONFIG_IAS=y" >> .config
echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
echo "CONFIG_DEFAULT_COUNTRYCODE=$1" >> .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_BUFFALO=y" >> .config
#echo "$2" >> .config
if [ $2 = "idexx_signatur" ]
then
	echo "CONFIG_IDEXX_SIGNATUR=y" >> .config
fi
make -f Makefile.pb42 buffalo_flash-clean buffalo_flash upnp-clean upnp
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr_600dhp
cd ../../../
#cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr_hp_ag300h/wzr-hp-ag300h-dd-wrt-webupgrade-$1.bin
if [[ "$2" == "idexx_signatur" ]];
then
	/root/firmware/pem/dosign.sh pb42/src/router/mips-uclibc/wzr_600dhp-firmware_$1.enc /root/firmware/pem/private_key_idexx.txt
else 
	/root/firmware/pem/dosign.sh pb42/src/router/mips-uclibc/wzr_600dhp-firmware_$1.enc
	/root/firmware/pem/dosign.sh pb42/src/router/mips-uclibc/wzr_600dhp-firmware_$1_1.99.enc
	cp pb42/src/router/mips-uclibc/wzr_ag300h_to_wzr_600dhp-firmware_$1.enc ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr_600dhp/wzr_ag300h_to_wzr_d600dhp-$1.bin
fi
cp pb42/src/router/mips-uclibc/wzr_600dhp-firmware_$1.enc.signed ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr_600dhp/wzr_600dhp-$1.bin
#cp pb42/src/router/mips-uclibc/wzr_600dhp-firmware_$1_1.99.enc.signed ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr_600dhp/wzr_600dhp-$1_1.99.bin
cp customer/buffalo/changelog.txt ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr_600dhp/
cd /GruppenLW/releases/CUSTOMER/$DATE/buffalo && zip -9 -u $REV-buffalo_wzr_600dhp.zip buffalo_wzr_600dhp/*.bin buffalo_wzr_600dhp/*.txt
cd /home/seg/DEV
