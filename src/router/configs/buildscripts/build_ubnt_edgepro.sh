#!/bin/sh

OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n octeon/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips64_octeon_64_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips64_octeon_64_gcc-5.2.0_uClibc-0.9.33.2/bin:$OLDPATH
cd octeon/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../

cp .config_e200 .config
make -f Makefile.octeon kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-edgerouter-pro
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-edgerouter-pro-usbboot
cd ../../../
cp octeon/src/router/mips64-uclibc/erouter.image ~/GruppenLW/releases/$DATE/ubnt-edgerouter-pro/edgerouter-e200-pro.image
cp octeon/src/router/mips64-uclibc/erouter.bin ~/GruppenLW/releases/$DATE/ubnt-edgerouter-pro/edgerouter-e200-pro-webupgrade.bin
cp octeon/src/router/mips64-uclibc/ubnt-er-sysupgrade.tar ~/GruppenLW/releases/$DATE/ubnt-edgerouter-pro/ubnt-er-sysupgrade.tar

cd octeon/src/router
echo "CONFIG_USBBOOT=y" >> .config
make -f Makefile.octeon kernel clean all install

cd ../../../
cp octeon/src/router/mips64-uclibc/erouter.image ~/GruppenLW/releases/$DATE/ubnt-edgerouter-pro-usbboot/edgerouter-e200-pro.image
cp octeon/src/router/mips64-uclibc/erouter.bin ~/GruppenLW/releases/$DATE/ubnt-edgerouter-pro-usbboot/edgerouter-e200-pro-webupgrade.bin
cp octeon/src/router/mips64-uclibc/ubnt-er-sysupgrade.tar ~/GruppenLW/releases/$DATE/ubnt-edgerouter-pro-usbboot/ubnt-er-sysupgrade.tar
