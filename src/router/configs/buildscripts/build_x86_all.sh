#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n x86/src/router/httpd)

export PATH=/xfs/toolchains/toolchain-i386_pentium-mmx_gcc-13.1.0_musl/bin:$OLDPATH

cd x86/src/linux/universal/linux-3.14
cp .config_smp .config
echo CONFIG_NET_ETHERIP=m >> .config
make oldconfig
cd ../../../../../
cd x86/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_i386_full .config
make -f Makefile.x86 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/x86
cd ../../../
cp x86/src/router/i386-uclibc/dd-wrt_vga.image ~/GruppenLW/releases/$DATE/x86/dd-wrt_full_smp_vga.image
cp x86/src/router/i386-uclibc/dd-wrt_serial.image ~/GruppenLW/releases/$DATE/x86/dd-wrt_full_smp_serial.image
cp x86/src/router/i386-uclibc/dd-wrt-vga-webupgrade.bin ~/GruppenLW/releases/$DATE/x86/dd-wrt-webupgrade_full_smp_vga.bin
cp x86/src/router/i386-uclibc/dd-wrt-serial-webupgrade.bin ~/GruppenLW/releases/$DATE/x86/dd-wrt-webupgrade_full_smp_serial.bin

cd x86/src/linux/universal/linux-3.14
cp .config_single .config
echo CONFIG_NET_ETHERIP=m >> .config
make oldconfig
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

cd x86/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_i386_nowifi .config
make -f Makefile.x86 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/x86
cd ../../../
cp x86/src/router/i386-uclibc/dd-wrt_vga.image ~/GruppenLW/releases/$DATE/x86/dd-wrt_public_vga.image
cp x86/src/router/i386-uclibc/dd-wrt_serial.image ~/GruppenLW/releases/$DATE/x86/dd-wrt_public_serial.image
cp x86/src/router/i386-uclibc/dd-wrt-vga-webupgrade.bin ~/GruppenLW/releases/$DATE/x86/dd-wrt-webupgrade_public_vga.bin
cp x86/src/router/i386-uclibc/dd-wrt-serial-webupgrade.bin ~/GruppenLW/releases/$DATE/x86/dd-wrt-webupgrade_public_serial.bin

