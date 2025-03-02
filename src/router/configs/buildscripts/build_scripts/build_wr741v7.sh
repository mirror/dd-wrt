#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-12.2.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wa7510n
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr741ndv1
mkdir -p ~/GruppenLW/releases/$DATE/alfa-aip-w411
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr741ndv4
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr741ndv5
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv1
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr703nv1
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv2
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv3
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv4
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-mr3220v2
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv5
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr743ndv1
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr743ndv2
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr840nv1
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv5
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv7
mkdir -p ~/GruppenLW/releases/$DATE/planex-wnrt627v1
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-mr3420v1
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-mr3220v1
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-mr3020
mkdir -p ~/GruppenLW/releases/$DATE/rosewill-rnx-n150rt
mkdir -p ~/GruppenLW/releases/$DATE/rosewill-rnx-n300rt
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr941ndv4
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wa901ndv1
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wa801ndv1
cp .config_mr3420 .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/MR3420-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-mr3420v1/tl-mr3420v1-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-MR3420-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-mr3420v1/factory-to-ddwrt.bin


