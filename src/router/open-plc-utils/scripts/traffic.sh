#!/bin/sh
# file: scripts/traffic.sh

# this script attempts to send traffic between NIC1 and NIC2 in both
# directions to establish a PHY Rate; the frame file can contain any 
# valid Ethernet frame;

# ====================================================================
# host symbols;
# --------------------------------------------------------------------

. /etc/environment
. ${SCRIPTS}/hardware.sh

# ====================================================================
# file symbols;
# --------------------------------------------------------------------

COUNT=10000
FRAME=frame.hex

# ====================================================================
# create dummy ethernet frame;
# --------------------------------------------------------------------

cat > ${FRAME} << EOF
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

if [ ! -f ${FRAME} ]; then
	echo "File ${FRAME} is missing or misplaced"
	exit 1
fi

# ====================================================================
# send traffic in both directions;
# --------------------------------------------------------------------

efsu -i ${ETH1} -hd ${NIC2} ${FRAME} -l ${COUNT}          
efsu -i ${ETH2} -hd ${NIC1} ${FRAME} -l ${COUNT}
efsu -i ${ETH1} -hd ${NIC2} ${FRAME} -l ${COUNT}
efsu -i ${ETH2} -hd ${NIC1} ${FRAME} -l ${COUNT} 

# ====================================================================
# echo device TX/RX PHY Rates;        
# --------------------------------------------------------------------

int6krate -ni ${ETH2}

