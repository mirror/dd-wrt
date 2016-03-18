#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n rt2880/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mipsel_gcc4.1.2/bin:$OLDPATH
cd rt2880/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_eap9550 .config
make -f Makefile.rt2880 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/senao-EAP9550
cd ../../../
cp rt2880/src/router/mipsel-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/senao-EAP9550/firmware.bin
cp rt2880/src/router/mipsel-uclibc/rt2880-webflash.bin ~/GruppenLW/releases/$DATE/senao-EAP9550/Senao-EAP9550-webflash.bin


