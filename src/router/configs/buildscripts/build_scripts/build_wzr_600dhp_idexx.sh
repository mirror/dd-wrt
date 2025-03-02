#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+="22084"
#DATE+=$(svnversion -n pb42/src/router/httpd)

#REV=$(svnversion -n pb42/src/router/httpd)
REV="22084"
REV+="-"
REV+=$(date +%Y-%m-%d)

export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-4.8-linaro_musl-1.1.2/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_r2_gcc-4.8-linaro_uClibc-0.9.33.2/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_r2_gcc-4.7.2_uClibc-0.9.33.2/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mipsel_24kc_gcc-12.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_r2_gcc-4.6-linaro_uClibc-0.9.33/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.1.2-uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
#[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
#[ -n "$DO_UPDATE" ] && svn update
cd ../../../


cp .config_wzr600dhp_idexx .config
echo "CONFIG_WZR600DHP=y" >> .config
#echo "CONFIG_IAS=y" >> .config
echo "CONFIG_DEFAULT_LANGUAGE=english" >> .config
echo "CONFIG_DEFAULT_COUNTRYCODE=MULTI" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/idexx/buffalo_wzr_600dhp
cd ../../../
cp pb42/src/router/mips-uclibc/wzr_600dhp-firmware_MULTI.enc ~/GruppenLW/releases/CUSTOMER/$DATE/idexx/buffalo_wzr_600dhp/wzr_600dhp-factory-to-ddwrt_idexx.bin
#cp pb42/src/router/mips-uclibc/wzrg450-firmware.tftp ~/GruppenLW/releases/$DATE/buffalo_wzr_hp_g450h/wzr_hp_g450h-$1.tftp
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/idexx/buffalo_wzr_600dhp/wzr_600dhp-firmware_MULTI.bin
#cd /GruppenLW/releases/CUSTOMER/$DATE/buffalo && zip -9 -u $REV-buffalo_wzr_hp_ag300h.zip buffalo_wzr_hp_ag300h/*.bin buffalo_wzr_hp_ag300h/*.txt
cd /home/seg/peter
