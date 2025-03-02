#!/bin/sh

# Test name, used everywhere as /tmp/$NM/foo
NM=$(basename "$0" .sh)

# Print heading for test phases
print()
{
    printf "\e[7m>> %-80s\e[0m\n" "$1"
}

SKIP()
{
    print "TEST: SKIP"
    [ $# -gt 0 ] && echo "$*"
    exit 77
}

FAIL()
{
    print "TEST: FAIL"
    [ $# -gt 0 ] && echo "$*"
    exit 99
}

CHECK()
{
    [ "$@" ] || FAIL "$*"
}

OK()
{
    print "TEST: OK"
    [ $# -gt 0 ] && echo "$*"
    exit 0
}

check_dep()
{
    if [ -n "$2" ]; then
	if ! $@; then
	    SKIP "$* is not supported on this system."
	fi
    elif ! command -v "$1"; then
	SKIP "Cannot find $1, skipping test."
    fi
}

show_mroute()
{
    # Show active routes (and counters)
    cat /proc/net/ip_mr_cache
    cat /proc/net/ip6_mr_cache
    echo "-----------------------------------------------------------------------------------"
    ip mroute
    ip -6 mroute
    echo "-----------------------------------------------------------------------------------"
    ../src/smcroutectl -pd -u "/tmp/$NM/sock"
}

emitter()
{
    print "Starting emitter(s) on $1 ..."
    for i in $(seq 1 $2); do
	ping -I $1 -W 1 -t 3 225.1.2.$i >/dev/null &
	echo $! >> "/tmp/$NM/PIDs"
    done
}

collect()
{
    print "Starting collector on $1 ..."
    tshark -w "/tmp/$NM/pcap" -lni "$@" 2>/dev/null &
    echo $! >> "/tmp/$NM/PIDs"
    sleep 2
}

# Set up a basic bridge topology, two VETH pairs with one end in the
# bridge and the other free.  Each pair is also in a separate VLAN.
#
# No IP address assignment is done in topo files, only topology setup.
#
# Topology:          ¦
#             vlan1  ¦  vlan2
#                  \ ¦ /
#       a1 -------- br0 --------- a2
#                    ¦
#       VLAN 1       ¦        VLAN 2
#
# Note: in addition to VLAN filtering, the bridge has both IGMP and MLD
#       snooping disabled, because the main purpose of these tests is to
#       verify the IPv4 and IPv6 routing functionality of SMCRoute.
#       Future tests may include verifying join/leave of groups (TODO)
topo_bridge()
{
    cat << EOF > "$NM-topo.ip"
link add br0 type bridge vlan_filtering 1 mcast_snooping 0
link add a1 type veth peer b1
link add a2 type veth peer b2
link set b1 master br0
link set b2 master br0

link set a1 up
link set b1 up
link set a2 up
link set b2 up
link set br0 up

link add link br0 vlan1 type vlan id 1
link add link br0 vlan2 type vlan id 2

link set vlan1 up
link set vlan2 up
EOF

    # Move b2 to VLAN 2
    # Set br0 as tagged member of both VLANs
    cat <<EOF > "$NM-bridge.ip"
vlan add vid 2 dev b2 pvid untagged
vlan del vid 1 dev b2

vlan add vid 1 dev br0 self
vlan add vid 2 dev br0 self
EOF

    ip     -force -batch "$NM-topo.ip"
    bridge -force -batch "$NM-bridge.ip"

    rm -f "$NM-topo.ip" "$NM-bridge.ip"
}


# Set up a basic dummy interface topology,
#
# No IP address assignment is done in topo files, only topology setup.
topo_basic()
{
    cat << EOF > "$NM-topo.ip"
link add a1 type dummy
link set a1 up
link set a1 multicast on

link add a2 type dummy
link set a2 up
link set a2 multicast on
EOF

    ip -force -batch "$NM-topo.ip"
    rm -f "$NM-topo.ip"

    return 2
}

# Same as basic topology, but with more inbound/outbound interfaces.
#
# No IP address assignment is done in topo files, only topology setup.
topo_plus()
{
    cat << EOF > "$NM-topo.ip"
link add a1 type dummy
link set a1 up
link set a1 multicast on

link add a2 type dummy
link set a2 up
link set a2 multicast on

link add a3 type dummy
link set a3 up
link set a3 multicast on

link add a4 type dummy
link set a4 up
link set a4 multicast on

link add b1 type dummy
link set b1 up
link set b1 multicast on

link add b2 type dummy
link set b2 up
link set b2 multicast on

link add b3 type dummy
link set b3 up
link set b3 multicast on

link add b4 type dummy
link set b4 up
link set b4 multicast on
EOF

    ip -force -batch "$NM-topo.ip"
    rm -f "$NM-topo.ip"
}

# Set up VLAN interfaces on top of dummy interfaces
# shellcheck disable=SC2048
topo_basic_vlan()
{
    num=$1
    shift

    i=1
    while [ $i -le "$num" ]; do
	iface=a$i
	i=$((i + 1))

	for vid in $*; do
	    ip link add "$iface.$vid" link $iface type vlan id "$vid"
	    ip link set "$iface.$vid" up
	    ip link set "$iface.$vid" multicast on
	done
    done
}

topo_isolated()
{
    left="$1"
    right="$2"
    lif=$(basename "$left")
    rif=$(basename "$right")

    touch "$left" "$right"
    PID=$$

    echo "$left"   > "/tmp/$NM/mounts"
    echo "$right" >> "/tmp/$NM/mounts"

    unshare --net="$left" -- ip link set lo up
    nsenter --net="$left" -- ip link add eth0 type veth peer "$lif"
    nsenter --net="$left" -- ip link set "$lif" netns $PID
    nsenter --net="$left" -- ip link set eth0 up
    ip link set "$lif" up

    unshare --net="$right" -- ip link set lo up
    nsenter --net="$right" -- ip link add eth0 type veth peer "$rif"
    nsenter --net="$right" -- ip link set "$rif" netns $PID
    nsenter --net="$right" -- ip link set eth0 up
    ip link set "$rif" up
}

# Same as bridge topology, but with the VETH endpoints constructed
# by the isolated topology.  We just rename the main namespace's
# bridge ports to match.
topo_isolated_bridge()
{
    left="$1"
    right="$2"
    lif=$(basename "$left")
    rif=$(basename "$right")

    topo_isolated "$@"

    echo "Creating br0, adding $lif and $rif as bridge ports"
    ip link add br0 type bridge vlan_filtering 1 mcast_snooping 0
    ip link set "$lif" master br0
    ip link set "$rif" master br0

    ip link set br0 up

    ip link add link br0 vlan1 type vlan id 1
    ip link add link br0 vlan2 type vlan id 2

    ip link set vlan1 up
    ip link set vlan2 up

    bridge vlan add vid 2 dev "$rif" pvid untagged
    bridge vlan del vid 1 dev "$rif"

    bridge vlan add vid 1 dev br0 self
    bridge vlan add vid 2 dev br0 self
}

# Variant of a variant ... this is the Multi Domain topology.  It has
# two isolated network namespaces and a shared segment connecting them.
# The intention is to emulate network setups where the same base subnet
# is reused in many places.  So that when interconnecting them a string
# of 1:1 NAT operations need to performed.  These type of setups are
# more common than one might think; factories, train cars, ... tanks.
#
# Note, the bridge is only used to connect the ends of the VETH pairs.
topo_multi()
{
    left="$1"
    right="$2"
    lif=$(basename "$left")
    rif=$(basename "$right")

    topo_isolated "$@"

    nsenter --net="$left" -- ip link add eth1 type dummy
    nsenter --net="$left" -- ip link set eth1 up
    nsenter --net="$left" -- ip link set eth1 multicast on

    nsenter --net="$right" -- ip link add eth1 type dummy
    nsenter --net="$right" -- ip link set eth1 up
    nsenter --net="$right" -- ip link set eth1 multicast on

    echo "Creating br0, adding $lif and $rif as bridge ports"
    ip link add br0 type bridge
    ip link set "$lif" master br0
    ip link set "$rif" master br0

    ip link set br0 up
}

topo_teardown()
{
    if [ -z "$NM" ]; then
	echo "NM variable unset, skippnig teardown"
	exit 1
    fi

    if [ -f "/tmp/$NM/pid" ]; then
	PID=$(cat "/tmp/$NM/pid")
	kill -9 "$PID"
    fi

    # shellcheck disable=SC2162
    if [ -f "/tmp/$NM/mounts" ]; then
        while read ln; do umount "$ln"; rm "$ln"; done < "/tmp/$NM/mounts"
    fi

    # shellcheck disable=SC2162
    if [ -f "/tmp/$NM/PIDs" ]; then
	while read ln; do kill "$ln" 2>/dev/null; done < "/tmp/$NM/PIDs"
    fi

    ip link del br0  2>/dev/null
    ip link del a1   2>/dev/null
    ip link del a2   2>/dev/null
    ip link del b1   2>/dev/null
    ip link del b2   2>/dev/null

    rm -rf "/tmp/$NM"
}

signal()
{
    echo
    if [ "$1" != "EXIT" ]; then
	print "Got signal, cleaning up"
    fi
    topo_teardown
}

# props to https://stackoverflow.com/a/2183063/1708249
trapit()
{
    func="$1" ; shift
    for sig ; do
        trap "$func $sig" "$sig"
    done
}

topo()
{
    if [ $# -lt 1 ]; then
	print "Too few arguments to topo()"
	exit 1
    fi
    t=$1
    shift

    case "$t" in
	bridge)
	    topo_bridge
	    ;;

	basic)
	    topo_basic
	    num=$?
	    case "$1" in
		vlan)
		    shift
		    topo_basic_vlan $num "$@"
		    ;;
	    esac
	    ;;

	isolated)
	    case "$1" in
		bridge)
		    shift
		    topo_isolated_bridge "$@"
		    ;;
		*)
	    	    topo_isolated "$@"
		    ;;
	    esac
	    ;;

	multi)
	    topo_multi "$@"
	    ;;

	plus)
	    topo_plus
	    ;;

	teardown)
	    topo_teardown
	    ;;
	*)
	    print "No such topology: $t"
	    exit 1
	    ;;
    esac
}

# Runs once when including lib.sh
mkdir "/tmp/$NM"
touch "/tmp/$NM/PIDs"
trapit signal INT TERM QUIT EXIT
