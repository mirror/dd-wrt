#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n x86/src/router/httpd)

export PATH=/xfs/toolchains/toolchain-i386_i486_gcc-5.1.0_musl-1.1.10/bin:$OLDPATH

cd x86/src/linux/universal/linux-3.14
cp .config_smp .config
cd ../../../../../
cd x86/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_i386_re .config
make -f Makefile.x86 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/erc/x86-re
cd ../../../
cp x86/src/router/i386-uclibc/dd-wrt_vga.image ~/GruppenLW/releases/CUSTOMER/$DATE/erc/x86-re/dd-wrt_full_smp_vga.image
cp x86/src/router/i386-uclibc/dd-wrt_serial.image ~/GruppenLW/releases/CUSTOMER/$DATE/erc/x86-re/dd-wrt_full_smp_serial.image
cp x86/src/router/i386-uclibc/dd-wrt-vga-webupgrade.bin ~/GruppenLW/releases/CUSTOMER/$DATE/erc/x86-re/dd-wrt-webupgrade_full_smp_vga.bin
cp x86/src/router/i386-uclibc/dd-wrt-serial-webupgrade.bin ~/GruppenLW/releases/CUSTOMER/$DATE/erc/x86-re/dd-wrt-webupgrade_full_smp_serial.bin
