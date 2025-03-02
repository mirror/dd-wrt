#!/bin/sh
# Verifies (*,G) routing between two emulated end devices.
#set -x

# shellcheck source=/dev/null
. "$(dirname "$0")/lib.sh"

LEFT=/tmp/$NM/left
RIGHT=/tmp/$NM/right
LIF=$(basename "$LEFT")
RIF=$(basename "$RIGHT")

print "Creating world ..."
topo isolated "$LEFT" "$RIGHT"

# IP World
ip addr add 10.0.0.1/24 dev "$LIF"
nsenter --net="$LEFT" -- ip addr add 10.0.0.10/24 dev eth0

ip addr add 20.0.0.1/24 dev "$RIF"
nsenter --net="$RIGHT" -- ip addr add 20.0.0.10/24 dev eth0

ip -br l
ip -br a

print "Creating config ..."
cat <<EOF > "/tmp/$NM/conf"
# vlan (*,G) multicast routing
phyint $LIF  enable
phyint $RIF enable
mroute from $LIF group 225.1.2.3 to $RIF
EOF
cat "/tmp/$NM/conf"

print "Starting smcrouted ..."
../src/smcrouted -f "/tmp/$NM/conf" -n -N -P "/tmp/$NM/pid" -l debug -u "/tmp/$NM/sock" &
sleep 1

print "Starting collector ..."
nsenter --net="$RIGHT" -- tshark -c 2 -lni eth0 -w "/tmp/$NM/pcap" dst 225.1.2.3 &
echo $! >> "/tmp/$NM/PIDs"
sleep 1

print "Starting emitter ..."
nsenter --net="$LEFT" -- ping -c 3 -W 1 -I eth0 -t 3 225.1.2.3
show_mroute

print "Analyzing ..."
lines=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.1.2.3 | tee "/tmp/$NM/result" | wc -l)
cat "/tmp/$NM/result"
echo " => $lines expected 2"

########################################################################### DONE
# one frame lost due to initial (*,G) -> (S,G) setup
[ "$lines" = "2" ] && OK
FAIL
