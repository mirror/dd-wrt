#!/bin/sh
i=$1
SLEEP=$2
PINGIP=$3
REBOOT=$(nvram get vpn_onfail_reboot)
logger -p user.info "openvpn watchdog $0 pinging via interface $i every $SLEEP sec. to $PINGIP"
sleep 120
ping -qc1 -W6 -n $PINGIP -I $i &> /dev/null && nvram set vpn_boot_delay=0
while sleep $SLEEP; do
	if ! ping -qc1 -W6 -n $PINGIP -I $i &> /dev/null; then
		sleep 9
		if ! ping -qc1 -W6 -n $PINGIP -I $i &> /dev/null; then
			if [[ $REBOOT -eq 1 ]]; then
				vpn_boot_delay=$(($(nvram get vpn_boot_delay)+1))
				[[ $vpn_boot_delay -gt 100 ]] && vpn_boot_delay=100
				nvram set vpn_boot_delay=$vpn_boot_delay
				sleep $(($vpn_boot_delay*$SLEEP))
				logger -p user.warning "openvpn watchdog: openvpn tunnel $i failed, now rebooting"
				reboot
			else
				logger -p user.warning "openvpn watchdog: openvpn tunnel $i failed, now restarting openvpn client, to reboot set vpn_onfail_reboot=1"
				restart_f openvpn &
				exit
			fi
		fi
	fi
done
