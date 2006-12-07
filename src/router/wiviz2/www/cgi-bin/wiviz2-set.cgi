#!/bin/sh
httpd -d "$QUERY_STRING" > /tmp/wiviz2-cfg
killall -USR2 wiviz2
