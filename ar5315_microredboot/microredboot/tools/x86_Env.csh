#!/bin/tcsh

# Set the top directory where the linux src tree resides
setenv TOPDIR `pwd`/..

# Unset the toolchain path
unsetenv TOOLPATH

# Unset the toolchain prefix
unsetenv TOOLPREFIX

# Unset the kernel path which is used by several build scripts
unsetenv KERNELPATH

# Unset the tftp directory
unsetenv TFTPPATH

# Unset the bus type (or, alternately, set to PCI)
unsetenv BUS

# Set the HAL directory if you have the HAL sources
if (${?HAL} == 0) then
setenv HAL $TOPDIR/src/802_11/madwifi/hal/main
endif

# For the HAL, only build for the non SoC platforms
setenv AH_SUPPORT_AR5210 1
setenv AH_SUPPORT_AR5211 1
setenv AH_SUPPORT_AR5212 1
setenv AH_SUPPORT_AR5312 0

# Set Phy Err Diagnositics (Radar detection) to be enabled for AP builds
setenv ATH_CAP_PHYERR_DIAG 1

# X86 does  not require descriptor swap.
export ATH_NEED_DESC_SWAP=0

if (!(-le $TOPDIR/src/802_11/madwifi/madiwifi/ath/if_ath_phyerr.c)) then
ln -s $TOPDIR/src/802_11/madwifi/phyerr/if_ath_phyerr.c $TOPDIR/src/802_11/madwifi/madwifi/ath/if_ath_phyerr.c
endif
