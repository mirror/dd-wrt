#!/bin/sh
export PATH=/home/openwrt/workspace/toolchains/3.4.6-uclibc-0.9.28/bin:$PATH

echo "#define BUILD_DATE \"$(date +%D)\"" > build.h 



cd ../src
cd linux/linux.v23
make clean
make dep
cd ../../
cd linux/linux.v24
make clean
cd ../../
make clean
make dep
cd ../opt


cd ../src/router
rm -dfr mipsel-uclibc/install
make httpd-clean
make rc-clean
make shared-clean
cd ../../opt
./install_sputnik.sh

cd ../src/router

rm -dfr mipsel-uclibc/install
make httpd-clean
make rc-clean
make shared-clean

cd ../../opt
./install_mini.sh
cd ../src/router
rm -dfr mipsel-uclibc/install
make httpd-clean
make rc-clean
make shared-clean
cd ../../opt
./install_normal.sh


cd ../src/router
rm -dfr mipsel-uclibc/install
make httpd-clean
make rc-clean
make shared-clean
cd ../../opt
./install_aqos.sh



#build vpn version
cd ../src/router
rm -dfr mipsel-uclibc/install
make httpd-clean
make rc-clean
make shared-clean
cd ../../opt
./install_openvpn.sh


cd ../src/router
rm -dfr mipsel-uclibc/install
mske httpd-clean
make rc-clean
make shared-clean
cd ../../opt
./install_voip.sh

cd ../src/router
rm -dfr mipsel-uclibc/install
mske httpd-clean
make rc-clean
make shared-clean
cd ../../opt
./install_voip.v24.sh

cd ../src/router
rm -dfr mipsel-uclibc/install
mske httpd-clean
make rc-clean
make shared-clean
cd ../../opt
./install_ggew.sh
