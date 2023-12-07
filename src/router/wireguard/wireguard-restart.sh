#!/bin/sh
# restart whole firewall to get the nat-loopback rules etc.
REBOOT=$(nvram get wg_onfail_reboot)
if [[ $REBOOT -eq 2 ]] 2>/dev/null; then
	restart firewall
	logger -p user.info "WireGuard watchdog tunnel and entire firewall restarted"
else
	( /etc/config/eop-tunnel.prewall & ) >/dev/null 2>&1
	logger -p user.info "WireGuard watchdog tunnel restarted"
	#sleep 1
	#/etc/config/eop-tunnel.firewall2  >/dev/null 2>&1
	#logger -p user.info "WireGuard watchdog: WG firewall only restarted"
fi
