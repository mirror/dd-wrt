#!/bin/sh
uqmi -d $1 --set-client-id wds,`cat /tmp/qmi-clientid` --keep-client-id wds --get-signal-info | \
tr -d '"' | tr -d ',' | \
awk -F ":" '
function ltrim(s) { sub(/^[ \t\r\n]+/, "", s); return s }
function rtrim(s) { sub(/[ \t\r\n]+$/, "", s); return s }
function trim(s) { return rtrim(ltrim(s)); }
function ws(s) {ret=s; for (i=0;i<8-length(s);i++) ret=ret" "; return ret; }
{
	out[trim($1)]=trim($2)
} END {
printf "nvram set wan_3g_signal=\"<pre>"
if (out["type"]=="lte")  {
	print "System mode\tLTE"; 
	print ws("SNR")"\t" out["snr"];
	print ws("RSSI") "\t" out["rssi"];
	print ws("RSRQ") "\t" out["rsrq"];
	print ws("RSRP") "\t" out["rsrp"];
}
else {
	print "System mode\t" toupper(out["type"]); 
	if (out["signal"])
		print ws("RSSI")"\t" out["signal"];
	else
		print ws("RSSI")"\t" out["rssi"];
} 
}' >/tmp/qmistatus-temp.$$

printf "IMEI:\t" >>/tmp/qmistatus-temp.$$
uqmi -d $1 --set-client-id wds,`cat /tmp/qmi-clientid` --keep-client-id wds --get-imei | \
	tr -d '"' >>/tmp/qmistatus-temp.$$
printf "IMSI:\t" >>/tmp/qmistatus-temp.$$
uqmi -d $1 --set-client-id wds,`cat /tmp/qmi-clientid` --keep-client-id wds --get-imsi | \
	tr -d '"' >>/tmp/qmistatus-temp.$$
printf "Number:\t" >>/tmp/qmistatus-temp.$$
uqmi -d $1 --set-client-id wds,`cat /tmp/qmi-clientid` --keep-client-id wds --get-msisdn | \
	tr -d '"' >>/tmp/qmistatus-temp.$$

echo "</pre>\"" >>/tmp/qmistatus-temp.$$
sh /tmp/qmistatus-temp.$$ >/dev/null 2>&1         
rm /tmp/qmistatus-temp.$$   
