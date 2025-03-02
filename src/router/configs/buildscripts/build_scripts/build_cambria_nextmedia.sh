#!/bin/sh

OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-10.0.1_musl/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
#customer wants 0.99.17
cat .config_xscale_cambria | grep -v "^CONFIG_QUAGGA_STABLE\|^CONFIG_SNORT\|^CONFIG_MC\|^CONFIG_PHP\|^CONFIG_TOR\|^CONFIG_TRANSMISSION\|^CONFIG_ASTERISK" >.config
echo "CONFIG_NEXTMEDIA=y" >> .config
echo "CONFIG_RAIEXTRA=y" >> .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_MIITOOL=y" >> .config
echo "CONFIG_IFL=y" >> .config
echo "CONFIG_NRPE=y" >> .config
echo "CONFIG_LINKS=y" >> .config
echo "CONFIG_SOFTFLOWD=y" >> .config
echo "CONFIG_PYTHON=y" >> .config
echo "CONFIG_NMAP=y" >> .config
echo "CONFIG_ARPALERT=y" >>.config
echo "CONFIG_SNMP-UTILS=y" >>.config
echo "CONFIG_ATH5K=y" >>.config
#echo "LINUXDIR=\$(SRCBASE)/linux/universal/linux-3.2" >> .config
make -f Makefile.armbe kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/nextmedia/cambria
cd ../../../

cp xscale/src/router/armeb-uclibc/gateworks-firmware-squashfs.bin ~/GruppenLW/releases/CUSTOMER/$DATE/nextmedia/cambria/cambria-nextmedia-firmware.bin
cp xscale/src/router/armeb-uclibc/gateworx-firmware.raw2 ~/GruppenLW/releases/CUSTOMER/$DATE/nextmedia/cambria/linux-nextmedia.bin
