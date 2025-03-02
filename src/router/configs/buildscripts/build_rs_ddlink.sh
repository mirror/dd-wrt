#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_rs_ddlink .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-routerstation_ddlink
cd ../../../
cp pb42/src/router/mips-uclibc/rs-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-routerstation_ddlink/ubnt-rs-firmware.bin
cp pb42/src/router/mips-uclibc/RS.dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-routerstation_ddlink


