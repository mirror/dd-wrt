#!/bin/sh
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
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wa7510n
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr741ndv1
mkdir -p ~/GruppenLW/releases/$DATE/alfa-aip-w411
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr741ndv4
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv1
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr703nv1
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv2
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv3
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv4
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr743nv1
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
cd ../../../

cd pb42/src/router
cp .config_wr741 .config
echo "CONFIG_WR841v7=y" >>.config 
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/WR841NDv7-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv7/tl-wr841nd-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR841NDv7-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv7/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/RNX-N300RT-firmware.bin ~/GruppenLW/releases/$DATE/rosewill-rnx-n300rt/rosewill-RNX-N300RT-webflash.bin
cp pb42/src/router/mips-uclibc/rosewill-RNX-N300RT-firmware.bin ~/GruppenLW/releases/$DATE/rosewill-rnx-n300rt/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/WNRT627V1-firmware.bin ~/GruppenLW/releases/$DATE/planex-wnrt627v1/planex-wnrt627v1-webflash.bin
cp pb42/src/router/mips-uclibc/planex-WNRT627V1-firmware.bin ~/GruppenLW/releases/$DATE/planex-wnrt627v1/factory-to-ddwrt.bin

