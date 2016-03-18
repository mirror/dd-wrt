#!/bin/sh

OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ar531x/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_mips32_gcc-5.2.0_musl-1.1.12/bin:$OLDPATH
cd ar531x/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../

cp .config_wrt54g2 .config
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/linksys-wrt54g2v11
cd ../../../
cp ar531x/src/router/mips-uclibc/vmlinux.wrt54g2 ~/GruppenLW/releases/$DATE/linksys-wrt54g2v11/linux.bin
cp ar531x/src/router/mips-uclibc/wrt54g2-firmware.bin ~/GruppenLW/releases/$DATE/linksys-wrt54g2v11/wrt54g2v11-firmware.bin

cp notes/WRT54G2/* ~/GruppenLW/releases/$DATE/linksys-wrt54g2v11
