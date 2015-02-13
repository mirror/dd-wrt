#!/bin/sh

# ====================================================================
#   Copyright (c) 2010-2012 Qualcomm Atheros Incorporated.
# --------------------------------------------------------------------

PROJECTS=VisualStudioNET
FOLDERS="ether key mdio mme nodes nvm pib plc ram serial tools"

# ====================================================================
#   compile all programs stand-alone to verify include statements;
# --------------------------------------------------------------------

for folder in ${FOLDERS}; do
	echo ${folder}
	cd ${folder}
	sh ${folder}.sh
	cd ..
done

