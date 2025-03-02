#!/bin/sh
#./build_x86_kmt.sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n x86_64/src/router/rc)


export PATH=/xfs/toolchains/toolchain-x86_64_gcc-13.1.0_musl/bin:$OLDPATH
cd x86_64/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_x64 .config
echo "CONFIG_SPEEDCHECKER=y" >> .config
echo "KERNELVERSION=6.6" >> .config
echo "CONFIG_IPTOOLS=y" >> .config
echo "CONFIG_IRQBALANCE=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_STRACE=y" >> .config
echo "CONFIG_ZFS=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_RAID=y" >> .config
echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_EXFAT=y" >> .config
echo "CONFIG_NTFSPROGS=y" >> .config
echo "CONFIG_DOSFSTOOLS=y" >> .config
echo "CONFIG_RSYNC=y" >> .config
echo "CONFIG_SMARTMONTOOLS=y" >> .config
echo "CONFIG_MAC80211_MESH=y" >> .config
echo "CONFIG_MAC80211_COMPRESS=y" >> .config
echo "CONFIG_FLASHROM=y" >> .config
echo "CONFIG_CAKE=y" >> .config
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
echo "CONFIG_MAC80211_ATH9K_HTC=y" >> .config
echo "CONFIG_MDNS=y" >> .config
echo "CONFIG_BOINC=y" >> .config
echo "CONFIG_MDNS_UTILS=y" >> .config
echo "CONFIG_REGISTER=y" >> .config
echo "CONFIG_BTOP=y" >> .config
#echo "CONFIG_LOCKDEBUG=y" >> .config
#echo "CONFIG_KERNELLTO=y" >> .config
echo "CONFIG_NUT=y" >> .config
echo "CONFIG_BLUEZ=y" >> .config
echo "CONFIG_TOR_FULL=y" >> .config
#echo "CONFIG_KERNELLTO_CP_CLONE=y" >> .config
sed -i 's/CONFIG_QUAGGA=y/CONFIG_FRR=y/g' .config
make -f Makefile.x64 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/x86-64
cd ../../../
cp x86_64/src/router/x86_64-uclibc/dd-wrt_vga.image ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64_full_vga.image
cp x86_64/src/router/x86_64-uclibc/dd-wrt_serial.image ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64_full_serial.image
cp x86_64/src/router/x86_64-uclibc/dd-wrt-vga-webupgrade.bin ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64-webupgrade_full_vga.bin
cp x86_64/src/router/x86_64-uclibc/dd-wrt-serial-webupgrade.bin ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64-webupgrade_full_serial.bin


cp x86_64/src/router/grub-efi/efi-vga.img ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64_full_efi_vga.image
cp x86_64/src/router/grub-efi/efi.img ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64_full_efi_serial.image

cp x86_64/src/router/grub-efi/efi-vga-big.img ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64_full_efi_vga_2GB.image
cp x86_64/src/router/grub-efi/efi-big.img ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64_full_efi_serial_2GB.image

cp x86_64/src/router/x86_64-uclibc/dd-wrt-vga-efi-webupgrade.bin ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64-webupgrade_full_vga_efi.bin
cp x86_64/src/router/x86_64-uclibc/dd-wrt-serial-efi-webupgrade.bin ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64-webupgrade_full_serial_efi.bin
cp x86_64/src/router/x86_64-uclibc/dd-wrt-vga-efi-webupgrade_2GB.bin ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64-webupgrade_full_vga_efi_2GB.bin
cp x86_64/src/router/x86_64-uclibc/dd-wrt-serial-efi-webupgrade_2GB.bin ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64-webupgrade_full_serial_efi_2GB.bin

cp x86_64/src/router/x86_64-uclibc/dd-wrt_vga_2GB.image ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64_full_vga_2GB.image
cp x86_64/src/router/x86_64-uclibc/dd-wrt_serial_2GB.image ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64_full_serial_2GB.image
cp x86_64/src/router/x86_64-uclibc/dd-wrt-vga-webupgrade_2GB.bin ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64-webupgrade_full_vga_2GB.bin
cp x86_64/src/router/x86_64-uclibc/dd-wrt-serial-webupgrade_2GB.bin ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64-webupgrade_full_serial_2GB.bin


cd x86_64/src/router
[ -n "$DO_UPDATE" ] && svn update
cp .config_x64_free .config
echo  "CONFIG_SPEEDCHECKER=y" >>.config
echo "KERNELVERSION=6.6" >> .config
echo "CONFIG_IRQBALANCE=y" >> .config
echo "CONFIG_IPTOOLS=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_ZFS=y" >> .config
echo "CONFIG_STRACE=y" >> .config
echo "CONFIG_WPA3=y" >> .config
echo "CONFIG_TDMA=y" >> .config
echo "CONFIG_RAID=y" >> .config
echo "CONFIG_EXFAT=y" >> .config
echo "CONFIG_NTFSPROGS=y" >> .config
echo "CONFIG_RSYNC=y" >> .config
echo "CONFIG_DOSFSTOOLS=y" >> .config
echo "CONFIG_SMARTMONTOOLS=y" >> .config
echo "CONFIG_FLASHROM=y" >> .config
echo "CONFIG_CAKE=y" >> .config
echo "CONFIG_SAMBA4=y" >> .config
echo "CONFIG_SMBD=y" >> .config
echo "CONFIG_APFS=y" >> .config
echo "CONFIG_SMARTDNS=y" >> .config
echo "CONFIG_NGINX=y" >> .config
echo "CONFIG_MRP=y" >> .config
echo "CONFIG_CFM=y" >> .config
echo "CONFIG_HTOP=y" >> .config
echo "CONFIG_IPSET=y" >> .config
echo "CONFIG_NTFS3=y" >> .config
echo "CONFIG_PLEX=y" >> .config
echo "CONFIG_MDNS=y" >> .config
echo "CONFIG_BOINC=y" >> .config
echo "CONFIG_BTOP=y" >> .config
echo "CONFIG_MDNS_UTILS=y" >> .config
echo "CONFIG_NUT=y" >> .config
echo "CONFIG_BLUEZ=y" >> .config
echo "CONFIG_TOR_FULL=y" >> .config
#echo "CONFIG_LOCKDEBUG=y" >> .config
#echo "CONFIG_KERNELLTO=y" >> .config
#echo "CONFIG_KERNELLTO_CP_CLONE=y" >> .config
sed -i 's/CONFIG_QUAGGA=y/CONFIG_FRR=y/g' .config
make -f Makefile.x64 kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/x86-64
cd ../../../
cp x86_64/src/router/x86_64-uclibc/dd-wrt_vga.image ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64_public_vga.image
cp x86_64/src/router/x86_64-uclibc/dd-wrt_serial.image ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64_public_serial.image
cp x86_64/src/router/x86_64-uclibc/dd-wrt-vga-webupgrade.bin ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64-webupgrade_public_vga.bin
cp x86_64/src/router/x86_64-uclibc/dd-wrt-serial-webupgrade.bin ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64-webupgrade_public_serial.bin


cp x86_64/src/router/grub-efi/efi-vga.img ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64_public_efi_vga.image
cp x86_64/src/router/grub-efi/efi.img ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64_public_efi_serial.image

cp x86_64/src/router/x86_64-uclibc/dd-wrt-vga-efi-webupgrade.bin ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64-webupgrade_public_vga_efi.bin
cp x86_64/src/router/x86_64-uclibc/dd-wrt-serial-efi-webupgrade.bin ~/GruppenLW/releases/$DATE/x86-64/dd-wrt_x64-webupgrade_public_serial_efi.bin




