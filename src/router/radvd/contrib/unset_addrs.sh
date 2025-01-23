#!/bin/bash

set -u

if (( $# < 1 )) ; then
	echo "usage: $0 iface" 1>&2
	exit -1
fi

iface=$1

ADDRS=$(ifconfig $iface | grep "inet6 addr:" | grep Scope:Global | awk '{print $3}')

for a in $ADDRS ; do
	ip addr del $a dev $iface
done

