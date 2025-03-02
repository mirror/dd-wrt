#!/bin/sh
# Verifies IPv4 (S,G) and (*,G) rules by injecting frames on one
# interface and verifying reception on another.
#set -x

# shellcheck source=/dev/null
. "$(dirname "$0")/lib.sh"

print "Creating world ..."
topo basic
ip addr add 10.0.0.1/24 dev a1
ip addr add 20.0.0.1/24 dev a2
ip -br a

print "Creating config ..."
cat <<EOF > "/tmp/$NM/conf"
# basic (*,G) multicast routing
phyint a1 enable
phyint a2 enable

mgroup from a1 source 10.0.0.1 group 225.3.2.1
mroute from a1 source 10.0.0.1 group 225.3.2.1 to a2

mgroup from a1 group 225.1.2.3
mroute from a1 group 225.1.2.3 to a2
EOF
cat "/tmp/$NM/conf"

print "Starting smcrouted ..."
../src/smcrouted -f "/tmp/$NM/conf" -n -N -P "/tmp/$NM/pid" -l debug -u "/tmp/$NM/sock" &
sleep 1

echo "-----------------------------------------------------------------------------------"
../src/smcroutectl -pu "/tmp/$NM/sock" show interfaces
echo "-----------------------------------------------------------------------------------"
ip maddress
echo "-----------------------------------------------------------------------------------"
cat /proc/net/mcfilter
echo "-----------------------------------------------------------------------------------"
../src/smcroutectl -pu "/tmp/$NM/sock" show groups

collect a2 -c12 'dst 225.3.2.1 or dst 225.1.2.3 or dst 225.1.2.4 or 225.1.2.5'

print "Adding IPC routes ..."
../src/smcroutectl -u "/tmp/$NM/sock" add a1 225.1.2.4 a2
../src/smcroutectl -u "/tmp/$NM/sock" add a1 10.0.0.1 225.1.2.5 a2
show_mroute

print "Starting emitter ..."
ping -c 3 -W 1 -I a1 -t 2 225.3.2.1 >/dev/null
ping -c 3 -W 1 -I a1 -t 2 225.1.2.3 >/dev/null
ping -c 3 -W 1 -I a1 -t 2 225.1.2.4 >/dev/null
ping -c 3 -W 1 -I a1 -t 2 225.1.2.5 >/dev/null
show_mroute

print "Analyzing ..."
lines1=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.1.2.3 | tee    "/tmp/$NM/result" | wc -l)
lines2=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.3.2.1 | tee -a "/tmp/$NM/result" | wc -l)
lines3=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.1.2.4 | tee -a "/tmp/$NM/result" | wc -l)
lines4=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.1.2.5 | tee -a "/tmp/$NM/result" | wc -l)
cat "/tmp/$NM/result"
echo " => $lines1 for 225.1.2.3, expected => 2"
echo " => $lines2 for 225.3.2.1, expected => 3"
echo " => $lines3 for 225.1.2.4, expected => 2"
echo " => $lines4 for 225.1.2.5, expected => 3"

########################################################################### DONE
# Expect one frame lost due to initial (*,G) -> (S,G) route setup, while
# we don't expect any frame loss in pure (S,G) routes
# shellcheck disable=SC2166 disable=SC2086
[ $lines1 -ge 2 -a $lines2 -eq 3 -a $lines3 -ge 2 -a $lines4 -eq 3 ] && OK
FAIL
