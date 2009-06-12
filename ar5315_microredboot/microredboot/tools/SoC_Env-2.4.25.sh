#!/bin/sh

if [ $# -eq 0 ]
then
	echo "Usage: SoC_Env-2.4.25.sh <ap30|ap51|ap48>"
else
# Set the top directory where the linux src tree resides
export TOPDIR=`pwd`/..


# Set the toolchain path
export TOOLPATH=$TOPDIR/tools/gcc-3.3.3-2.4.25/toolchain_mips

# Set the toolchain prefix
export TOOLPREFIX=mips-linux-

# Set the kernel path which is used by several build scripts
export KERNELPATH=$TOPDIR/src/kernels/mips-linux-2.4.25

# Set the tftp directory so some scripts can automatically copy the images 
# to the tftpboot dir so redboot can find the files
export TFTPPATH=/tftpboot

# For the SoC, set the bus type to AHB
export BUS=AHB

# Set the HAL directory if you have the HAL sources
if [ -z $HAL ]; then
export HAL=$TOPDIR/src/802_11/madwifi/hal/main
fi

# Souce in the configuration for given AP.
. $TOPDIR/src/configs/config.$1

# Set Phy Err Diagnostics (Radar detection) to be enabled for AP builds
export ATH_CAP_PHYERR_DIAG=1

# Set the path for the user so the toolchain is found

export PATH=`pwd`:$TOOLPATH/bin:$PATH
fi
