#!/bin/sh
# Limited verifier of the smcrouted .conf file checker

# shellcheck source=/dev/null
. "$(dirname "$0")/lib.sh"

print "Creating world ..."
topo basic
ip addr add 10.0.0.1/24 dev a1
ip addr add 20.0.0.1/24 dev a2
ip -br l

print "Creating config ..."
cat <<EOF > "/tmp/$NM/conf"
# basic (*,G) multicast routing
phyint a1 enable
phyint a3 enable
phyint

mgroup from a1 source 10.0.0.1 group 225.3.2.1
mroute from a1 source 10.0.0.1 group 225.3.2.1 to a2 a3

mgroup from a1 group 225.1.2.3
mroute from a1 group 225.1.2.3 to a2

mroute from a3 group 225.1.2.3 to a1

mgroup from b4 group 225.1.2.3
EOF
cat "/tmp/$NM/conf"

print "Verifying config ..."
../src/smcrouted -N -F "/tmp/$NM/conf" -l info
rc=$?
echo "Return code: $rc"

[ $rc -eq 78 ] && OK
FAIL
