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
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt-UAP-AC-LR
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt-UAP-AC-MESH
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt-UAP-AC-PRO
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt-NanoBeam-AC_Gen2

#cp .config_ubntm .config
#echo "CONFIG_TMK=y" >> .config
#echo "CONFIG_BRANDING=y" >> .config
#echo "CONFIG_TDMA=y" >> .config
#echo "CONFIG_UBNTXW=y" >> .config
#echo "CONFIG_UAPAC=y" >> .config
#echo "CONFIG_ATH10K=y" >> .config
#echo "CONFIG_WPA3=y" >> .config
#echo "CONFIG_WIREGUARD=y" >> .config
#make -f Makefile.pb42 kernel clean all install
#cd ../../../
#cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/ubnt-UAP-AC-LR/flash-upgrade.bin
#cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/ubnt-UAP-AC-MESH/flash-upgrade.bin
#
#cd pb42/src/router
#cp .config_ubntm .config
#echo "CONFIG_TMK=y" >> .config
#echo "CONFIG_BRANDING=y" >> .config
#echo "CONFIG_TDMA=y" >> .config
#echo "CONFIG_UBNTXW=y" >> .config
#echo "CONFIG_UAPAC=y" >> .config
#echo "CONFIG_ATH10K=y" >> .config
#echo "CONFIG_FREERADIUS=y" >> .config
#echo "CONFIG_IPTRAF=y" >> .config
#echo "CONFIG_TCPDUMP=y" >> .config
#echo "CONFIG_IPVS=y" >> .config
#echo "CONFIG_TOR=y" >> .config
#echo "CONFIG_PRIVOXY=y" >> .config
#echo "CONFIG_BATMANADV=y" >> .config
#echo "CONFIG_WPA3=y" >> .config
#echo "CONFIG_MC=y" >> .config
#echo "CONFIG_WIREGUARD=y" >> .config
#make -f Makefile.pb42 kernel clean all install
#cd ../../../
#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-UAP-AC-LR/ubnt-UAP-AC-LR-webflash-firmware.bin
#cp notes/unifi/install_uap* ~/GruppenLW/releases/$DATE/ubnt-UAP-AC-LR
#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-UAP-AC-MESH/ubnt-UAP-AC-M-webflash-firmware.bin
#cp notes/unifi/install_uap* ~/GruppenLW/releases/$DATE/ubnt-UAP-AC-MESH
#
#
#
#cd pb42/src/router
#cp .config_ubntm .config
#echo "CONFIG_TMK=y" >> .config
#echo "CONFIG_BRANDING=y" >> .config
#echo "CONFIG_TDMA=y" >> .config
#echo "CONFIG_UBNTXW=y" >> .config
#echo "CONFIG_UAPAC=y" >> .config
#echo "CONFIG_UAPACPRO=y" >> .config
#echo "CONFIG_ATH10K=y" >> .config
#echo "CONFIG_WPA3=y" >> .config
#echo "CONFIG_WIREGUARD=y" >> .config
#make -f Makefile.pb42 kernel clean all install
#cd ../../../
#cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/ubnt-UAP-AC-PRO/flash-upgrade.bin
#
#cd pb42/src/router
#cp .config_ubntm .config
#echo "CONFIG_TMK=y" >> .config
#echo "CONFIG_BRANDING=y" >> .config
#echo "CONFIG_TDMA=y" >> .config
#echo "CONFIG_UBNTXW=y" >> .config
#echo "CONFIG_UAPAC=y" >> .config
#echo "CONFIG_UAPACPRO=y" >> .config
#echo "CONFIG_ATH10K=y" >> .config
#echo "CONFIG_FREERADIUS=y" >> .config
#echo "CONFIG_IPTRAF=y" >> .config
#echo "CONFIG_TCPDUMP=y" >> .config
#echo "CONFIG_IPVS=y" >> .config
#echo "CONFIG_TOR=y" >> .config
#echo "CONFIG_PRIVOXY=y" >> .config
#echo "CONFIG_BATMANADV=y" >> .config
#echo "CONFIG_WPA3=y" >> .config
#echo "CONFIG_MC=y" >> .config
#echo "CONFIG_WIREGUARD=y" >> .config
#make -f Makefile.pb42 kernel clean all install
#cd ../../../
#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-UAP-AC-PRO/ubnt-UAP-AC-LR-webflash-firmware.bin
#cp notes/unifi/install_uap* ~/GruppenLW/releases/$DATE/ubnt-UAP-AC-PRO

cd pb42/src/router
cp .config_ubntm .config
echo "CONFIG_TMK=y" >> .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_UBNTXW=y" >> .config
echo "CONFIG_UAPAC=y" >> .config
echo "CONFIG_NANOAC=y" >> .config
echo "CONFIG_UAPACPRO=y" >> .config
echo "CONFIG_ATH10K=y" >> .config
echo "CONFIG_FREERADIUS=y" >> .config
echo "CONFIG_IPTRAF=y" >> .config
echo "CONFIG_TCPDUMP=y" >> .config
echo "CONFIG_IPVS=y" >> .config
echo "CONFIG_TOR=y" >> .config
echo "CONFIG_PRIVOXY=y" >> .config
echo "CONFIG_BATMANADV=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_MC=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt-NanoBeam-AC_Gen2/flash-upgrade.bin
cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt-NanoBeam-AC_Gen2/ubnt-NanoBeam-AC_Gen2-webflash-firmware.bin
cp notes/unifi/install_uap/install_uapv2.txt ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/ubnt-NanoBeam-AC_Gen2/initial_flashing.txt

