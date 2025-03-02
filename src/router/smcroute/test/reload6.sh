#!/bin/sh
# Verifies SIGHUP/reload functionality
# XXX: add group verification as well
#set -x

check_output()
{
    ip -6 mroute |tee "/tmp/$NM/result" | grep -q "$2"
    oif=$?
    grep -q "$1" "/tmp/$NM/result"
    iif=$?
    # shellcheck disable=SC2166
    [ "$oif" = "0" -a "$iif" = "0" ] || FAIL
}

# shellcheck source=/dev/null
. "$(dirname "$0")/lib.sh"

print "Creating world ..."
topo plus
ip addr add 2001:1::1/64 dev a1
ip addr add 2001:2::1/64 dev a2
ip addr add 2001:3::1/64 dev a3
ip addr add 2001:4::1/64 dev a4
ip addr add 2001:5::1/64 dev b1
ip addr add 2001:6::1/64 dev b2
ip addr add 2001:7::1/64 dev b3
ip addr add 2001:8::1/64 dev b4
ip -br a

print "Creating config #1 ..."
cat <<EOF > "/tmp/$NM/conf"
phyint a1 enable ttl-threshold 1
phyint b1 enable ttl-threshold 1
phyint b3 enable ttl-threshold 3

mgroup from a1 source fc00::1 group ff2e::42
mroute from a1 source fc00::1 group ff2e::42 to b1 b3

mgroup from a1 group ff2e::43
mroute from a1 group ff2e::43 to b1
mroute from a1 group ff2e::44 to b1
EOF
cat "/tmp/$NM/conf"

print "Starting smcrouted ..."
../src/smcrouted -f "/tmp/$NM/conf" -N -n -P "/tmp/$NM/pid" -l debug -u "/tmp/$NM/sock" &
sleep 1

../src/smcroutectl -pu "/tmp/$NM/sock" show groups
ip -6 maddr
cat /proc/net/ip6_mr_vif
show_mroute

check_output "Iif: a1" "Oifs: b1 b3"

print "Creating config #2 ..."
cat <<EOF > "/tmp/$NM/conf"
phyint a1 enable ttl-threshold 1
phyint a3 enable ttl-threshold 3
phyint b2 enable ttl-threshold 2
phyint b3 enable ttl-threshold 3
phyint b4 enable ttl-threshold 4

mgroup from a1 source fc00::1 group ff2e::42
mroute from a1 source fc00::1 group ff2e::42 to b2 b3

mgroup from a3 group ff2e::44
mroute from a3 group ff2e::44 to b3 b4
EOF
cat "/tmp/$NM/conf"
../src/smcroutectl -u "/tmp/$NM/sock" reload
sleep 1

../src/smcroutectl -pu "/tmp/$NM/sock" show groups
ip -6 maddr
cat /proc/net/ip6_mr_vif
show_mroute

check_output "Iif: a1" "Oifs: b3 b2"

print "Updating route from smcroutectl ..."
../src/smcroutectl -u "/tmp/$NM/sock" add a1 fc00::1 ff2e::42 b4
show_mroute

check_output "Iif: a1" "Oifs: b3 b2 b4"
OK
