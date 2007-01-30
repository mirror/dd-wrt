#!/bin/sh
# The Milkfish - SIP Tracer
# Author: budrus@users.berlios.de
# Website: milkfish.org
# License: GPL
# Date: 060930

[ $(nvram get milkfish_siptrace) = "off" ] && exit 0
if [ -z $1 ]; then
 while true
   do
   read input
   echo $input >> /tmp/sip_trace.tmp
   if [ -z $input ]; then
     [ -e /var/log/sip_trace.log ] && head -n500 /var/log/sip_trace.log >> /tmp/sip_trace.tmp
     mv /tmp/sip_trace.tmp /var/log/sip_trace.log
     exit 0
   fi
 done
fi 