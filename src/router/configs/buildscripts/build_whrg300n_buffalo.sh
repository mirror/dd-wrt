#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n rt2880/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mipsel_24kec+dsp_gcc-4.8-linaro_uClibc-0.9.33.2/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mipsel_gcc-linaro_uClibc-0.9.32/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mipsel_gcc4.1.2/bin:$OLDPATH
cd rt2880/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_whrg300n_buffalo .config
make -f Makefile.rt2880 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/WHR-G300N-Buffalo
cd ../../../
cp rt2880/src/router/mipsel-uclibc/aligned.uimage_TFTP.bin ~/GruppenLW/releases/$DATE/WHR-G300N-Buffalo/firmware.tftp
cp rt2880/src/router/mipsel-uclibc/rt2880-webflash.bin ~/GruppenLW/releases/$DATE/WHR-G300N-Buffalo/WHR-G300N-webflash.bin
#./broadcom/opt/bufenc/encrypt rt2880/src/router/mipsel-uclibc/rt2880-webflash.bin ~/GruppenLW/releases/$DATE/WHR-G300N-Buffalo/WHR-G300N-webflash_enc.bin WHR-G300N-DD-WRT


