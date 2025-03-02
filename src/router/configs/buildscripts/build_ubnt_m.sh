#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/libutils)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.1.2-uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH
cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_ubntm .config
#echo "CONFIG_TDMA=y" >> .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-bullet_m2_hp
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-bullet_m5_hp
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-rocket_m2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-rocket_m5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-rocket_m365
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-rocket_m3
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-rocket_m900
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m3
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m365
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-AirGrid_M2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-AirWire
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-AirGrid_M5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-Loco_M2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-Loco_M5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-Pico_M2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-Pico_M5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-Loco_M900
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M3
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M365
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M900
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-PowerBridge_M5
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-PowerBridge_M365
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-PowerBridge_M10
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-AirRouter
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-PowerAP-N
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-AirRouter_HP
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-UAP
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-UAP-v2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-UAP-LR-v2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-UAP-LR
#mkdir -p ~/GruppenLW/releases/$DATE/ubnt-UniFi-AC-LR
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-WispStation_M5

mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoBeam_M2-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoBeam_M5-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoBeam_M5-M400-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-PowerBeam_M5-M400-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-PowerBeam-AC-Gen2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-LiteBeam-AC-Gen2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-LiteAP-AC
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoStation-5AC-loco
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoBeam-AC-Gen1
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoBeam-AC-Gen2
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoStation_M5-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-NanoStation_M2-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-Rocket_M2-Titanium-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-Rocket_M5-Titanium-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-Rocket_M5-X3-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-Rocket_M5-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-Rocket_M2-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-AirGrid_M5-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-Loco_M5-XW
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-Loco_M2-XW

cd ../../../

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-PowerAP-N/powerAP-N-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-PowerAP-N/PAPN-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-PowerAP-N/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-PowerAP-N

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-AirRouter_HP/airrouter-hp-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-AirRouter_HP/AR-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-AirRouter_HP/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-AirRouter_HP

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-AirRouter/airrouter-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-AirRouter/AR-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-AirRouter/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-AirRouter


cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-bullet_m2_hp/bs2m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-bullet_m2_hp/BS2M-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-bullet_m2_hp/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-bullet_m2_hp

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-Loco_M2/lc2m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-Loco_M2/LC2M-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-Loco_M2/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-Loco_M2

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-Loco_M5/lc5m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-Loco_M5/LC5M-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-Loco_M5/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-Loco_M5

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-Pico_M5/pc5m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-Pico_M5/PC5M-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-Pico_M5/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-Pico_M5

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-Pico_M2/pc2m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-Pico_M2/PC2M-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-Pico_M2/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-Pico_M2

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-AirWire/awm-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-AirWire/AWM-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-AirWire/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-AirWire


cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-Loco_M900/lc900m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-Loco_M900/LC900M-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-Loco_M900/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-Loco_M900

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-bullet_m5_hp/bs5m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-bullet_m5_hp/BS5M-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-bullet_m5_hp/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-bullet_m5_hp

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-rocket_m2/r2m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-rocket_m2/R2M-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-rocket_m2/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-rocket_m2

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-rocket_m5/r5m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-rocket_m5/R5M-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-rocket_m5/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-rocket_m5

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-rocket_m365/r365m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-rocket_m365/R365M-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-rocket_m365/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-rocket_m365

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-rocket_m3/r3m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-rocket_m3/R3M-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-rocket_m3/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-rocket_m3

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-rocket_m900/r900m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-rocket_m900/R900M-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-rocket_m900/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-rocket_m900

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m2/ns2m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m2/NS2M-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m2/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m2

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m5/ns5m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m5/NS5M-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m5/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m5

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m3/ns3m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m3/NS3M-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m3/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m3

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m365/ns365m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m365/NS365M-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m365/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-NanoStation_m365

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-AirGrid_M2/airgridm2-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-AirGrid_M2/AGM2-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-AirGrid_M2/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-AirGrid_M2

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-AirGrid_M5/airgridm5-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-AirGrid_M5/AGM5-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-AirGrid_M5/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-AirGrid_M5

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M2/nanobridgem2-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M2/NBM2-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M2/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-AirGrid_M2

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M5/nanobridgem5-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M5/NBM5-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M5/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-AirGrid_M5

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M3/nanobridgem3-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M3/NBM3-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M3/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M3

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M365/nanobridgem365-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M365/NBM365-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M365/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M365

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M900/nanobridgem900-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M900/NBM900-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M900/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-NanoBridge_M900

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-PowerBridge_M5/powerbridgem5-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-PowerBridge_M5/PBM5-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-PowerBridge_M5/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-PowerBridge_M5

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-PowerBridge_M365/powerbridgem365-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-PowerBridge_M365/PBM365-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-PowerBridge_M365/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-PowerBridge_M365

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-PowerBridge_M10/powerbridgem10-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-PowerBridge_M10/PBM10-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-PowerBridge_M10/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-PowerBridge_M10

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-WispStation_M5/wispstationm5-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-WispStation_M5/WSM5-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-WispStation_M5/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-WispStation_M5

cd pb42/src/router

cp .config_ubntm .config
echo "CONFIG_UBNTXW=y" >> .config
echo "CONFIG_LOCOXW=y" >> .config
#echo "CONFIG_TDMA=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBeam_M2-XW/ubnt-NanoBeam_M2-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBeam_M2-XW/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-NanoBeam_M2-XW/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-NanoBeam_M2-XW

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBeam_M5-XW/ubnt-NanoBeam_M5-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBeam_M5-XW/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-NanoBeam_M5-XW/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-NanoBeam_M5-XW

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-AirGrid_M5-XW/ubnt-Airgrid_M5-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-AirGrid_M5-XW/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-AirGrid_M5-XW/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-AirGrid_M5-XW

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-Loco_M5-XW/ubnt-Loco_M5-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-Loco_M5-XW/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-Loco_M5-XW/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-Loco_M5-XW

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-Loco_M2-XW/ubnt-Loco_M2-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-Loco_M2-XW/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-Loco_M2-XW/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-Loco_M2-XW

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-UAP-v2/UniFiAPv2-webflash-firmware.bin
cp pb42/src/router/ubnt-mtd/mtd ~/GruppenLW/releases/$DATE/ubnt-UAP-v2/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-UAP-v2

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-UAP-LR-v2/UniFiAPv2-webflash-firmware.bin
cp pb42/src/router/ubnt-mtd/mtd ~/GruppenLW/releases/$DATE/ubnt-UAP-LR-v2/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-UAP-LR-v2


cd pb42/src/router


cp .config_ubntm .config
echo "CONFIG_UBNTXW=y" >> .config
echo "CONFIG_M400XW=y" >> .config
#echo "CONFIG_TDMA=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBeam_M5-M400-XW/ubnt-NanoBeam_M5-M400-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBeam_M5-M400-XW/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-NanoBeam_M5-M400-XW/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-NanoBeam_M5-M400-XW

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-PowerBeam_M5-M400-XW/ubnt-NanoBeam_M5-M400-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-PowerBeam_M5-M400-XW/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-PowerBeam_M5-M400-XW/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-NanoBeam_M5-M400-XW

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-Rocket_M5-XW/ubnt-Rocket_M5-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-Rocket_M5-XW/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-Rocket_M5-XW/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-Rocket_M5-XW

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-Rocket_M2-XW/ubnt-Rocket_M2-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-Rocket_M2-XW/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-Rocket_M2-XW/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-Rocket_M2-XW

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-Rocket_M2-Titanium-XW/ubnt-Rocket_M2-Titanium-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-Rocket_M2-Titanium-XW/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-Rocket_M2-Titanium-XW/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-Rocket_M2-Titanium-XW

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-Rocket_M5-Titanium-XW/ubnt-Rocket_M5-Titanium-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-Rocket_M5-Titanium-XW/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-Rocket_M5-Titanium-XW/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-Rocket_M5-Titanium-XW

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-Rocket_M5-X3-XW/ubnt-Rocket_M5-X3-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-Rocket_M5-X3-XW/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-Rocket_M5-X3-XW/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-Rocket_M5-X3-XW


cd pb42/src/router
cp .config_ubntm .config
echo "CONFIG_UBNTXW=y" >> .config
echo "CONFIG_UAPAC=y" >> .config
echo "CONFIG_NANOAC=y" >> .config
echo "CONFIG_UAPACPRO=y" >> .config
echo "CONFIG_POWERBEAMAC_GEN2=y" >> .config
#echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_ATH10K=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
#echo "CONFIG_FREERADIUS=y" >> .config
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
#echo "CONFIG_OPTIMIZE_FOR_SPEED=y" >> .config

make -f Makefile.pb42 kernel clean all install
cd ../../../
cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-PowerBeam-AC-Gen2/ubnt-PowerBeam-AC-Gen2-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-PowerBeam-AC-Gen2/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-PowerBeam-AC-Gen2/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-PowerBeam-AC-Gen2

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-LiteBeam-AC-Gen2/ubnt-LiteBeam-AC-Gen2-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-LiteBeam-AC-Gen2/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-LiteBeam-AC-Gen2/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-LiteBeam-AC-Gen2

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-NanoStation-5AC-loco/ubnt-NanoStation-5AC-loco-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-Loco5-AC-Gen2/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-NanoStation-5AC-loco/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-NanoStation-5AC-loco

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBeam-AC-Gen1/ubnt-NanoBeam-AC-Gen1-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBeam5-AC-Gen2/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-NanoBeam-AC-Gen1/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-NanoBeam-AC-Gen1

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-LiteAP-AC/ubnt-LiteAP-AC-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-NanoBeam5-AC-Gen2/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-LiteAP-AC/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-LiteAP-AC


cd pb42/src/router


cp .config_ubntm .config
echo "CONFIG_UBNTXW=y" >> .config
#echo "CONFIG_TDMA=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-NanoStation_M5-XW/ubnt-NanoStation_M5-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-NanoStation_M5-XW/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-NanoStation_M5-XW/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-NanoStation_M5-XW

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-NanoStation_M2-XW/ubnt-NanoStation_M2-XW-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntxw-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-NanoStation_M2-XW/XW-DD-WRT.bin
cp pb42/src/router/ubnt-mtd/mtd2 ~/GruppenLW/releases/$DATE/ubnt-NanoStation_M2-XW/flash-upgrade.bin
cp notes/ubnt/* ~/GruppenLW/releases/$DATE/ubnt-NanoStation_M2-XW


#cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-UniFi-AC-LR/UniFiAC-LR-webflash-firmware.bin
#cp pb42/src/router/mips-uclibc/ubntbz2-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-UniFi-AC-LR/UAC-LR-DD-WRT.bin


cd pb42/src/router
cp .config_unifi .config
#echo "CONFIG_TDMA=y" >> .config
make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-UAP/UniFiAP-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt-UAP-LR/UniFiAP-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntbz-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-UAP/UAP-DD-WRT.bin
cp pb42/src/router/mips-uclibc/ubntbz-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt-UAP-LR/UAP-DD-WRT.bin
cp notes/unifi/install.txt ~/GruppenLW/releases/$DATE/ubnt-UAP
cp notes/unifi/install.txt ~/GruppenLW/releases/$DATE/ubnt-UAP-LR
cp notes/unifi/install_uapv2* ~/GruppenLW/releases/$DATE/ubnt-UAP-v2


