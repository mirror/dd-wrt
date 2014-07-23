#!/bin/sh

if [ "$(nvram get openvpn_enable)" = "1" ]; then
echo -e "Server: "
echo -e "`/bin/echo "state" | /usr/bin/nc 127.0.0.1 14 |awk -F"," '{ print $2}'` `/bin/echo "state" | /usr/bin/nc 127.0.0.1 14 |awk -F"," '{print $3}'` <br><br>"
echo -e "Local Address: `/bin/echo "state" | /usr/bin/nc 127.0.0.1 14 |awk -F"," '{print $4}'` <br>"
echo -e "Remote Address: `ifconfig tun2 |awk '{print $3}'|grep P-t-P|awk -F ":" '{print $2}'` <br><br><br>"
fi

if [ "$(nvram get openvpncl_enable)" = "1" ]; then
echo -e "Client: "
echo -e "`/bin/echo "state" | /usr/bin/nc 127.0.0.1 16 |awk -F"," '{ print $2}'` `/bin/echo "state" | /usr/bin/nc 127.0.0.1 16 |awk -F"," '{print $3}'` <br><br>"
echo -e "Local Address: `/bin/echo "state" | /usr/bin/nc 127.0.0.1 16 |awk -F"," '{print $4}'` <br>"
echo -e "Remote Address: `ifconfig tun1 |awk '{print $3}'|grep P-t-P|awk -F ":" '{print $2}'` <br>"
fi