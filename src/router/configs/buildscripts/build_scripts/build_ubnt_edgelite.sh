#!/bin/sh
OLDPATH=$PATH
DATE=$(date +%m-%d-%Y)
DATE+="-r"
DATE+=$(svnversion -n octeon/src/router/httpd)
export PATH=/xfs/toolchains/toolchain-mips64_octeon_64_gcc-7.3.0_musl/bin:$OLDPATH
cd octeon/src/router
cp .config_e100 .config
echo "CONFIG_SPEEDCHECKER=y" >>.config
echo "CONFIG_LSOF=y" >> .config
echo "CONFIG_MIKROTIK_BTEST=y" >> .config
echo "CONFIG_STRACE=y" >>.config
echo "CONFIG_WIREGUARD=y" >> .config
echo "CONFIG_SCREEN=y" >> .config
sed -i 's/CONFIG_QUAGGA=y/CONFIG_FRR=y/g' .config

make -f Makefile.octeon kernel clean all install
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-edgerouter-lite
mkdir -p ~/GruppenLW/releases/$DATE/ubnt-unifi-ugw3
cd ../../../
cp octeon/src/router/mips64-uclibc/erouter.image ~/GruppenLW/releases/$DATE/ubnt-edgerouter-lite/edgerouter-e100-lite.image
cp octeon/src/router/mips64-uclibc/erouter.bin ~/GruppenLW/releases/$DATE/ubnt-edgerouter-lite/edgerouter-e100-lite-webupgrade.bin

cp octeon/src/router/mips64-uclibc/erouter.image ~/GruppenLW/releases/$DATE/ubnt-unifi-ugw3/ugw3-e120-lite.image
cp octeon/src/router/mips64-uclibc/erouter.bin ~/GruppenLW/releases/$DATE/ubnt-unifi-ugw3/ugw3-e120-lite-webupgrade.bin
