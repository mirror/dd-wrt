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
cp .config_tw6600_maksat .config
make -f Makefile.ar531x build_date
make -f Makefile.ar531x kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/aw6600_maksat
mkdir -p ~/GruppenLW/releases/$DATE/aw6600_maksat/original_to_dd-wrt
cd ../../../
cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/aw6600_maksat
cp ar531x/src/router/mips-uclibc/lzma_vmlinusaw ~/GruppenLW/releases/$DATE/aw6600_maksat/lzma_vmlinus
cp ar531x/src/router/mips-uclibc/whrag108-firmware.bin ~/GruppenLW/releases/$DATE/aw6600_maksat/aw6600-maksat-firmware.bin


cp ar531x/tools/aw6660/* ~/GruppenLW/releases/$DATE/aw6600_maksat/original_to_dd-wrt

#prepare overtake image
ar531x/tools/inject ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/aw6600_maksat/original_to_dd-wrt/firmware.img 196608
ar531x/tools/inject ar531x/src/router/mips-uclibc/lzma_vmlinusaw ~/GruppenLW/releases/$DATE/aw6600_maksat/original_to_dd-wrt/firmware.img 2949120
ar531x/tools/fischecksum ~/GruppenLW/releases/$DATE/aw6600_maksat/original_to_dd-wrt/firmware.img
