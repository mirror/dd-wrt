#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_dir862 .config
echo "CONFIG_SMARTDNS=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_QCA9888=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir862
mkdir -p ~/GruppenLW/releases/$DATE/dlink-dir866
mkdir -p ~/GruppenLW/releases/$DATE/trendnet-tew824dru
cd ../../../
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/$DATE/dlink-dir862/dir862-webflash.bin
cp pb42/src/router/mips-uclibc/dir862-uimage.bin ~/GruppenLW/releases/$DATE/dlink-dir862/factory-to-ddwrt.img

cd pb42/src/router

echo "CONFIG_DIR866=y" >> .config
echo "CONFIG_QCA9888=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/ap96-firmware.bin ~/GruppenLW/releases/$DATE/dlink-dir866/dir866-webflash.bin
cp pb42/src/router/mips-uclibc/dir866-uimage.bin ~/GruppenLW/releases/$DATE/dlink-dir866/factory-to-ddwrt.img

cd pb42/src/router
echo "CONFIG_TEW824=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/tew824-firmware.bin ~/GruppenLW/releases/$DATE/trendnet-tew824dru/tew824dru-webflash.bin
cp pb42/src/router/mips-uclibc/root.tew824 ~/GruppenLW/releases/$DATE/trendnet-tew824dru/factory-to-ddwrt.bin
