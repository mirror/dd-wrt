#!/bin/sh
#./build_x86_kmt.sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n x86_64/src/router/httpd)


export PATH=/xfs/toolchains/toolchain-x86_64_gcc-5.2.0_musl-1.1.11/bin:$OLDPATH
cd x86_64/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_x64 .config
make -f Makefile.x64 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/x86_64
cd ../../../
cp x86_64/src/router/x86_64-uclibc/dd-wrt_vga.image ~/GruppenLW/releases/$DATE/x86_64/dd-wrt_full_vga.image
cp x86_64/src/router/x86_64-uclibc/dd-wrt_serial.image ~/GruppenLW/releases/$DATE/x86_64/dd-wrt_full_serial.image
cp x86_64/src/router/x86_64-uclibc/dd-wrt-vga-webupgrade.bin ~/GruppenLW/releases/$DATE/x86_64/dd-wrt-webupgrade_full_vga.bin
cp x86_64/src/router/x86_64-uclibc/dd-wrt-serial-webupgrade.bin ~/GruppenLW/releases/$DATE/x86_64/dd-wrt-webupgrade_full_serial.bin

cd x86_64/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_x64_free .config
make -f Makefile.x64 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/x86_64
cd ../../../
cp x86_64/src/router/x86_64-uclibc/dd-wrt_vga.image ~/GruppenLW/releases/$DATE/x86_64/dd-wrt_public_vga.image
cp x86_64/src/router/x86_64-uclibc/dd-wrt_serial.image ~/GruppenLW/releases/$DATE/x86_64/dd-wrt_public_serial.image
cp x86_64/src/router/x86_64-uclibc/dd-wrt-vga-webupgrade.bin ~/GruppenLW/releases/$DATE/x86_64/dd-wrt-webupgrade_public_vga.bin
cp x86_64/src/router/x86_64-uclibc/dd-wrt-serial-webupgrade.bin ~/GruppenLW/releases/$DATE/x86_64/dd-wrt-webupgrade_public_serial.bin



