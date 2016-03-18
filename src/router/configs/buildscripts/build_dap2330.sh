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
echo "CONFIG_DAP2330=y" >> .config
sed -i 's/CONFIG_ATH10K=y/# CONFIG_ATH10K is not set/g' .config
sed -i 's/CONFIG_USB=y/# CONFIG_USB is not set/g' .config
sed -i 's/CONFIG_NTFS3G=y/# CONFIG_NTFS3G is not set/g' .config
sed -i 's/CONFIG_MINIDLNA=y/# CONFIG_MINIDLNA is not set/g' .config
sed -i 's/CONFIG_SAMBA3=y/# CONFIG_SAMBA3 is not set/g' .config
sed -i 's/CONFIG_UQMI=y/# CONFIG_UQMI is not set/g' .config

make -f Makefile.pb42 kernel clean all install
cd ../../../
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dap2330
cp pb42/src/router/mips-uclibc/web-dap2330.img ~/GruppenLW/releases/$DATE/dlink-dap2330/factory-to-ddwrt.bin
cp pb42/src/router/mips-uclibc/webflash-dap2330.trx ~/GruppenLW/releases/$DATE/dlink-dap2330/dap2330-webflash.bin
