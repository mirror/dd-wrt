#!/bin/sh

OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n octeon/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-mips64_octeon_64_gcc-5.1.0_uClibc-0.9.33.2/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips64_octeon_64_gcc-5.2.0_uClibc-0.9.33.2/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-mips64_octeon_64_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
cd octeon/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../

cp .config_e100 .config
make -f Makefile.octeon kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-edgerouter-lite
cd ../../../
cp octeon/src/router/mips64-uclibc/erouter.image ~/GruppenLW/releases/$DATE/ubnt-edgerouter-lite/edgerouter-e100-lite.image
cp octeon/src/router/mips64-uclibc/erouter.bin ~/GruppenLW/releases/$DATE/ubnt-edgerouter-lite/edgerouter-e100-lite-webupgrade.bin

