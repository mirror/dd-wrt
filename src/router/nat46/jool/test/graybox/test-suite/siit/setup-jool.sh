#!/bin/sh

modprobe -rq jool_siit
modprobe -rq jool
ip addr flush dev to_client_v6 scope global
ip addr flush dev to_client_v4 scope global
ip link set to_client_v6 up
ip link set to_client_v4 up

ip addr add 2001:db8:1c0:2:1::/64 dev to_client_v6
ip addr add 198.51.100.1/24 dev to_client_v4
ip route add 2001:db8:3::/120 via 2001:db8:1c0:2:21::

sysctl -w net.ipv4.conf.all.forwarding=1 > /dev/null
sysctl -w net.ipv6.conf.all.forwarding=1 > /dev/null

modprobe jool_siit
jool_siit instance add --netfilter -6 2001:db8:100::/40
jool_siit eamt add 2001:db8:3::/120 1.0.0.0/24
jool_siit eamt add 2001:db8:2::/120 10.0.0.0/24
jool_siit global update rfc6791v4-prefix 203.0.113.8

# Relevant whenever the kernel responds an ICMPv6 error on behalf of Jool.
sysctl -w net.ipv6.auto_flowlabels=0 > /dev/null
