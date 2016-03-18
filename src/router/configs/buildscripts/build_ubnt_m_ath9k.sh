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
cp .config_ubntm_ath9k .config
make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_bullet_m2_hp_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_bullet_m5_hp_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_rocket2m_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_rocket5m_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_rocket_m365_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_rocket_m35_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_rocket_m900_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoStation_2m_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoStation_5m_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoStation_m3_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoStation_365m_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_AirGrid_M2_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_AirWire_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_AirGrid_M5_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Loco_M2_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Loco_M5_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Pico_M2_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Pico_M5_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_Loco_M900_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M2_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M3_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M365_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M900_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M5_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M5_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M365_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_PowerAP-N_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_AirRouter_ath9k
mkdir -p ~/GruppenLW/releases/$DATE/ubnt_UniFi_AP_ath9k

cd ../../../

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_PowerAP-N_ath9k/powerAP-N-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_PowerAP-N_ath9k/PAPN-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_AirRouter_ath9k/airrouter-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_AirRouter_ath9k/AR-DD-WRT.bin


cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_bullet_m2_hp_ath9k/bs2m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_bullet_m2_hp_ath9k/BS2M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M2_ath9k/lc2m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M2_ath9k/LC2M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M5_ath9k/lc5m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M5_ath9k/LC5M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Pico_M5_ath9k/pc5m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Pico_M5_ath9k/PC5M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Pico_M2_ath9k/pc2m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Pico_M2_ath9k/PC2M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_AirWire_ath9k/awm-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_AirWire_ath9k/AWM-DD-WRT.bin


cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M900_ath9k/lc900m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_Loco_M900_ath9k/LC900M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_bullet_m5_hp_ath9k/bs5m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_bullet_m5_hp_ath9k/BS5M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_rocket2m_ath9k/r2m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_rocket2m_ath9k/R2M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_rocket5m_ath9k/r5m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_rocket5m_ath9k/R5M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_rocket_m365_ath9k/r365m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_rocket_m365_ath9k/R365M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_rocket_m35_ath9k/r35m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_rocket_m35_ath9k/R35M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_rocket_m900_ath9k/r900m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_rocket_m900_ath9k/R900M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_2m_ath9k/ns2m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_2m_ath9k/NS2M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_m3_ath9k/ns3m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_m3_ath9k/NS3M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_5m_ath9k/ns5m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_5m_ath9k/NS5M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_365m_ath9k/ns365m-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoStation_365m_ath9k/NS365M-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_AirGrid_M2_ath9k/airgridm2-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_AirGrid_M2_ath9k/AGM2-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_AirGrid_M5_ath9k/airgridm5-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_AirGrid_M5_ath9k/AGM5-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M2_ath9k/nanobridgem2-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M2_ath9k/NBM2-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M3_ath9k/nanobridgem3-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M3_ath9k/NBM3-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M365_ath9k/nanobridgem365-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M365_ath9k/NBM365-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M900_ath9k/nanobridgem900-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M900_ath9k/NBM900-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M5_ath9k/nanobridgem5-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_NanoBridge_M5_ath9k/NBM5-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M5_ath9k/powerbridgem5-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M5_ath9k/PBM5-DD-WRT.bin

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M365_ath9k/powerbridgem365-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntm-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_PowerBridge_M365_ath9k/PBM365-DD-WRT.bin



cd pb42/src/router
cp .config_unifi_ath9k .config
make -f Makefile.pb42 kernel clean all install
cd ../../../

cp pb42/src/router/mips-uclibc/ar7420-firmware.bin ~/GruppenLW/releases/$DATE/ubnt_UniFi_AP_ath9k/UniFiAP-webflash-firmware.bin
cp pb42/src/router/mips-uclibc/ubntbz-dd-wrt.bin ~/GruppenLW/releases/$DATE/ubnt_UniFi_AP_ath9k/UAP-DD-WRT.bin
cp notes/unifi/* ~/GruppenLW/releases/$DATE/ubnt_UniFi_AP_ath9k

