#!/bin/sh
#Copyright (C) BlueWave Projects and Services 2015-2023
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
		systemctl restart dnsmasq &
	else
		/etc/init.d/dnsmasq restart &
	fi
}


if [ "$setconf" = "" ]; then
	exit 1

elif [ "$setconf" = "restart_only" ]; then
	restart_dnsmasq
	printf "%s" "done"
	exit 0

elif [ "$setconf" = "revert" ]; then

	if [ ! -z "$uciconfig" ]; then
		uci revert dhcp
	fi

	printf "%s" "done"
	exit 0

elif [ "$setconf" = "ipsetconf" ]; then
	ipsetconf=$2	

	if [ -z "$uciconfig" ]; then
		sed -i '/System\|walledgarden/d' $conflocation
		echo "ipset=$ipsetconf" >> $conflocation
	else
		# OpenWrt
		# Note we do not commit here so that the config changes do NOT survive a reboot and can be reverted without writing to config files
		del_ipset="del_list dhcp.@dnsmasq[0].ipset='$ipsetconf'"
		add_ipset="add_list dhcp.@dnsmasq[0].ipset='$ipsetconf'"
		echo $del_ipset | uci batch
		echo $add_ipset | uci batch
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
		cpidconfig=$(uci get dhcp.lan.dhcp_option_force 2>/dev/null)
		dellist="del_list dhcp.lan.dhcp_option_force="

		if [ -z "$gatewayfqdn" ]; then
			delete_114s
			printf "%s" "done"
			exit 0
		fi

		addlist="add_list dhcp.lan.dhcp_option_force='114,http://$gatewayfqdn'"

		if [ -z "$cpidconfig" ]; then
			echo $addlist | uci batch

		elif [ "$cpidconfig" != "114,http://$gatewayfqdn" ]; then
			delete_114s
			echo $addlist | uci batch
		fi
	fi

	printf "%s" "done"
	exit 0

else
	exit 1 
fi

