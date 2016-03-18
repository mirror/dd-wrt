#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)

export PATH=/xfs/toolchains/toolchain-i386_gcc-linaro_uClibc-0.9.32/bin:$OLDPATH

cd /xfs/projects/x86/src/linux/x86/linux-2.6.20
cp .config_smp .config
make oldconfig
cd ../../../../../
cd x86/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_i386_full .config
make -f Makefile.x86 kernel clean all install
mkdir -p /GruppenLW/releases/$DATE/x86
cd ../../../
cp x86/src/router/i486-uclibc/dd-wrt_vga.image /GruppenLW/releases/$DATE/x86/dd-wrt_full_smp_vga.image
cp x86/src/router/i486-uclibc/dd-wrt_serial.image /GruppenLW/releases/$DATE/x86/dd-wrt_full_smp_serial.image
cp x86/src/router/i486-uclibc/dd-wrt-vga-webupgrade.bin /GruppenLW/releases/$DATE/x86/dd-wrt-webupgrade_full_smp_vga.bin
cp x86/src/router/i486-uclibc/dd-wrt-serial-webupgrade.bin /GruppenLW/releases/$DATE/x86/dd-wrt-webupgrade_full_smp_serial.bin
