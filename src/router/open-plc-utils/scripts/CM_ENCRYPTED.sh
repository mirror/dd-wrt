#!/bin/sh
# file: scripts/CM_ENCRYPTED.sh

# This script formats and sends a CM_ENCRYPTED_PAYBOOT message to a
# specific slave device; the message is written as a text file then
# passed to efsu to send;  
 
# ====================================================================
# host symbols;
# --------------------------------------------------------------------

. ./hardware.sh

# ====================================================================
# file symbols;
# --------------------------------------------------------------------

COUNT=1
FRAME=test.hex

# ====================================================================
#  display usage information;e
# --------------------------------------------------------------------

usage()
{
cat << EOF
usage: ${options}

This scripts sets the up and down bandwidth on Slave devices in the network

OPTIONS
	-h	show this message
	-s	slave device name
	-u	Upstream Bandwidth required in Mbps
	-d	Downstream Bandwidth required in Mbps

Note: Bandwidth value support 0-15 input values and 0 selects full line rate

EOF
}

# ====================================================================
#  command line parser routine
# --------------------------------------------------------------------

device=

while getopts "hs:u:d:" OPTION
do
	case ${OPTION} in
	h)
	usage
	exit 1
	;;
	s)
	device=${OPTARG}
	;;
	esac
done

if [[ -z ${device} ]]; then
	usage
	exit 1
elif [ ${device} = slave1 ]; then 
	target=${slave1}
elif [ ${device} = slave2 ]; then 
	target=${slave2}
elif [ ${device} = slave3 ]; then 
	target=${slave3}
elif [ ${device} = slave4 ]; then 
	target=${slave4}
elif [ ${device} = slave5 ]; then 
	target=${slave5}
else 
	target=${master}
fi

# ====================================================================
# Format the MME with processed values from command line
# --------------------------------------------------------------------

da="ff ff ff ff ff ff"
sa="ff ff ff ff ff ff"
mtype="88 e1"
mmv="01"
mmtype="06 60"
FMI="00 00"
PEKS="0F"
AVLN="00"
PID="04"
PRN="09 75"
PMN="01"
UUID="55 aa 55 aa 55 aa 55 aa 55 aa 55 aa 55 aa 55 aa"
LEN="00 00"
HLE="AA 55 aa 55 aa 55 aa 55 aa 55 aa 55 aa 55 aa 55"
fill="ff ff ff ff ff ff ff ff ff" 

cat > ${FRAME} <<EOF
${da} ${sa} ${mtype} ${mmv} ${mmtype} ${FMI} ${PEKS} ${AVLN} ${PID} ${PRN} ${PMN} ${UUID} ${LEN} ${HLE} ${fill} ${fill} ${fill} ${fill} ${fill} ${fill} ${fill} ${fill}
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

efsu -i ${ETH2} -h ${FRAME} -l ${COUNT} -v

rm ${FRAME}

