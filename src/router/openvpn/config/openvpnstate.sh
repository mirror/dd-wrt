#!/bin/sh
echo -e "Server: \n"
echo -e "`/bin/echo "state" | /usr/bin/nc 127.0.0.1 5002 |awk -F"," '{ print $2}'`: `/bin/echo "state" | /usr/bin/nc 127.0.0.1 5001 |awk -F"," '{print $3}'` \n"
echo -e "Local Address: `/bin/echo "state" | /usr/bin/nc 127.0.0.1 5001 |awk -F"," '{print $4}'` \n"
echo -e "Remote Address: `ifconfig tun0 |awk '{print $3}'|grep P-t-P|awk -F ":" '{print $2}'`"

echo -e "Client: \n"
echo -e "`/bin/echo "state" | /usr/bin/nc 127.0.0.1 5001 |awk -F"," '{ print $2}'`: `/bin/echo "state" | /usr/bin/nc 127.0.0.1 5001 |awk -F"," '{print $3}'` \n"
echo -e "Local Address: `/bin/echo "state" | /usr/bin/nc 127.0.0.1 5001 |awk -F"," '{print $4}'` \n"
echo -e "Remote Address: `ifconfig tun0 |awk '{print $3}'|grep P-t-P|awk -F ":" '{print $2}'` \n" 
