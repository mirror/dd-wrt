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
cp .config_dir810l .config
make -f Makefile.rt2880 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir810l-b1
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir810l-a1
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir810l-c1
cd ../../../

cp rt2880/src/router/mipsel-uclibc/mt7620-webflash.bin ~/GruppenLW/releases/$DATE/dlink-dir810l-b1/dlink-dir810lb1-webflash.bin
cp rt2880/src/router/mipsel-uclibc/aligned-mt7620.dir810lb1 ~/GruppenLW/releases/$DATE/dlink-dir810l-b1/factory-to-ddwrt.bin

cp rt2880/src/router/mipsel-uclibc/mt7620-webflash.bin ~/GruppenLW/releases/$DATE/dlink-dir810l-a1/dlink-dir810la1-webflash.bin
cp rt2880/src/router/mipsel-uclibc/aligned-mt7620.dir810la1 ~/GruppenLW/releases/$DATE/dlink-dir810l-a1/factory-to-ddwrt.bin

cp rt2880/src/router/mipsel-uclibc/mt7620-webflash.bin ~/GruppenLW/releases/$DATE/dlink-dir810l-c1/dlink-dir810lc1-webflash.bin
cp rt2880/src/router/mipsel-uclibc/aligned-mt7620.dir810lc1 ~/GruppenLW/releases/$DATE/dlink-dir810l-c1/factory-to-ddwrt.bin


