#!/bin/sh
#./build_x86_kmt.sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n x86/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-i386_i486_gcc-5.1.0_musl-1.1.10/bin:$OLDPATH

cd x86/src/linux/universal/linux-4.4
cp .config_single .config
echo CONFIG_NET_ETHERIP=m >> .config

cd ../../../../../

cd x86/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_i386_full .config
make -f Makefile.x86 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/x86
cd ../../../
cp x86/src/router/i386-uclibc/dd-wrt_vga.image ~/GruppenLW/releases/$DATE/x86/dd-wrt_full_vga.image
cp x86/src/router/i386-uclibc/dd-wrt_serial.image ~/GruppenLW/releases/$DATE/x86/dd-wrt_full_serial.image
cp x86/src/router/i386-uclibc/dd-wrt-vga-webupgrade.bin ~/GruppenLW/releases/$DATE/x86/dd-wrt-webupgrade_full_vga.bin
cp x86/src/router/i386-uclibc/dd-wrt-serial-webupgrade.bin ~/GruppenLW/releases/$DATE/x86/dd-wrt-webupgrade_full_serial.bin
