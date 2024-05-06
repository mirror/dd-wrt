#!/bin/sh

ip addr flush dev to_jool_v6 scope global
ip link set to_jool_v6 up
ip addr add 2001:db8::5/96 dev to_jool_v6
ip -6 route add 64:ff9b::/96 via 2001:db8::1
