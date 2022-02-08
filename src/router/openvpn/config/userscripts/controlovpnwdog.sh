#!/bin/sh
logger -p user.info "openvpn watchdog control $0 started"
c=$1
[[ -z "$c" ]] && c=1
killall openvpn-watchdog.sh >/dev/null 2>&1
if [[ $(nvram get openvpncl_wdog) -eq 1 && $c -ne 0 ]]; then
	TUN=$(grep "dev" /tmp/openvpncl/openvpn.conf | tail -1 | awk '{print $2}')
	[[ -z "$TUN" ]] && TUN="tun1"
	PINGIP=$(nvram get openvpncl_wdog_pingip)
	[[ -z "$PINGIP" ]] && PINGIP="8.8.8.8"
	SLEEPT=$(nvram get openvpncl_wdog_sleept)
	[[ -z "$SLEEPT" || "$SLEEPT" -lt 10 ]] && SLEEPT="10"
	/usr/bin/openvpn-watchdog.sh $TUN $SLEEPT $PINGIP &
fi
exit 0