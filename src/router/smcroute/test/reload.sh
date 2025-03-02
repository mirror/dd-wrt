#!/bin/sh
# Verifies SIGHUP/reload functionality
# XXX: add group verification as well
#set -x

check_output()
{
    echo "ip mroute:"
    ip mroute
    ip mroute |tee "/tmp/$NM/result" | grep -q "$2"
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
ip addr add 10.0.0.1/24 dev a1
ip addr add 20.0.0.1/24 dev a2
ip addr add 30.0.0.1/24 dev a3
ip addr add 40.0.0.1/24 dev a4
ip addr add 50.0.0.1/24 dev b1
ip addr add 60.0.0.1/24 dev b2
ip addr add 70.0.0.1/24 dev b3
ip addr add 80.0.0.1/24 dev b4
ip -br a

print "Creating config #1 ..."
cat <<EOF > "/tmp/$NM/conf"
phyint a1 enable
phyint b1 enable
phyint b3 enable

mgroup from a1 source 10.0.0.1 group 225.3.2.1
mroute from a1 source 10.0.0.1 group 225.3.2.1 to b1 b3

mgroup from a1 group 225.1.2.3
mroute from a1 group 225.1.2.3 to b1
mroute from a1 group 225.1.2.4 to b1
EOF
cat "/tmp/$NM/conf"

print "Starting smcrouted ..."
../src/smcrouted -f "/tmp/$NM/conf" -N -n -P "/tmp/$NM/pid" -l debug -u "/tmp/$NM/sock" &
sleep 1

cat /proc/net/ip_mr_vif
cat /proc/net/ip_mr_cache
../src/smcroutectl -pu "/tmp/$NM/sock" show groups
show_mroute

check_output "Iif: a1" "Oifs: b1 b3"

print "Creating config #2 ..."
cat <<EOF > "/tmp/$NM/conf"
phyint a1 enable ttl-threshold 1
phyint a3 enable ttl-threshold 3
phyint b2 enable ttl-threshold 2
phyint b3 enable ttl-threshold 3
phyint b4 enable ttl-threshold 4

mgroup from a1 source 10.0.0.1 group 225.3.2.1
mroute from a1 source 10.0.0.1 group 225.3.2.1 to b2 b3

mgroup from a3 group 225.1.2.4
mroute from a3 group 225.1.2.4 to b3 b4
EOF
cat "/tmp/$NM/conf"
../src/smcroutectl -u "/tmp/$NM/sock" reload
sleep 1

cat /proc/net/ip_mr_vif
cat /proc/net/ip_mr_cache
../src/smcroutectl -pu "/tmp/$NM/sock" show groups
show_mroute

check_output "Iif: a1" "Oifs: b3(ttl 3) b2(ttl 2)"

print "Updating route from smcroutectl ..."
../src/smcroutectl -u "/tmp/$NM/sock" add a1 10.0.0.1 225.3.2.1 b4
show_mroute

check_output "Iif: a1" "Oifs: b3(ttl 3) b2(ttl 2) b4(ttl 4)"
OK
