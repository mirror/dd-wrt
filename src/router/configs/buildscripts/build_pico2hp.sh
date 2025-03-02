#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531xr2/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_4kec_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin:$OLDPATH
cd ar531xr2/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_pico2hp .config
make -f Makefile.ar531x build_date
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-pico2hp
cd ../../../
#cp ar531xr2/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/ubnt-ls5
#cp ar531xr2/src/router/mips-uclibc/vmlinux.bin.l7 ~/GruppenLW/releases/$DATE/ubnt-ls5/kernel
cp ar531xr2/src/router/mips-uclibc/ls2-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-pico2hp/ubnt-pico2hp-firmware.bin
cp ar531xr2/src/router/mips-uclibc/pS2.dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-pico2hp/pS2.dd-wrt.bin


