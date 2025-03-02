#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.1.2-uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cat .config_ubntm | grep -v "^CONFIG_CHILLILOCAL\|^CONFIG_SPUTNIK_APD\|^CONFIG_HOTSPOT" >.config
echo "CONFIG_TELCOM=y" >>.config
echo "CONFIG_BRANDING=y" >>.config
echo "CONFIG_TIEXTRA1=y" >>.config
echo "CONFIG_TIEXTRA2=y" >>.config
echo "CONFIG_EAP_SIM=y" >>.config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/telkom/ubnt-ns2m_telkom
cd ../../../

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/telkom/ubnt-ns2m_telkom/ns2m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/CUSTOMER/$DATE/telkom/ubnt-ns2m_telkom/NS2M-DD-WRT.bin

