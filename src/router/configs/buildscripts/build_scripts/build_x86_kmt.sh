#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n x86/src/router/httpd)

export PATH=/xfs/toolchains/toolchain-i386_pentium-mmx_gcc-10.2.0_musl/bin:$OLDPATH

cd x86/src/linux/universal/linux-4.9
cp .config_smp .config
cd ../../../../../
cd x86/src/router
[ -n "$DO_UPDATE" ] && svn update
cd opt/etc/config
[ -n "$DO_UPDATE" ] && svn update
cd ../../../
#cp .config_i386_kmt .config
cat .config_i386_full | grep -v "^CONFIG_WIKAR\|^CONFIG_KROMO\|^CONFIG_XIRIAN\|^CONFIG_BRAINSLAYER\|^CONFIG_NOCAT\|CONFIG_MC\|CONFIG_SNORT" > .config 
echo "CONFIG_SPEEDCHECKER=y" >> .config
echo "CONFIG_WIREGUARD=y" >> .config
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
echo "CONFIG_LSM=y" >>.config
#echo "CONFIG_DRIVER_WIRED=y" >>.config
echo "CONFIG_LIBMBIM=y" >>.config
echo "CONFIG_MBEDTLS=y" >>.config
make -f Makefile.x86 kernel clean all install
mkdir -p ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/x86-kmt
cd ../../../
cp x86/src/router/i386-uclibc/dd-wrt_vga.image ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/x86-kmt/dd-wrt_full_smp_vga.image
cp x86/src/router/i386-uclibc/dd-wrt_serial.image ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/x86-kmt/dd-wrt_full_smp_serial.image
cp x86/src/router/i386-uclibc/dd-wrt-vga-webupgrade.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/x86-kmt/dd-wrt-webupgrade_full_smp_vga.bin
cp x86/src/router/i386-uclibc/dd-wrt-serial-webupgrade.bin ~/GruppenLW/releases/CUSTOMER/$DATE/kmt/x86-kmt/dd-wrt-webupgrade_full_smp_serial.bin
