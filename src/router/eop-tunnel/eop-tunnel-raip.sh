#!/bin/sh
nv=/usr/sbin/nvram

#debug
deb=$(nvram get console_debug)
if [[ $deb -eq 1 ]]; then
	set -x
fi

i=$1
# egc Start with PBR to make sure killswitch is working
TID=$((20+$i))
if [ ! -z "$($nv get oet${i}_pbr | sed '/^[[:blank:]]*#/d')" ]; then
	logger -p user.info "WireGuard PBR via oet${i} table $TID"
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
	# due to a bug in 'ip rule' command rogue 'from all' can be added for non valid ip addresses so remove that:
	for z in $(ip rule | grep "from all lookup $TID" | cut -d: -f1); do 
		ip rule del prior $z
		logger -p user.info "WireGuard ERROR: non-valid IP address in PBR, tunnel oet${i}"
	done
	if [[ $($nv get oet${i}_killswitch) -eq 1 ]]; then
		logger -p user.info "WireGuard Killswitch on PBR activated for oet${i}"
		ip route add prohibit default table $TID
	else
		ip route add default via $(nvram get wan_gateway) table $TID
	fi
	ip route flush cache
fi
# end PBR
#make sure WG/router is up and ready wait for date and start of dnsmasq or if tunnel is pingable
# debug todo check if nvram get ntp_succes and/or ntp_done can be used to see if time is set
#logger -p user.info "WireGuard debug st start of time check: ntp_success: $(nvram get ntp_success); ntp_done:$(nvram get ntp_done) "
MINTIME=$($nv get wg_mintime)
[[ -z $MINTIME ]] && MINTIME=1
sleep $MINTIME
MAXTIME=$($nv get wg_maxtime) #0 = no maxtime
[[ -z $MAXTIME ]] && MAXTIME=90
SLEEPCT=$MINTIME
#while [[ $(nvram get ntp_done) -ne 1 ]]; do
while [[ $(date +%Y) -lt 2020 ]]; do
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
# debug
#logger -p user.info "WireGuard debug at end of time check: ntp_success: $(nvram get ntp_success); ntp_done:$(nvram get ntp_done) "
peers=$((`$nv get oet${i}_peers` - 1))
for p in `seq 0 $peers`; do
		# oet${i}_aip_rten${p} #nvram variable to allow routing of Allowed IP's, 1=add route
		if [[ $($nv get oet${i}_aip_rten${p}) -eq 1 ]] 
		then 
			#replace 0.0.0.0/0 with 0.0.0.0/1,128.0.0.0/1
			for aip in $($nv get oet${i}_aip${p} | sed "s/0.0.0.0\\/0/0.0.0.0\\/1,128.0.0.0\\/1/g" | sed "s/,/ /g") ; do
				# check if PBR is set then skip default gateway, if default route without endpoint then warning
				if [[ ! -z "$(echo $aip |  grep -e '/0\|/1')" ]]; then
					if [ ! -z "$($nv get oet${i}_pbr | sed '/^[[:blank:]]*#/d')" ]; then
						continue
					elif [ $($nv get oet${i}_endpoint${p}) -eq 0 ]; then
						logger -p user.info "WireGuard WARNING default route detected without endpoint for peer $($nv get oet${i}_namep${p}) in tunnel oet${i}, consult the manual!"
					fi
				fi
				logger -p user.info "WireGuard route $aip added via oet${i}"
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
# egc add routes to PBR table
if [ ! -z "$($nv get oet${i}_pbr | sed '/^[[:blank:]]*#/d')" ]; then
	#add default routes
	ip route add 0.0.0.0/1 dev oet${i} table $TID >/dev/null 2>&1
	ip route add 128.0.0.0/1 dev oet${i} table $TID>/dev/null 2>&1
fi
# check how many tunnels, for last active tunnel copy local routes to all existing PBR tables
#tunnels=$(nvram get oet_tunnels)
for x in $(seq $(nvram get oet_tunnels) -1 1); do 
	if [[ $(nvram get oet${x}_en) -eq 1 ]]; then
		#lasttunnel=$x
		break
	fi
done
if [[ $i -eq $x ]]; then #this is the last active tunnel now add local routes to all tables
	maxtable=$((20 + $x))
	# logger -p user.info "WireGuard maxtunnel $x"
	for FTID in $(seq 21 1 $maxtable); do
		if [[ ! -z  "$(ip route show table $FTID)" ]]; then
			logger -p user.info "WireGuard adding local routes to table $FTID"
			ip route show | grep -Ev '^default |^0.0.0.0/1 |^128.0.0.0/1 ' | while read route; do
				ip route add $route table $FTID >/dev/null 2>&1
			done
		fi
	done
fi
# end PBR
# add DNS server
DNSTIME=$($nv get wg_dnstime)
[[ -z $DNSTIME ]] && DNSTIME=1
sleep $DNSTIME
if [[ ! -z "$($nv get oet${i}_dns | sed '/^[[:blank:]]*#/d')" ]]; then	# consider not adding when PBR is used
	# wait till dnsmasq is running
	SLEEPDNSCT=0
	MAXDNSTIME=45
	while [[ ! -f /tmp/resolv.dnsmasq ]]; do
		SLEEPDNSCT=$((SLEEPDNSCT+2))
		sleep 2
		if [[ $SLEEPDNS -gt $MAXDNSTIME && $MAXDNSTIME -ne 0 ]]; then
			logger -p user.info "WireGuard ERROR max. waiting $SLEEPDNSCT sec. for DNSMasq"
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
		if [[ -s /tmp/resolv.dnsmasq ]]; then
			#is not empty insert else append
			sed -i "1s/^/nameserver $wgdns\n/" /tmp/resolv.dnsmasq
		else
			echo -e "nameserver $wgdns" >> /tmp/resolv.dnsmasq
		fi
		echo "nameserver $wgdns" >> /tmp/resolv.dnsmasq_oet${i}
	done
	$nv set wg_get_dns="$nvram_dns"
fi
#end DNS server
# execute route-up script
if [[ ! -z "$($nv get oet${i}_rtupscript | sed '/^[[:blank:]]*#/d')" ]]; then
	# wait for availability of directory
	sh /usr/bin/is-mounted.sh $(dirname "$($nv get oet${i}_rtupscript)")
	sleep 1
	sh $($nv get oet${i}_rtupscript) &
fi
ip route flush cache
# end route-up
