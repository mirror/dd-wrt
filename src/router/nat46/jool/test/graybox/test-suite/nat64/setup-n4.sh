#!/bin/sh

ip addr flush dev to_jool_v4 scope global
ip link set to_jool_v4 up
ip addr add 192.0.2.5/24 dev to_jool_v4
sysctl -w net.ipv4.conf.all.forwarding=1 > /dev/null
