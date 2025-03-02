#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n magicbox/src/router/httpd)
export PATH=/opt/staging_dir_powerpc/bin:$OLDPATH
cd magicbox/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_openrb .config
echo "CONFIG_MYNETWAY=y" >> .config
echo "CONFIG_BRANDING=y" >> .config

make -f Makefile.magicbox kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/mynetway/openrb
cd ../../../
cp magicbox/src/router/powerpc-uclibc/dd-wrt-uImage ~/GruppenLW/releases/CUSTOMER/$DATE/mynetway/openrb
cp magicbox/src/router/powerpc-uclibc/magicbox-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/mynetway/openrb/openrb-firmware.bin

