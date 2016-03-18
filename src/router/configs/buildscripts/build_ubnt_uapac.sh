#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_34kc_gcc-5.3.0_musl-1.1.14/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.1.2-uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_UAP-AC-LR

cp .config_ubntm .config
echo "CONFIG_UBNTXW=y" >> .config
echo "CONFIG_UAPAC=y" >> .config
echo "CONFIG_ATH10K=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/ubnt_UAP-AC-LR/flash-upgrade.bin

cd pb42/src/router
cp .config_ubntm .config
echo "CONFIG_UBNTXW=y" >> .config
echo "CONFIG_UAPAC=y" >> .config
echo "CONFIG_ATH10K=y" >> .config
echo "CONFIG_FREERADIUS=y" >> .config
echo "CONFIG_IPTRAF=y" >> .config
echo "CONFIG_TCPDUMP=y" >> .config
echo "CONFIG_IPVS=y" >> .config
echo "CONFIG_TOR=y" >> .config
echo "CONFIG_PRIVOXY=y" >> .config
echo "CONFIG_BATMANADV=y" >> .config
echo "CONFIG_MC=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_UAP-AC-LR/ubnt_UAP-AC-LR-webflash-firmware.bin
cp notes/unifi/install_uap* ~/GruppenLW/releases/$DATE/ubnt_UAP-AC-LR
