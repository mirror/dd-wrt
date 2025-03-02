#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-13.1.0_musl/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_xscale .config

echo "CONFIG_ALFA_BRANDING1=y" >> .config
echo "CONFIG_BRANDING=y" >> .config
make -f Makefile.armbe kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/alfa_ixp_b1
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/alfa_ixp_b1/alfa_ixp_branding1-webupgrade.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/alfa_ixp_b1/linux-branding1.bin

cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update

echo "CONFIG_ALFA_BRANDING2=y" >> .config
echo "CONFIG_BRANDING=y" >> .config
make -f Makefile.armbe kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/alfa_ixp_b2
cd ../../../
cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/$DATE/alfa_ixp_b2/alfa_ixp_branding2-webupgrade.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/$DATE/alfa_ixp_b2/linux-branding2.bin


