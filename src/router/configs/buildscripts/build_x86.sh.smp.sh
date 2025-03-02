#!/bin/sh
#./build_x86_kmt.sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n x86/src/router/httpd)
#export PATH=/xfs/toolchains/toolchain-i386_i486_gcc-5.1.0_musl-1.1.10/bin:$OLDPATH
export PATH=/xfs/toolchains/toolchain-i386_pentium-mmx_gcc-13.1.0_musl/bin:$OLDPATH


cd x86/src/linux/universal/linux-6.6
cp .config_smp .config
echo CONFIG_NET_ETHERIP=m >> .config
cd ../../../../../
cd x86/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_i386_full .config
echo "CONFIG_SPEEDCHECKER=y" >> .config
echo "KERNELVERSION=6.6" >> .config
echo "CONFIG_IRQBALANCE=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_MAC80211_COMPRESS=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_STRACE=y" >> .config
echo "CONFIG_ZFS=y" >> .config
echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_RAID=y" >> .config
echo "CONFIG_EXFAT=y" >> .config
echo "CONFIG_NTFSPROGS=y" >> .config
echo "CONFIG_DOSFSTOOLS=y" >> .config
echo "CONFIG_SMARTMONTOOLS=y" >> .config
echo "CONFIG_RSYNC=y" >> .config
echo "CONFIG_CAKE=y" >> .config
echo "CONFIG_FLASHROM=y" >> .config
echo "CONFIG_SAMBA4=y" >> .config
echo "CONFIG_SMBD=y" >> .config
echo "CONFIG_APFS=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
echo "CONFIG_NGINX=y" >> .config
echo "CONFIG_IWLWIFI=y" >> .config
echo "CONFIG_MRP=y" >> .config
echo "CONFIG_CFM=y" >> .config
echo "CONFIG_HTOP=y" >> .config
echo "CONFIG_IPSET=y" >> .config
echo "CONFIG_NTFS3=y" >> .config
echo "CONFIG_PLEX=y" >> .config
echo "CONFIG_MDNS=y" >> .config
echo "CONFIG_MDNS_UTILS=y" >> .config
echo "CONFIG_BOINC=y" >> .config
echo "CONFIG_BTOP=y" >> .config
#echo "CONFIG_LOCKDEBUG=y" >> .config
echo "CONFIG_KERNELLTO=y" >> .config
echo "CONFIG_KERNELLTO_CP_CLONE=y" >> .config
echo "CONFIG_MAC80211_ATH9K_HTC=y" >> .config
sed -i 's/CONFIG_QUAGGA=y/CONFIG_FRR=y/g' .config

make -f Makefile.x86 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/x86
mkdir -p ~/GruppenLW/releases/$DATE/x86_64
cd ../../../
cp x86/src/router/i386-uclibc/dd-wrt_vga.image ~/GruppenLW/releases/$DATE/x86/dd-wrt_full_smp_vga.image
cp x86/src/router/i386-uclibc/dd-wrt_serial.image ~/GruppenLW/releases/$DATE/x86/dd-wrt_full_smp_serial.image
cp x86/src/router/i386-uclibc/dd-wrt-vga-webupgrade.bin ~/GruppenLW/releases/$DATE/x86/dd-wrt-webupgrade_full_smp_vga.bin
cp x86/src/router/i386-uclibc/dd-wrt-serial-webupgrade.bin ~/GruppenLW/releases/$DATE/x86/dd-wrt-webupgrade_full_smp_serial.bin

cp x86/src/router/i386-uclibc/dd-wrt_vga_2GB.image ~/GruppenLW/releases/$DATE/x86/dd-wrt_full_smp_vga_2GB.image
cp x86/src/router/i386-uclibc/dd-wrt_serial_2GB.image ~/GruppenLW/releases/$DATE/x86/dd-wrt_full_smp_serial_2GB.image
cp x86/src/router/i386-uclibc/dd-wrt-vga-webupgrade_2GB.bin ~/GruppenLW/releases/$DATE/x86/dd-wrt-webupgrade_full_smp_vga_2GB.bin
cp x86/src/router/i386-uclibc/dd-wrt-serial-webupgrade_2GB.bin ~/GruppenLW/releases/$DATE/x86/dd-wrt-webupgrade_full_smp_serial_2GB.bin
