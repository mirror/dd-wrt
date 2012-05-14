#!/bin/sh
/bin/echo -e "Serverlog \n"
/bin/echo "log 500" | /usr/bin/nc 127.0.0.1 5002 | /bin/grep -v "^>" | /usr/bin/awk -F "," '{
	printf strftime("%Y%m%d %H:%M:%S ",$1);
	for (i=2;i<=NF;i++)
		printf $i" "
	printf "<br>\n";
	}'

/bin/echo -e " \n"
/bin/echo /usr/bin/cat /tmp/openvpn/openvpn.conf

/bin/echo -e " \n"
/bin/echo -e " \n"
/bin/echo -e " \n"

/bin/echo -e "Clientlog \n"
/bin/echo "log 500" | /usr/bin/nc 127.0.0.1 5001 | /bin/grep -v "^>" | /usr/bin/awk -F "," '{
	printf strftime("%Y%m%d %H:%M:%S ",$1);
	for (i=2;i<=NF;i++)
		printf $i" "
	printf "<br>\n";
	}'

/bin/echo -e " \n"
/bin/echo /usr/bin/cat /tmp/openvpncl/openvpncl.conf