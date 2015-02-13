#!/bin/sh
# file: tr069prog.sh

# ====================================================================
#
# This script programs a unique TR-069 ACS Username and ACS Password 
# into a INT6400 powerline device and updates the Mfg feed file; 
# the original MAC, DAK and NMK are preserved; the MFG, NET and USR
# HFIDs are set to the values defined in this file; the ACS Username
# is derived from the original MAC; the ACS Password is derived from
# the local host random number generator;
#
# devices should be programmed and tested using the PTS then updated
# using his script; Atheros Powerline Toolkit 1.3.1 is needed;
#
# --------------------------------------------------------------------

#
# Define environment;
#

ETH=eth0                # PC ethernet port 
NVMFILE=tr069.nvm       # This is the NVM file to program
REFPIB=tr069.pib        # This is the PIB template to use
PIBFILE=tmp.pib         # Temp PIB to modify
MFGFILE=mfgfeed.txt     # Manufacturing Feed File

#
# Determine device identity;
#

MAC=$(int6kid -Ai ${ETH})
DAK=$(int6kid -Di ${ETH})
NMK=$(int6kid -Mi ${ETH})

#
# Define default HFIDs;
#

MFG="MANUFACTURER MODEL-AB-02-01"
NET="MANUFACTURER MODEL-AB-02-01"
USR="MANUFACTURER MODEL-AB-02-01"

#
# Make a copy of PIB to edit
#

cp ${REFPIB} ${PIBFILE}

#
# Set MAC, DAK, NMK, MFG_HFID, NET_HFID and USR_HFID in PIB file
#

modpib ${PIBFILE} -M ${MAC} -D ${DAK} -N ${NMK} -S "${MFG}" -T "${NET}" -U "${USR}" 

UMAC=$(echo $MAC | sed 'y/abcdef/ABCDEF/' | sed 's/://g' | sed 's/ //g') 
OUI=$(echo $UMAC | cut -c 1-6)

#
# Build the ACS Username
#

UNAME="$OUI"-"$UMAC"

#
# Generate a 16 character lower case random ACS password
#

RANDOMPWORD=$(</dev/urandom tr -dc a-z0-9| (head -c $1 > /dev/null 2>&1 || head -c 16))

#
# Set Username and Password in PIB file
#

setpib ${PIBFILE} 2DCC username ${UNAME}
setpib ${PIBFILE} 2ECD password ${RANDOMPWORD}
setpib ${PIBFILE} 2FCE byte 01

#
# Write NVM and PIB
#

int6kp -i ${ETH} -P ${PIBFILE} -N ${NVMFILE} -FF -D ${DAK}
FW=$(int6k -qri ${ETH} | rev | cut -d " " -f1 | rev)

#
# Write the record to Mfg feed file
#

echo $UMAC"|"$FW"|"$RANDOMPWORD"|1.0|"$RANDOMPWORD"|000000" >> ${MFGFILE}

