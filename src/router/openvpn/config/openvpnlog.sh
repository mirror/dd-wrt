#!/bin/sh
if [ "$(nvram get openvpn_enable)" = "1" ]; then
PORT=`grep "^management " /tmp/openvpn/openvpn.conf | awk '{print $3}'`
if [ x${PORT} = x ]
then
	PORT=14
fi
/bin/echo -e "Serverlog: <br>"
/bin/echo "log 500" | /usr/bin/nc 127.0.0.1 ${PORT }| /bin/grep -v "^>" | /usr/bin/awk -F "," '{
	printf strftime("%Y%m%d %H:%M:%S ",$1);
	for (i=2;i<=NF;i++)
		printf $i" "
	printf "<br>\n";
	}'

/bin/echo -e "<br>"
#/bin/cat /tmp/openvpn/openvpn.conf

/bin/echo -e "<br><br><br>"
fi
if [ "$(nvram get openvpncl_enable)" = "1" ]; then
PORT=`grep "^management " /tmp/openvpncl/openvpn.conf | awk '{print $3}'`
if [ x${PORT} = x ]
then
	PORT=16
fi
/bin/echo -e "Clientlog: <br>"
/bin/echo "log 500" | /usr/bin/nc 127.0.0.1 ${PORT} | /bin/grep -v "^>" | /usr/bin/awk -F "," '{
	printf strftime("%Y%m%d %H:%M:%S ",$1);
	for (i=2;i<=NF;i++)
		printf $i" "
	printf "<br>\n";
	}'

/bin/echo -e "<br>"
#/bin/cat /tmp/openvpncl/openvpn.conf
fi
