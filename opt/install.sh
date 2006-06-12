#!/bin/sh
export MYPATH=$PATH
export PATH=/home/dd-wrt/toolchains/3.4.6/bin:$MYPATH
echo "#define BUILD_DATE \"$(date +%D)\"" > build.h 

export SRCBASE=$(cd "../src" && pwd -P)
echo $SRCBASE

cd ../src
cd linux/brcm/linux.v23
make oldconfig
make clean
make dep
make
make modules
cd ../../../

cd linux/brcm/linux.v24
make oldconfig
make clean
make dep
make
make modules
cd ../../../

make clean
cd ../opt
export PATH=/home/dd-wrt/toolchains/4.1.0/bin:$MYPATH

#build mini
cd ../src/router
rm -dfr mipsel-uclibc/install
make httpd-clean
make rc-clean
make shared-clean
cd ../../opt
./install_mini.sh

#build standard
cd ../src/router
rm -dfr mipsel-uclibc/install
make httpd-clean
make rc-clean
make shared-clean
cd ../../opt
./install_normal.sh

#build aqos
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

#build voip
cd ../src/router
rm -dfr mipsel-uclibc/install
make httpd-clean
make rc-clean
make shared-clean
cd ../../opt
./install_voip.sh

#build voip.v24
cd ../src/router
rm -dfr mipsel-uclibc/install
make httpd-clean
make rc-clean
make shared-clean
cd ../../opt
./install_voip.v24.sh

#build sputnik
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
./install_ggew.sh

cd ../src/router
rm -dfr mipsel-uclibc/install
make httpd-clean
make rc-clean
make shared-clean
cd ../../opt
./install_newmedia.sh

# cd ../src/router
# rm -dfr mipsel-uclibc/install
# make httpd-clean
# make rc-clean
# make shared-clean
# cd ../../opt
# ./install_fon.sh
