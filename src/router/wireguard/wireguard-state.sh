#!/bin/sh
#parameter passing with oet number and peerkey
i=$1
p=$2

#VAR=$(/usr/bin/wg | sed "/oet${i}/,/interface/!d;/interface/d" | sed "/${p//\//\\\/}/,/peer/!d;/peer/d;/preshared/d;/endpoint/d;/allowed/d;/persistent/d")
VAR=$(/usr/bin/wg | sed "/${p//\//\\\/}/,/peer/!d" | grep -e 'latest' -e 'transfer' -e 'endpoint' )
if [[ ! -z "$VAR" ]]; then
	/bin/echo "${VAR//$'\n'/\\n}" | tr '\n' ' '
fi

