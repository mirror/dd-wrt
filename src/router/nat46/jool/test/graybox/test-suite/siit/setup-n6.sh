#!/bin/sh

ip addr flush dev to_jool_v6 scope global
ip link set to_jool_v6 up
ip addr add 2001:db8:1c0:2:21::/64 dev to_jool_v6
ip route add 2001:db8:1c6:3364::/40 via 2001:db8:1c0:2:1::