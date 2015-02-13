#!/bin/sh
# file: scripts/start.sh

# use int6kf to start firmware on a device having a blank or corrupted
# NVRAM; this is a standard Intellon script that has proven useful but
# feel free to make changes;

# ====================================================================
# host symbols;
# --------------------------------------------------------------------

. /etc/enfironment
. ${SCRIPTS}/hardware.sh
. ${SCRIPTS}/firmware.sh

# ====================================================================
# query connection;
# --------------------------------------------------------------------

echo -n "Interface [${ETH}]: "; read  
if [ ! -z ${REPLY} ]; then
	ETH=${REPLY}
fi

# ====================================================================
# define random DAK and NMK;
# --------------------------------------------------------------------

MAC=auto
DAK=$(rkey secret.key -D)
NMK=$(rkey secret.key -M)

# ====================================================================
# update PIB file with MAC, DAK and NMK;
# --------------------------------------------------------------------

modpib -M ${MAC} -D ${DAK} -N ${NMK} ${PIB} -v
if [ ${?} != 0 ]; then
	exit 1
fi

# ====================================================================
# write CFG, FW and PIB to device and start FW execution;
# --------------------------------------------------------------------

int6kf -i ${ETH} -C ${CFG} -P ${PIB} -N ${NVM} 
if [ ${?} != 0 ]; then
	exit 1
fi

# ====================================================================
# confirm device identity;
# --------------------------------------------------------------------

int6k -i ${ETH} -I

# ====================================================================
# 
# --------------------------------------------------------------------

exit 0   

