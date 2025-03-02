#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n pb42/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mips_gcc-4.3.3+cs_uClibc-0.9.30.1/usr/bin:$OLDPATH
#export PATH=/xfs/toolchains/staging_dir_mips_pb42/bin:$OLDPATH

cd pb42/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_dw02_412h .config

cat .config_dw02_412h | grep -v "CONFIG_SUPERCHANNEL" > .config
# cat .config_dw02_412h_full | grep -v "^CONFIG_OPENDPI\|^CONFIG_ZFS" > .config
#cat .config_dw02_412h | grep -v "CONFIG_OPENDPI=y" > .config
echo "CONFIG_BRANDING=y" >> .config
echo "CONFIG_NDTRADE=y" >> .config
echo "CONFIG_NOTRIAL=y" >> .config
echo CONFIG_AP135=y >> .config
echo CONFIG_AP143=y >> .config
echo "CONFIG_CAKE=y" >> .config
echo CONFIG_MAC80211_MESH=y >> .config
echo CONFIG_WPA3=y >> .config
echo CONFIG_SAMBA4=y >> .config
echo "CONFIG_SMBD=y" >> .config
echo "CONFIG_MDNS=y" >> .config
echo "CONFIG_MDNS_UTILS=y" >> .config
echo "CONFIG_REGISTER=y" >> .config
echo "CONFIG_VLAN=y" >> .config
echo "HOSTAPDVERSION=2023-09-08" >> .config

make -f Makefile.pb42 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/ndtrade/DW02-412H
cd ../../../

#cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/ndtrade/DW02-412H/factory-to-ddwrt.bin
cp pb42/src/router/mips-uclibc/tftp/image.tftp ~/GruppenLW/releases/CUSTOMER/$DATE/ndtrade/DW02-412H/factory-to-ddwrt.tftp
#cp pb42/src/router/mips-uclibc/dw02_412h-firmware.trx ~/GruppenLW/releases/CUSTOMER/$DATE/ndtrade/DW02-412H/dw02_412h-webflash.bin

#  cd pb42/src/router
#  
#  #cp .config_dw02_412h_full .config
#  cat .config_dw02_412h_full | grep -v "^CONFIG_SUPERCHANNEL\|^CONFIG_ASTERISK" > .config
#  echo "CONFIG_BRANDING=y" >> .config
#  echo "CONFIG_NDTRADE=y" >> .config
#  echo "CONFIG_NOTRIAL=y" >> .config
#  echo CONFIG_AP135=y >> .config
#  echo CONFIG_AP143=y >> .config
#  echo "CONFIG_CAKE=y" >> .config
#  echo CONFIG_MAC80211_MESH=y >> .config
#  echo CONFIG_WPA3=y >> .config
#  echo CONFIG_SAMBA4=y >> .config
#  echo "CONFIG_SMBD=y" >> .config
#  echo "CONFIG_MDNS=y" >> .config
#  echo "CONFIG_MDNS_UTILS=y" >> .config
#  echo "CONFIG_REGISTER=y" >> .config
#  echo "CONFIG_LLTD=y" >> .config
#  echo "CONFIG_VLAN=y" >> .config
#  make -f Makefile.pb42 kernel clean all install
#  mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/ndtrade/DW02-412H
#  cd ../../../
#  
 #cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/ndtrade/DW02-412H/factory-to-ddwrt.bin
 echo "==> cp webflash"
 cp pb42/src/router/mips-uclibc/dw02_412h-firmware.trx ~/GruppenLW/releases/CUSTOMER/$DATE/ndtrade/DW02-412H/dw02_412h-webflash.bin
 #scp -P 993 pb42/src/router/mips-uclibc/aligned.uimage 10.20.30.105:/root/tftpd
 #
 
 cd pb42/src/router
 
cp .config_dw02_412h_full .config
# cat .config_dw02_412h_full | grep -v "CONFIG_ASTERISK" > .config
# cat .config_dw02_412h_full | grep -v "^CONFIG_OPENDPI\|^CONFIG_ZFS\|^CONFIG_ASTERISK" > .config
 # cat .config_dw02_412h_full | grep -v "^CONFIG_OPENDPI\|^CONFIG_ASTERISK" > .config
 echo "CONFIG_BRANDING=y" >> .config
 echo "CONFIG_NDTRADE=y" >> .config
 echo "CONFIG_NOTRIAL=y" >> .config
 echo CONFIG_AP135=y >> .config
 echo CONFIG_AP143=y >> .config
 echo "CONFIG_CAKE=y" >> .config
 echo CONFIG_MAC80211_MESH=y >> .config
 echo CONFIG_WPA3=y >> .config
 echo CONFIG_SAMBA4=y >> .config
 echo "CONFIG_SMBD=y" >> .config
 echo "CONFIG_MDNS=y" >> .config
 echo "CONFIG_MDNS_UTILS=y" >> .config
 echo "CONFIG_REGISTER=y" >> .config
 echo "CONFIG_LLTD=y" >> .config
 echo "HOSTAPDVERSION=2023-09-08" >> .config
 make -f Makefile.pb42 kernel clean all install
 mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/ndtrade/DW02-412H
 cd ../../../
 
 #cp pb42/src/router/mips-uclibc/aligned.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/ndtrade/DW02-412H/factory-to-ddwrt.bin
 echo "==> cp scflash"
 cp pb42/src/router/mips-uclibc/dw02_412h-firmware.trx ~/GruppenLW/releases/CUSTOMER/$DATE/ndtrade/DW02-412H/dw02_412h-sc-webflash.bin
 #scp -P 993 pb42/src/router/mips-uclibc/aligned.uimage 10.20.30.105:/root/tftpd

echo "/GruppenLW/releases/CUSTOMER/$DATE/ndtrade/DW02-412H/"

