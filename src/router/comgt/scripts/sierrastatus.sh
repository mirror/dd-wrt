#!/bin/sh
if [ -f /tmp/.sierrastatus.sh.lock ]
then
	if [ x$3 != xignorelock ]
	then
		echo "SIERRASTATUS LOCK EXISTS DIAL IN PROGRESS" | logger
		exit 0
	fi
fi
comgt -s -d $1 /etc/comgt/sierrastatus.comgt 2>/dev/null | tr -d '"' | awk -F"\t" '
function ltrim(s) { sub(/^[ \t\r\n]+/, "", s); return s }
function rtrim(s) { sub(/[ \t\r\n]+$/, "", s); return s }
function trim(s) { return rtrim(ltrim(s)); }
function ws(s) {ret=s; for (i=0;i<8-length(s);i++) ret=ret" "; return ret; }
BEGIN {
	lc=0;
	printf "nvram set wan_3g_signal=\"<pre>"
}
{
if (lc >1) {
	count=split ($1,a,":"); 
	if (count >1) {
		if (trim(a[1]) != "Current Time" && 
			trim(a[1]) != "Bootup Time" )  {
		printf ws(trim(a[1]))"\t";
		printf trim(a[2]);
		printf "\n";
		}
	}
	if ($3 != "")
		count=split ($3,b,":"); 
	else
		count=split ($2,b,":"); 
	if (count >1) {
		printf ws(trim(b[1]))"\t";
		printf trim(b[2]);
		if (trim(b[1]) == "Temperature")
			printf " C";
		printf "\n";
	}
}
lc++;
} END {
}
' >/tmp/sierrastatus.$$
printf "IMEI:   \t" >>/tmp/sierrastatus.$$
export COMGTATC="AT+CGSN" ; comgt -s -d $1 /etc/comgt/atcommand.comgt  | \
	grep ^[0-9] | tr -d '"' >>/tmp/sierrastatus.$$
printf "ICCID:   \t" >>/tmp/sierrastatus.$$
export COMGTATC="AT!ICCID?" ; comgt -s -d $1 /etc/comgt/atcommand.comgt  | \
	grep "^ICCID:" | awk '{print $2}'| tr -d '"' >>/tmp/sierrastatus.$$
printf "IMSI:   \t" >>/tmp/sierrastatus.$$
export COMGTATC="AT+CIMI" ; comgt -s -d $1 /etc/comgt/atcommand.comgt  | \
	grep ^[0-9] | tr -d '"' >>/tmp/sierrastatus.$$
printf "Number:  \t" >>/tmp/sierrastatus.$$
export COMGTATC="AT+CNUM" ; comgt -s -d $1 /etc/comgt/atcommand.comgt  | \
	grep "^+CNUM" | tr -d '"' | awk -F "," '{print $2}'  | \
	tr -d '"' >>/tmp/sierrastatus.$$
printf "</pre>\"" >>/tmp/sierrastatus.$$ 

sh /tmp/sierrastatus.$$ >/dev/null 2>&1
#cat /tmp/sierrastatus.$$
rm /tmp/sierrastatus.$$
if [ x$2 = xdip ]
then
  CURIP=`comgt -s -d $1 /etc/comgt/sierrawanip.comgt 2>/dev/null | grep "^!SCPADDR" | cut -c 14- | awk -F "\"" '{print $1}'`
  WWANIP=`ifconfig wwan0 | grep "inet addr" | cut -c 21- | awk '{print $1}'`
  if [ x${CURIP} = x"0.0.0.0" ]
  then
    echo "disconnected"
    echo 0 >/tmp/sierradipstatus
  else
    echo 1 >/tmp/sierradipstatus
  fi
  if [ x${CURIP} != x${WWANIP} ]
  then
    echo "ip changed"
    echo 0 >/tmp/sierradipstatus
  fi
fi
echo "SIERRASTATUS CALLED" | logger
