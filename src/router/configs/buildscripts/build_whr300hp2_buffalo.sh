#!/bin/sh -x
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n rt2880/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mipsel_24kc_gcc-13.1.0_musl/bin:$OLDPATH
#export PATH=/xfs/toolchains/toolchain-mipsel_mips32_gcc-8.2.0_musl/bin:$OLDPATH
cd rt2880/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
cp .config_whr300hp2_buffalo .config
#echo "CONFIG_BRANDING=y" >> .config
#echo "CONFIG_BUFFALO=y" >> .config
#echo "CONFIG_IAS=y" >> .config
[[ $1 = "idexx_signatur" ]] && {
	echo "CONFIG_IDEXX_SIGNATUR=y" >> .config
	echo "CONFIG_LOCK_US=y" >> .config
}
make -f Makefile.rt2880 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo-whr_300hp2
##mkdir -p /tmp/CUSTOMER/$DATE/buffalo/buffalo-whr_300hp2
cd ../../../
#cp rt2880/src/router/mipsel-uclibc/aligned.uimage ~/GruppenLW/releases/$DATE/WHR-G300N/firmware.bin
if [[ "$1" == "idexx_signatur" ]];
then
#	/root/firmware/pem/dosign.sh rt2880//src/router/mipsel-uclibc/mt7620-webflash.bin /root/firmware/pem/private_key_idexx.txt
#	cp rt2880/src/router/mipsel-uclibc/mt7620-webflash.bin.signed ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo-whr_300hp2/buffalo-whr_300hp2.bin
	/root/firmware/pem/dosign.sh rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-300-us /root/firmware/pem/private_key_idexx.txt
	cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-300-us.signed ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo-whr_300hp2/buffalo-whr_300hp2.bin
##	/root/firmware/pem/dosign.sh rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-300-us /root/firmware/pem/private_key_idexx.txt
##	cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-300-us.signed /tmp/CUSTOMER/$DATE/buffalo/buffalo-whr_300hp2/buffalo-whr_300hp2.bin
else
	mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo-whr_600d

	cp rt2880/src/router/mipsel-uclibc/mt7620-webflash.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo-whr_300hp2/buffalo-whr_300hp2-webflash.bin
	cp rt2880/src/router/mipsel-uclibc/aligned-mt7620.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo-whr_300hp2/firmware.uimage
	cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-300-jp ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo-whr_300hp2/firmware-jp.bin
	cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-300-eu ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo-whr_300hp2/firmware-eu.bin
	cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-300-us ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo-whr_300hp2/firmware-us.bin
	
	cp rt2880/src/router/mipsel-uclibc/mt7620-webflash.bin ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo-whr_600d/buffalo-whr_600d-webflash.bin
	cp rt2880/src/router/mipsel-uclibc/aligned-mt7620.uimage ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo-whr_600d/firmware.uimage
	
	cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-600-jp ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo-whr_600d/firmware-jp.bin
	cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-600-eu ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo-whr_600d/firmware-eu.bin
	cp rt2880/src/router/mipsel-uclibc/CRC_aligned-mt7620.uimage-600-us ~/GruppenLW/releases/CUSTOMER/$DATE/buffalo/buffalo-whr_600d/firmware-us.bin
	#cp rt2880/src/router/mipsel-uclibc/whrg300n-firmware.tftp ~/GruppenLW/releases/$DATE/WHR-G300N/whrg300n-firmware.tftp
fi
