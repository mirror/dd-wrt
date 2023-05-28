#!/bin/sh
#Copyright (C) The openNDS Contributors 2004-2022
#Copyright (C) BlueWave Projects and Services 2015-2023
#This software is released under the GNU GPL license.

# This is an example script for BinAuth
# It writes a local log and can override authentication requests and quotas.
#
# The client User Agent string is forwarded to this script.
#
# If BinAuth is enabled, NDS will call this script as soon as it has received an authentication, deauthentication or shutdown request
#

##################
# functions:

get_client_zone () {
	# Gets the client zone, (if we don't already have it) ie the connection the client is using, such as:
	# local interface (br-lan, wlan0, wlan0-1 etc.,
	# or remote mesh node mac address
	# This zone name is only displayed here but could be used to customise the login form for each zone

	if [ -z "$client_zone" ]; then
		client_mac=$clientmac
		client_if_string=$(/usr/lib/opennds/get_client_interface.sh $client_mac)
		failcheck=$(echo "$client_if_string" | grep "get_client_interface")

		if [ -z $failcheck ]; then
			client_if=$(echo "$client_if_string" | awk '{printf $1}')
			client_meshnode=$(echo "$client_if_string" | awk '{printf $2}' | awk -F ':' '{print $1$2$3$4$5$6}')
			local_mesh_if=$(echo "$client_if_string" | awk '{printf $3}')

			if [ ! -z "$client_meshnode" ]; then
				client_zone="MeshZone:$client_meshnode LocalInterface:$local_mesh_if"
			else
				client_zone="LocalZone:$client_if"
			fi
		else
			client_zone=""
		fi
	else
		client_zone=$(printf "${client_zone//%/\\x}")
	fi
}

get_option_from_config() {
	local param=""

	if [ -e "/etc/config/opennds" ]; then
		param=$(uci -q get opennds.@opennds[0].$option | awk '{printf("%s", $0)}')

	elif [ -e "/etc/opennds/opennds.conf" ]; then
		param=$(cat "/etc/opennds/opennds.conf" | awk -F"$option" '{printf("%s", $2)}')
	fi

	eval $option=$param
}

configure_log_location() {
	# Generate the Logfile location; use the tmpfs "temporary" directory to prevent flash wear.
	# Alternately you may choose to manually override the settings generated here.
	# For example mount a USB storage device and manually set logdir and logname instead of this code
	#
	# DEFAULT Location depends upon OS distro in use:
	tempdir="/tmp /run /var"

	# set default values
	mountpoint="/tmp"
	logdir="/tmp/ndslog/"
	logname="binauthlog.log"

	for var in $tempdir; do
		_mountpoint=$(df | awk -F ' ' '$1=="tmpfs" && $6=="'$var'" {print $6}')
		if [ "$_mountpoint" = "$var" ]; then
			mountpoint="$var"
			logdir="$mountpoint/ndslog/"
			break
		fi
	done

	# Check if config overrides mountpoint for logdir
	log_mountpoint=""
	option="log_mountpoint"
	get_option_from_config

	if [ ! -z "$log_mountpoint" ]; then
		logdir="$log_mountpoint/ndslog/"
	else
		log_mountpoint="$mountpoint"
	fi

	# Get PID For syslog
	ndspid=$(pgrep '/usr/bin/opennds')
}

write_log () {
	mountcheck=$(df | grep "$log_mountpoint")

	if [ ! -z "$logname" ]; then

		if [ ! -d "$logdir" ]; then
			mkdir -p "$logdir"
		fi

		logfile="$logdir""$logname"
		awkcmd="awk ""'\$6==""\"$log_mountpoint\"""{print \$4}'"
		datetime=$(date)

		if [ ! -f "$logfile" ]; then
			echo "$datetime, New log file created" > $logfile
		fi

		if [ ! -z "$mountcheck" ]; then
			# Truncate the log file if max_log_entries is set
			max_log_entries=""
			option="max_log_entries"
			get_option_from_config

			if [ ! -z "$max_log_entries" ]; then
				mv "$logfile" "$logfile.cut"
				tail -n "$max_log_entries" "$logfile.cut" >> "$logfile"
				rm "$logfile.cut"
			fi

			available=$(df | grep "$log_mountpoint" | eval "$awkcmd")

			if [ "$log_mountpoint" = "$mountpoint" ]; then
				# Logging to tmpfs, so dynamically adjust max log size
				# then check the logfile is not too big
				min_freespace_to_log_ratio=10
				filesize=$(ls -s -1 $logfile | awk -F' ' '{print $1}')
				sizeratio=$(($available/$filesize))

				if [ $sizeratio -ge $min_freespace_to_log_ratio ]; then
					echo "$datetime, $log_entry" >> $logfile
				else
					echo "Log file too big, please archive contents and reduce max_log_entries" | logger -p "daemon.err" -s -t "opennds[$ndspid]: "
				fi
			else
				# Logging to dedicated storage eg usb drive
				# so just check for free space and let the log file grow

				if [ "$available" > 10 ];then
					echo "$datetime, $log_entry" >> $logfile
				else
					echo "Log file too big, please archive contents and reduce max_log_entries" | logger -p "daemon.err" -s -t "opennds[$ndspid]: "
				fi
			fi
		else
			echo "Log location is NOT a mountpoint - logging disabled" | logger -p "daemon.err" -s -t "opennds[$ndspid]: "
		fi
	fi
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

configure_log_location

#
# Get the action method from NDS ie the first command line argument.
#
# Possible values are:
# "auth_client" - NDS requests validation of the client (legacy - deprecated)
# "client_auth" - NDS has authorised the client (legacy - deprecated)
# "client_deauth" - NDS has deauthenticated the client on request (logout)
# "idle_deauth" - NDS has deauthenticated the client because the idle timeout duration has been exceeded
# "timeout_deauth" - NDS has deauthenticated the client because the session length duration has been exceeded
# "downquota_deauth" - NDS has deauthenticated the client because the client's download quota has been exceeded
# "upquota_deauth" - NDS has deauthenticated the client because the client's upload quota has been exceeded
# "ndsctl_auth" - NDS has authorised the client because of an ndsctl command
# "ndsctl_deauth" - NDS has deauthenticated the client because of an ndsctl command
# "shutdown_deauth" - NDS has deauthenticated the client because it received a shutdown command
#
action=$1

if [ $action = "auth_client" ]; then
	# Arguments passed are as follows
	# $1 method
	# $2 client mac
	# $3 originurl (aka redir, this is the query string returned to openNDS when auth_client is requested - not very useful so not usually logged)
	# $4 client useragent
	# $5 client ip
	# $6 client token
	# $7 custom data string

	# customdata is by default b64encoded.
	# You can use ndsctl to decode it (all functions of ndsctl are locked from use within binauth except b64encode and b64decode)
	# Note the format of the decoded customdata is set in the FAS or Themespec scripts so unencoded special characters may cause issues.
	# For example, to decode customdata use:
	# customdata=$(ndsctl b64decode "$customdata")

	log_entry="method=$1, clientmac=$2, clientip=$5, useragent=$4, token=$6, custom=$7"

else
	# All other methods
	# Arguments passed are as follows
	# $1 method
	# $2 client mac
	# $3 bytes incoming
	# $4 bytes outgoing
	# $5 session start time
	# $6 session end time
	# $7 client token
	# $8 custom data string

	customdata=$8

	# Build the log entry:
	log_entry="method=$1, clientmac=$2, bytes_incoming=$3, bytes_outgoing=$4, session_start=$5, session_end=$6, token=$7, custom=$customdata"

	action=$(echo "$1" | awk -F"_" '{printf("%s", $2)}')

	# Send the deauth log to FAS if fas_secure_enabled = 3, if not =3 library call does nothing
	if [ "$action" = "deauth" ]; then
		returned=$(/usr/lib/opennds/libopennds.sh "send_to_fas_deauthed" "$log_entry")
	fi

fi

# In the case of ThemeSpec, get the client id information from the cid database
# Client variables found in the database are:
# 
# clientip
# clientmac
# gatewayname
# version
# client_type
# hid
# gatewayaddress
# gatewaymac
# originurl
# clientif

# Additional data defined by custom parameters, images and files is included
# For example ThemeSpec "theme_user-email-login-custom-placeholders.sh" config options include:
# input
# logo_message
# banner1_message
# banner2_message
# banner3_message
# logo_png
# banner1_jpg
# banner2_jpg
# banner3_jpg
# advert1_htm

# Parse the database by client mac ($2):
cidfile=$(grep -r "$2" "$mountpoint/ndscids" | awk -F 'ndscids/' '{print $2}' | awk -F ':' '{printf $1}')

if [ ! -z "$cidfile" ]; then
	# populate the local variables:
	. $mountpoint/ndscids/$cidfile

	# Add a selection of client data variables to the log entry
	log_entry="$log_entry, client_type=$client_type, gatewayname=$gatewayname, ndsversion=$version, originurl=$originurl"
else
	clientmac=$2
fi

# Get the client zone (the network zone the client is connected to
# This might be a local wireless interface, a remote mesh node, or a cable connected wireless access point
get_client_zone

# Add client_zone to the log entry
log_entry="$log_entry, client_zone=$client_zone"

# Append to the log.
write_log

#Quotas and session length set elsewhere can be overridden here if action=auth_client, otherwise will be ignored.
# Set length of session in seconds (eg 24 hours is 86400 seconds - if set to 0 then defaults to global or FAS sessiontimeout value):
session_length=0

custom=$8
custom=$(/usr/lib/opennds/unescape.sh -url "$custom")
custom=$(ndsctl b64decode "$custom")
session_length=$(echo "$custom" | awk -F"session_length=" '{printf "%d", $2}')

# Set Rate and Quota values for the client
# The session length, rate and quota values are determined globaly or by FAS/PreAuth on a per client basis.
# rates are in kb/s, quotas are in kB. Setting to 0 means no limit
upload_rate=0
download_rate=0
upload_quota=0
download_quota=0

# Finally before exiting, output the session length, upload rate, download rate, upload quota and download quota (only effective for auth_client).

echo "$session_length $upload_rate $download_rate $upload_quota $download_quota"

# Exit, setting level (only effective for auth_client)
#
# exit 0 tells NDS it is ok to allow the client to have access.
# exit 1 would tell NDS to deny access.
exit 0
