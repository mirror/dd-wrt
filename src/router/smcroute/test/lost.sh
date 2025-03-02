#!/bin/sh
# Verify handling when IIF for an (S,G) is changed or lost

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
topo basic
ip addr add 10.0.0.1/24 dev a1
ip addr add 20.0.0.1/24 dev a2
ip -br a

print "Creating config #1 ..."
cat <<EOF > "/tmp/$NM/conf"
phyint a1 enable
phyint a2 enable

mgroup from a1 group 225.1.2.3
mroute from a1 group 225.1.2.3 to a2

mgroup from a2 group 225.1.2.4
mroute from a2 source 1.2.3.4 group 225.1.2.4 to a1

mgroup from a1 group 225.2.2.5
mroute from a1 group 225.2.2.5 to a1

mgroup from a2 group 225.1.2.6
mroute from a2 source 1.2.3.5 group 225.1.2.6 to a1
EOF
cat "/tmp/$NM/conf"

print "Starting smcrouted ..."
../src/smcrouted -f "/tmp/$NM/conf" -N -n -P "/tmp/$NM/pid" -l debug -u "/tmp/$NM/sock" &
sleep 1

cat /proc/net/ip_mr_vif
cat /proc/net/ip_mr_cache
../src/smcroutectl -pu "/tmp/$NM/sock" show groups
show_mroute

check_output "(1.2.3.4,225.1.2.4)              Iif: a2         Oifs: a1"

print "Creating config #2 ..."
cat <<EOF > "/tmp/$NM/conf"
phyint a1 enable
phyint a2 enable

mgroup from a1 group 225.1.2.3
mroute from a1 group 225.1.2.3 to a2

mgroup from a1 group 225.1.2.4
mroute from a1 source 1.2.3.4 group 225.1.2.4 to a2

mgroup from a2 group 225.2.2.5
mroute from a2 group 225.2.2.5 to a1

mgroup from a2 group 225.1.2.6
mroute from a2 source 1.2.3.5 group 225.1.2.6 to a1
EOF
cat "/tmp/$NM/conf"
../src/smcroutectl -u "/tmp/$NM/sock" reload
sleep 1

check_output "(1.2.3.4,225.1.2.4)              Iif: a1         Oifs: a2"

print "Deleting and restoring interface a1 => new ifindex ..."
ip link del a1
ip link add a1 type dummy
ip link set a1 up
ip link set a1 multicast on
ip addr add 10.0.0.1/24 dev a1

../src/smcroutectl -u "/tmp/$NM/sock" reload
sleep 1

check_output "(1.2.3.4,225.1.2.4)              Iif: a1         Oifs: a2"

print "Deleting interface a1 ..."
ip link del a1
../src/smcroutectl -u "/tmp/$NM/sock" reload
sleep 1
check_output "(1.2.3.5,225.1.2.6)              Iif: a2          State: resolved"

OK
