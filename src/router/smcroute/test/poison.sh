#!/bin/sh
# Verify stop-filter, or "poison pill", routes for non-matching routes,
# and that they do not affect flows from matching routes.

check_output()
{
    if ! ip $1 mroute | grep -q "$2"; then
	FAIL "Did not even register $2"
    fi

    if ip $1 mroute | grep -q "$2" | grep -q "$3"; then
	FAIL "No stop-filter route set for $2"
    fi
}

# shellcheck source=/dev/null
. "$(dirname "$0")/lib.sh"

print "Creating world ..."
topo plus
ip -d l
ip addr add 10.0.0.1/24  dev a1
ip addr add 20.0.0.1/24  dev a2
ip addr add 30.0.0.1/24  dev b3

ip addr add 2001:1::1/64 dev a1
ip addr add  fc00::42/64 dev a1
ip addr add 2001:2::1/64 dev a2
ip addr add 2001:3::1/64 dev b3
ip -br a

print "Creating config ..."
cat <<EOF > "/tmp/$NM/conf"
# basic (*,G/LEN) multicast routing
phyint a1 enable
phyint a2 enable
phyint b3 enable

mroute from a1 group 225.1.2.3/24 to b3
mroute from a2 group 225.1.2.3/24 to b3
mroute from a1 group ff2e::42/121 to b3
EOF
cat "/tmp/$NM/conf"

print "Starting smcrouted ..."
../src/smcrouted -f "/tmp/$NM/conf" -n -N -P "/tmp/$NM/pid" -l debug -u "/tmp/$NM/sock" &
sleep 1

collect b3 -c15 'dst 225.1.2.1 or dst ff2e::42'
ip -d l
ip -br a

print "Starting emitter ..."
ping    -c 3 -W 1 -I b3       -t 2 225.1.2.1   >/dev/null
ping    -c 3 -W 1 -I a2       -t 2 225.1.2.1   >/dev/null
ping    -c 3 -W 1 -I a1       -t 2 225.1.2.1   >/dev/null
ping -6 -c 3 -W 1 -I b3       -t 2 ff2e::42    >/dev/null
ping -6 -c 3 -W 1 -I fc00::42 -t 2 ff2e::42    >/dev/null

show_mroute

print "Analyzing ..."
check_output -4 "(30.0.0.1,225.1.2.1)" "Oifs: b3"
check_output -6 "(2001:3::1,ff2e::42)" "Oifs: b3"

lines1=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.1.2.1 | tee    "/tmp/$NM/result" | wc -l)
lines2=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep ff2e::42  | tee -a "/tmp/$NM/result" | wc -l)

cat "/tmp/$NM/result"
echo " => $lines1 for 225.1.2.1,  expected >= 7"
echo " => $lines2 for ff2e::42,   expected >= 5"

########################################################################### DONE
# Expect one frame lost due to initial (*,G) -> (S,G) route setup
# shellcheck disable=SC2166 disable=SC2086
[ $lines1 -ge 7 -a $lines2 -ge 5 ] && OK
FAIL
