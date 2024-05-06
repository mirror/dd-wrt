#!/bin/sh

ip addr flush dev to_jool_v6 scope global
ip route del 2001:db8:1c6:3364::/40 via 2001:db8:1c0:2:1::