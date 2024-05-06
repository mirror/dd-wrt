#!/bin/sh

ip addr flush dev to_jool_v4 scope global
ip link set to_jool_v4 up
ip addr add 198.51.100.2/24 dev to_jool_v4
ip route add 192.0.2.0/24 via 198.51.100.1