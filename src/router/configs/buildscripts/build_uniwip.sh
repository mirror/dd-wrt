#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n mpc83xx/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-powerpc_e300c3_gcc-4.8-linaro_uClibc-0.9.33.2/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd mpc83xx/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
mkdir -p ~/GruppenLW/releases/$DATE/octagon_systems-uniwip
cp .config_uniwip .config
make -f Makefile.mpc83xx kernel clean all install
cd ../../../
cp mpc83xx/src/router/powerpc-uclibc/uniwip.image.main ~/GruppenLW/releases/$DATE/octagon_systems-uniwip/
cp mpc83xx/src/router/powerpc-uclibc/uniwip-firmware.bin ~/GruppenLW/releases/$DATE/octagon_systems-uniwip/

