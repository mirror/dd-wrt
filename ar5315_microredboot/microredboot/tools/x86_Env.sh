#!/bin/sh

# Set the top directory where the linux src tree resides
export TOPDIR=`pwd`/..

# Unset the toolchain path
unset TOOLPATH

# Unset the toolchain prefix
unset TOOLPREFIX

# Unset the kernel path which is used by several build scripts
unset KERNELPATH

# Unset the tftp directory
unset TFTPPATH

# Unset the bus type (or, alternately, set to PCI)
unset BUS

# Set the HAL directory if you have the HAL sources
if [ -z $HAL ]; then
export HAL=$TOPDIR/src/802_11/madwifi/hal/main
fi

# For the HAL, only build for the non SoC platforms
export AH_SUPPORT_AR5210=1
export AH_SUPPORT_AR5211=1
export AH_SUPPORT_AR5212=1
export AH_SUPPORT_AR5312=0

# Set Phy Err Diagnostics (Radar detection) to be enabled for AP builds
export ATH_CAP_PHYERR_DIAG=1

# X86 does  not require descriptor swap.
export ATH_NEED_DESC_SWAP=0
export AH_NEED_DESC_SWAP=0

if [ ! -h $TOPDIR/src/802_11/madwifi/madwifi/ath/if_ath_phyerr.c ]; then
ln -s $TOPDIR/src/802_11/madwifi/phyerr/if_ath_phyerr.c $TOPDIR/src/802_11/madwifi/madwifi/ath/if_ath_phyerr.c
fi
