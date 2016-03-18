#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_wr1043v2 .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr1043nd-v2
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wr1043nd-v3
cd ../../../
cp pb42/src/router/mips-uclibc/TL-WR1043V2-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr1043nd-v2/tplink_tl-wr1043nd-v2.bin
cp pb42/src/router/mips-uclibc/tplink-TL-WR1043V2-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr1043nd-v2/factory-to-ddwrt.bin
cd pb42/src/router
echo "CONFIG_WR1043V3=y" >>.config 
make -f Makefile.pb42 libutils-clean libutils install
cd ../../../

cp pb42/src/router/mips-uclibc/TL-WR1043V3-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr1043nd-v3/tplink_tl-wr1043nd-v3.bin
cp pb42/src/router/mips-uclibc/tplink-TL-WR1043V3-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr1043nd-v3/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/TL-WR1043V3-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr1043nd-v3/tplink_tl-wr1043nd-v3-US.bin
cp pb42/src/router/mips-uclibc/tplink-TL-WR1043V3-firmwareUS.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr1043nd-v3/factory-to-ddwrt-US.bin

