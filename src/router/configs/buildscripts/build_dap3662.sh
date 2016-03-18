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
cp .config_dir862 .config
echo "CONFIG_DAP3662=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dap3662
cp pb42/src/router/mips-uclibc/web-dap3662.img ~/GruppenLW/releases/$DATE/dlink-dap3662/factory-to-ddwrt.bin
cp pb42/src/router/mips-uclibc/webflash-dap3662.trx ~/GruppenLW/releases/$DATE/dlink-dap3662/dap3662-webflash.bin
