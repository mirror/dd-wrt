#!/bin/sh
[[ -n "$(nvram get wg_debug_raip)" ]] && { DEBUG=1; set -x; }
{
str_contains() { [ "${1//$2}" != "$1" ]; }
is_mac_address() { echo "$1" | grep -qE '^([0-9A-Fa-f]{2}:){5}([0-9A-Fa-f]{2})$'; }
is_ipv4() { echo "$1" | grep -qE '^((25[0-5]|2[0-4][0-9]|1?[0-9]{1,2})\.){3}(25[0-5]|2[0-4][0-9]|1?[0-9]{1,2})(/(3[0-2]|[12]?[0-9]))?$'; }
is_ipv6() { ! is_mac_address "$1" && str_contains "$1" ':'; }
nv=/usr/sbin/nvram
ipsetcmd="/usr/sbin/ipset"
tunnels=$($nv get oet_tunnels)
fset=$1
ipv6_en=$($nv get ipv6_enable)
WAN_IF=$(get_wanface)
[[ "$($nv get wan_proto)" != "disabled" ]] && GATEWAY="$($nv get wan_gateway)" || GATEWAY="$($nv get lan_gateway)"
GATEWAY6="$(ip -6 route show table main | awk '/default via/ { print $3;exit; }')"
restartdns=$($nv get wg_restart_dnsmasq); [[ -z $restartdns ]] && restartdns=0
for i in $(seq 1 $tunnels); do
	if [[ $($nv get oet${i}_en) -eq 1 ]]; then
		if [[ $($nv get oet${i}_proto) -eq 2 ]] && [[ $($nv get oet${i}_failgrp) -ne 1 || $($nv get oet${i}_failstate) -eq 2 ]]; then
			TID=$((20+i))
			FWMARKM="$((100*i))/0xffffff00"
			WGMACRT="/tmp/wg_mac_oet${i}"
			rm -f "$WGMACRT"
			# Start with PBR to make sure killswitch is working
			#if [ ! -z "$($nv get oet${i}_pbr | sed '/^[[:blank:]]*#/d')" ]; then
			if [[ $($nv get oet${i}_spbr) -ne 0 ]]; then
				logger -p user.info "WireGuard PBR via oet${i} table $TID"
				echo $($nv get oet${i}_spbr_ip), | while read -d ',' line; do
					[[ "${line:0:1}" = "#" ]] && continue
					line=$(eval echo $line) #use eval to execute nvram parameters
					if is_ipv4 "$line"; then
						#echo " [$line] = IPv4"
						logger -p user.info "WireGuard PBR $line:IPv4 via oet${i} table $TID"
						ip rule del prio 4${TID} from $line table $TID >/dev/null 2>&1
						ip rule add prio 4${TID} from $line table $TID
					elif is_ipv6 "$line"; then
						#echo " [$line] = IPv6"
						logger -p user.info "WireGuard PBR $line:IPv6 via oet${i} table $TID"
						if [[ $ipv6_en -eq 1 ]]; then
							ip -6 rule del prio 4${TID} table $TID from $line >/dev/null 2>&1
							ip -6 rule add prio 4${TID} table $TID from $line
						fi
					elif is_mac_address "$line"; then
						logger -p user.info "WireGuard PBR $line:MAC via oet${i} table $TID"
						#echo " [$line] = MAC"
						#iptables -t mangle -A PREROUTING -m mac --mac-source "$line" -j MARK --set-mark $TID
						echo "$line" >> "$WGMACRT"
						ip rule del prio 4${TID} fwmark $FWMARKM table $TID >/dev/null 2>&1
						ip rule add prio 4${TID} fwmark $FWMARKM table $TID
						if [[ $ipv6_en -eq 1 ]]; then
							ip -6 rule del prio 4${TID} fwmark $FWMARKM table $TID >/dev/null 2>&1
							ip -6 rule add prio 4${TID} fwmark $FWMARKM table $TID
						fi
					else
						#echo " [$line] = OTHER"
						logger -p user.info "WireGuard PBR $line:text via oet${i} table $TID"
						ip rule del prio 4${TID} $line table $TID >/dev/null 2>&1
						ip rule add prio 4${TID} $line table $TID
						if [[ $ipv6_en -eq 1 ]]; then
							ip -6 rule del prio 4${TID} $line table $TID >/dev/null 2>&1
							ip -6 rule add prio 4${TID} $line table $TID
						fi
					fi
				done
				# due to a bug in 'ip rule' command rogue 'from all' can be added for non valid ip addresses so remove that:
				for z in $(ip rule | grep "from all lookup $TID" | cut -d: -f1); do 
					ip rule del prior $z
					logger -p user.err "WireGuard ERROR: non-valid IPv4 address in PBR, tunnel oet${i}"
				done
				if [[ $ipv6_en -eq 1 ]]; then
					for z in $(ip -6 rule | grep "from all lookup $TID" | cut -d: -f1); do 
						ip -6 rule del prior $z
						logger -p user.err "WireGuard ERROR: non-valid IPv6 address in PBR, tunnel oet${i}"
					done
				fi
				if [[ $($nv get oet${i}_killswitch) -eq 1 && $($nv get oet${i}_spbr) -eq 1 ]]; then
					logger -p user.info "WireGuard Killswitch on PBR activated for oet${i}"
					ip route add prohibit default table $TID >/dev/null 2>&1
					if [[ $ipv6_en -eq 1 ]]; then
						ip -6 route add prohibit default table $TID $line >/dev/null 2>&1
					fi
				else
					ip route add default via $GATEWAY table $TID >/dev/null 2>&1
					if [[ $ipv6_en -eq 1 ]]; then
						ip -6 route add default via $GATEWAY6 dev $WAN_IF table $TID $line >/dev/null 2>&1
					fi
				fi
				ip route flush cache
			fi
			# Destination based routing
			if [[ $($nv get oet${i}_dpbr) -ne 0 ]]; then
				#add IPSET
				if [[ ! -z "$($nv get oet${i}_ipsetfile | sed '/^[[:blank:]]*#/d')" ]] && [[ "$($nv get oet${i}_dpbr)" != "0" ]]; then
					restartdns=1
					#ipset -! -N $(basename $($nv get oet${i}_ipsetfile)) hash:ip
					IPSET_F="$($nv get oet${i}_ipsetfile)"
					IPSET_F6="${IPSET_F}6"
					#IPSET="$(basename $IPSET_F)"
					IPSET="${IPSET_F##*/}"
					IPSET6="${IPSET}6"
					FWMARK="${i}/0xff"
					if [[ $($nv get oet${i}_ipsetsave) -eq 1 ]]; then
						if is-mounted $(dirname $IPSET_F) 25 && [[ -s $IPSET_F ]]; then
							$ipsetcmd restore -! < "${IPSET_F}"
							logger -p user.info "WireGuard IPSET: $IPSET restored from $IPSET_F for oet${i}"
						else
							logger -p user.err "WireGuard IPSET: oet${i} $IPSET_F is missing or empty, creating new $IPSET"
							$ipsetcmd -N "${IPSET}" hash:net family inet >/dev/null 2>&1
							[[ $ipv6_en -eq 1 ]] && $ipsetcmd -N "${IPSET6}" hash:net family inet6 >/dev/null 2>&1
						fi
					else
						$ipsetcmd -N "$IPSET" hash:net family inet >/dev/null 2>&1
						[[ $ipv6_en -eq 1 ]] && $ipsetcmd -N "${IPSET6}" hash:net family inet6
						logger -p user.info "WireGuard IPSET: $IPSET created for oet${i}"
					fi
					#make tables and rule
					if [[ $($nv get oet${i}_dpbr) -eq 2 ]];then 
						TABLE=2$TID
						ip route add default via "${GATEWAY}" dev $WAN_IF table $TABLE >/dev/null 2>&1
						[[ $ipv6_en -eq 1 ]] && ip -6 route add default via "${GATEWAY6}" dev $WAN_IF table $TABLE >/dev/null 2>&1
					else 
						TABLE=1$TID
						ip route add default dev oet${i} table $TABLE >/dev/null 2>&1
						[[ $ipv6_en -eq 1 ]] && ip -6 route add default dev oet${i} table $TABLE >/dev/null 2>&1
					fi
					#ip rule del prio $TID table $TABLE fwmark $FWMARK >/dev/null 2>&1
					ip rule add prio $TID table $TABLE fwmark $FWMARK
					if [[ $ipv6_en -eq 1 ]] ; then
						#ip -6 rule del prio $TID table $TABLE fwmark $FWMARK >/dev/null 2>&1
						ip -6 rule add prio $TID table $TABLE fwmark $FWMARK
					fi
					logger -p user.info "WireGuard IPSET: $IPSET fwmark:$FWMARK set to table:$TABLE"
				fi
				#WGDELDPBR="/tmp/wgdeldpbr_oet${i}"
				WGDPBRIP="/tmp/wgdpbrip_oet${i}"
				rm $WGDPBRIP >/dev/null 2>&1
				for dpbr in $($nv get oet${i}_dpbr_ip | sed "s/,/ /g"); do
					if [[ ${dpbr:0:1} != "#" ]]; then
						if [[ "$dpbr" != "${dpbr#*[0-9].[0-9]}" || "$dpbr" != "${dpbr#*:[0-9a-fA-F]}" ]]; then
							echo "$dpbr" >> $WGDPBRIP
						#elif [ "$dpbr" != "${dpbr#*:[0-9a-fA-F]}" ]; then
						#	logger -p user.warning "WireGuard $dpbr is IPv6, not yet implemented"
						else
							if ! nslookup "$dpbr" >/dev/null 2>&1; then
								logger -p user.err "WireGuard ERROR domain $dpbr not found"
							elif [[ $ipv6_en -eq 1 ]]; then
								nslookup "$dpbr" 2>/dev/null | awk '/^Name:/,0 {if (/^Addr[^:]*:/) print $3}' >> $WGDPBRIP
							else
								nslookup "$dpbr" 2>/dev/null | awk '/^Name:/,0 {if (/^Addr[^:]*: [0-9]{1,3}\./) print $3}' >> $WGDPBRIP
							fi
						fi
					fi
				done
				if [[ -s $WGDPBRIP ]]; then
					while read dpbrip; do
						if [[ $($nv get oet${i}_dpbr) -eq 1 ]]; then
							ip route add $dpbrip dev oet${i} >/dev/null 2>&1
						elif [[ $($nv get oet${i}_dpbr) -eq 2 ]]; then
							if [[ "$dpbrip" != "${dpbrip#*[0-9].[0-9]}" ]]; then
								ip route add $dpbrip via $GATEWAY >/dev/null 2>&1
							else
								if [[ $ipv6_en -eq 1 ]]; then
									ip -6 route add $dpbrip via $GATEWAY6 dev $WAN_IF >/dev/null 2>&1
								fi
							fi
						fi
					done < $WGDPBRIP
				fi
			fi
			#add routes for allowed IP's
			peers=$(($($nv get oet${i}_peers) - 1))
			for p in $(seq 0 $peers); do
					# oet${i}_aip_rten${p} #nvram variable to allow routing of Allowed IP's, 1=add route
					if [[ $($nv get oet${i}_aip_rten${p}) -eq 1 ]]; then
						#replace 0.0.0.0/0 with 0.0.0.0/1,128.0.0.0/1 and ::/0 with ::/1, 8000::/1
						#for aip in $($nv get oet${i}_aip${p} | sed "s/0.0.0.0\\/0/0.0.0.0\\/1,128.0.0.0\\/1/g" | sed "s/::\\/0/::\\/1,8000::\\/1/g" | sed "s/,/ /g"); do
						for aip in $($nv get oet${i}_aip${p} | sed "s/0.0.0.0\\/0/0.0.0.0\\/1,128.0.0.0\\/1/g;s/::\\/0/::\\/1,8000::\\/1/g;s/::0\\/0/::\\/1,8000::\\/1/g" | sed "s/,/ /g"); do
							# check if PBR is set then skip default gateway, if default route without endpoint then warning
							if [[ ! -z "$(echo $aip | sed -e 's/\/128//g' | grep -e '/0\|/1')" ]]; then
								if [[ $($nv get oet${i}_spbr) -eq 1 ]]; then
									continue
								elif [ $($nv get oet${i}_endpoint${p}) -eq 0 ]; then
									logger -p user.err "WireGuard ERROR default route detected without endpoint for peer $($nv get oet${i}_namep${p}) in tunnel oet${i}, consult the manual!"
								fi
							fi
							case $aip in
							 *[0-9].*) #IPv4
								ip route add $aip dev oet${i}
								logger -p user.info "WireGuard IPv4 route $aip added via oet${i}"
								;;
							 *[0-9a-fA-F:]:*) #IPv6
								if [[ "$ipv6_en" = "1" ]]; then
									ip -6 route add $aip dev oet${i}
									logger -p user.info "WireGuard IPv6 route $aip added via oet${i}"
								fi
								;;
							 *)
								logger -p user.info "WireGuard wrong AllowedIPs $aip in tunnel oet${i}, peer $p "
								;;
							esac
						done
					fi
			done
			#add route to DNS server via tunnel
			if [ ! -z "$($nv get oet${i}_dns | sed '/^[[:blank:]]*#/d')" ]; then
				for wgdns in $($nv get oet${i}_dns | sed "s/,/ /g") ; do
					ip route add $wgdns dev oet${i} >/dev/null 2>&1
					logger -p user.info "WireGuard DNS server $wgdns routed via oet${i}"
				done
			fi
			# add DNS server
			if [[ ! -z "$($nv get oet${i}_dns | sed '/^[[:blank:]]*#/d')" ]]; then
				DNSTIME=$($nv get wg_dnstime)
				[[ -z $DNSTIME ]] && DNSTIME=0 || sleep $DNSTIME
				SLEEPDNSCT=$DNSTIME
				MAXDNSTIME=45
				while [[ ! -f /tmp/resolv.dnsmasq ]]; do
					SLEEPDNSCT=$((SLEEPDNSCT+2))
					sleep 2
					if [[ $SLEEPDNSCT -gt $MAXDNSTIME && $MAXDNSTIME -ne 0 ]]; then
						logger -p user.err "WireGuard ERROR max. waiting $SLEEPDNSCT sec. for DNSMasq, check DNSMasq settings!"
						break
					fi
				done
				logger -p user.info "WireGuard waited $SLEEPDNSCT sec. for DNSMasq"
				# check if other instances did not set already
				if [[ -e /tmp/resolv.dnsmasq_oet ]]; then
					logger -p user.warning "WireGuard DNS WARNING, already set when running oet${i} will overwrite"
					#consider adding sleep i otherwise this tunnel will change resolv.dnsmasq within one second and it will not get polled
					#sleep i
				fi
				#if PBR is used with split DNS
				if [[ $($nv get oet${i}_spbr) -eq 1 && $($nv get oet${i}_dnspbr) -eq 1 && ! -z "$($nv get oet${i}_dns | sed '/^[[:blank:]]*#/d')" ]]; then
					logger -p user.info "WireGuard DNS Split tunnel for PBR oet${i}"
				else
					cp -n /tmp/resolv.dnsmasq /tmp/resolv.dnsmasq_oet
					rm -f /tmp/resolv.dnsmasq
					for wgdns in $($nv get oet${i}_dns | sed "s/,/ /g"); do
						#nvram_dns="$wgdns $nvram_dns"
						nvram_dns="$nvram_dns $wgdns"
						echo -e "nameserver $wgdns" >> /tmp/resolv.dnsmasq
					done
					$nv set wg_get_dns="$nvram_dns"
					#if [[ $($nv get wgtouchdns) -eq 1 ]]; then
					#	sleep 1
					#	touch /tmp/resolv.dnsmasq
					#fi
				fi
			fi
			#routing of Split DNS
			if [[ $($nv get oet${i}_dnspbr) -eq 1 ]]; then
				dns4=$($nv get oet${i}_dns4)
				dns6=$($nv get oet${i}_dns6)
				WGDNSRT="/tmp/wgdnsrt_oet${i}"
				rm $WGDNSRT >/dev/null 2>&1
				if [[ $($nv get oet${i}_spbr) -eq 2 ]]; then
					# route dnsservers via WAN
					ip route add $dns4 via $GATEWAY >/dev/null 2>&1
					if [[ $ipv6_en -eq 1 ]]; then
						ip -6 route add $dns6 via $GATEWAY6 dev $WAN_IF >/dev/null 2>&1
						echo "ip -6 route del $dns6 via $GATEWAY6 dev $WAN_IF " >> $WGDNSRT
					fi
					echo "ip route del $dns4 via $GATEWAY " >> $WGDNSRT
					
				elif [[ $($nv get oet${i}_spbr) -eq 1 ]]; then
					# route dnsservers via tunnel no need to delete as interface is taken down
					ip route add $dns4 dev oet${i}
					if [[ $ipv6_en -eq 1 ]]; then
						ip -6 route add $dns6 dev oet${i}
					fi
				fi
			fi
			# add routes to PBR table
			if [[ $($nv get oet${i}_spbr) -eq 1 ]]; then
				#add default routes
				ip route add 0.0.0.0/1 dev oet${i} table $TID >/dev/null 2>&1
				ip route add 128.0.0.0/1 dev oet${i} table $TID >/dev/null 2>&1
				if [[ $ipv6_en -eq 1 ]]; then
					ip -6 route add ::/1 dev oet${i} table $TID >/dev/null 2>&1
					ip -6 route add 8000::/1 dev oet${i} table $TID >/dev/null 2>&1
				fi
			fi
			# check how many tunnels, for last active tunnel copy local routes to all existing PBR tables
			for x in $(seq $($nv get oet_tunnels) -1 1); do
				if [[ $($nv get oet${x}_en) -eq 1 ]] && [[ $($nv get oet${x}_failgrp) -ne 1 || $($nv get oet${x}_failstate) -eq 2 ]]; then
					break
				fi
			done
			if [[ $i -eq $x ]]; then #this is the last active tunnel now add local routes to all tables
				#Allow time to have DNS lookup being done and routes are added to the main table # not necessary as we set lock
				#sleep 5
				maxtable=$((20 + $x))
				# alternatively to research
				# ip -4 rule add table main suppress_prefixlength 0 >/dev/null 2>&1
				# ip -4 rule add table main suppress_prefixlength 1 >/dev/null 2>&1
				# ip -6 rule add table main suppress_prefixlength 0 >/dev/null 2>&1
				# ip -6 rule add table main suppress_prefixlength 1 >/dev/null 2>&1
				for FTID in $(seq 21 1 $maxtable); do
					if [[ ! -z  "$(ip route show table $FTID)" ]]; then
						logger -p user.info "WireGuard adding local routes to table $FTID"
						ip route show | grep -Ev '^default |^0.0.0.0/1 |^128.0.0.0/1 ' | while read route; do
							ip route add $route table $FTID >/dev/null 2>&1
						done
						if [[ $ipv6_en -eq 1 ]]; then
							#ip -6 route show | grep -Ev '^default |^::/1 |^8000::/1 |^::/2 |^4000::/2 |^8000::/2 |^c000::/2 ' | sed -E 's/ proto kernel//; s/ expires -?[0-9]+sec//; s/  +/ /g' while read route; do
							# remove only sec but leave expire: ip -6 route show | grep -Ev '^default |^::/1 |^8000::/1 |^::/2 |^4000::/2 |^8000::/2 |^c000::/2 ' | sed -E 's/ proto kernel//; s/( expires -?[0-9]+)sec/\1/;'
							ip -6 route show | grep -Ev '^default |^::/1 |^8000::/1 |^::/2 |^4000::/2 |^8000::/2 |^c000::/2 ' | while read route; do
								# remove proto kernel and expires sec value, sec is not allowed
								route="$(echo "$route" | sed -E 's/ proto kernel//; s/ expires -?[0-9]+sec//')"
								ip -6 route add $route table $FTID >/dev/null 2>&1
							done
						fi
					fi
				done
				# Consider also restarting the firewall when the last tunnel is done
				# /etc/config/eop-tunnel.firewall  >/dev/null 2>&1
			fi
			# execute route-up script
			if [[ ! -z "$($nv get oet${i}_rtupscript | sed '/^[[:blank:]]*#/d')" ]]; then
				# wait for availability of directory
				sh /usr/bin/is-mounted.sh $(dirname "$($nv get oet${i}_rtupscript)")
				sleep 1
				sh $($nv get oet${i}_rtupscript) &
			fi
			ip route flush cache
			[[ $ipv6_en -eq 1 ]] && ip -6 route flush cache
			# execute watchdog script
			if [[ $($nv get oet${i}_failstate) -eq 2 || $($nv get oet${i}_wdog) -eq 1 ]]; then
				# only start if not already running
				if ! ps | grep -q "[w]ireguard-fwatchdog\.sh $i"; then
					logger -p user.info "WireGuard wireguard-fwatchdog $i not running yet"
				else
					logger -p user.info "WireGuard wireguard-fwatchdog $i already running will be killed first"
					ps | grep "[w]ireguard-fwatchdog\.sh $i" | awk '{print $1}' | xargs kill -9 >/dev/null 2>&1
				fi
				# send tunnelnumber, sleeptime (sec), ping address, reset (1=Yes)
				PINGTIME=$($nv get oet${i}_failtime)
				[[ -z "$PINGTIME" ]] && PINGTIME="30"
				PINGIP=$($nv get oet${i}_failip)
				[[ -z "$PINGIP" ]] && PINGIP="8.8.8.8"
				sh /usr/bin/wireguard-fwatchdog.sh $i $PINGTIME $PINGIP $fset &
			fi
			#restart dnsmasq when the last tunnel has been setup to reread resolv.dnsmasq, due to a bug this does not happen on change of resolv.dnsmasq
			# for now disabled waiting for DNSMasq to be repaired 2.86 works but 287test4 is buggy and does not want to replace a DNS resolver which is already in memory
			#restartdns=$($nv get wg_restart_dnsmasq); [[ -z $restartdns ]] && restartdns=0 #moved to begin
			#[[ $i -eq $x && $restartdns -ge 1 ]] && { sleep $restartdns; restart dnsmasq; }
			# restart dnsmasq if ipset is set
			[[ $i -eq $x && $restartdns -ge 1 ]] && { logger -p user.info "WireGuard: DNSMasq restarted due to active ipset"; restart dnsmasq; } &
			# reload but not restart works of resolv.dnsmasq only if "no-poll" is set
			#[[ $i -eq $x && $restart -ge 1 ]] && { sleep $restart; kill -1 $(pidof dnsmasq); }
		fi
	fi
done
# LOCK released in eop-tunnel-firewall.sh
#release_lock
/usr/bin/eop-tunnel-firewall.sh &
#( nohup /usr/bin/eop-tunnel-firewall.sh & ) >/dev/null
} 2>&1 | logger $([ ${DEBUG+x} ] && echo '-p user.debug') \
    -t $(echo $(basename $0) | grep -Eo '^.{0,23}')[$$] &
