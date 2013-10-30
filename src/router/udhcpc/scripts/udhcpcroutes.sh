#!/bin/sh
[ -n "$1" ] && routefile=$1 || routefile="/tmp/udhcpstaticroutes"

rm -rf ${routefile}
set_classless_routes() {
local max=128
	local type
	while [ -n "$1" -a -n "$2" -a $max -gt 0 ]; do
		if [ ${1##*/} -eq 32 ] 
		then 
			type=host
			target=${1%/32}
		else
			type=net
			target=$1
		fi
		
		echo "udhcpc: adding route for $type $target via $2 on interface $interface " | logger
		echo route add -${type} "$target" gw "$2" dev "$interface" >>${routefile}
		max=$(($max-1))
		shift 2
	done
}

set_dhcp_routes() {
	local max=128
	local host
	local gw
	while [ -n "$1" -a $max -gt 0 ]; do
		host=${1%%/*}
		gw=${1##*/}
		echo "udhcpc: adding route for $host via $gw on interface $interface" | logger
		echo route add -host "$host" gw "$gw" dev "$interface" >>${routefile}
		max=$(($max-1))
		shift 1
	done
}


[ -n "$staticroutes" ] && set_classless_routes $staticroutes
[ -n "$msstaticroutes" ] && set_classless_routes $msstaticroutes
[ -n "$routes" ] && set_dhcp_routes $routes 
