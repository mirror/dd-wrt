#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_dir825c1 .config
echo "CONFIG_JWAP606=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/jjplus-jwap606
cd ../../../
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/$DATE/jjplus-jwap606/jjplus-jwap606-firmware.bin
cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/jjplus-jwap606/vmlinux_jwap603_f1e.lzma.uimage
