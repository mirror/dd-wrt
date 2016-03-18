#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n magicbox/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-powerpc_gcc-4.6-linaro_uClibc-0.9.33/bin:$OLDPATH
#export PATH=/opt/staging_dir_powerpc/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-powerpc_gcc-linaro_uClibc-0.9.32/bin:$OLDPATH
cd magicbox/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_rb600 .config
make -f Makefile.magicbox kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/mikrotik-rb600
cd ../../../
cp magicbox/src/router/rb600.img ~/GruppenLW/releases/$DATE/mikrotik-rb600/rb600.img
cp magicbox/src/router/rb600.bin ~/GruppenLW/releases/$DATE/mikrotik-rb600/rb600-webupgrade.bin
cd magicbox/src/router
cp .config_rb600 .config
echo "CONFIG_RB1000=y" >> .config
make -f Makefile.magicbox kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/mikrotik-rb1000
cd ../../../
cp magicbox/src/router/rb600.img ~/GruppenLW/releases/$DATE/mikrotik-rb1000/rb1000.img
cp magicbox/src/router/rb600.bin ~/GruppenLW/releases/$DATE/mikrotik-rb1000/rb1000-webupgrade.bin

cd magicbox/src/router
cp .config_rb600 .config
echo "CONFIG_RB800=y" >> .config
make -f Makefile.magicbox kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/mikrotik-rb800
cd ../../../
cp magicbox/src/router/rb600.img ~/GruppenLW/releases/$DATE/mikrotik-rb800/rb800.img
cp magicbox/src/router/rb600.bin ~/GruppenLW/releases/$DATE/mikrotik-rb800/rb800-webupgrade.bin


