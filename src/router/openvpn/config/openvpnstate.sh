#!/bin/sh

if [ "$(nvram get openvpn_enable)" = "1" ]; then
PORT=$(awk '/^management /{x=$3}END{print x}' /tmp/openvpn/openvpn.conf)
if [ x${PORT} = x ]
then
	PORT=14
fi
echo -e "Server: "
echo -e "`/bin/echo "state" | /usr/bin/nc 127.0.0.1 ${PORT} |awk -F"," '{ print $2}'` `/bin/echo "state" | /usr/bin/nc 127.0.0.1 ${PORT} |awk -F"," '{print $3}'` <hr/>"
if [ "$(nvram get openvpn_tuntap)" = "tun" ]; then
	echo -e "Local Address: `/bin/echo "state" | /usr/bin/nc 127.0.0.1 ${PORT} |awk -F"," '{print $4}'` <br>"
	echo -e "Remote Address: `ifconfig tun2 |awk '{print $3}'|grep P-t-P|awk -F ":" '{print $2}'` <br>"
else # TAP
	echo -e "TAP Mode: Bridged<br>"
	echo -e "MAC Address: `ifconfig tap2 |grep HWaddr|awk '{print $5}'` <br><br>"
fi
fi

if [ "$(nvram get openvpncl_enable)" = "1" ]; then
PORT=$(awk '/^management /{x=$3}END{print x}' /tmp/openvpncl/openvpn.conf)
if [ x${PORT} = x ]
then
	PORT=16
fi
echo -e "Client: "
echo -e "`/bin/echo "state" | /usr/bin/nc 127.0.0.1 ${PORT} |awk -F"," '{ print $2}'` `/bin/echo "state" | /usr/bin/nc 127.0.0.1 ${PORT} |awk -F"," '{print $3}'` <hr/>"
if [ "$(nvram get openvpncl_tuntap)" = "tun" ]; then
	echo -e "Local Address: `/bin/echo "state" | /usr/bin/nc 127.0.0.1 ${PORT} |awk -F"," '{print $4}'` <br>"
	echo -e "Remote Address: `ifconfig tun1 |awk '{print $3}'|grep P-t-P|awk -F ":" '{print $2}'` <br>"
else
	if [ "$(nvram get openvpncl_bridge)" = "1" ]; then
		echo -e "TAP mode: Bridged <br>"
	else
		echo -e "TAP mode: Unbridged <br>"
	fi
	echo -e "MAC Address: `ifconfig tap1 |grep HWaddr|awk '{print $5}'` <br>"
	echo -e "Local IP: `ifconfig tap1 |awk -F ":" '/inet addr/ {print $2}' |awk -F " " '{print $1}'` <br>"
	if [ "$(nvram get openvpncl_nat)" = "1" ]; then
		echo -e "NAT: On<br>"
	fi
	if [ "$(nvram get openvpncl_sec)" = "1" ]; then
		echo -e "Firewall Protection: On<br>"
	fi
fi
fi
