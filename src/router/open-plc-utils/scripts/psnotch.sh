#!/bin/sh
# file: scripts/pibnotch.sh

# This is an experimental script used to test dynamic PIB notching;
# if you do not know what that is then you probably don't need this
# script;

# ====================================================================
# 
# --------------------------------------------------------------------

. /etc/environment
. ${SCRIPTS}/hardware.sh
. ${SCRIPTS}/firmware.sh

# ====================================================================
# 
# ETH1 is Host Interface for Device 1
# ETH2 is Host Interface for Device 2
#
# PLD1 is MAC of Powerline Device 1
# PLD2 is MAC of Powerline Device 2
#
# PIB the PIB file used to load prescalars
# MAP the MAP file used to save tonemaps read from device
# OLD the OLD prescalars (no notches)
# NEW the NEW prescalars (with notches)
# 
# --------------------------------------------------------------------

PIB=abc.pib    # the PIB file used to load prescalars
MAP=tonemap    # the MAP file used to save tonemaps read from device
OLD=scalars    # the OLD prescalars (no notches)
NEW=notches    # the NEW prescalars (with notches)

# ====================================================================
# 1. print message;
# 2. load OLD prescalars into PIB
# 3. download and flash PIB
# --------------------------------------------------------------------

echo
echo Setup
echo
psout ../firmware/v3.3.6.pib > ${OLD}
psin < ${OLD} ${PIB}
int6k -i ${ETH2} -P ${PIB} ${PLD1} -C2

# ====================================================================
# 1. Increment loop counter and print message
# 2. request tonemap between PLD1 and PLD2; save in MAP
# 3. read OLD prescalars and write NEW prescalars based on MAP; only
#    notch prescalars 57 through 63 if tone threshold is below 3;
# 4. load NEW prescalars into PIB;
# 5. plot prescalars from PIB discarding unwanted lines;
# 6. print a message
# 7. download and flash PIB to apply changes;
# 8. wait some time
# 9. load OLD prescalars into PIB
# 10. print a message
# 11. download and flash PIB to sample changes;
# 12. wait some time
# 13. repeat forever;
# --------------------------------------------------------------------

while [ 1 ]; do
	echo
	echo Check $((++count))
	echo
	int6ktone -qhi ${ETH2} ${PLD1} ${PLD2} > ${MAP}
	psnotch -v -L 40 -U 66 -l 57 -u 62 -t 3 -f ${MAP} < ${OLD} > ${NEW} 
	psin < ${NEW} ${PIB}
	psgraph ${PIB} | head -n 66 | tail -n 36
	echo
	echo blocking ...
	echo
	int6k -i ${ETH2} -P ${PIB} ${PLD1} -C2
	sleep 10
	psin < ${OLD} ${PIB}
	echo
	echo sampling ...
	echo
	int6k -i ${ETH2} -P ${PIB} ${PLD1} -C2
	sleep 60
done

