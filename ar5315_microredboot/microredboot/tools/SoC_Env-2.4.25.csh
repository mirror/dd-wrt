#!/bin/tcsh -f

alias Usage 'echo "  Usage:  source SoC_Env-2.4.25.csh apXX\!*" ; exit 1'

if ( $#argv == 0 ) then
    Usage
else
    set plat=$1
endif

# Set the top directory where the linux src tree resides
setenv TOPDIR `pwd`/..

# Set the toolchain path
setenv TOOLPATH $TOPDIR/tools/gcc-3.3.3-2.4.25/toolchain_mips

# Set the toolchain prefix
setenv TOOLPREFIX mips-linux-

setenv LSDK_KERNELVERSION 2.4.25

# Set the kernel path which is used by several build scripts
setenv KERNELPATH $TOPDIR/src/kernels/mips-linux-$LSDK_KERNELVERSION

# Set the tftp directory so some scripts can automatically copy the images 
# to the tftpboot dir so redboot can find the files
setenv TFTPPATH /tftpboot

# For the SoC, set the bus type to AHB
setenv BUS AHB

# Set the HAL directory if you have the HAL sources
if (${?HAL} == 0) then
setenv HAL $TOPDIR/src/802_11/madwifi/hal/main
endif

# Add the configuration parameters for a given AP
source $TOPDIR/src/configs/config_$plat.csh

# Set the path for the user so the toolchain is found

set path = (`pwd` $TOOLPATH/bin $path)
rehash
