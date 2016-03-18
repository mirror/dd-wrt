#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n adm5120/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mipsel_gcc4.1.2/bin:$OLDPATH
cd adm5120/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_wp54g_telcom .config
make -f Makefile.adm5120 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/compex-WP54G_telcom
mkdir -p ~/GruppenLW/releases/$DATE/compex-WP54AG_telcom
mkdir -p ~/GruppenLW/releases/$DATE/compex-WPP54G_telcom
mkdir -p ~/GruppenLW/releases/$DATE/compex-WPP54AG_telcom
cd ../../../
cp adm5120/src/router/mipsel-uclibc/compex-firmware-wp54g.bin ~/GruppenLW/releases/$DATE/compex-WP54G_telcom/tftp-image-wp54g.bin
cp adm5120/src/router/mipsel-uclibc/adm5120-webflash.bin ~/GruppenLW/releases/$DATE/compex-WP54G_telcom/WP54G-webflash.bin

cp adm5120/src/router/mipsel-uclibc/compex-firmware-wp54ag.bin ~/GruppenLW/releases/$DATE/compex-WP54AG_telcom/tftp-image-wp54ag.bin
cp adm5120/src/router/mipsel-uclibc/adm5120-webflash.bin ~/GruppenLW/releases/$DATE/compex-WP54AG_telcom/WP54AG-webflash.bin

cp adm5120/src/router/mipsel-uclibc/compex-firmware-wpp54g.bin ~/GruppenLW/releases/$DATE/compex-WPP54G_telcom/tftp-image-wpp54g.bin
cp adm5120/src/router/mipsel-uclibc/adm5120-webflash.bin ~/GruppenLW/releases/$DATE/compex-WPP54G_telcom/WPP54G-webflash.bin

cp adm5120/src/router/mipsel-uclibc/compex-firmware-wpp54ag.bin ~/GruppenLW/releases/$DATE/compex-WPP54AG_telcom/tftp-image-wpp54ag.bin
cp adm5120/src/router/mipsel-uclibc/adm5120-webflash.bin ~/GruppenLW/releases/$DATE/compex-WPP54AG_telcom/WPP54AG-webflash.bin


