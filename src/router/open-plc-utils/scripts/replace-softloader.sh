#!/bin/sh

# this script replaces the intellon softloader with runtime firmware;
# it assumes the softloader is running when the script is started;

# ====================================================================
# file symbols;
# --------------------------------------------------------------------

ETH=eth4
PIB=../firmware/v3.3.6.pib
NVM=../firmware/v3.3.6-0-8-B-RC1.nvm
MAC=00B05200CA08
DAK=689F074B8B0275A2710B0B5779AD1630
NMK=50D3E4933F855B7040784DF815AA8DB7

# ====================================================================
#  
# --------------------------------------------------------------------

modpib -M ${MAC} -D ${DAK} -N ${NMK} ${PIB}
int6k -i ${ETH} -P ${PIB} -N ${NVM} -F
int6k -i ${ETH} -P ${PIB} -N ${NVM} -FF

# ====================================================================
#  
# --------------------------------------------------------------------

 exit 0

