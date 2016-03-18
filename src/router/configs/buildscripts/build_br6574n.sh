#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n rt2880/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mipsel_gcc4.1.2/bin:$OLDPATH
cd rt2880/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_br6574n .config
make -f Makefile.rt2880 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/Edimax-BR6574N
cd ../../../
#cp rt2880/src/router/mipsel-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/WHR-G300N/firmware.bin
cp rt2880/src/router/mipsel-uclibc/rt2880-webflash.bin ~/GruppenLW/releases/$DATE/Edimax-BR6574N/BR6574N-webflash.bin
cp rt2880/src/router/mipsel-uclibc/edimax-6574.bin ~/GruppenLW/releases/$DATE/Edimax-BR6574N/image.tftp
#cd rt2880/src/router
#cp .config_br6574n_small .config
#make -f Makefile.rt2880 kernel clean all install
#cd ../../../
#cp rt2880/src/router/mipsel-uclibc/edimax.bin ~/GruppenLW/releases/$DATE/Edimax-BR6574N/factory-to-dd-wrt.bin



#cp rt2880/src/router/mipsel-uclibc/whrg300n-firmware.tftp ~/GruppenLW/releases/$DATE/WHR-G300N/whrg300n-firmware.tftp


