#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.1.2-uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
#cp .config_ubntm .config
cat .config_ubntm | grep -v "^CONFIG_WIKAR\|^CONFIG_NOMESSAGE\|^CONFIG_WPA3" > .config 

echo "CONFIG_TMK=y" >> .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_WPA3=n" >> .config
echo "CONFIG_CONFIG_DEBUG_SYSLOG=n" >> .config
echo "CONFIG_NOMESSAGE=n" >> .config

make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/ubnt-m_kmt
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt-UniFi_AP


cd ../../../

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/ubnt-m_kmt/ubnt-m_webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/CUSTOMER/$DATE/ubnt-m_kmt/UBNT_M_TO-KMT.bin

exit

cd pb42/src/router
cp .config_unifi .config
echo "CONFIG_TMK=y" >> .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_WPA3=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt-UniFi_AP/UniFiAP-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntbz-dd-wrt.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt-UniFi_AP/UAP-DD-WRT.bin
cp notes/unifi/install.txt ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt-UniFi_AP

