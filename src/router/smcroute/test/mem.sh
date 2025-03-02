#!/bin/sh
# Check for memory leaks with valgrind, should cover the basic use-cases
# with a .conf file, reloading said .conf file, and IPC.

# shellcheck source=/dev/null
. "$(dirname "$0")/lib.sh"

print "Checking dependencies ..."
lsb_release -a
uname -a
check_dep valgrind

print "Creating world ..."
topo plus
ip addr add 10.0.0.1/24 dev a1
ip addr add 20.0.0.1/24 dev a2
ip addr add 20.0.0.1/24 dev a3
ip -br l

print "Creating config ..."
cat <<EOF > "/tmp/$NM/conf"
# basic (*,G) multicast routing
phyint a1   enable
phyint a3   enable
phyint foo0 enable
phyint

mgroup from a1 source 10.0.0.1 group 225.3.2.1
mroute from a1 source 10.0.0.1 group 225.3.2.1 to a2 foo0

mgroup from a1 group 225.1.2.3
mroute from a1 group 225.1.2.3 to a2

mroute from a3 group 225.1.2.3 to a1

mgroup from b4 group 225.1.2.3
EOF
cat "/tmp/$NM/conf"

print "Creating background ops ..."
cat <<EOF > "/tmp/$NM/ops.sh"
#!/bin/sh

sleep 2
../src/smcroutectl -dp -u "/tmp/$NM/sock" reload
sleep 1
../src/smcroutectl -dp -u "/tmp/$NM/sock" add a1 225.1.2.4 a3
../src/smcroutectl -dp -u "/tmp/$NM/sock" add a1 10.0.0.1 225.1.2.5 a3
echo
../src/smcroutectl -dp -u "/tmp/$NM/sock" show int
echo
../src/smcroutectl -dp -u "/tmp/$NM/sock" show gr
echo
../src/smcroutectl -dp -u "/tmp/$NM/sock" show ro
echo
EOF
cat "/tmp/$NM/ops.sh"
chmod +x "/tmp/$NM/ops.sh"
"/tmp/$NM/ops.sh" &

print "Checking for memory leaks ..."
valgrind -s --leak-check=full --show-leak-kinds=all --log-file="/tmp/$NM/valgrind.log" ../src/smcrouted -f "/tmp/$NM/conf" -n -N -l debug -P "/tmp/$NM/pid" -u "/tmp/$NM/sock" -D 5
rc=$?
cat "/tmp/$NM/valgrind.log" >> "/tmp/$NM/log"

wait
[ $rc -eq 0 ] || FAIL "Failed starting valgrind, return code $rc"
if ! grep -q "no leaks are possible" "/tmp/$NM/log"; then
    cat "/tmp/$NM/valgrind.log"
    FAIL "Leaks detected."
fi
OK
