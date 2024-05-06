#!/bin/sh

modprobe -rq jool_siit

ip addr flush dev to_client_v6 scope global
ip addr flush dev to_client_v4 scope global
ip route del 2001:db8:3::/120 via 2001:db8:1c0:2:21::
