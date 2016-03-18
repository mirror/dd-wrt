#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_wrt160nl${1} .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/linksys_wrt160nl${1}
cd ../../../
cp pb42/src/router/mips-uclibc/wrt160nl-firmware.bin ~/GruppenLW/releases/$DATE/linksys_wrt160nl${1}/wrt160nl-firmware.bin
cd pb42/src/router
cp .config_wrt160nl_preflash .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/linksys_wrt160nl${1}
cd ../../../
cp pb42/src/router/mips-uclibc/wrt160nl-firmware.bin ~/GruppenLW/releases/$DATE/linksys_wrt160nl${1}/linksys-to-ddwrt-firmware.bin


