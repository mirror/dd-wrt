#!/bin/sh
#Copyright (C) The openNDS Contributors 2004-2022
#Copyright (C) BlueWave Projects and Services 2015-2025
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

urlencode() {
	entitylist="
		s/%/%25/g
		s/\s/%20/g
		s/\"/%22/g
		s/>/%3E/g
		s/</%3C/g
		s/'/%27/g
		s/\`/%60/g
	"
	local buffer="$1"

	for entity in $entitylist; do
		urlencoded=$(echo "$buffer" | sed "$entity")
		buffer=$urlencoded
	done

	urlencoded=$(echo "$buffer" | awk '{ gsub(/\$/, "\\%24"); print }')
}

get_option_from_config() {

	type uci &> /dev/null
	uci_status=$?

	if [ $uci_status -eq 0 ]; then
		param=$(uci export opennds | grep "option" | grep "$option" | awk -F"'" 'NF > 1 {printf "%s ", $2}')
	else
		param=$(cat /etc/config/opennds | grep "option" | grep "$option" | awk -F"#" '{printf "%s\n", $1}' | awk -F"'" 'NF > 1 {printf "%s ", $2}')
	fi

	# remove trailing space character
	param=$(echo "$param" | sed 's/.$//')

	# urlencode
	urlencode "$param"
	param=$urlencoded
	eval $option="$param" &>/dev/null
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
	fulllog="binauthlog.log"
	authlog="authlog.log"

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
	ndspid=$(pgrep -f '/usr/bin/opennds')
}

write_log () {
	/usr/lib/opennds/libopennds.sh "write_log" "$loginfo" "$logname" "$date_inhibit"
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
# "auth_client" - NDS requests validation of the client
# "client_auth" - NDS has authorised the client
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

	loginfo="method=$1, clientmac=$2, clientip=$5, useragent=$4, token=$6, custom=$7"

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
	loginfo="method=$1, clientmac=$2, bytes_incoming=$3, bytes_outgoing=$4, session_start=$5, session_end=$6, token=$7, custom=$customdata"

	action=$(echo "$1" | awk -F"_" '{printf("%s", $NF)}')

	# Send the deauth log to FAS if fas_secure_enabled = 3, if not =3 library call does nothing
	if [ "$action" = "deauth" ]; then
		returned=$(/usr/lib/opennds/libopennds.sh "send_to_fas_deauthed" "$loginfo")
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
	loginfo="$loginfo, client_type=$client_type, gatewayname=$gatewayname, ndsversion=$version, originurl=$originurl"
else
	clientmac=$2
fi

# Get the client zone (the network zone the client is connected to
# This might be a local wireless interface, a remote mesh node, or a cable connected wireless access point
get_client_zone

# Add client_zone to the log entry
loginfo="$loginfo, client_zone=$client_zone"

# Append to the log.
logname="$fulllog"
logtype=""
date_inhibit=""

write_log &> /dev/null

# Append to the authenticated clients list
session_end=$6

if [ "$action" = "auth_client" ] || [ "$action" = "auth" ]; then
	logname="$authlog"
	b64mac=$(ndsctl b64encode "$clientmac")
	b64mac=$(echo "$b64mac" | tr -d "=")
	loginfo="$b64mac=$session_end"
	logtype="raw"
	logfile="$logdir""$logname"

	if [ -f "$logdir""$logname" ]; then
		sed -i "/\b$b64mac\b/d" "$logfile"
	fi

	date_inhibit="date_inhibit"

	write_log &> /dev/null
fi

# Values for quotas and session length can be overridden here if action=auth_client, and passed in the custom string.
# The custom string must be parsed in custombinauth.sh script for the required values.
# exitlevel can also be set in the custonbinauth.sh script (0=allow, 1=deny)
session_length=0
upload_rate=0
download_rate=0
upload_quota=0
download_quota=0
exitlevel=0

if [ "$action" = "auth_client" ]; then
	custom=$7
else
	custom=$8
fi

# Include custom binauth script
custombinauthpath="/usr/lib/opennds/custombinauth.sh"

if [ -e "$custombinauthpath" ]; then
	. $custombinauthpath
fi

# Finally before exiting, output the session length, upload rate, download rate, upload quota and download quota (only effective for auth_client).
# The custom binauth script migh change these values
echo "$session_length $upload_rate $download_rate $upload_quota $download_quota"

# Exit, setting level (only effective for auth_client)
#
# exit 0 tells NDS it is ok to allow the client to have access (default).
# exit 1 would tell NDS to deny access.
# The custom binauth script might change this value
exit $exitlevel
