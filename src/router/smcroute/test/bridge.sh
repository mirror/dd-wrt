#!/bin/sh
# Verifies (*,G) routing between VLANs on top of a VLAN filtering bridge
# with bridge ports as VETH interfaces.  The endpoints of the VETH pairs
# are placed in isolated network namespaces to allow IP networking as
# one expects.
#set -x

# shellcheck source=/dev/null
. "$(dirname "$0")/lib.sh"

LEFT=/tmp/$NM/b1
RIGHT=/tmp/$NM/b2

print "Creating world ..."
topo isolated bridge "$LEFT" "$RIGHT"

# IP World
ip addr add 10.0.0.1/24 dev vlan1
nsenter --net="$LEFT"  -- ip addr add 10.0.0.10/24 dev eth0

ip addr add 20.0.0.1/24 dev vlan2
nsenter --net="$RIGHT" -- ip addr add 20.0.0.10/24 dev eth0
bridge link
bridge vlan
ip -br a

print "Creating config ..."
cat <<EOF > "/tmp/$NM/conf"
# vlan (*,G) multicast routing
phyint vlan1 enable
phyint vlan2 enable
mroute from vlan1 group 225.1.2.3 to vlan2
EOF
cat "/tmp/$NM/conf"

print "Starting smcrouted ..."
../src/smcrouted -f "/tmp/$NM/conf" -n -N -P "/tmp/$NM/pid" -u "/tmp/$NM/sock" &
sleep 1

print "Starting collector ..."
nsenter --net="$RIGHT" -- tshark -c 2 -lni eth0 -w "/tmp/$NM/pcap" 'dst 225.1.2.3' 2>/dev/null &
echo $! >> "/tmp/$NM/PIDs"
sleep 1

print "Starting emitter ..."
nsenter --net="$LEFT" -- ping -c 3 -W 1 -I eth0 -t 3 225.1.2.3
show_mroute

print "Analyzing ..."
lines=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.1.2.3 | tee "/tmp/$NM/result" | wc -l)
cat "/tmp/$NM/result"
echo " => $lines for 225.1.2.3, expected => 2"

########################################################################### DONE
# one frame lost due to initial (*,G) -> (S,G) setup
[ "$lines" = "2" ] && OK
FAIL
