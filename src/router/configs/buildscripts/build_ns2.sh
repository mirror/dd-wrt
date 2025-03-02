#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531xr2/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_4kec_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin:$OLDPATH
cd ar531xr2/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_ns2 .config
make -f Makefile.ar531x build_date
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-ns2
cd ../../../
#cp ar531xr2/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/ubnt-ls2
#cp ar531xr2/src/router/mips-uclibc/vmlinux.bin.l7 ~/GruppenLW/releases/$DATE/ubnt-ls2
cp ar531xr2/src/router/mips-uclibc/ls2-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-ns2/ubnt-ns2-firmware.bin
cp ar531xr2/src/router/mips-uclibc/NS2.dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-ns2/NS2.dd-wrt.bin


