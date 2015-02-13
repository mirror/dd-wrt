#!/bin/sh
# file: scripts/firmware.sh

#
# this file contains a list of *.cfg, *.pib and *.nvm files available
# on the host for use by scripts; scripts such as start.sh, flash.sh, 
# upgrade.sh and pts.sh all include this file; 
#
# the last definition is the one that takes effect so you can merely 
# move the definition you want to the bottom of this file;
#
# to include this file in a new script:
#
#   . ${SCRIPTS}/firmware.sh 
#

# ====================================================================
# a la carte; 
# --------------------------------------------------------------------

. /etc/environment
CFG=${FIRMWARE}/sdram64mb.cfg
CFG=${FIRMWARE}/sdram16mb.cfg

# ====================================================================
# a la carte;            
# --------------------------------------------------------------------

PIB=${FIRMWARE}/v2.0.4.pib
NVM=${FIRMWARE}/v2.0.4-0-0-A-FINAL.nvm
NVM=${FIRMWARE}/v2.0.4-0-0-B-FINAL.nvm

PIB=${FIRMWARE}/v3.0.1.pib
NVM=${FIRMWARE}/v3.0.1-0-1-A-RC3.nvm
NVM=${FIRMWARE}/v3.0.1-0-1-B-RC3.nvm

PIB=${FIRMWARE}/v3.0.5.pib
NVM=${FIRMWARE}/v3.0.5-0-2-A-FINAL.nvm
NVM=${FIRMWARE}/v3.0.5-0-2-B-FINAL.nvm

PIB=${FIRMWARE}/v3.1.0.pib
NVM=${FIRMWARE}/v3.1.0-0-3-A-FINAL.nvm
NVM=${FIRMWARE}/v3.1.0-0-3-B-FINAL.nvm

PIB=${FIRMWARE}/v3.1.3.pib
NVM=${FIRMWARE}/v3.1.3-0-3-A-FINAL.nvm
NVM=${FIRMWARE}/v3.1.3-0-3-B-FINAL.nvm

PIB=${FIRMWARE}/v3.1.4.pib
NVM=${FIRMWARE}/v3.1.4-0-3-A-FINAL.nvm
NVM=${FIRMWARE}/v3.1.4-0-3-B-FINAL.nvm

PIB=${FIRMWARE}/v3.1.7.pib
NVM=${FIRMWARE}/v3.1.7-0-3-A-FINAL.nvm
NVM=${FIRMWARE}/v3.1.7-0-3-B-FINAL.nvm

PIB=${FIRMWARE}/v3.1.8.pib
NVM=${FIRMWARE}/v3.1.8-0-3-A-FINAL.nvm
NVM=${FIRMWARE}/v3.1.8-0-3-B-FINAL.nvm

PIB=${FIRMWARE}/v3.1.9.pib
NVM=${FIRMWARE}/v3.1.9-0-3-A-FINAL.nvm
NVM=${FIRMWARE}/v3.1.9-0-3-B-FINAL.nvm

PIB=${FIRMWARE}/v3.3.0.pib
NVM=${FIRMWARE}/v3.3.0-0-5-A-RC3.nvm
NVM=${FIRMWARE}/v3.3.0-0-5-C-RC3.nvm
NVM=${FIRMWARE}/v3.3.0-0-5-B-RC3.nvm

PIB=${FIRMWARE}/v3.3.0.pib
NVM=${FIRMWARE}/v3.3.0-0-5-A-RC5.nvm
NVM=${FIRMWARE}/v3.3.0-0-5-C-RC5.nvm
NVM=${FIRMWARE}/v3.3.0-0-5-B-RC5.nvm

PIB=${FIRMWARE}/v3.3.0.pib
NVM=${FIRMWARE}/v3.3.0-0-5-A-RC8.nvm
NVM=${FIRMWARE}/v3.3.0-0-5-C-RC8.nvm
NVM=${FIRMWARE}/v3.3.0-0-5-B-RC8.nvm

PIB=${FIRMWARE}/v3.3.1.pib
NVM=${FIRMWARE}/v3.3.1-0-5-A-RC5.nvm
NVM=${FIRMWARE}/v3.3.1-0-5-B-RC5.nvm
NVM=${FIRMWARE}/v3.3.1-0-5-C-RC5.nvm

PIB=${FIRMWARE}/v3.3.4.pib
NVM=${FIRMWARE}/v3.3.4-0-8-A-RC1.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-B-RC1.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-C-RC1.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-D-RC1.nvm

PIB=${FIRMWARE}/v3.3.4.pib
NVM=${FIRMWARE}/v3.3.4-0-8-A-RC2.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-B-RC2.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-C-RC2.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-D-RC2.nvm

PIB=${FIRMWARE}/v3.3.4.pib
NVM=${FIRMWARE}/v3.3.4-0-8-A-RC3.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-B-RC3.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-C-RC3.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-D-RC3.nvm

PIB=${FIRMWARE}/v3.3.4.pib
NVM=${FIRMWARE}/v3.3.4-0-8-A-RC5.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-B-RC5.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-C-RC5.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-D-RC5.nvm

PIB=${FIRMWARE}/v3.3.4.pib
NVM=${FIRMWARE}/v3.3.4-0-8-A-RC6.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-B-RC6.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-C-RC6.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-D-RC6.nvm

PIB=${FIRMWARE}/v3.3.4.pib
NVM=${FIRMWARE}/v3.3.4-0-8-A-RC7.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-B-RC7.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-C-RC7.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-D-RC7.nvm

PIB=${FIRMWARE}/v3.3.4.pib
NVM=${FIRMWARE}/v3.3.4-0-8-A-RC9.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-B-RC9.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-C-RC9.nvm
NVM=${FIRMWARE}/v3.3.4-0-8-D-RC9.nvm

PIB=${FIRMWARE}/v3.3.5.pib
NVM=${FIRMWARE}/v3.3.5-0-8-A-RC1.nvm
NVM=${FIRMWARE}/v3.3.5-0-8-B-RC1.nvm
NVM=${FIRMWARE}/v3.3.5-0-8-C-RC1.nvm
NVM=${FIRMWARE}/v3.3.5-0-8-D-RC1.nvm

PIB=${FIRMWARE}/v3.3.5.pib
NVM=${FIRMWARE}/v3.3.5-0-8-A-RC2.nvm
NVM=${FIRMWARE}/v3.3.5-0-8-B-RC2.nvm
NVM=${FIRMWARE}/v3.3.5-0-8-C-RC2.nvm
NVM=${FIRMWARE}/v3.3.5-0-8-D-RC2.nvm

PIB=${FIRMWARE}/v3.3.5.pib
NVM=${FIRMWARE}/v3.3.5-0-8-A-RC3.nvm
NVM=${FIRMWARE}/v3.3.5-0-8-B-RC3.nvm
NVM=${FIRMWARE}/v3.3.5-0-8-C-RC3.nvm
NVM=${FIRMWARE}/v3.3.5-0-8-D-RC3.nvm

PIB=${FIRMWARE}/v3.3.5.pib
NVM=${FIRMWARE}/v3.3.5-0-8-A-RC4.nvm
NVM=${FIRMWARE}/v3.3.5-0-8-B-RC4.nvm
NVM=${FIRMWARE}/v3.3.5-0-8-C-RC4.nvm
NVM=${FIRMWARE}/v3.3.5-0-8-D-RC4.nvm

PIB=${FIRMWARE}/v3.3.5.pib
NVM=${FIRMWARE}/v3.3.5-0-8-A-RC5.nvm
NVM=${FIRMWARE}/v3.3.5-0-8-B-RC5.nvm
NVM=${FIRMWARE}/v3.3.5-0-8-C-RC5.nvm
NVM=${FIRMWARE}/v3.3.5-0-8-D-RC5.nvm

PIB=${FIRMWARE}/v3.3.6.pib
NVM=${FIRMWARE}/v3.3.6-0-8-A-RC3.nvm
NVM=${FIRMWARE}/v3.3.6-0-8-B-RC3.nvm
NVM=${FIRMWARE}/v3.3.6-0-8-C-RC3.nvm
NVM=${FIRMWARE}/v3.3.6-0-8-D-RC3.nvm

PIB=${FIRMWARE}/v3.3.6.pib
NVM=${FIRMWARE}/v3.3.6-0-8-A-RC4.nvm
NVM=${FIRMWARE}/v3.3.6-0-8-B-RC4.nvm
NVM=${FIRMWARE}/v3.3.6-0-8-C-RC4.nvm
NVM=${FIRMWARE}/v3.3.6-0-8-D-RC4.nvm

PIB=${FIRMWARE}/v3.3.6.pib
NVM=${FIRMWARE}/v3.3.6-0-8-A-RC1.nvm
NVM=${FIRMWARE}/v3.3.6-0-8-B-RC1.nvm
NVM=${FIRMWARE}/v3.3.6-0-8-C-RC1.nvm
NVM=${FIRMWARE}/v3.3.6-0-8-D-RC1.nvm

# ====================================================================
#
# --------------------------------------------------------------------

