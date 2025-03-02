#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531x/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_mips32_gcc-8.2.0_musl/bin:$OLDPATH
cd ar531x/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_ns2_corenet .config
#cp .config_ns2 .config
make -f Makefile.ar531x build_date
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/corenet/ubnt_ns2_corenet
cd ../../../
#cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/ubnt_ls2
#cp ar531x/src/router/mips-uclibc/vmlinux.bin.l7 ~/GruppenLW/releases/$DATE/ubnt_ls2
cp ar531x/src/router/mips-uclibc/ls2-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/corenet/ubnt_ns2_corenet/ubnt_ns2-firmware_corenet.bin
cp ar531x/src/router/mips-uclibc/NS2.dd-wrt.bin ~/GruppenLW/releases/CUSTOMER/$DATE/corenet/ubnt_ns2_corenet/NS2.dd-wrt_corenet.bin


