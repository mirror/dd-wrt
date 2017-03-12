#!/bin/bash

# Public domain, not copyrighted..

set -u

# number of bgpd instances, not more than 255 at this point.  At least 3 are
# needed to connect in a ring.
NUM=7

# The NUM peers can be connected in a ring topology.
#
# This sets the proportion of other peers that each peer should be
# configured to connect to E.g., 20 means each BGP instance will peer with
# 20% of the other peers before and after it in the ring.  So 10% of the
# peers prior to this instance in the ring, and 10% of the following peers. 
# 100 should lead to a full-mesh, for an odd total number of peers.
#
# A value of 1 will result in each instance having at least 2 peers in the ring.
#
# A value of 0 will disable creating a ring, in which case the only peers 
# configured will be those in the EXPEERS list.
PEERPROP=100

# number of routes each BGP instance should advertise
ADV=10
# First octet to use for the IPv4 advertisements.  The advertisements
# will be /32s under this /8.  E.g.  ADVPREF=10 will mean
# 10.x.y.z/32's are advertised.
ADVPREF=10

# Base VTY port to allocate Quagga telnet vtys from. VTYBASE+ID will be
# the port.
VTYBASE=2610
# Base ASN to allocate ASNs to instances.
ASBASE=64500
PREFIX=192.168.145.
#PREFIX=3ffe:123:456::
ADDRPLEN=32
CONFBASE=/tmp
PIDBASE=/var/run/quagga
USER=quagga
GROUP=quagga

# MRAI to specify, where an implementation supports it.
MRAI=1
# Connect retry timer
CONNECTRETRY=1

# The binary locations for BGP instances. 
declare -A BGP_BINS=(
	[quagga]=/usr/sbin/bgpd
	[bird]=/usr/sbin/bird
	[birdgit]=/home/paul/code/bird/bird
	[quaggagit]=/home/paul/code/quagga/bgpd/bgpd
	[exabgp]=/home/paul/code/exabgp/sbin/exabgp
)

# Configuration generation functions for the BGP instances.
declare -A BGP_CONFIGGEN=(
	[quagga]=quagga_config
	[quaggagit]=quagga_config
	[bird]=bird_config
	[birdgit]=bird_config
	[exabgp]=exabgp_config
)

# Launch functions for the BGP instances.
declare -A BGP_LAUNCH=(
	[quagga]=quagga_launch
	[quaggagit]=quagga_launch
	[bird]=bird_launch
	[birdgit]=bird_launch
	[quaggagit]=quagga_launch
	[exabgp]=exabgp_launch
)

# the instances to run, in the order they should appear in the ring
# (repeated over until there are $NUM instances).  The value must exist as a
# key into the above two arrays.
declare -a BGP_INSTANCES=(
	quagga
	bird
	quaggagit
	exabgp
)

# Peers to configure, that are external to this script. One list of IPs, with
# corresponding list of their ASes.
#
# e.g.:
#EXPEERS=(192.168.147.{1..10})
#EXPEERASES=($(seq $((ASBASE+11)) $(($ASBASE+20))))

EXPEERS=()
EXPEERASES=()

############################################################################
# Can override any of the above from a supplied file with declarations
CONFWRITE=Y
if [ $# -gt 0 ] ; then
	echo "multiple-bgpd.sh: sourcing config from $1"
	[ -f "$1" ] && . "$1"
	
	# keep config, if exists
	[ $# -gt 1 ] && [ "$2" = "k" ] && CONFWRITE=N	
fi

############################################################################
# Internal variables.

# Number of peers for each instance to peer with
PEERNUM=$(( ($NUM-1) * $PEERPROP / 100  ))
[ "$PEERNUM" -gt $(($NUM-1)) ] && PEERNUM=$(($NUM-1))

# the 'range', i.e.  how many of the previous and next peers in the ring to
# connect to
PEERRANGE=$(( $PEERNUM/2 ))
[ "$PEERPROP" -gt 0 -a "$NUM" -ge 3 -a  "$PEERRANGE" -le 0 ] && PEERRANGE=1

# and a convenience expansion
PEEREXP=""
if [ "$PEERRANGE" -gt 0 ]; then
	PEEREXP=($(seq -${PEERRANGE} ${PEERRANGE}))
	# dont need 0
	unset PEEREXP[PEERRANGE]
fi

#echo ${PEEREXP[@]}

############################################################################
## helpers

# translate instance ID to its address.
id2addr () {
	local ID=$1
	echo ${PREFIX}${ID}
}

# return the ID of a peer, in terms of an offset on the given instance's ID.
#
# E.g., given an ID of 1 and an offset of -1, if there are 10 instances overall,
# this will return 10.
peeridoff () {
	local ID=$1
	local OFF=$2
	echo $(( (($ID + $OFF - 1  + $NUM) % $NUM) + 1  ))
}

# return IPv4 address to advertise, for given instance ID and number.
advipaddr () {
	local ID=$1
	local N=$2
	echo "$ADVPREF.$(( ($N >> 16) %256 )).$(( ($N >> 8) % 256 )).$(( $N % 256  ))"
}

############################################################################
# launch functions
#
# do not daemonise, so that all launched instances can be killed by killing
# the script.
#

quagga_launch () {
	local ID=$1
	local ASN=$2
	local ADDR=$3
	local BIN=$4
	local CONF=$5
	${BIN} -i "${PIDBASE}"/bgpd${ID}.pid \
		   -l ${ADDR} \
		   -f "${CONF}" \
		   -u $USER -g $GROUP \
		   -P $((${VTYBASE}+${ID}))
}

exabgp_launch () {
	local ID=$1
	local ASN=$2
	local ADDR=$3
	local BIN=$4
	local CONF=$5
	
	env exabgp.api.file="${PIDBASE}"/exabgp${ID}.ctl \
	exabgp.daemon.pid="${PIDBASE}"/bgpd${ID}.pid \
	exabgp.daemon.daemonize=false \
	exabgp.tcp.bind=${ADDR} \
	exabgp.log.enable=false \
	exabgp.daemon.user=quagga \
	${BIN} ${CONF}
}

bird_launch () {
	local ID=$1
	local ASN=$2
	local ADDR=$3
	local BIN=$4
	local CONF=$5
	${BIN} -P "${PIDBASE}"/bird${ID}.pid \
		   -c "${CONF}" \
		   -s "${PIDBASE}"/bird${ID}.ctl \
		   -f
}

#######################################################################
#
# functions to write the configuration for instances
#

exabgp_config () {
	local ID=$1
	local ASN=$2
	local ADDR=$3
	
	local N
	local P
	
	cat <<- EOF
		group default {
		  local-address $ADDR;
		  local-as $ASN;
		  router-id $ADDR;
		  
		  capability {
		    asn4 enable;
		  }
	EOF
	
	for N in $(seq 1 $ADV) ; do
		echo "  static {"
		echo "    route `advipaddr $ID $N`/32 {"
		echo "      next-hop $ADDR;"
		echo "    }"
		echo "  }" 
	done

	for P in ${PEEREXP[@]};  do
		[ "$P" -eq 0 ] && continue;
		
		#local PID=$(( (($ID + $P - 1  + $NUM) % $NUM) + 1  ))
		local PID=`peeridoff $ID $P`
		#local PADDR="${PREFIX}${PID}"
		local PADDR=`id2addr $PID`
		local PAS=$((${ASBASE} + $PID))
		
		echo "  neighbor $PADDR {"
		#echo "    local-address $ADDR;"
		#echo "    local-as $ASN;"
		#echo "    graceful-restart;"
		#echo "    router-id $ADDR;"
		echo "    peer-as $PAS;"
		echo "  }"
	done
	
	for P in ${!EXPEERS[@]};  do
		echo "  neighbor ${EXPEERS[$P]} {"
		echo "    peer-as ${EXPEERASES[$P]};"
		echo "  }"
	done
	
	cat <<- EOF
		}
	EOF
}

quagga_config () {
	local ID=$1
	local ASN=$2
	local ADDR=$3
	
	local N
	local P
	
	# Edit config to suit.
	cat <<- EOF
		password foo
		service advanced-vty
		!
		router bgp ${ASN}
		 bgp router-id ${ADDR}
		 !maximum-paths 32
		 !bgp bestpath as-path multipath-relax
	EOF

	for N in $(seq 1 $ADV) ; do
		echo " network `advipaddr $ID $N`/32"
	done

	cat <<- EOF
		 neighbor default peer-group
		 neighbor default update-source ${ADDR}
		 neighbor default capability orf prefix-list both
		 !neighbor default soft-reconfiguration inbound
		 neighbor default advertisement-interval $MRAI
		 neighbor default timers connect $CONNECTRETRY
		 neighbor default route-map test out
	EOF

	for P in ${PEEREXP[@]};  do
		[ "$P" -eq 0 ] && continue;
		
		local PID=`peeridoff $ID $P`
		local PADDR=`id2addr $PID`
		local PAS=$((${ASBASE} + $PID))
		echo " neighbor ${PADDR} remote-as ${PAS}"
		echo " neighbor ${PADDR} peer-group default"
	done

	for P in ${!EXPEERS[@]};  do
		echo " neighbor ${EXPEERS[$P]} remote-as ${EXPEERASES[$P]}"
		echo " neighbor ${EXPEERS[$P]} peer-group default"
	done

	cat <<- EOF
		!
		 address-family ipv6
		 network 3ffe:${ID}::/48
		 network 3ffe:${ID}:1::/48 pathlimit 1
		 network 3ffe:${ID}:2::/48 pathlimit 3
		 network 3ffe:${ID}:3::/48 pathlimit 3
		 neighbor default activate
		 neighbor default capability orf prefix-list both
		 neighbor default default-originate
		 neighbor default route-map test out
	EOF

	for P in ${PEEREXP[@]};  do
		[ "$P" -eq 0 ] && continue;
		
		local PID=`peeridoff $ID $P`
		local PADDR=`id2addr $PID`
		local PAS=$((${ASBASE} + $PID))
		echo " neighbor ${PADDR} peer-group default"
	done

	cat <<- EOF
		 exit-address-family
		!
		! bgpd still has problems with extcommunity rt/soo
		route-map test permit 10
		 set extcommunity rt ${ASN}:1
		 set extcommunity soo ${ASN}:2
		 set community ${ASN}:1
		!
		line vty
		 exec-timeout 0 0
		!
		end
	EOF
}

bird_config () {
	local ID=$1
	local ASN=$2
	local ADDR=$3

	cat <<- EOF
		#log "/var/log/bird.log" all;
		#debug protocols all;

		# Override router ID
		router id ${ADDR};
		listen bgp address ${ADDR};
		
		protocol kernel { device routes; import all; }
		protocol device { import all; }
		
		function avoid_martians()
		prefix set martians;
		{
		  martians = [ 
		  	       224.0.0.0/4+, 240.0.0.0/4+
		  	     ];

		  # Avoid RFC1918 and similar networks
		  if net ~ martians then return false;
		  return true;
		}
		
		filter import_filter
		{
		  if ! (avoid_martians()) then reject;
		  accept;
		}
		
		filter set_comm
		{
		  bgp_community.add ((${ASN}, 1));
		  accept;
		}
		
		template bgp peer_conf {
		  local as ${ASN};
		  source address ${ADDR};
		  import filter import_filter;
		  export filter set_comm;
		  multihop;
		}
	EOF
	
	local P;
	
	for P in ${PEEREXP[@]};  do
		[ "$P" -eq 0 ] && continue;
		
		local PID=`peeridoff $ID $P`
		local PADDR=`id2addr $PID`
		local PAS=$((${ASBASE} + $PID))
		echo "protocol bgp from peer_conf {"
		echo " neighbor ${PADDR} as  ${PAS};"
		echo "}"
	done
	
	for P in ${!EXPEERS[@]};  do
		echo "protocol bgp from peer_conf {"
		echo " neighbor ${EXPEERS[$P]} as ${EXPEERASES[$P]};"
		echo "}"
	done
	
	
	for N in $(seq 1 $ADV) ; do
		echo " network `advipaddr $ID $N`/32"
	done
}

#######################################################################

for ID in $(seq 1 $NUM); do
	BGP_INST=${BGP_INSTANCES[${ID} % ${#BGP_INSTANCES[@]}]}
	BGPBIN=${BGP_BINS[$BGP_INST]}
	CONF="${CONFBASE}"/${BGP_INST}_bgpd${ID}.conf
	ASN=$(($ASBASE + ${ID}))
	ADDR=`id2addr $ID`
	
	#if [ ! -e "$CONF" ] ; then
	if [ ! -e "$CONF" -o "$CONFWRITE" = "Y" ] ; then 
		${BGP_CONFIGGEN[$BGP_INST]} $ID $ASN $ADDR > "$CONF"
		chown $USER:$GROUP "$CONF"
	fi
	# You may want to automatically add configure a local address
	# on a loop interface.
	#
	# Solaris: ifconfig vni${H} plumb ${ADDR}/${ADDRPLEN} up
	# Linux:
	#ip address add ${ADDR}/${ADDRPLEN} dev lo 2> /dev/null
	
	ip link add dummy${ID} type dummy 2> /dev/null
	ip link set dev dummy${ID} up
	ip address add ${ADDR}/${ADDRPLEN} dev dummy${ID} 2> /dev/null
	
	${BGP_LAUNCH[$BGP_INST]} $ID $ASN $ADDR $BGPBIN $CONF &
	
	sleep 0.1
done

echo "multiple-bgpd.sh: waiting..."

wait
