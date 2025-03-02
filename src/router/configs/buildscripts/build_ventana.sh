#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n ventana/src/router/libutils)
#export PATH=/xfs/toolchains/toolchain-laguna-new/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-arm_cortex-a9+neon_gcc-13.1.0_musl_eabi/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-arm_v7-a_gcc-4.8-linaro_uClibc-0.9.33.2-eabi-imx6/bin:$OLDPATH
cd ventana/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_ventana .config
echo "CONFIG_SMP=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_ZFS=y" >> .config
echo "CONFIG_RAID=y" >> .config
echo "CONFIG_NTFSPROGS=y" >> .config
echo "CONFIG_BTRFSPROGS=y" >> .config
echo "CONFIG_XFSPROGS=y" >> .config
echo "CONFIG_IRQBALANCE=y" >> .config
echo "CONFIG_RSYNC=y" >> .config
echo "CONFIG_CAKE=y" >> .config
echo "CONFIG_SAMBA4=y" >> .config
echo "CONFIG_SMBD=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
echo "CONFIG_IWLWIFI=y" >> .config
echo "KERNELVERSION=4.9" >> .config
echo "CONFIG_MAC80211_RT2800=y" >> .config
echo "CONFIG_MAC80211_RTLWIFI=y" >> .config
echo "CONFIG_MT7615=y" >> .config
echo "CONFIG_MT7662=y" >> .config
echo "CONFIG_BRCMFMAC=y" >> .config
echo "CONFIG_MRP=y" >> .config
echo "CONFIG_CFM=y" >> .config
echo "CONFIG_HTOP=y" >> .config
echo "CONFIG_P7ZIP=y" >> .config
echo "CONFIG_MDNS=y" >> .config
echo "CONFIG_MDNS_UTILS=y" >> .config
echo "CONFIG_QCA9984=y" >> .config
#echo "CONFIG_PLEX=y" >> .config
#echo "CONFIG_LOCKDEBUG=y" >> .config

#cp .config_laguna .config
make -f Makefile.ventana kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/gateworks-ventana-gw5xxx
#mkdir -p ~/GruppenLW/releases/$DATE/gateworks-ventana-gw53xx
#mkdir -p ~/GruppenLW/releases/$DATE/gateworks-ventana-gw52xx
#mkdir -p ~/GruppenLW/releases/$DATE/gateworks-ventana-gw51xx
cd ../../../
cp ventana/src/router/arm-uclibc/root-aligned.ubi ~/GruppenLW/releases/$DATE/gateworks-ventana-gw5xxx/gw5xxx.ubi
cp ventana/src/router/arm-uclibc/webflash-ventana.trx ~/GruppenLW/releases/$DATE/gateworks-ventana-gw5xxx/ventana-webflash.bin



