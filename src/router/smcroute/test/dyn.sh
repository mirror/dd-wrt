#!/bin/sh
# Verifies (*,G/LEN) and (S/LEN,G) routing by injecting frames on one
# interface and verifying reception on another.
#set -x

# shellcheck source=/dev/null
. "$(dirname "$0")/lib.sh"

print "Creating world ..."
topo basic
ip addr add 10.0.0.1/24 dev a1
ip addr add 20.0.0.1/24 dev a2
ip addr add 2001:1::1/64 dev a1
ip addr add   fc00::42/64 dev a1
ip addr add 2001:2::1/64 dev a2
ip -br a

print "Creating config ..."
cat <<EOF > "/tmp/$NM/conf"
# basic (*,G/LEN) multicast routing
phyint a1 enable
phyint a2 enable

mroute from a1 group 225.1.2.3/24 to a2
mroute from a1 group ff2e::42/121 to a2

mroute from a1 source 10.0.0.1/24 group 225.3.2.1 to a2
mroute from a1 source fc00::1/64  group ff04::114 to a2

mroute from a1 source 10.0.0.1 group 225.3.3.3/24 to a2
EOF
cat "/tmp/$NM/conf"

print "Starting smcrouted ..."
../src/smcrouted -f "/tmp/$NM/conf" -n -N -P "/tmp/$NM/pid" -l debug -u "/tmp/$NM/sock" &
sleep 1

echo "-----------------------------------------------------------------------------------"
../src/smcroutectl -pu "/tmp/$NM/sock" show interfaces
echo "-----------------------------------------------------------------------------------"
../src/smcroutectl -pu "/tmp/$NM/sock" show routes

collect a2 -c21 'dst 225.1.2.1 or dst 225.1.2.201 or dst ff2e::42 or dst ff2e::7f or dst 225.3.2.1 or dst ff04::114 or dst 225.3.3.1'

print "Starting emitter ..."
ping    -c 3 -W 1 -I a1       -t 2 225.1.2.1   >/dev/null
ping    -c 3 -W 1 -I a1       -t 2 225.1.2.201 >/dev/null
ping -6 -c 3 -W 1 -I fc00::42 -t 2 ff2e::42    >/dev/null
ping -6 -c 3 -W 1 -I fc00::42 -t 2 ff2e::7f    >/dev/null
ping    -c 3 -W 1 -I a1       -t 2 225.3.2.1   >/dev/null
ping -6 -c 3 -W 1 -I fc00::42 -t 2 ff04::114   >/dev/null
ping    -c 3 -W 1 -I a1       -t 2 225.3.3.1   >/dev/null
show_mroute

print "Analyzing ..."
lines1=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.1.2.1   | tee "/tmp/$NM/result" | wc -l)
lines2=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.1.2.201 | tee "/tmp/$NM/result" | wc -l)
lines3=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep ff2e::42    | tee "/tmp/$NM/result" | wc -l)
lines4=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep ff2e::7f    | tee "/tmp/$NM/result" | wc -l)
lines5=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.3.2.1   | tee "/tmp/$NM/result" | wc -l)
lines6=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep ff04::114   | tee "/tmp/$NM/result" | wc -l)
lines7=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.3.3.1   | tee "/tmp/$NM/result" | wc -l)
cat "/tmp/$NM/result"
echo " => $lines1 for 225.1.2.1,   expected >= 2"
echo " => $lines2 for 225.1.2.201, expected >= 2"
echo " => $lines3 for ff2e::42,    expected >= 2"
echo " => $lines4 for ff2e::7f,    expected >= 2"
echo " => $lines5 for 225.3.2.1,   expected >= 2"
echo " => $lines6 for ff04::114,   expected >= 2"
echo " => $lines7 for 225.3.3.1,   expected >= 2"

########################################################################### DONE
# Expect one frame lost due to initial (*,G) -> (S,G) route setup
# shellcheck disable=SC2166 disable=SC2086
[ $lines1 -ge 2 -a $lines2 -ge 2 -a $lines3 -ge 2 -a $lines4 -ge 2 -a $lines5 -ge 2 -a $lines6 -ge 2 -a $lines7 -ge 2 ] && OK
FAIL
