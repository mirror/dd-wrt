#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n magicbox/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-powerpc_gcc-linaro_uClibc-0.9.32/bin:$OLDPATH
export PATH=/opt/staging_dir_powerpc/bin:$OLDPATH
cd magicbox/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_openrb .config
make -f Makefile.magicbox kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/openrb
cd ../../../
cp magicbox/src/router/powerpc-uclibc/dd-wrt-uImage ~/GruppenLW/releases/$DATE/openrb
cp magicbox/src/router/powerpc-uclibc/magicbox-firmware.bin ~/GruppenLW/releases/$DATE/openrb/openrb-firmware.bin

