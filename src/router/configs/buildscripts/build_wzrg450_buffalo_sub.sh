#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)

REV=$(svnversion -n pb42/src/router/httpd)
REV+="-"
REV+=$(date +%Y-%m-%d)

export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.1.2-uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
#cp .config_wzrg450_buffalo .config
#echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
#echo "CONFIG_DEFAULT_COUNTRYCODE=$1" >> .config
#echo "$2" >> .config
#make -f Makefile.pb42 kernel clean all install
#mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr_hp_g450h
#cd ../../../
#cp pb42/src/router/mips-uclibc/wzrg450-firmware_$1.enc ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr_hp_g450h/wzr_hp_g450h-$1.bin
#cp pb42/src/router/mips-uclibc/wzrg450-firmware.tftp ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr_hp_g450h/wzr_hp_g450h-$1.tftp
#cp customer/buffalo/changelog.txt ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr_hp_g450h/
#cd /GruppenLW/releases/CUSTOMER/$DATE/buffalo && zip -9 -u $REV-buffalo_wzr_hp_g450h.zip buffalo_wzr_hp_g450h/*.bin buffalo_wzr_hp_g450h/*.txt
#cd /home/seg/DEV


#cd pb42/src/router

cp .config_wzrg450_buffalo .config
echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
echo "CONFIG_DEFAULT_COUNTRYCODE=$1" >> .config
echo "CONFIG_BUFFALO_SA=y" >> .config
#echo "CONFIG_IAS=y" >> .config
echo "$2" >> .config
#make -f Makefile.pb42 libutils-clean libutils buffalo_flash-clean buffalo_flash upnp-clean upnp
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr_hp_g450h_SA
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_bhr_4grv
cd ../../../
#cp pb42/src/router/mips-uclibc/wzrg450-firmware_$1.enc ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr_hp_g450h_SA/wzr_hp_g450h-$1.bin
/root/firmware/pem/dosign.sh pb42/src/router/mips-uclibc/wzrg450-firmware_$1.enc
cp pb42/src/router/mips-uclibc/wzrg450-firmware_$1.enc.signed ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr_hp_g450h_SA/wzr_hp_g450h-$1.bin
cp pb42/src/router/mips-uclibc/wzrg450-firmware.tftp ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr_hp_g450h_SA/wzr_hp_g450h-$1.tftp
cp pb42/src/router/mips-uclibc/bhr4grv-firmware_$1.enc ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_bhr_4grv/bhr-4grv-factory-to-ddwrt.bin
#cp pb42/src/router/mips-uclibc/wzrg450-firmware.tftp ~/GruppenLW/releases/$DATE/buffalo_bhr_4grv/bhr-4grv-$1.tftp
#cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_bhr_4grv/bhr-4grv-dd-wrt-webupgrade-$1.bin


cp customer/buffalo/changelog.txt ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo_wzr_hp_g450h_SA/
cd /GruppenLW/releases/CUSTOMER/$DATE/buffalo && zip -9 -u $REV-buffalo_wzr_hp_g450h_SA.zip buffalo_wzr_hp_g450h_SA/*.bin buffalo_wzr_hp_g450h_SA/*.txt
cd /home/seg/DEV
