#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ks8695/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-arm_v4_gcc-4.8-linaro_uClibc-0.9.33.2_eabi/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_gcc4.2.3/bin:$OLDPATH
cd ks8695/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_openrisc .config
make -f Makefile.openrisc kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/Alekto_OpenRisc
cd ../../../
#cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/ubnt_ls5
#cp ar531x/src/router/mips-uclibc/vmlinux.bin.l7 ~/GruppenLW/releases/$DATE/ubnt_ls5/kernel
cp ks8695/src/router/arm-uclibc/disc.img ~/GruppenLW/releases/CUSTOMER/$DATE/Alekto_OpenRisc/disc_cf.img
cp ks8695/src/router/arm-uclibc/disc_sd.img ~/GruppenLW/releases/CUSTOMER/$DATE/Alekto_OpenRisc/disc_sd.img


