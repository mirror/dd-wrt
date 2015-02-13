#!/bin/sh
# file: scripts/host.sh

# edit a PIB file then kick off the INT6300 Host Emulator; if you do
# not want to edit the PIB file before starting then comment out the
# line that does that;

# ====================================================================
# host symbols;
# --------------------------------------------------------------------

. ${SCRIPTS}/hardware.sh
. ${SCRIPTS}/firmware.sh

# ====================================================================
# file symbols;
# --------------------------------------------------------------------

MAC=auto
DAK=$(rkey secret.key -D)
NMK=$(rkey secret.key -M)

# ====================================================================
# confirm MAC;
# --------------------------------------------------------------------

echo -n "MAC Address [${MAC}]: "; read
if [ ! -z ${REPLY} ]; then
	MAC="${REPLY}"                  
fi

# ====================================================================
# modify PIB;
# --------------------------------------------------------------------

modpib ${PIB} -C0 -M ${MAC} -N ${NMK} -D ${DAK}
if [ ${?} != 0 ]; then
	exit 1
fi

# ====================================================================
# service host action request messages;
# --------------------------------------------------------------------

# Force Flash NVRAM 
# int64host -i ${ETH} -P ${PIB} -N ${NVM} -p abc.pib -n abc.nvm -FF
# Flash NVRAM (continuous loop because FW/PIB sent to host) 
# int64host -i ${ETH} -P ${PIB} -N ${NVM} -p abc.pib -n abc.nvm -F
int64host -i ${ETH} -P ${PIB} -N ${NVM} -p abc.pib -n abc.nvm
if [ ${?} != 0 ]; then
	exit 1
fi

