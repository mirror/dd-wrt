#!/bin/sh
#Copyright (C) BlueWave Projects and Services 2015-2023
#This software is released under the GNU GPL license.

# functions:

# Get configured option
get_option_from_config() {
	option_value=""

	if [ -e "/etc/config/opennds" ]; then
		option_value=$(uci -q get opennds.@opennds[0].$option | awk '{printf("%s", $0)}')

	elif [ -e "/etc/opennds/opennds.conf" ]; then
		option_value=$(cat "/etc/opennds/opennds.conf" | awk -F"$option " '{printf("%s", $2)}')
	fi
}

# Function to send commands to openNDS:
do_ndsctl () {
	local timeout=4

	for tic in $(seq $timeout); do
		ndsstatus="ready"
		ndsctlout=$(eval ndsctl "$ndsctlcmd")

		for keyword in $ndsctlout; do

			if [ $keyword = "locked" ]; then
				ndsstatus="busy"
				sleep 1
				break
			fi

			if [ $keyword = "Failed" ]; then
				ndsstatus="failed"
				break
			fi

			if [ $keyword = "authenticated." ]; then
				ndsstatus="authenticated"
				break
			fi

		done

		if [ "$ndsstatus" = "authenticated" ]; then
			break
		fi

		if [ "$ndsstatus" = "failed" ]; then
			break
		fi

		if [ "$ndsstatus" = "ready" ]; then
			break
		fi
	done
}

#### end of functions ####

#########################################
#					#
#  Start - Main entry point		#
#					#
#  This script starts executing here	#
#					#
#					#
#########################################

# Get tmpfs mountpoint
mountpoint=$(/usr/lib/opennds/libopennds.sh tmpfs)

# Clean up any previous ndsctl debuglevel setting
if [ -e "$mountpoint/ndsdebuglevel" ]; then
	rm $mountpoint/ndsdebuglevel
fi

#wait a while for openNDS to get started
sleep 5

# Get status as ndsctlout
ndsctlcmd="status 2>/dev/null"
do_ndsctl

# Get the configured debuglevel and version
debuglevel=$(echo "$ndsctlout" | grep "Debug Level" | awk '{printf $4}')
version=$(echo "$ndsctlout" | grep "Version" | awk '{printf $2}')
echo "debuglevel=$debuglevel"

# Get PID For syslog
ndspid=$(pgrep '/usr/bin/opennds')

#get arguments and set variables
url=$1
gatewayhash=$2
phpcli=$3

option="nat_traversal_poll_interval"
get_option_from_config
loop_interval=$option_value

if [ "$loop_interval" = "" ] || [ "$loop_interval" -le 0 ] || [ "$loop_interval" -ge 60 ]; then
	loop_interval=10
fi

postrequest="/usr/lib/opennds/post-request.php"

# Save startup arguments for libopennds
echo "url=$1" > $mountpoint/ndscids/authmonargs
echo "gatewayhash=$2" >> $mountpoint/ndscids/authmonargs
echo "phpcli=$3" >> $mountpoint/ndscids/authmonargs

# Construct our user agent string:
user_agent="openNDS(authmon;NDS:$version;)"

# If we are on OpenWrt, check if ca-bundle is installed
owrt=$(type "opkg" 2>/dev/null | grep "/")

if [ ! -z "$owrt" ]; then
	cabundle=$(opkg list-installed | grep "ca-bundle")

	if [ -z "$cabundle" ]; then
		echo "authmon - FATAL ERROR: ca-bundle not installed - Terminating" | logger -p "daemon.err" -s -t "opennds[$ndspid]"
		ndsctl stop
		exit 1
	fi
fi

# Call postrequest with action.
# Action can be "list" (list and delete from FAS auth log), "view" (view and leave in FAS auth log) or "clear" (clear any stale FAS auth log entries)

# Initialise by clearing stale FAS auth log entries
action="clear"
payload="none"
ret=$($phpcli -f "$postrequest" "$url" "$action" "$gatewayhash" "$user_agent" "$payload")

if [ $debuglevel -ge 3 ]; then
	echo "authmon - action $action, response [$ret]" | logger -p "daemon.debug" -s -t "opennds[$ndspid]"
fi

if [ $debuglevel -ge 1 ]; then
	echo "authmon - nat_traversal_poll_interval is $loop_interval second(s)" | logger -p "daemon.notice" -s -t "opennds[$ndspid]"
fi


# Main loop:
while true; do
	# Check if debuglevel has been changed by ndsctl

	if [ -e "$mountpoint/ndsdebuglevel" ]; then
		debuglevel=$(cat $mountpoint/ndsdebuglevel)
		echo "debuglevel=$debuglevel"
	fi

	# Get remote authlist from the FAS:
	action="view"
	payload="none"
	acklist="*"

	authlist=$($phpcli -f "$postrequest" "$url" "$action" "$gatewayhash" "$user_agent" "$payload")

	if [ $debuglevel -ge 3 ]; then
		echo "authmon - authlist $authlist" | logger -p "daemon.debug" -s -t "opennds[$ndspid]"
	fi

	validator=${authlist:0:1}

	if [ "$validator" = "*" ]; then
		authlist=${authlist:2:1024}

		if [ ${#authlist} -ge 3 ]; then

			# Set the maximum number of clients to authenticate in one go
			# (This is necessary due to argument string length limits in some shell implementations eg Busybox ash)
			authcount=4

			for authparams_enc in $authlist; do
				authparams=$(printf "${authparams_enc//%/\\x}")

				if [ $debuglevel -ge 2 ]; then
					echo "authmon - authentication parameters $authparams" | logger -p "daemon.info" -s -t "opennds[$ndspid]"
				fi

				ndsctlcmd="auth $authparams 2>/dev/null"
				do_ndsctl
				authcount=$((--authcount))

				if [ "$ndsstatus" = "authenticated" ]; then
					client_rhid=$(echo "$authparams" | awk '{printf($1)}')
					acklist="$acklist $client_rhid"
				fi

				if [ "$ndsstatus" = "busy" ]; then
					echo "authmon - ERROR: ndsctl is in use by another process" | logger -p "daemon.err" -s -t "opennds[$ndspid]"
				fi

				if [ "$authcount" < 1 ]; then
					break
				fi
			done
		fi
	else
		for keyword in $authlist; do

			if [ $keyword = "ERROR:" ]; then
				echo "authmon - [$authlist]" | logger -p "daemon.err" -s -t "opennds[$ndspid]"
				echo "authmon - Check Internet connection, FAS url, and package ca-bundle installation" |
					logger -p "daemon.err" -s -t "opennds[$ndspid]"
				break
			fi
		done
	fi

	# acklist is a space separated list of the rhid's of sucessfully authenticated clients.
	# Send acklist to the FAS for upstream processing:
	ackresponse=$($phpcli -f "$postrequest" "$url" "$action" "$gatewayhash" "$user_agent" "$acklist")
	if [ $debuglevel -ge 3 ]; then
		echo "authmon - remote FAS response [$ackresponse]" | logger -p "daemon.debug" -s -t "opennds[$ndspid]"
	fi

	# Sleep for a while:
	sleep $loop_interval
done

