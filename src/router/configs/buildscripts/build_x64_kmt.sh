#!/bin/sh
#./build_x86_kmt.sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n x86_64/src/router/httpd)


export PATH=/xfs/toolchains/toolchain-x86_64_gcc-5.2.0_musl-1.1.11/bin:$OLDPATH
cd x86_64/src/router
[ -n "$DO_UPDATE" ] && svn update
#cp .config_x64 .config
cat .config_x64 | grep -v "^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_NOCAT\|CONFIG_MC\|CONFIG_SNORT\|CONFIG_ASTERISK" > .config 
echo "CONFIG_TMK=y" >>.config  
  
echo "CONFIG_NOTRIAL=y" >>.config  
#echo "CONFIG_SERCD=y" >>.config  
#echo "CONFIG_NPROBE=y" >>.config  
echo "CONFIG_BRANDING=y" >>.config  
#echo "CONFIG_NTPD=y" >>.config  
echo "CONFIG_PHP=y" >>.config
#echo "CONFIG_GPSD=y" >>.config
echo "CONFIG_VNCREPEATER=y" >>.config
echo "CONFIG_GPSI=y" >>.config
echo "CONFIG_NLD=y" >>.config
echo "CONFIG_GPIOWATCHER=y" >>.config
echo "CONFIG_STATUS_GPIO=y" >>.config
echo "CONFIG_ATH5K=y" >>.config
echo "CONFIG_ATH5K_PCI=y" >>.config
echo "CONFIG_IBSS_RSN=y" >>.config
#echo "CONFIG_MIITOOL=y" >>.config
echo "CONFIG_AP=y" >>.config
echo "CONFIG_NSMD=y" >>.config
echo "CONFIG_ETHPERF=y" >>.config
echo "CONFIG_ARPALERT=y" >>.config
make -f Makefile.x64 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/x86_64
cd ../../../
cp x86_64/src/router/x86_64-uclibc/dd-wrt_vga.image ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/x86_64/dd-wrt_full_vga.image
cp x86_64/src/router/x86_64-uclibc/dd-wrt_serial.image ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/x86_64/dd-wrt_full_serial.image
cp x86_64/src/router/x86_64-uclibc/dd-wrt-vga-webupgrade.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/x86_64/dd-wrt-webupgrade_full_vga.bin
cp x86_64/src/router/x86_64-uclibc/dd-wrt-serial-webupgrade.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/x86_64/dd-wrt-webupgrade_full_serial.bin
