#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_wr1043v2 .config
echo CONFIG_ARCHERC7=y >> .config
echo CONFIG_ATH10K=y >> .config
echo CONFIG_CPUTEMP=y >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/tplink_archer-c7-v1
cd ../../../
cp pb42/src/router/mips-uclibc/ARCHER-C7v1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_archer-c7-v1/tplink_archer-c7.bin
cp pb42/src/router/mips-uclibc/tplink-ARCHER-C7v1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_archer-c7-v1/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/ARCHER-C7v1-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink_archer-c7-v1/tplink_archer-c7-US.bin
cp pb42/src/router/mips-uclibc/tplink-ARCHER-C7v1-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink_archer-c7-v1/factory-to-ddwrt-US.bin


