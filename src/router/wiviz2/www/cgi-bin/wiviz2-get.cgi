#!/bin/sh

WIVIZ_PATH=/usr/sbin/wiviz2

echo Content-type: text/html
echo 
killall -USR1 wiviz2 >/dev/null 2>&1
if [ 0 -ne $? ]
 then #### Wi-Viz daemon not running, start it
	if [ \! -x $WIVIZ_PATH ]
		then
		chmod 755 $WIVIZ_PATH
		fi
  $WIVIZ_PATH >/dev/null </dev/null 2>&1 &
  killall -USR1 wiviz2 > /dev/null
 fi
echo "<html><head><script language='JavaScript1.2'>"
cat /tmp/wiviz2-dump
echo "</script></head><body>This file contains JavaScript.</body></html>"
