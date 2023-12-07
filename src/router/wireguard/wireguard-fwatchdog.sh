#!/bin/sh
i=$1
[[ -z "$i" ]] && i=1
SLEEP=$2
[[ -z "$SLEEP" ]] && SLEEP=30
PINGIP=$3
[[ -z "$PINGIP" ]] && PINGIP="8.8.8.8"
RESET=$4
[[ -z "$RESET" ]] && RESET=0
FAILGRP=$(nvram get oet${i}_failgrp)
REBOOT=$(nvram get wg_onfail_reboot)
sleep 120
logger -p user.info "WireGuard watchdog $0 on tunnel oet${i} running"
ping -qc1 -W6 -n $PINGIP -I oet${i} &> /dev/null && nvram set wg_boot_delay=0
while sleep $SLEEP; do
	[[ "$(nvram get oet${i}_en)" == "0" ]] && continue
	while ! ping -qc1 -W6 -n $PINGIP -I oet${i} &> /dev/null; do
		sleep 7
		if ! ping -qc1 -W6 -n $PINGIP -I oet${i} &> /dev/null; then
			logger -p user.warning "WireGuard watchdog oet${i} is DOWN, Reboot or Reset of WireGuard is executed"
			[[ $FAILGRP -eq 1 ]] && { nvram set oet${i}_failstate=1; logger -p user.warning "WireGuard watchdog: oet${i} set to fail"; }
			#if [[ $RESET -eq 1 && $REBOOT -eq 1 && $FAILGRP -eq 1 ]]; then
			#needs testing
			if [[ $REBOOT -eq 1 ]] && [[ $RESET -eq 1 || $FAILGRP -eq 0 ]]; then
				wg_boot_delay=$(($(nvram get wg_boot_delay)+1))
				[[ $wg_boot_delay -gt 100 ]] && wg_boot_delay=100
				nvram set wg_boot_delay=$wg_boot_delay
				sleep $(($wg_boot_delay*$SLEEP))
				/sbin/reboot &
			elif [[ $RESET -eq 1 && $FAILGRP -eq 1 ]]; then
				logger -p user.warning "WireGuard watchdog last tunnel failed, Resetting tunnel state, to reboot set wg_onfail_reboot=1"
				tunnels=$(nvram get oet_tunnels)
				for i in $(seq 1 $tunnels); do
					nvram set oet${i}_failstate=0
				done
			fi
			( /usr/bin/wireguard-restart.sh & ) >/dev/null 2>&1 
			exit
		fi
		break
	done
done
