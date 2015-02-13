#!/bin/sh
# file: scripts/pts.sh

# This is an example PTS script for bash shells; Both Linux and Cygwin
# bash are suitable shells;

# Define proper values in files hardware.sh and firmware.sh, connect
# two INT6300 devices that share a power strip then run this script;

# THe script will define symbols, create a frame file and program one
# device as a golden node; it then enters a loop that programs the
# other device, runs two rate tests and resets the golden node;

# ====================================================================
# host symbols;
# --------------------------------------------------------------------

. /etc/environment
. ${SCRIPTS}/hardware.sh
. ${SCRIPTS}/firmware.sh

# ====================================================================
# file symbols;
# --------------------------------------------------------------------

CNT=1000
PKT=frame.hex
LOG=time.log

DAK1=00:11:22:33:44:55:66:77:88:99:AA:BB:CC:DD:EE:FF
DAK2=FF:EE:DD:CC:BB:AA:99:88:77:66:55:44:33:22:11:00

# ====================================================================
# create dummy ethernet frame;
# --------------------------------------------------------------------

cat > ${PKT} << EOF
FF FF FF FF FF FF FF FF FF FF FF FF 08 00 FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF FF
EOF

# ====================================================================
# check environment;
# --------------------------------------------------------------------

  if [ ! -f ${PIB} ]; then
	echo "File ${PIB} is missing or misplaced."
	exit 1
elif [ ! -f ${NVM} ]; then
	echo "File ${NVM} is missing or misplaced."
	exit 1
elif [ ! -f ${PKT} ]; then
	echo "File ${PKT} is missing or misplaced."
	exit 1
fi

# ====================================================================
# check connections;
# --------------------------------------------------------------------

int6kwait -xqsi ${ETH1} -c10
if [ ${?} != 0 ]; then
	echo "Reference Unit is not connected."
	exit 1
fi

int6kwait -xqsi ${ETH2} -c10
if [ ${?} != 0 ]; then
	echo "Production Unit is not connected."
	exit 1
fi

# ====================================================================
# program golden node;
# --------------------------------------------------------------------

clear

echo
echo Program Reference Unit
echo 

MAC=00:B0:52:00:00:AA
NMK=$(rkey secret.key -M)
DAK=$(int6kid -Di ${ETH1})
modpib -C0 -M ${MAC} -N ${NMK} -D ${DAK1} ${PIB}
int6kp -i ${ETH1} -P ${PIB} -N ${NVM} -D ${DAK} -FF 

# ====================================================================
# program and test devices;
# --------------------------------------------------------------------

while [ 1 ]; do  
clear
echo $(date)

echo
echo Program Production Unit $((++unit))
echo 

MAC=00:B0:52:00:00:BB
NMK=$(rkey secret.key -M)
DAK=$(int6kid -Di ${ETH2})
modpib -C0 -M ${MAC} -N ${NMK} -D ${DAK2} ${PIB}
int6kp -i ${ETH2} -P ${PIB} -N ${NVM} -D ${DAK} -FF

echo
echo Stabilize Devices
echo 

int6kwait -w20

echo
echo Associate Devices
echo 

int6k -i ${ETH1} -B1
int6kwait -w3
int6k -i ${ETH2} -B1
int6kwait -rsai ${ETH2}  
int6kwait -rsai ${ETH2} 
# int6kwait -w10

echo
echo Rate Test One             
echo

efsu -i ${ETH1} -hd ${NIC2} ${PKT} -l ${CNT}          
efsu -i ${ETH2} -hd ${NIC1} ${PKT} -l ${CNT}
efsu -i ${ETH1} -hd ${NIC2} ${PKT} -l ${CNT}
efsu -i ${ETH2} -hd ${NIC1} ${PKT} -l ${CNT} 
int6krate -ni ${ETH2}

echo
echo Reset Devices                 
echo

int6k -Ri ${ETH1}
int6kwait -rsai ${ETH1} 
int6k -Ri ${ETH2}
int6kwait -rsai ${ETH2} 

echo
echo Stabilize Devices
echo 

int6kwait -w20

echo
echo Rate Test Two                 
echo

efsu -i ${ETH1} -hd ${NIC2} ${PKT} -l ${CNT}          
efsu -i ${ETH2} -hd ${NIC1} ${PKT} -l ${CNT}
efsu -i ${ETH1} -hd ${NIC2} ${PKT} -l ${CNT}
efsu -i ${ETH2} -hd ${NIC1} ${PKT} -l ${CNT} 
int6krate -ni ${ETH2}

echo
echo Reset Reference Unit
echo 

int6k -Ti ${ETH1}
int6kwait -w10
done

