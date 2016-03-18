#!/bin/sh
#./build_rs.sh
#./build_rs_ddlink.sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_lsx .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_LS-SR71A
cd ../../../
#cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/ubnt_ls2
#cp ar531x/src/router/mips-uclibc/vmlinux.bin.l7 ~/GruppenLW/releases/$DATE/ubnt_ls2
cp pb42/src/router/mips-uclibc/lsx-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_LS-SR71A/LS-SR71A-firmware.bin
cp pb42/src/router/mips-uclibc/LSX.dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_LS-SR71A/LS-SR71A.dd-wrt.bin


cd pb42/src/router

cp .config_lsx_proto .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_LS-SR71A_prototype
cd ../../../
#cp ar531x/src/router/mips-uclibc/root.fs ~/GruppenLW/releases/$DATE/ubnt_ls2
#cp ar531x/src/router/mips-uclibc/vmlinux.bin.l7 ~/GruppenLW/releases/$DATE/ubnt_ls2
cp pb42/src/router/mips-uclibc/lsx-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_LS-SR71A_prototype/LS-SR71A_prototype-firmware.bin
cp pb42/src/router/mips-uclibc/LSX.dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_LS-SR71A_prototype/LS-SR71A_prototype.dd-wrt.bin

