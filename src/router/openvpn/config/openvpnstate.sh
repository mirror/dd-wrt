echo "`/bin/echo "state" | /usr/bin/nc 127.0.0.1 5001 |awk -F"," '{ print $2}'`: `/bin/echo "state" | /usr/bin/nc 127.0.0.1 5001 |awk -F"," '{print $3}'` - "
echo "Local Address: `/bin/echo "state" | /usr/bin/nc 127.0.0.1 5001 |awk -F"," '{print $4}'` - "
echo "Remote Address: `ifconfig tun0 |awk '{print $3}'|grep P-t-P|awk -F ":" '{print $2}'`"
