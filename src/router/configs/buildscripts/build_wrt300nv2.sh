#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n xscale/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-armeb_xscale_gcc-5.2.0_musl-1.1.11/bin:$OLDPATH
cd xscale/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_wrt300nv2_pre .config
make -f Makefile.armbe kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/Linksys_WRT300N_V2
cd ../../../
cp xscale/src/router/tools/wrt300n/wrt300n.bin ~/GruppenLW/releases/$DATE/Linksys_WRT300N_V2/dd-wrt.v24_wrt300nv2.bin

