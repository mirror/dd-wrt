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
cp .config_wpe72 .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/compex_wpe72
cd ../../../
cp pb42/src/router/mips-uclibc/lsx-firmware.bin ~/GruppenLW/releases/$DATE/compex_wpe72/wpe72-firmware.bin
cp pb42/src/router/mips-uclibc/wpe72.img ~/GruppenLW/releases/$DATE/compex_wpe72/wpe72-image.tftp



cd pb42/src/router
cp .config_wpe72_4M .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/compex_wpe72_4M
cd ../../../
cp pb42/src/router/mips-uclibc/lsx-firmware.bin ~/GruppenLW/releases/$DATE/compex_wpe72_4M/wpe72-firmware.bin
cp pb42/src/router/mips-uclibc/wpe72.img ~/GruppenLW/releases/$DATE/compex_wpe72_4M/wpe72-image.tftp


