#!/bin/sh
# Verifies .conf include statement
#set -x

activate()
{
    if [ -f /tmp/$NM/pid ]; then
	../src/smcroutectl -u "/tmp/$NM/sock" reload
    else
	../src/smcrouted -c 8 -f "/tmp/$NM/conf" -n -N -P "/tmp/$NM/pid" -l debug -u "/tmp/$NM/sock" &
    fi
    sleep 1
}

get_routes()
{
    ip -6 mroute
    ip mroute
}

check_output()
{
    show_mroute
    get_routes | grep -q "$2"
    [ "$?" = "$1" ] || FAIL "$3"
}

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

######################################################### Plain conf, no include
print "Creating /tmp/$NM/conf ..."
cat <<EOF > "/tmp/$NM/conf"
phyint a1 enable
phyint a2 enable

mroute from a1 source 10.0.0.10 group 225.3.2.1 to a2
EOF
cat "/tmp/$NM/conf"

print "Starting smcrouted ..."
activate
check_output 0 "(10.0.0.10,225.3.2.1)" "Failed reading initial .conf file"

######################################################## Include files from conf
print "Creating /tmp/$NM/conf2 ..."
echo "include /tmp/$NM/conf2" >> "/tmp/$NM/conf"
cat <<EOF >"/tmp/$NM/conf2"
mroute from a1 source fc00::1 group ff04:0:0:0:0:0:0:114 to a2
include /tmp/$NM/*.foo
EOF
echo ">> /tmp/$NM/conf"
cat "/tmp/$NM/conf"
echo ">> /tmp/$NM/conf2"
cat "/tmp/$NM/conf2"

activate
check_output 0 "(fc00::1,ff04::114)" "Failed reading conf2"

######################################################### Creating a.foo & b.foo
print "Creating /tmp/$NM/{a,b}.foo ..."
cat <<EOF >"/tmp/$NM/a.foo"
mroute from a1 source 10.0.0.1 group 225.1.2.3 to a2
EOF
echo ">> /tmp/$NM/a.foo"
cat "/tmp/$NM/a.foo"

cat <<EOF >"/tmp/$NM/b.foo"
mroute from a1 source 10.0.0.2 group 225.1.2.1 to a2
EOF
echo ">> /tmp/$NM/b.foo"
cat "/tmp/$NM/b.foo"

activate
check_output 0 "(10.0.0.1,225.1.2.3)" "Failed reading a.foo"
check_output 0 "(10.0.0.2,225.1.2.1)" "Failed reading b.foo"

################################################################# Removing b.foo
print "Removing b.foo ..."
rm "/tmp/$NM/b.foo"
activate
check_output 1 "(10.0.0.2,225.1.2.1)" "Failed removing old route from b.foo"

########################################################################### DONE
OK
