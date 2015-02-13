#!/bin/sh
# file: slac/pl16-pev.sh

# ====================================================================
# define environment variables;
# --------------------------------------------------------------------

PLC=eth1
SFL=0011-NvmSoftloader-QCA7000-v1.1.0.11-RC.nvm
PIB=0011-QCA7000-SpiSlave-HomePlugGP_NorthAmerica.pib
NVM=0011-MAC-QCA7000-v1.1.0.11-X-RC.nvm

# ====================================================================
# edit PIB file and replace to PL16 PEV factory PIB;
# --------------------------------------------------------------------

modpib -v -M next ${PIB} -U "PEV"
plcinit -P ${PIB} -N ${NVM} -D $(plcID -D) -F 
plcinit -P ${PIB} -D $(plcID -D)

