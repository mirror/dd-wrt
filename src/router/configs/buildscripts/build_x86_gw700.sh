#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n x86/src/router/httpd)

export PATH=/xfs/toolchains/toolchain-i386_i486_gcc-5.1.0_musl-1.1.10/bin:$OLDPATH


cd x86/src/linux/universal/linux-3.12
cp .config_single .config
cd ../../../../../


cd x86/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_i386_sputnik .config
make -f Makefile.x86 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/x86
cd ../../../
cp x86/src/router/i386-uclibc/dd-wrt_vga.image ~/GruppenLW/releases/$DATE/x86/dd-wrt_gw700_vga.image
cp x86/src/router/i386-uclibc/dd-wrt_serial.image ~/GruppenLW/releases/$DATE/x86/dd-wrt_gw700_serial.image
cp x86/src/router/i386-uclibc/dd-wrt-vga-webupgrade.bin ~/GruppenLW/releases/$DATE/x86/dd-wrt-webupgrade_gw700_vga.bin
cp x86/src/router/i386-uclibc/dd-wrt-serial-webupgrade.bin ~/GruppenLW/releases/$DATE/x86/dd-wrt-webupgrade_gw700_serial.bin

