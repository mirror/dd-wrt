#!/bin/sh
logger -p user.info "openvpn watchdog control $0 starting"
c="${1:-1}"
TAP="${2:-0}"
killall openvpn-watchdog.sh >/dev/null 2>&1
if [[ $(nvram get openvpncl_wdog) -eq 1 && $c -ne 0 ]]; then
	if [[ $TAP == "1" ]]; then
		VPNIF="br0"
	else
		VPNIF=$(grep "dev" /tmp/openvpncl/openvpn.conf | tail -1 | awk '{print $2}')
	fi
	[[ -z "$VPNIF" ]] && VPNIF="tun1"
	PINGIP=$(nvram get openvpncl_wdog_pingip)
	[[ -z "$PINGIP" ]] && PINGIP="8.8.8.8"
	SLEEPT=$(nvram get openvpncl_wdog_sleept)
	[[ -z "$SLEEPT" || "$SLEEPT" -lt 10 ]] && SLEEPT="10"
	/usr/bin/openvpn-watchdog.sh "$VPNIF" "$SLEEPT" "$PINGIP" &
fi
exit 0