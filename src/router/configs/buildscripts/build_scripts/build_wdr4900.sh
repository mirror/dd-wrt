#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n mpc85xx/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-powerpc_8540_gcc-8.2.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-powerpc_603e_gcc-5.4.0_musl-1.1.16.2/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd mpc85xx/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
mkdir -p ~/GruppenLW/releases/$DATE/tplink_tl-wdr4900v1
cp .config_wdr4900 .config
echo "CONFIG_SPEEDCHECKER=y" >> .config
echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_SMBD=y" >> .config
echo "CONFIG_MDNS=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
make -f Makefile.mpc85xx kernel clean all install
cd ../../../
cp mpc85xx/src/router/powerpc-uclibc/wdr4900-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr4900v1/tl-wdr4900-webflash.bin
cp mpc85xx/src/router/powerpc-uclibc/tplink-wdr4900-firmware.bin ~/GruppenLW/releases/$DATE/tplink_tl-wdr4900v1/factory-to-ddwrt.bin

