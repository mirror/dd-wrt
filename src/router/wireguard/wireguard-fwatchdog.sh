#!/bin/sh
#debug
deb=$(nvram get console_debug)
if [[ $deb -eq 1 ]]; then
	set -x
fi
i=$1
[[ -z "$i" ]] && i=1
SLEEP=$2
[[ -z "$SLEEP" ]] && SLEEP=60
PINGIP=$3
[[ -z "$PINGIP" ]] && PINGIP="8.8.8.8"
RESET=$4
[[ -z "$RESET" ]] && RESET=0
REBOOT=$(nvram get wg_onfail_reboot)
logger -p user.info "WireGuard watchdog $0 on tunnel oet${i} running"
sleep 120
ping -qc1 -W6 -n $PINGIP -I oet${i} &> /dev/null && nvram set wg_boot_delay=0
while sleep $SLEEP; do
	[[ "$(nvram get oet${i}_en)" == "0" ]] && continue
	while ! ping -qc1 -W6 -n $PINGIP -I oet${i} &> /dev/null; do
		sleep 11
		if ! ping -qc1 -W6 -n $PINGIP -I oet${i} &> /dev/null; then
			logger -p user.info "WireGuard watchdog: oet${i} is DOWN, and is set to FAILED, Reboot or Reset of WireGuard with fail over is executed"
			nvram set oet${i}_failstate=1
			if [[ $RESET -eq 1 && $REBOOT -eq 1 ]]; then
				wg_boot_delay=$(($(nvram get wg_boot_delay)+1))
				[[ $wg_boot_delay -gt 100 ]] && wg_boot_delay=100
				nvram set wg_boot_delay=$wg_boot_delay
				sleep $(($wg_boot_delay*$SLEEP))
				reboot
			elif [[ $RESET -eq 1 ]]; then
				logger -p user.info "WireGuard watchdog: last tunnel failed, Resetting tunnel state, to reboot set wg_onfail_reboot=1"
				tunnels=$(nvram get oet_tunnels)
				for i in $(seq 1 $tunnels); do
					nvram set oet${i}_failstate=0
				done
			fi
			/etc/config/eop-tunnel.prewall  >/dev/null 2>&1
			/etc/config/eop-tunnel.firewall  >/dev/null 2>&1
		fi	
		break
	done
done
