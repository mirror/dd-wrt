#!/bin/sh
# Verify interop between multiple routers and 1:1 NAT.  Same subnet and
# source IP of multicast emitters.
#
#         netns: R1                         netns: R2
#        .-------------.                   .-------------.
#        |  smcrouted  |                   |  smcrouted  |
#        |    /   \    |       br0         |    /   \    |
#   MC --> eth1   eth0 |      /   \        | eth0   eth1 <-- MC
#        |            `------'     '-------'             |
#        '-------------'  192.168.0.0/24   '-------------'
#          10.0.0.0/24                       10.0.0.0/24
#
# Since we use tshark (because tcpdump doesn't work in an unshare), and
# it doesn't seem to be able to dump the Ethernet header, we set the TTL
# of the two ping's to different values to be able to separate the two
# when inspecting the pcap from br0.
#
# Note: modern versions of iptables have introduced the xtables.lock file
#       the default path for it can be overridden from v1.8.7 and later,
#       a workaround is to `chmod a+rw /var/run/xtables.lock`
#set -x

# shellcheck source=/dev/null
. "$(dirname "$0")/lib.sh"

print "Checking dependencies ..."
lsb_release -a
uname -a
XTABLES_LOCK=$(mktemp -p "/tmp/$NM")
export XTABLES_LOCK
check_dep unshare -mrun iptables -L 2>/dev/null

print "Creating world ..."
topo multi R1 R2

# IP world ...
nsenter --net=R1 -- ip addr add 192.168.0.10/24 dev eth0
nsenter --net=R1 -- ip addr add 10.0.0.1/24     dev eth1
nsenter --net=R1 -- ip route add 192.168.20.0/24 via 192.168.0.20
nsenter --net=R1 -- iptables -t nat -A PREROUTING  -d 192.168.10.0/24 -j NETMAP --to 10.0.0.0/24
nsenter --net=R1 -- iptables -t nat -A POSTROUTING -s 10.0.0.0/24     -j NETMAP --to 192.168.10.0/24
nsenter --net=R1 -- ip -br l
nsenter --net=R1 -- ip -br a

nsenter --net=R2 -- ip addr add 192.168.0.20/24 dev eth0
nsenter --net=R2 -- ip addr add 10.0.0.1/24     dev eth1
nsenter --net=R2 -- ip route add 192.168.10.0/24 via 192.168.0.10
nsenter --net=R2 -- iptables -t nat -A PREROUTING  -d 192.168.20.0/24 -j NETMAP --to 10.0.0.0/24
nsenter --net=R2 -- iptables -t nat -A POSTROUTING -s 10.0.0.0/24     -j NETMAP --to 192.168.20.0/24
nsenter --net=R2 -- ip -br l
nsenter --net=R2 -- ip -br a

print "Verifying connectivity ..."
printf "R1 (192.168.0.10) "
if ! nsenter --net=R1 -- ping -c 3 192.168.20.1; then
    FAIL "R1: cannot reach ED2 via R2"
fi

print "Creating config ..."
cat <<EOF >"/tmp/$NM/shared.conf"
# shared.conf for both netns
phyint eth0 enable
phyint eth1 enable

mgroup from eth1 group 225.1.2.3
mroute from eth1 group 225.1.2.3 to eth0

# Disable for now, maybe use in another test.  With this
# disabled we get WRONGVIF messages from the kernel that
# can extend this test to verify.
#mgroup from eth0 group 225.1.2.3
#mroute from eth0 group 225.1.2.3 to eth1
EOF
cat "/tmp/$NM/shared.conf"

print "Starting smcrouted instances ..."
nsenter --net=R1 -- ../src/smcrouted -f "/tmp/$NM/shared.conf" -n -N -i R1 -l debug -u "/tmp/$NM/R1.sock" &
echo $! >> "/tmp/$NM/PIDs"
nsenter --net=R2 -- ../src/smcrouted -f "/tmp/$NM/shared.conf" -n -N -i R2 -l debug -u "/tmp/$NM/R2.sock" &
echo $! >> "/tmp/$NM/PIDs"
sleep 1

collect br0 -c10 'dst 225.1.2.3'

print "Starting emitters ..."
nsenter --net=R1 -- ping -c 5 -W 1 -I eth1 -t 11 225.1.2.3 > /dev/null &
sleep 1
nsenter --net=R2 -- ping -c 5 -W 1 -I eth1 -t 21 225.1.2.3 > /dev/null &
sleep 5

print "R1 multicast routes and 1:1 NAT ..."
nsenter --net=R1 -- ip mroute
nsenter --net=R1 -- ../src/smcroutectl -d -u  "/tmp/$NM/R1.sock"
nsenter --net=R1 -- iptables -v -L -t nat

print "R2 multicast routes and 1:1 NAT ..."
nsenter --net=R2 -- ip mroute
nsenter --net=R2 -- ../src/smcroutectl -d -u  "/tmp/$NM/R2.sock"
nsenter --net=R2 -- iptables -v -L -t nat

print "Analyzing br0 pcap, data from R1 and R2 ..."
lines1=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.1.2.3 | grep 'ttl=10' | tee    "/tmp/$NM/result" | wc -l)
lines2=$(tshark -r "/tmp/$NM/pcap" 2>/dev/null | grep 225.1.2.3 | grep 'ttl=20' | tee -a "/tmp/$NM/result" | wc -l)
cat "/tmp/$NM/result"

echo " => $lines1 for group 225.1.2.3 from R1, expected >= 4"
echo " => $lines2 for group 225.1.2.3 from R2, expected >= 4"

# Expect one frame loss due to initial (*,G) -> (S,G) route setup
# shellcheck disable=SC2086 disable=SC2166
[ $lines1 -ge 4 -a $lines2 -ge 4 ] && OK
FAIL
