#!/bin/sh
#Copyright (C) BlueWave Projects and Services 2015-2025
#This software is released under the GNU GPL license.

# functions:

# Get configured option
get_option_from_config() {
	option_value=$(/usr/lib/opennds/libopennds.sh get_option_from_config $option)
}

# Function to send commands to openNDS:
do_ndsctl () {
	local timeout=15

	for tic in $(seq $timeout); do
		ndsstatus="ready"
		ndsctlcmd="$ndsctlcmd 2>&1"
		ndsctlout=$(eval ndsctl "$ndsctlcmd")

		for keyword in $ndsctlout; do

			if [ $keyword = "locked" ]; then
				ndsstatus="busy"
				sleep 1
				break
			fi

			if [ $keyword = "probably" ]; then
				ndsstatus="not_started"
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

		if [ "$ndsstatus" = "not_started" ]; then
			continue
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

# Get our PID For syslog use
ndspid=$(pgrep -f '/usr/lib/opennds/authmon.sh')

#get arguments and set variables
url=$1
gatewayhash=$2
remotecli=$3

if [ "$remotecli" = "wget" ]; then
	remoterequest="/usr/lib/opennds/libopennds.sh \"wget_request\""
else
	remoterequest="$remotecli -f /usr/lib/opennds/post-request.php"
fi

option="nat_traversal_poll_interval"
get_option_from_config
loop_interval=$option_value

if [ "$loop_interval" = "" ] || [ "$loop_interval" -le 0 ] || [ "$loop_interval" -ge 60 ]; then
	loop_interval=5
fi

# Construct our user agent string:
user_agent="openNDS(authmon;NDS:$version;)"

# Save startup arguments for libopennds
echo "url=\"$1\"" > $mountpoint/ndscids/authmonargs
echo "gatewayhash=\"$2\"" >> $mountpoint/ndscids/authmonargs
echo "remotecli=\"$3\"" >> $mountpoint/ndscids/authmonargs
echo "remoterequest=\"$remoterequest\"" >> $mountpoint/ndscids/authmonargs
echo "user_agent=\"$user_agent\"" >> $mountpoint/ndscids/authmonargs

# Call remoterequest with action.
# Action can be "list" (list and delete from FAS auth log), "view" (view and leave in FAS auth log) or "clear" (clear any stale FAS auth log entries)

# Initialise by clearing stale FAS auth log entries
action="clear"
payload="none"
acklist="*"


ret=$(eval "$remoterequest" "\"$url\"" "\"$action\"" "\"$gatewayhash\"" "\"$user_agent\"" "\"$payload\"")

if [ "$debuglevel" -ge 3 ]; then
	echo "authmon - action $action, response [$ret]" | logger -p "daemon.debug" -t "authmon[$ndspid]"
fi

if [ "$debuglevel" -ge 1 ]; then
	echo "authmon - nat_traversal_poll_interval is $loop_interval second(s)" | logger -p "daemon.notice" -t "authmon[$ndspid]"
fi


# Main loop:
while true; do
	# Check if debuglevel has been changed by ndsctl

	if [ -e "$mountpoint/ndsdebuglevel" ]; then
		debuglevel=$(cat $mountpoint/ndsdebuglevel)
	fi

	# Get remote authlist from the FAS:
	action="view"
	payload="none"
	acklist="*"

	authlist=$(eval "$remoterequest" "\"$url\"" "\"$action\"" "\"$gatewayhash\"" "\"$user_agent\"" "\"$payload\"")

	if [ $debuglevel -ge 3 ]; then
		echo "authmon - authlist $authlist" | logger -p "daemon.debug" -t "authmon[$ndspid]"
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
					echo "authmon - authentication parameters $authparams" | logger -p "daemon.info" -t "authmon[$ndspid]"
				fi

				ndsctlcmd="auth $authparams 2>/dev/null"
				do_ndsctl
				authcount=$((--authcount))

				if [ "$ndsstatus" = "authenticated" ]; then
					client_rhid=$(echo "$authparams" | awk '{printf($1)}')
					acklist="$acklist $client_rhid"
				fi

				if [ "$ndsstatus" = "busy" ]; then
					echo "authmon - ERROR: ndsctl is in use by another process" | logger -p "daemon.err" -t "authmon[$ndspid]"
				fi

				if [ "$authcount" -lt 1 ]; then
					break
				fi
			done
		fi
	else
		for keyword in $authlist; do

			if [ $keyword = "ERROR:" ]; then
				echo "authmon - [$authlist]" | logger -p "daemon.err" -t "authmon[$ndspid]"
				echo "authmon - Check Internet connection, FAS url, and package ca-bundle installation" |
					logger -p "daemon.err" -t "authmon[$ndspid]"
				break
			fi
		done
	fi

	# acklist is a space separated list of the rhid's of sucessfully authenticated clients.
	# Send acklist to the FAS for upstream processing:

	ackresponse=$(eval "$remoterequest" "\"$url\"" "\"$action\"" "\"$gatewayhash\"" "\"$user_agent\"" "\"$acklist\"")

	if [ $debuglevel -ge 3 ]; then
		echo "authmon - remote FAS response [$ackresponse]" | logger -p "daemon.debug" -t "authmon[$ndspid]"
	fi

	# Sleep for a while:
	sleep $loop_interval
done

