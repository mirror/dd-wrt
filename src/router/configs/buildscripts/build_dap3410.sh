#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.1.2-uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dap3410

cp .config_dap34xx .config
echo "CONFIG_UBNTXW=y" >> .config
echo "CONFIG_DAP3410=y" >> .config

make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/dlink-dap3410/dap3410-dd-wrt-webupgrade.bin
cp pb42/src/router/mips-uclibc/dap3410.bin ~/GruppenLW/releases/$DATE/dlink-dap3410/factory-to-ddwrt.bin
cp notes/dap3410/* ~/GruppenLW/releases/$DATE/dlink-dap3410


