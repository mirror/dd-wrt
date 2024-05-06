#!/bin/sh

modprobe -rq jool

ip route del 2001:db8:1::/96 via 2001:db8::5
ip addr flush dev to_client_v6 scope global
ip addr flush dev to_client_v4 scope global
