#!/bin/sh
# Verify multicast routing between two routers over a GRE tunnel
#
#         netns: host1                      netns: host2
#        .-------------.                   .-------------.
#        |  smcrouted  |                   |  smcrouted  |
#        |    /   \    |       br0         |    /   \    |
#   MC --> eth1   eth0 |      /   \        | eth0   eth1 <-- MC
#        |            `------'     '-------'             |
#        '-------------'  192.168.0.0/24   '-------------'
#          10.0.0.0/24                       10.0.0.0/24
#
# Note: you may have to `modprobe ip_gre` before the test.

# shellcheck source=/dev/null
. "$(dirname "$0")/lib.sh"

print "Checking dependencies ..."
lsb_release -a
uname -a
check_dep grep -q ip_gre /proc/modules

print "Creating world ..."
topo multi host1 host2

# IP world ...
echo "Links, addresses, and routes for host1 ====================================="
nsenter --net=host1 -- ip addr add 192.168.0.10/24 dev eth0
nsenter --net=host1 -- ip addr add 10.0.0.1/24     dev eth1
nsenter --net=host1 -- ip tunnel add tun0 mode gre remote 192.168.0.20 local 192.168.0.10 ttl 255
nsenter --net=host1 -- ip addr add 172.16.0.10/24  dev tun0
nsenter --net=host1 -- ip link set tun0 multicast on
nsenter --net=host1 -- ip link set tun0 up
nsenter --net=host1 -- ip route add 20.0.0.0/24 via 172.16.0.10
nsenter --net=host1 -- ip -br l
nsenter --net=host1 -- ip -br a
nsenter --net=host1 -- ip -br r

echo "Links, addresses, and routes for host2 ====================================="
nsenter --net=host2 -- ip addr add 192.168.0.20/24 dev eth0
nsenter --net=host2 -- ip addr add 20.0.0.1/24     dev eth1
nsenter --net=host2 -- ip tunnel add tun0 mode gre remote 192.168.0.10 local 192.168.0.20 ttl 255
nsenter --net=host2 -- ip addr add 172.16.0.20/24  dev tun0
nsenter --net=host2 -- ip link set tun0 multicast on
nsenter --net=host2 -- ip link set tun0 up
nsenter --net=host2 -- ip route add 10.0.0.0/24 via 172.16.0.20
nsenter --net=host2 -- ip -br l
nsenter --net=host2 -- ip -br a
nsenter --net=host2 -- ip -br r

print "Verifying connectivity ..."
printf "host1 (172.16.0.10) "
if ! nsenter --net=host1 -- ping -c 3 172.16.0.20; then
    FAIL "host1: cannot reach host2 over GRE tunnel"
fi

print "Creating config ..."
cat <<EOF >"/tmp/$NM/shared.conf"
# shared.conf for both netns
phyint tun0 enable
phyint eth1 enable

mgroup from eth1 group 225.1.2.3
mroute from eth1 group 225.1.2.3 to tun0

mgroup from tun0 group 225.1.2.3
mroute from tun0 group 225.1.2.3 to eth1
EOF
cat "/tmp/$NM/shared.conf"

print "Starting smcrouted instances ..."
nsenter --net=host1 -- ../src/smcrouted -f "/tmp/$NM/shared.conf" -n -N -i host1 -l debug -u "/tmp/$NM/host1.sock" &
echo $! >> "/tmp/$NM/PIDs"
nsenter --net=host2 -- ../src/smcrouted -f "/tmp/$NM/shared.conf" -n -N -i host2 -l debug -u "/tmp/$NM/host2.sock" &
echo $! >> "/tmp/$NM/PIDs"
sleep 1

print "Starting collector on eth1@host2 ..."
nsenter --net=host2 -- tshark -w "/tmp/$NM/pcap" -lni eth1 -c5 'dst 225.1.2.3' 2>/dev/null &
echo $! >> "/tmp/$NM/PIDs"

print "Starting emitters ..."
nsenter --net=host1 -- ping -c 5 -W 1 -I eth1 -t 10 225.1.2.3 > /dev/null &
sleep 5

print "Analyzing pcap from eth1@host2 ..."
lines1=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.1.2.3 | tee "/tmp/$NM/result" | wc -l)
cat "/tmp/$NM/result"

echo " => $lines1 for group 225.1.2.3 from host1, expected >= 4"

# Expect one frame loss for each initial (*,G) -> (S,G) route setup
# shellcheck disable=SC2086
[ $lines1 -ge 3 ] && OK
FAIL
