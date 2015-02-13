#!/bin/sh
# file: scripts/flashp.sh

# use the int6kfp to initialize an INT6300 device having a blank or 
# corrupted NVRAM; the Bootloader must be running for this script to
# work properly;

# ====================================================================
# host symbols;
# --------------------------------------------------------------------

. ${SCRIPTS}/hardware.sh
. ${SCRIPTS}/firmware.sh

# ====================================================================
# confirm connection;
# --------------------------------------------------------------------

echo -n "Interface [${ETH}]: "; read  
if [ ! -z ${REPLY} ]; then
	ETH=${REPLY}
fi

# ====================================================================
# check connection;
# --------------------------------------------------------------------

int6kwait -xqsi ${ETH} 
if [ ${?} != 0 ]; then
	echo "Device is not connected"
	exit 1
fi

# ====================================================================
# randomize identity;
# --------------------------------------------------------------------

MAC=auto
DAK=$(rkey secret.key -D)
NMK=$(rkey secret.key -M)

# ====================================================================
# confirm address;
# --------------------------------------------------------------------

echo -n "MAC [${MAC}]: "; read
if [ ! -z ${REPLY} ]; then
	MAC="${REPLY}"                  
fi

echo -n "DAK [${DAK}]: "; read
if [ ! -z ${REPLY} ]; then
	DAK="${REPLY}"                  
fi

echo -n "NMK [${NMK}]: "; read
if [ ! -z ${REPLY} ]; then
	NMK="${REPLY}"                  
fi

# ====================================================================
# modify PIB;
# --------------------------------------------------------------------

modpib -M ${MAC} -D ${DAK} -N ${NMK} ${PIB} -v
if [ ${?} != 0 ]; then
	exit 1
fi

# ====================================================================
# flash NVRAM with firmware and factory PIB;
# --------------------------------------------------------------------

int6kfp -i ${ETH} -C ${CFG} -P ${PIB} -N ${NVM} -D ${DAK} -FF
if [ ${?} != 0 ]; then
	exit 1
fi

# ====================================================================
# confirm identity;
# --------------------------------------------------------------------

int6k -i ${ETH} -I

# ====================================================================
# return success;
# --------------------------------------------------------------------

exit 0   

