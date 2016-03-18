#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531x/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_mips32_gcc-5.2.0_musl-1.1.12/bin:$OLDPATH
cd ar531x/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_rtg32 .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/asus-rt-g32
cd ../../../
#cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/ubnt_ls2
#cp ar531x/src/router/mips-uclibc/vmlinux.bin.l7 ~/GruppenLW/releases/$DATE/ubnt_ls2
cp ar531x/src/router/mips-uclibc/rtg32-firmware.bin ~/GruppenLW/releases/$DATE/asus-rt-g32/Asus_RTG32-firmware.bin
#cp ar531x/src/router/mips-uclibc/vmlinux.rtg32 ~/GruppenLW/releases/$DATE/Asus_RTG32/vmlinux.bin.l7
#cp ar531x/src/router/mips-uclibc/root.g32 ~/GruppenLW/releases/$DATE/Asus_RTG32/rootfs
cp ar531x/src/router/mips-uclibc/RTG32.dd-wrt.bin ~/GruppenLW/releases/$DATE/asus-rt-g32/asus-to-dd-wrt.bin


