#!/bin/sh
# Verifies IPv4 mrdisc messages on all interfaces it is enabled on
# Runs for two intervals to ensure interval timer works properly.
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
phyint a1 enable mrdisc
phyint a2 enable mrdisc
EOF
cat "/tmp/$NM/conf"

print "Starting smcrouted ..."
../src/smcrouted -f "/tmp/$NM/conf" -n -N -P "/tmp/$NM/pid" -l debug -u "/tmp/$NM/sock" -m 4 &
sleep 1

collect a2 -c3 'dst 224.0.0.106'
sleep 10

print "Analyzing ..."
lines=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 224.0.0.106 | tee "/tmp/$NM/result" | wc -l)
cat "/tmp/$NM/result"
echo " => $lines for 224.0.0.106 (MRDISC), expected >= 2"

# shellcheck disable=SC2086
[ $lines -ge 2 ] && OK
FAIL
