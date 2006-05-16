#!/bin/sh

# clean everything
rm *.bin
rm *.trx
cd ../src/linux/linux
#make clean
cd ../../
make clean
cd router/busybox
make clean
#cd ../../linux/linux
#cd ../../../opt
