#!/bin/sh

if [ -z "$1" ]; then
echo "usage: setup.sh [option] ..."
echo "options:"
echo "reset -  resets to skytron factory defaults"
echo "ip [value] - sets the lan ip for the router"
echo "bandwidth [value] - sets the up/downstream bandwith in kbits/s"
echo "show - shows the current configuration"
echo "passwd - sets the skytron administration password (login is always root)"
exit;
fi


if [ "$1" = "show" ]; then
echo "WAN IP Address = $(nvram get wan_ipaddr)"
echo "WAN Netmask    = $(nvram get wan_netmask)"
echo "WAN Gateway    = $(nvram get wan_gateway)"
echo "WAN DNS        = $(nvram get wan_ds)"
echo "SSID           = $(nvram get wl_ssid)"
echo "Bandwith       = $(nvram get wshaper_downlink)/$(nvram get wshaper_uplink)"
echo "LAN IP Address = $(nvram get lan_ipaddr)"
echo "LAN Netmask    = $(nvram get lan_netmask)"
DHCP=$(nvram get lan_proto)
if [ "$DHCP" = "dhcp" ]; then
    echo "DHCP           = on"
else
    echo "DHCP           = off"
fi

fi

if [ "$1" = "reset" ]; then
erase nvram
nvram set wan_ipaddr = "10.254.254.254"
nvram set wan_netmask = "255.0.0.0"
nvram set wan_gateway = "10.0.0.1"
nvram set wl_ssid = "SKYTRON Network"
nvram set wan_dns = "213.146.232.2 213.146.230.2"
nvram set wshaper_uplink = "800"
nvram set wshaper_downlink = "800"
nvram set lan_ipaddr = "192.168.0.1"
nvram set lan_netmask = "255.255.255.0"
nvram set lan_proto = "dhcp"
nvram commit
reboot
fi

if [ "$1" = "bandwidth" ]; then
nvram set wshaper_uplink = "$2"
nvram set wshaper_downlink = "$2"
nvram commit
rc restart
fi

if [ "$1" = "passwd" ]; then
nvram set skyhttp_passwd="$2"
nvram commit
reboot
fi

if [ "$1" = "ip" ]; then
nvram set lan_ipaddr="$2"
nvram commit
reboot
fi

echo "possibly you did something wrong. i dont know nothing about $1"