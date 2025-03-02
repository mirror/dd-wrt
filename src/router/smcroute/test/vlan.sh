#!/bin/sh
# Verify IPv4 (*,G) routing on top of VLAN interfaces
#set -x

# shellcheck source=/dev/null
. "$(dirname "$0")/lib.sh"

print "Creating world ..."
topo basic vlan 100 110
ip addr add 10.100.0.1/24 dev a1.100
ip addr add 10.110.0.1/24 dev a1.110
ip addr add 20.100.0.1/24 dev a2.100
ip addr add 20.110.0.1/24 dev a2.110
ip -br a

print "Creating config ..."
cat <<EOF > "/tmp/$NM/conf"
# vlan (*,G) multicast routing
phyint a1.100 enable
phyint a1.110 enable
phyint a2.110 enable
mroute from a1.100 group 225.1.2.3 to a1.110 a2.110
mroute from a2.110 group 225.3.2.1 to a1.110 a1.100
EOF
cat "/tmp/$NM/conf"

print "Starting smcrouted ..."
../src/smcrouted -f "/tmp/$NM/conf" -n -N -P "/tmp/$NM/pid" -l debug -u "/tmp/$NM/sock" &
sleep 1

print "Starting collectors ..."
tshark -c 6 -lni a1.110 -w "/tmp/$NM/pcap1" 'icmp and (dst 225.1.2.3 or dst 225.3.2.1)' 2>/dev/null &
echo $! >> "/tmp/$NM/PIDs"
tshark -c 3 -lni a2.110 -w "/tmp/$NM/pcap2" 'icmp and dst 225.1.2.3' 2>/dev/null &
echo $! >> "/tmp/$NM/PIDs"
tshark -c 3 -lni a1.100 -w "/tmp/$NM/pcap3" 'icmp and dst 225.3.2.1' 2>/dev/null &
echo $! >> "/tmp/$NM/PIDs"
sleep 1

print "Starting emitters ..."
ping -c 3 -W 1 -I a1.100 -t 2 225.1.2.3 >/dev/null
ping -c 3 -W 1 -I a2.110 -t 2 225.3.2.1 >/dev/null
show_mroute

print "Analyzing ..."
lines1=$(tshark -r "/tmp/$NM/pcap1" 2>/dev/null | grep 225.1.2.3 | tee    "/tmp/$NM/result1" | wc -l)
lines2=$(tshark -r "/tmp/$NM/pcap1" 2>/dev/null | grep 225.3.2.1 | tee -a "/tmp/$NM/result1" | wc -l)
lines3=$(tshark -r "/tmp/$NM/pcap2" 2>/dev/null | grep 225.1.2.3 | tee    "/tmp/$NM/result2" | wc -l)
lines4=$(tshark -r "/tmp/$NM/pcap3" 2>/dev/null | grep 225.3.2.1 | tee    "/tmp/$NM/result3" | wc -l)
echo "Receieved on a1.110, expected >=2 pkt of 225.1.2.3 and >=2 pkt 225.3.2.1:"
cat "/tmp/$NM/result1"
echo "Receieved on a2.110, expected >=2 pkt of 225.1.2.3:"
cat "/tmp/$NM/result2"
echo "Receieved on a1.100, expected >=2 pkt of 225.3.2.1:"
cat "/tmp/$NM/result3"

########################################################################### DONE
# one frame lost due to initial (*,G) -> (S,G) setup
# shellcheck disable=SC2086 disable=SC2166
[ $lines1 -ge 2 -a $lines2 -ge 2 -a $lines3 -ge 2 -a $lines4 -ge 2 ] && OK
FAIL
