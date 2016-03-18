#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_wrt160nl${1} .config
echo "CONFIG_E2100=y" >>.config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/linksys_E2100L${1}
cd ../../../
cp pb42/src/router/mips-uclibc/e2100l-firmware.bin ~/GruppenLW/releases/$DATE/linksys_E2100L${1}/e2100l-firmware.bin

#cd pb42/src/router
#cp .config_wrt160nl_preflash .config
#echo "CONFIG_E2100=y" >>.config
#make -f Makefile.pb42 kernel clean all install
#mkdir -p ~/GruppenLW/releases/$DATE/linksys_E2100L
#cd ../../../
#cp pb42/src/router/mips-uclibc/e2100l-firmware.bin ~/GruppenLW/releases/$DATE/linksys_E2100L/linksys-to-ddwrt-firmware.bin


