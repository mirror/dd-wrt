#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n magicbox/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-powerpc_8540_gcc-5.2.0_musl-1.1.11/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-powerpc_gcc-linaro_uClibc-0.9.32/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-powerpc_603e_gcc-5.4.0_musl-1.1.16/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-powerpc_gcc-linaro_uClibc-0.9.32/bin:$OLDPATH
#export PATH=/opt/staging_dir_powerpc/bin:$OLDPATH
cd magicbox/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_rb600 .config
echo "CONFIG_RB800=y" >> .config
echo "CONFIG_ANCHORFREE=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_RAID=y" >> .config
echo "CONFIG_ZFS=y" >> .config
echo "CONFIG_NFS=y" >> .config
echo "CONFIG_SAMBA=y" >> .config
echo "CONFIG_SAMBA3=y" >> .config
echo "CONFIG_SAMBA4=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
echo "CONFIG_SMBD=y" >> .config
echo "CONFIG_MDNS=y" >> .config
echo "CONFIG_MDNS_UTILS=y" >> .config


make -f Makefile.magicbox kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/mikrotik-rb800
cd ../../../
#cp magicbox/src/router/rb600.img ~/GruppenLW/releases/$DATE/mikrotik-rb800/rb800.img
#cp magicbox/src/router/rb600.bin ~/GruppenLW/releases/$DATE/mikrotik-rb800/rb800-webupgrade.bin
