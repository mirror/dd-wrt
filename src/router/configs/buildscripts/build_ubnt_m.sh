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
cp .config_ubntm .config
make -f Makefile.pb42 kernel clean all install
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
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M365
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M10
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_AirRouter
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_PowerAP-N
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_AirRouter_HP
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_UAP
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_UAP-LR
#mkdir -p ~/GruppenLW/releases/$DATE/ubnt_UniFi_AC-LR
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_WispStation_M5

mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBeam_M2-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBeam_M5-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoStation_M5-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Rocket_M2-Titanium-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-Titanium-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-X3-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Airgrid_M5-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Loco_M5-XW

cd ../../../

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_PowerAP-N/powerAP-N-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_PowerAP-N/PAPN-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_AirRouter_HP/airrouter-hp-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_AirRouter_HP/AR-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_AirRouter/airrouter-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_AirRouter/AR-DD-WRT.bin


cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_bullet_m2_hp/bs2m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_bullet_m2_hp/BS2M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M2/lc2m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M2/LC2M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M5/lc5m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M5/LC5M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Pico_M5/pc5m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Pico_M5/PC5M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Pico_M2/pc2m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Pico_M2/PC2M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_AirWire/awm-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_AirWire/AWM-DD-WRT.bin


cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M900/lc900m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M900/LC900M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_bullet_m5_hp/bs5m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_bullet_m5_hp/BS5M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_rocket_m2/r2m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_rocket_m2/R2M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_rocket_m5/r5m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_rocket_m5/R5M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_rocket_m365/r365m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_rocket_m365/R365M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_rocket_m3/r3m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_rocket_m3/R3M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_rocket_m900/r900m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_rocket_m900/R900M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_m2/ns2m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_m2/NS2M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_m5/ns5m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_m5/NS5M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_m3/ns3m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_m3/NS3M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_m365/ns365m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_m365/NS365M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_AirGrid_M2/airgridm2-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_AirGrid_M2/AGM2-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_AirGrid_M5/airgridm5-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_AirGrid_M5/AGM5-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M2/nanobridgem2-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M2/NBM2-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M5/nanobridgem5-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M5/NBM5-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M3/nanobridgem3-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M3/NBM3-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M365/nanobridgem365-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M365/NBM365-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M900/nanobridgem900-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M900/NBM900-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M5/powerbridgem5-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M5/PBM5-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M365/powerbridgem365-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M365/PBM365-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M10/powerbridgem10-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M10/PBM10-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_WispStation_M5/wispstationm5-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_WispStation_M5/WSM5-DD-WRT.bin

cd pb42/src/router

cp .config_ubntm .config
echo "CONFIG_UBNTXW=y" >> .config
echo "CONFIG_LOCOXW=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBeam_M2-XW/ubnt_NanoBeam_M2-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBeam_M2-XW/XW-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBeam_M5-XW/ubnt_NanoBeam_M5-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBeam_M5-XW/XW-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Airgrid_M5-XW/ubnt_Airgrid_M5-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Airgrid_M5-XW/XW-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M5-XW/ubnt_Loco_M5-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M5-XW/XW-DD-WRT.bin

cd pb42/src/router
cp .config_ubntm .config
echo "CONFIG_UBNTXW=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_M5-XW/ubnt_NanoStation_M5-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_M5-XW/XW-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M2-Titanium-XW/ubnt_Rocket_M2-Titanium-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M2-Titanium-XW/XW-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-Titanium-XW/ubnt_Rocket_M5-Titanium-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-Titanium-XW/XW-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-X3-XW/ubnt_Rocket_M5-X3-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Rocket_M5-X3-XW/XW-DD-WRT.bin


#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_UniFi_AC-LR/UniFiAC-LR-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntbz2-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_UniFi_AC-LR/UAC-LR-DD-WRT.bin


cd pb42/src/router
cp .config_unifi .config
make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_UAP/UniFiAP-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_UAP-LR/UniFiAP-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntbz-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_UAP/UAP-DD-WRT.bin
cp pb42/src/router/mips-uclibc/ubntbz-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_UAP-LR/UAP-DD-WRT.bin
cp notes/unifi/install.txt ~/GruppenLW/releases/$DATE/ubnt_UAP
cp notes/unifi/install.txt ~/GruppenLW/releases/$DATE/ubnt_UAP-LR


