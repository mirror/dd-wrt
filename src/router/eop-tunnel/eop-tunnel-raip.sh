#!/bin/sh
nv=/usr/sbin/nvram
i=$1
#make sure WG/router is up and ready wait for date and start of dnsmasq or if tunnel is pingable
##sleep 35
# default sleep time
MINTIME=$($nv get wg_mintime)
[[ -z $MINTIME ]] && MINTIME=1
sleep $MINTIME
MAXTIME=$($nv get wg_maxtime) #0 = no maxtime
[[ -z $MAXTIME ]] && MAXTIME=65
SLEEPCT=$MINTIME
while [[ $(date +%Y) -lt 2019 ]]; do
	sleep 2
	SLEEPCT=$((SLEEPCT+2)) 
	if [[ $SLEEPCT -gt $MAXTIME && $MAXTIME -ne 0 ]]; then
		break
	fi
done
if [[ $SLEEPCT -gt $MAXTIME && $MAXTIME -ne 0 ]]; then
	logger -p user.info "WireGuard stopped waiting after $SLEEPCT seconds, trying to set routes for oet${i} anyway, is there a connection or NTP problem?"
else
	logger -p user.info "WireGuard waited $SLEEPCT seconds to set routes for oet${i}"
fi

peers=$((`$nv get oet${i}_peers` - 1))
for p in `seq 0 $peers`
	do
		# oet${i}_aip_rten${p} #nvram variable to allow routing of Allowed IP's, 1=add route
		if [[ $($nv get oet${i}_aip_rten${p}) -eq 1 ]] 
		then 
			#replace 0.0.0.0/ with 0.0.0.0/1,128.0.0.0/1
			#for aip in $($nv get oet${i}_aip${p} | sed "s/0.0.0.0\\/0/0.0.0.0\\/1,128.0.0.0\\/1/g" | sed "s/,/ /g") ; do
			for aip in $($nv get oet${i}_aip${p} | sed "s/,/ /g") ; do
				# check if PBR is set then skip default gateway
				if [ ! -z "$($nv get oet${i}_pbr | sed '/^[[:blank:]]*#/d')" ] && [ $aip = 0.0.0.0/1 -o $aip = 128.0.0.0/1 -o $aip = 0.0.0.0/0 ] ; then
					continue
				fi
				logger -p user.info "route $aip added via oet${i}" #debug
				ip route add $aip dev oet${i} >/dev/null 2>&1
			done
		fi			
	done
# end add routes

#egc add route to DNS server via tunnel
if [ ! -z "$($nv get oet${i}_dns | sed '/^[[:blank:]]*#/d')" ]; then
	for wgdns in $($nv get oet${i}_dns | sed "s/,/ /g") ; do
		ip route add $wgdns dev oet${i} >/dev/null 2>&1
		logger -p user.info "WireGuard DNS server $wgdns routed via oet${i}"
	done
fi
#end routing of DNS server

# egc PBR
# if there are multiple tunnels with PBR the local routes of the last tunnel are not copied to the routing table of the first tunnel
TID=$((20+$i))
while ip rule delete from 0/0 to 0/0 table $TID >/dev/null 2>&1; do true; done
ip route flush table $TID
if [ ! -z "$($nv get oet${i}_pbr | sed '/^[[:blank:]]*#/d')" ]; then
	echo $($nv get oet${i}_pbr), | while read -d ',' line; do
		line=$(echo $line | sed -e 's/^[[:space:]]*//')
		[ ${line:0:1} = "#" ] && continue
		#check if ip addres is used then add "from"
		case $line in
		 [0-9]*)
			ip rule add table $TID from $line
			;;
		 *)
			ip rule add table $TID $line
			;;
		esac
	done
	#add default route to alternate table
	ip route add default dev oet${i} table $TID
	#add local routes to alternate table
	ip route show | grep -Ev '^default |^0.0.0.0/1 |^128.0.0.0/1 ' | while read route; do
		ip route add $route table $TID
	done
	
# #Experimental
	# # if there are multiple tunnels with PBR the local routes of the last tunnel are not copied to the routing table of the preceding tunnel
	# # Preceding table
	# z=$(($i-1))
	# FTID=$(($TID-1))
	# if [ ! -z "$($nv get oet${z}_pbr | sed '/^[[:blank:]]*#/d')" ]; then
		# # Preceding table exists so let add the routes from this PBR table to the preceding PBR table
		# ip route show table $TID | grep -Ev '^default |^0.0.0.0/1 |^128.0.0.0/1 ' | while read route; do
			# ip route add $route table $FTID >/dev/null 2>&1
		# done
	# fi
# #end experimental

fi
# end PBR

# add DNS server
DNSTIME=$($nv get wg_dnstime)
[[ -z $DNSTIME ]] && DNSTIME=1
sleep $DNSTIME
if [[ ! -z "$($nv get oet${i}_dns | sed '/^[[:blank:]]*#/d')" ]]; then	# consider not adding when PBR is used
	# wait till dnsmasq is running
	SLEEPDNSCT=0
	MAXDNSTIME=65
	while [[ ! -f /tmp/resolv.dnsmasq ]]; do
		SLEEPDNSCT=$((SLEEPDNSCT+2))
		sleep 2
		if [[ $SLEEPDNS -gt $MAXDNSTIME && $MAXDNSTIME -ne 0 ]]; then
			logger -p user.info "WireGuard ERROR waiting $SLEEPDNSCT sec. for DNSMasq"
			break
		fi
	done
	logger -p user.info "WireGuard waited $SLEEPDNSCT sec. for DNSMasq"
	
	if [[ -f /tmp/resolv.dnsmasq_oet${i} ]]; then
		cat /tmp/resolv.dnsmasq_oet${i} | while read -r  remdns; do
			sed -i "/$remdns/d" /tmp/resolv.dnsmasq
		done
		rm /tmp/resolv.dnsmasq_oet${i}
	fi
	
	nvram_dns="$($nv get wg_get_dns)"
	for wgdns in $($nv get oet${i}_dns | sed "s/,/ /g") ; do
		# add to wg_get_dns to add when DNSMasq restarts
		nvram_dns=${nvram_dns//$wgdns/}
		nvram_dns="$wgdns $nvram_dns"
		sed -i "1s/^/nameserver $wgdns\n/" /tmp/resolv.dnsmasq
		echo "nameserver $wgdns" >> /tmp/resolv.dnsmasq_oet${i}
	done
	$nv set wg_get_dns="$nvram_dns"
fi
#end DNS server

# execute route-up script
if [[ ! -z "$($nv get oet${i}_rtupscript | sed '/^[[:blank:]]*#/d')" ]]; then
	# wait for availability of jffs
	sh /usr/bin/is-mounted.sh /jffs
	sleep 1
	sh $($nv get oet${i}_rtupscript) &
fi

ip route flush cache
# end route-up
