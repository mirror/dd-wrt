#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n mt7621/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mipsel_1004kc+dsp_gcc-5.1.0_musl-1.1.10/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mipsel_gcc4.1.2/bin:$OLDPATH
cd mt7621/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_dir860 .config
make -f Makefile.rt2880 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir860l-b1
cd ../../../

cp mt7621/src/router/mipsel-uclibc/dir860l-webflash.bin ~/GruppenLW/releases/$DATE/dlink-dir860l-b1/dlink-dir860lb1-webflash.bin
cp mt7621/src/router/mipsel-uclibc/web-dir860.img ~/GruppenLW/releases/$DATE/dlink-dir860l-b1/factory-to-ddwrt.bin

