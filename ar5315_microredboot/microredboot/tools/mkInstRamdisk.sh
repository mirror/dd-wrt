#!/bin/sh

if [ $# != 1 ]; then
echo "usage: kernel_kernel.sh  apType "
echo "ex   : release_kernel.sh  ap30 "
exit 1
fi

if [ -z $KERNELPATH ]; then
echo "KERNELPATH is not defined! "
exit 1
fi

cd $KERNELPATH/arch/mips/ar531x/ROOTDISK
sudo make clean
sudo make
sudo make install
cd ../../../../
make

# only install if INSTALLPATH is defined
if [ $INSTALLPATH ]; then
  mkdir -p $INSTALLPATH 
  cp -f vmlinux $INSTALLPATH/vmlinux.$1.ram
fi

# only copy to tftpboot dir if TFTPPATH is defined
if [ $TFTPPATH ]; then
  sudo cp -f vmlinux $TFTPPATH/vmlinux.$1.ram
fi
