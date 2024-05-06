#!/bin/sh

ip route del 64:ff9b::/96 via 2001:db8::1
ip addr flush dev to_jool_v6 scope global
