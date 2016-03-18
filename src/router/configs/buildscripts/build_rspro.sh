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
cp .config_rspro .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_routerstation_pro
cd ../../../
cp pb42/src/router/mips-uclibc/rspro-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_routerstation_pro/ubnt_rspro-firmware.bin
cp pb42/src/router/mips-uclibc/RSPRO.dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_routerstation_pro


