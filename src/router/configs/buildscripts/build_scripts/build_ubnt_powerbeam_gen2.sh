#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-12.2.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.1.2-uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_bullet_m2_hp
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_bullet_m5_hp
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_rocket_m2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_rocket_m5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_rocket_m365
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_rocket_m3
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_rocket_m900
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoStation_m2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoStation_m5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoStation_m3
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoStation_m365
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_AirGrid_M2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_AirWire
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_AirGrid_M5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Loco_M2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Loco_M5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Pico_M2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Pico_M5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Loco_M900
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M3
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M365
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M900
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M365
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M10
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_AirRouter
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_PowerAP-N
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_AirRouter_HP
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_UAP
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_UAP-v2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_UAP-LR-v2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_UAP-LR
#mkdir -p ~/GruppenLW/releases/$DATE/ubnt_UniFi_AC-LR
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_WispStation_M5

mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBeam_M2-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBeam_M5-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBeam_M5-M400-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_PowerBeam_M5-M400-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_PowerBeam_AC-GEN2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoStation_M5-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoStation_M2-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Rocket_M2-Titanium-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-Titanium-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-X3-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Rocket_M2-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_AirGrid_M5-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Loco_M5-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Loco_M2-XW

cp .config_ubntm .config
echo "CONFIG_UBNTXW=y" >> .config
echo "CONFIG_UAPAC=y" >> .config
echo "CONFIG_NANOAC=y" >> .config
echo "CONFIG_UAPACPRO=y" >> .config
echo "CONFIG_POWERBEAMAC_GEN2=y" >> .config
echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_ATH10K=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_FREERADIUS=y" >> .config
echo "CONFIG_IPTRAF=y" >> .config
#echo "CONFIG_TCPDUMP=y" >> .config
echo "CONFIG_IPVS=y" >> .config
echo "CONFIG_TOR=y" >> .config
echo "CONFIG_PRIVOXY=y" >> .config
echo "CONFIG_BATMANADV=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_MC=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_IPERF=y" >> .config
echo "CONFIG_HTOP=y" >> .config
echo "CONFIG_MTR=y" >> .config
echo "CONFIG_SPEEDTEST_CLI=y" >> .config

make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_PowerBeam_AC-GEN2/ubnt_PowerBeam_AC-GEN2-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_PowerBeam_AC-GEN2/XW-DD-WRT.bin
cp pb42/src/router/ubnt_mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt_PowerBeam_AC-GEN2/flash-upgrade.bin

