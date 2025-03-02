#!/bin/sh
# Verifies IPv4 and IPv6 (*,G) => learned (S,G) flushing
#set -x

# shellcheck source=/dev/null
. "$(dirname "$0")/lib.sh"

print "Creating world ..."
topo basic

# IP world ...
ip addr add 10.0.0.1/24  dev a1
ip addr add 2001:1::1/64 dev a1
ip addr add   fc00::1/64 dev a1
ip addr add 20.0.0.1/24  dev a1
ip addr add 2001:2::1/64 dev a2
ip -br a

print "Creating config ..."
cat <<EOF > "/tmp/$NM/conf"
# ipv4 and ipv6 (*,G) multicast routing
phyint a1 enable
phyint a2 enable

mroute from a1 source 10.0.0.10 group 225.3.2.1 to a2
mroute from a1 group 225.1.2.3 to a2

mroute from a1 source fc00::1 group ff04:0:0:0:0:0:0:114 to a2
mroute from a1 group ff2e::42 to a2
EOF
cat "/tmp/$NM/conf"

print "Starting smcrouted ..."
../src/smcrouted -c 10 -f "/tmp/$NM/conf" -n -N -P "/tmp/$NM/pid" -l debug -u "/tmp/$NM/sock" &
sleep 1

collect a2 -c12 'dst ff04::114 or dst ff2e::42 or dst ff2e::43 or dst ff2e::44'

print "Starting emitter ..."
ping    -c 3 -W 1 -I a1        -t 3 225.1.2.3 >/dev/null
ping -6 -c 3 -W 1 -I 2001:1::1 -t 3 ff2e::42  >/dev/null
show_mroute

ip mroute | grep -q "(10.0.0.1,225.1.2.3)"
[ $? -eq 0 ] || FAIL "Failed learning IPv4 (*,G) route"

ip -6 mroute | grep -q "(2001:1::1,ff2e::42)"
[ $? -eq 0 ] || FAIL "Failed learning IPv6 (*,G) route"

print "Verifying (*,G) flush, please wait ... "
sleep 11
show_mroute
ip mroute | grep -q "(10.0.0.1,225.1.2.3)"
[ $? -eq 1 ] || FAIL "Failed flushing IPv4 (*,G) route"

ip -6 mroute | grep -q "(2001:1::1,ff2e::42)"
[ $? -eq 1 ] || FAIL "Failed flushing IPv6 (*,G) route"

print "Analyzing ..."
lines2=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep ff2e::42  | tee -a "/tmp/$NM/result" | wc -l)
cat "/tmp/$NM/result"
echo " => $lines2 frames for group ff2e::42,  expected >= 2"

########################################################################### DONE
# shellcheck disable=SC2086
[ $lines2 -ge 2 ] && OK
FAIL
