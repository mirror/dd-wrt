#!/bin/sh
/etc/config/eop-tunnel.prewall  >/dev/null 2>&1
logger -p user.info "WireGuard watchdog: tunnel restarted"
sleep 1
# consider restarting whole firewall to get the nat-loopback rules
#restart firewall
/etc/config/eop-tunnel.firewall  >/dev/null 2>&1
logger -p user.info "WireGuard watchdog: firewall restarted"
