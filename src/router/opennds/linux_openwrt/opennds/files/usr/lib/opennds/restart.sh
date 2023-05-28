#!/bin/sh

# Check if opennds is running
ndspid=$(pidof opennds)
fwhook=$(uci -q get opennds.@opennds[0].fwhook_enabled)

if [ ! -z $ndspid ]; then
	if [ "$fwhook" != "0" ]; then
		echo "fwhook received a signal that the system firewall is restarting" | logger -p "daemon.notice" -s -t "opennds[$ndspid]"

		# Set "users_to_router allow"
		/usr/lib/opennds/libopennds.sh users_to_router allow
	fi
fi
