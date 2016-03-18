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
cp .config_wr741 .config
make -f Makefile.pb42 kernel clean all install
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
cd ../../../
cp pb42/src/router/mips-uclibc/WR741NDv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr741ndv1/tl-wr741ndv1-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR741NDv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr741ndv1/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/AIP-W411-firmware.bin ~/GruppenLW/releases/$DATE/alfa-aip-w411/alfa-aip-w411-webflash.bin
cp pb42/src/router/mips-uclibc/alfa-AIP-W411-firmware.bin ~/GruppenLW/releases/$DATE/alfa-aip-w411/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/RNX-N150RT-firmware.bin ~/GruppenLW/releases/$DATE/rosewill-rnx-n150rt/rosewill-RNX-N150RT-webflash.bin
cp pb42/src/router/mips-uclibc/rosewill-RNX-N150RT-firmware.bin ~/GruppenLW/releases/$DATE/rosewill-rnx-n150rt/factory-to-ddwrt.bin

cd pb42/src/router
echo "CONFIG_WA7510=y" >>.config 
make -f Makefile.pb42 services-clean services  libutils-clean libutils install
cd ../../../
cp pb42/src/router/mips-uclibc/WA7510N-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wa7510n/tl-wa7510n-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WA7510N-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wa7510n/factory-to-ddwrt.bin


#
cd pb42/src/router
cp .config_wr741 .config
echo "CONFIG_WR841v5=y" >>.config 
make -f Makefile.pb42 services-clean services  libutils-clean libutils install
cd ../../../
cp pb42/src/router/mips-uclibc/WR841NDv5-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv5/tl-wr841ndv5-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR841NDv5-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv5/factory-to-ddwrt.bin


cd pb42/src/router
cp .config_wr741 .config
echo "CONFIG_WR841v7=y" >>.config 
make -f Makefile.pb42 services-clean services libutils-clean libutils install
cd ../../../
cp pb42/src/router/mips-uclibc/WR841NDv7-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv7/tl-wr841ndv7-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR841NDv7-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr841ndv7/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/RNX-N300RT-firmware.bin ~/GruppenLW/releases/$DATE/rosewill-rnx-n300rt/rosewill-RNX-N300RT-webflash.bin
cp pb42/src/router/mips-uclibc/rosewill-RNX-N300RT-firmware.bin ~/GruppenLW/releases/$DATE/rosewill-rnx-n300rt/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/planex-WNRT627V1-firmware.bin ~/GruppenLW/releases/$DATE/planex-wnrt627v1/planex-wnrt627v1-webflash.bin
cp pb42/src/router/mips-uclibc/WNRT627V1-firmware.bin ~/GruppenLW/releases/$DATE/planex-wnrt627v1/factory-to-ddwrt.bin




cd pb42/src/router
cp .config_wr741 .config
echo "CONFIG_WA901v1=y" >>.config 
make -f Makefile.pb42 services-clean services  libutils-clean libutils install
cd ../../../
cp pb42/src/router/mips-uclibc/WA901NDv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wa901ndv1/tl-wa901ndv1-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WA901NDv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wa901ndv1/factory-to-ddwrt.bin

cd pb42/src/router
cp .config_wr741 .config
echo "CONFIG_WA901v1=y" >>.config 
echo "CONFIG_WA801v1=y" >>.config 
make -f Makefile.pb42 services-clean services  libutils-clean libutils install
cd ../../../
cp pb42/src/router/mips-uclibc/WA801NDv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wa801ndv1/tl-wa801ndv1-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WA801NDv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wa801ndv1/factory-to-ddwrt.bin


cd pb42/src/router


cp .config_wr741 .config
echo "CONFIG_WR740v1=y" >>.config 
make -f Makefile.pb42 services-clean services  libutils-clean libutils install
cd ../../../
cp pb42/src/router/mips-uclibc/WR740NDv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv1/tl-wr740nv1-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR740NDv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv1/factory-to-ddwrt.bin
cp pb42/src/router/mips-uclibc/WR740NDv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv2/tl-wr740nv2-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR740NDv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv2/factory-to-ddwrt.bin
cp pb42/src/router/mips-uclibc/WR740NDv3-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv3/tl-wr740nv3-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR740NDv3-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv3/factory-to-ddwrt.bin
cp pb42/src/router/mips-uclibc/WR740NDv3ww-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv3/tl-wr740nv3-webflash-ww.bin
cp pb42/src/router/mips-uclibc/tplink-WR740NDv3ww-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv3/factory-to-ddwrt-ww.bin

cd pb42/src/router
cp .config_wr741 .config
echo "CONFIG_WR743=y" >>.config 
make -f Makefile.pb42 services-clean services  libutils-clean libutils install
cd ../../../

cp pb42/src/router/mips-uclibc/WR743NDv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr743ndv1/tl-wr743ndv1-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR743NDv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr743ndv1/factory-to-ddwrt.bin


#
cd pb42/src/router
cp .config_wr741 .config
echo "CONFIG_WR941v4=y" >>.config 
make -f Makefile.pb42 services-clean services  libutils-clean libutils install
cd ../../../
cp pb42/src/router/mips-uclibc/WR941NDv4-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr941ndv4/tl-wr941ndv4-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR941NDv4-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr941ndv4/factory-to-ddwrt.bin


cd pb42/src/router
cp .config_wr741 .config
echo "CONFIG_WR841v7=y" >>.config 
echo "CONFIG_WR840v1=y" >>.config 
make -f Makefile.pb42 services-clean services  libutils-clean libutils install
cd ../../../
cp pb42/src/router/mips-uclibc/WR840Nv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr840nv1/tl-wr840nv1-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR840Nv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr840nv1/factory-to-ddwrt.bin


cd pb42/src/router
cp .config_wr741v4 .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/WR741NDv4-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr741ndv4/tl-wr741ndv4-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR741NDv4-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr741ndv4/factory-to-ddwrt.bin

cd pb42/src/router
cp .config_wr741v4 .config
echo "CONFIG_WR740V4=y" >>.config 
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/WR740NDv4-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv4/tl-wr740nv4-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR740NDv4-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv4/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/WR740NDv5-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv5/tl-wr740nv5-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR740NDv5-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr740nv5/factory-to-ddwrt.bin

cp pb42/src/router/mips-uclibc/WR740NDv5-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr741ndv5/tl-wr741ndv5-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR740NDv5-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr741ndv5/factory-to-ddwrt.bin

cd pb42/src/router


cp .config_wr741v4 .config
echo "CONFIG_WR743V2=y" >>.config 
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/WR743NDv2-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr743ndv2/tl-wr743ndv2-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR743NDv2-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr743ndv2/factory-to-ddwrt.bin

cd pb42/src/router


cp .config_wr703 .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/WR703Nv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr703nv1/tl-wr703v1-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-WR703Nv1-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wr703nv1/factory-to-ddwrt.bin

cd pb42/src/router
cp .config_wr703 .config

echo "CONFIG_MR3020=y" >>.config 
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/MR3020-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-mr3020/tl-MR3020-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-MR3020-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-mr3020/factory-to-ddwrt.bin


cd pb42/src/router
cp .config_mr3420 .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/MR3420-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-mr3420v1/tl-mr3420v1-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-MR3420-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-mr3420v1/factory-to-ddwrt.bin

cd pb42/src/router
cp .config_mr3420 .config
echo "CONFIG_MR3220=y" >>.config 
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/MR3220-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-mr3220v1/tl-mr3220v1-webflash.bin
cp pb42/src/router/mips-uclibc/tplink-MR3220-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-mr3220v1/factory-to-ddwrt.bin

