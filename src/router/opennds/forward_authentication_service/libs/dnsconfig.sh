#!/bin/sh
#Copyright (C) BlueWave Projects and Services 2015-2025
#This software is released under the GNU GPL license.
#
# Warning - shebang sh is for compatibliity with busybox ash (eg on OpenWrt)
# This is changed to bash automatically by Makefile for generic Linux
#

# Define the dnsmask config file and hosts file locations for generic Linux
# Edit these if your system uses a non standard locations:
conflocation="/etc/dnsmasq.conf"
hosts="/etc/hosts"
#

setconf="$1"
uciconfig=$(uci show dhcp 2>/dev/null)

ipset_to_nftset () {

	local timeout=$loopcount

	for tic in $(seq $timeout); do
		ipset list "$ipsetname" &>/dev/null
		ipsetstat=$?
		elements=$(ipset list "$ipsetname" 2>/dev/null | awk -F"." 'NF==4 {printf ", %s", $0}')

		if [ $ipsetstat -gt 0 ]; then
			break
		fi

		elements=$(ipset list "$ipsetname" 2>/dev/null | awk -F"." 'NF==4 {printf ", %s", $0}')
		elements=${elements:2}

		if [ ! -z "$elements" ] && [ "$elements" != "$last_elements" ]; then
			nft add element ip nds_filter "$ipsetname" {"$elements"}
		fi

		last_elements="$elements"
		sleep 1
	done
}

delete_114s() {

	if [ ! -z "$cpidconfig" ]; then

		for option114 in $cpidconfig; do
			is_114=$(echo "$option114" | grep "114")

			if [ ! -z "$is_114" ]; then
				echo "$dellist'$option114'" | uci batch
			fi
		done
	fi
}

restart_dnsmasq() {
	if [ "$uciconfig" = "" ]; then
		systemctl restart dnsmasq
	else
		service dnsmasq restart
	fi
}

reload_dnsmasq() {
	if [ "$uciconfig" = "" ]; then
		systemctl reload dnsmasq
	else
		service dnsmasq reload
	fi
}


if [ "$setconf" = "" ]; then
	exit 1

elif [ "$setconf" = "restart_only" ]; then
	restart_dnsmasq
	printf "%s" "done"
	exit 0

elif [ "$setconf" = "reload_only" ]; then
	reload_dnsmasq
	printf "%s" "done"
	exit 0

elif [ "$setconf" = "revert" ]; then

	if [ ! -z "$uciconfig" ]; then
		uci revert dhcp
	fi

	printf "%s" "done"
	exit 0

elif [ "$setconf" = "hostconf" ]; then
	gw_ip=$2
	gw_fqdn=$3

	if [ -z "$uciconfig" ]; then
		# Generic Linux
		host_entry="$gw_ip $gw_fqdn"
		# generate a tmp filename on tmpfs
		ram_hosts=$(mktemp --tmpdir=/run/tmpfiles.d)

		cp -p /etc/hosts "$ram_hosts" &&
		(
			# Add record GW_IP GW_FQDN in /etc/hosts file if GW_IP does not exist
			grep -qw "^$gw_ip" $ram_hosts || (echo "$host_entry" >> $ram_hosts)

			# Add GW_FQDN as an alias to GW_IP record where GW_FDQN alias is missing
			sed -i "/^$gw_ip[[:space:]]\+/ { /[[:space:]]$gw_fqdn[[:space:]]/I! {  /[[:space:]]$gw_fqdn$/I! s/[[:space:]]*$/ $gw_fqdn/}}" $ram_hosts

			# Remove GW_FQDN from IPs different from GW_IP
			sed -i "/^$gw_ip[[:space:]]\+/! { s/\([[:space:]]\)$gw_fqdn[[:space:]]/\1/Ig;s/\([[:space:]]\)$gw_fqdn$//I}" $ram_hosts

			# rewrite /etc/hosts file only after an update
			diff -b $ram_hosts /etc/hosts &>/dev/null ||  mv $ram_hosts /etc/hosts
		)

		# cleanup
		rm -f $ram_hosts

	else
		# OpenWrt
		# Note we do not commit here so that the config changes do NOT survive a reboot and can be reverted without writing to config files
		host_entry="/$gw_fqdn/$gw_ip"
		del_dns="del_list dhcp.@dnsmasq[0].address='$host_entry'"
		add_dns="add_list dhcp.@dnsmasq[0].address='$host_entry'"
		echo $del_dns | uci batch
		echo $add_dns | uci batch
	fi

	printf "%s" "done"
	exit 0

elif [ "$setconf" = "cpidconf" ]; then
	gatewayfqdn=$2

	if [ "$uciconfig" = "" ]; then
		# Generic Linux
		sed -i '/System\|114,http:/d' $conflocation

		if [ ! -z "$gatewayfqdn" ]; then
			echo "dhcp-option-force=114,http://$gatewayfqdn" >> $conflocation
		fi
	else
		# OpenWrt
		# Note we do not commit here so that the config changes do NOT survive a reboot and can be reverted without writing to config files

		# Get the network zone
		gwif=$(uci get opennds.@opennds[0].gatewayinterface 2> /dev/null | awk '{printf "%s", $1}')

		if [ -z "$gwif" ]; then
			gwif="br-lan"
		fi

		network_zone=$(uci show network | grep "device='$gwif'" | awk -F "." '{printf "%s", $2}')

		if [ ! -z "$network_zone" ]; then
			cpidconfig=$(uci get dhcp.lan.dhcp_option_force 2>/dev/null)
			dellist="del_list dhcp.$network_zone.dhcp_option_force='114,http://$gatewayfqdn'"

			if [ -z "$gatewayfqdn" ]; then
				delete_114s
				printf "%s" "done"
				exit 0
			fi

			addlist="add_list dhcp.$network_zone.dhcp_option_force='114,http://$gatewayfqdn'"

			if [ -z "$cpidconfig" ]; then
				echo $addlist | uci batch

			elif [ "$cpidconfig" != "114,http://$gatewayfqdn" ]; then
				delete_114s
				echo $addlist | uci batch
			fi
		fi
	fi

	printf "%s" "done"
	exit 0

elif [ "$1" = "ipset_to_nftset" ]; then
	ipsetname=$2

	if [ -z "$2" ]; then
		exit 4
	fi

	if [ -z "$3" ]; then
		loopcount=1
	else
		loopcount=$3
	fi

	ipset_to_nftset
	exit 0
else
	exit 1 
fi

