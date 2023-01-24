#!/bin/sh
nv=/usr/sbin/nvram
tunnels=$($nv get oet_tunnels)

fset=$1

MINTIME=$($nv get wg_mintime)
[[ -z $MINTIME ]] && MINTIME=1
MAXTIME=$($nv get wg_maxtime) #0 = no maxtime
[[ -z $MAXTIME ]] && MAXTIME=105

LOCK="/tmp/oet.lock"
acquire_lock() { logger -p user.info "WireGuard acquiring $LOCK for $$"; while ! mkdir $LOCK >/dev/null 2>&1; do sleep 2; done; logger -p user.info "WireGuard $LOCK acquired for $$"; }
release_lock() { rmdir $LOCK >/dev/null 2>&1; logger -p user.info "WireGuard released $LOCK for $$"; }

trap '{ release_lock; logger -p user.info "WireGuard script $0 running on oet${i} fatal error"; exit 1; }' SIGHUP SIGINT SIGTERM

waitfortime () {
	#set lock to make sure earlier tunnels are finished
	acquire_lock
	#debug todo check if nvram get ntp_succes and/or ntp_done can be used to see if time is set
	#logger -p user.info "WireGuard debug st start of time check: ntp_success: $($nv get ntp_success); ntp_done:$($nv get ntp_done)"
	SLEEPCT=$MINTIME
	#while [[ $($nv get ntp_done) -ne 1 ]]; do
	while [[ $(date +%Y) -lt 2021 ]]; do
		sleep 2
		SLEEPCT=$((SLEEPCT+2))
		if [[ $SLEEPCT -gt $MAXTIME && $MAXTIME -ne 0 ]]; then
			break
		fi
	done
	if [[ $SLEEPCT -gt $MAXTIME && $MAXTIME -ne 0 ]]; then
		logger -p user.err "WireGuard ERROR: stopped waiting after $SLEEPCT seconds, trying to set routes for oet${i} anyway, is there a connection or NTP problem?"
	else
		logger -p user.info "WireGuard waited $SLEEPCT seconds to set routes for oet${i}"
	fi
	#debug
	#logger -p user.info "WireGuard debug at end of time check: ntp_success: $($nv get ntp_success); ntp_done:$($nv get ntp_done) "
}

waitfortime

for i in $(seq 1 $tunnels); do
	if [[ $($nv get oet${i}_en) -eq 1 ]]; then
		if [[ $($nv get oet${i}_proto) -eq 2 ]] && [[ $($nv get oet${i}_failgrp) -ne 1 || $($nv get oet${i}_failstate) -eq 2 ]]; then

			TID=$((20+$i))

			# Start with PBR to make sure killswitch is working
			#if [ ! -z "$($nv get oet${i}_pbr | sed '/^[[:blank:]]*#/d')" ]; then
			if [[ $($nv get oet${i}_spbr) -ne 0 ]]; then
				logger -p user.info "WireGuard PBR via oet${i} table $TID"
				echo $($nv get oet${i}_spbr_ip), | while read -d ',' line; do
					line=$(echo $line)
					#[ ${line:0:1} = "#" ] && continue
					case $line in
					 "#"*)
						continue
						;;
					 [0-9]*)
						ip rule del table $TID from $line >/dev/null 2>&1
						ip rule add table $TID from $line
						;;
					 *)
						ip rule del table $TID $line >/dev/null 2>&1
						ip rule add table $TID $line
						;;
					esac
				done
				# due to a bug in 'ip rule' command rogue 'from all' can be added for non valid ip addresses so remove that:
				for z in $(ip rule | grep "from all lookup $TID" | cut -d: -f1); do 
					ip rule del prior $z
					logger -p user.err "WireGuard ERROR: non-valid IP address in PBR, tunnel oet${i}"
				done
				if [[ $($nv get oet${i}_killswitch) -eq 1 && $($nv get oet${i}_spbr) -eq 1 ]]; then
					logger -p user.info "WireGuard Killswitch on PBR activated for oet${i}"
					ip route add prohibit default table $TID >/dev/null 2>&1
				else
					ip route add default via $($nv get wan_gateway) table $TID >/dev/null 2>&1
				fi
				ip route flush cache
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
							logger -p user.info "WireGuard route $aip added via oet${i}"
							ip route add $aip dev oet${i} >/dev/null 2>&1
						done
					fi
			done

			# Destination based routing
			if [[ $($nv get oet${i}_dpbr) -ne 0 ]]; then
				#WGDELDPBR="/tmp/wgdeldpbr_oet${i}"
				WGDPBRIP="/tmp/wgdpbrip_oet${i}"
				rm $WGDPBRIP >/dev/null 2>&1
				for dpbr in $($nv get oet${i}_dpbr_ip | sed "s/,/ /g"); do
					if [[ ${dpbr:0:1} != "#" ]]; then
						if [ "$dpbr" != "${dpbr#*[0-9].[0-9]}" ]; then
							echo "$dpbr" >> $WGDPBRIP
						elif [ "$dpbr" != "${dpbr#*:[0-9a-fA-F]}" ]; then
							logger -p user.warning "WireGuard $dpbr is IPv6, not yet implemented"
						else
							if ! nslookup "$dpbr" >/dev/null 2>&1; then
								logger -p user.err "WireGuard ERROR domain $dpbr not found"
							else
								nslookup "$dpbr" 2>/dev/null | awk '/^Name:/,0 {if (/^Addr[^:]*: [0-9]{1,3}\./) print $3}' >> $WGDPBRIP
							fi
						fi
					fi
				done
				if [[ -s $WGDPBRIP ]]; then
					# rm $WGDELDPBR >/dev/null 2>&1
					while read dpbrip; do
						# echo "ip route del $dpbrip" >> $WGDELDPBR
						if [[ $($nv get oet${i}_dpbr) -eq 1 ]]; then
							ip route add $dpbrip dev oet${i} >/dev/null 2>&1
						elif [[ $($nv get oet${i}_dpbr) -eq 2 ]]; then
							ip route add $dpbrip via $($nv get wan_gateway) >/dev/null 2>&1
						fi
					done < $WGDPBRIP
				fi
			fi

			#add route to DNS server via tunnel
			if [ ! -z "$($nv get oet${i}_dns | sed '/^[[:blank:]]*#/d')" ]; then
				for wgdns in $($nv get oet${i}_dns | sed "s/,/ /g") ; do
					ip route add $wgdns dev oet${i} >/dev/null 2>&1
					logger -p user.info "WireGuard DNS server $wgdns routed via oet${i}"
				done
			fi
			# add DNS server
			DNSTIME=$($nv get wg_dnstime)
			[[ -z $DNSTIME ]] && DNSTIME=1
			sleep $DNSTIME
			if [[ ! -z "$($nv get oet${i}_dns | sed '/^[[:blank:]]*#/d')" ]]; then
				SLEEPDNSCT=0
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
			# add routes to PBR table
			if [[ $($nv get oet${i}_spbr) -eq 1 ]]; then
				#add default routes
				ip route add 0.0.0.0/1 dev oet${i} table $TID >/dev/null 2>&1
				ip route add 128.0.0.0/1 dev oet${i} table $TID >/dev/null 2>&1
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
				for FTID in $(seq 21 1 $maxtable); do
					if [[ ! -z  "$(ip route show table $FTID)" ]]; then
						logger -p user.info "WireGuard adding local routes to table $FTID"
						ip route show | grep -Ev '^default |^0.0.0.0/1 |^128.0.0.0/1 ' | while read route; do
							ip route add $route table $FTID >/dev/null 2>&1
						done
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
			# execute watchdog script
if [[ $($nv get oet${i}_failstate) -eq 2 || $($nv get oet${i}_wdog) -eq 1 ]]; then
				# only start if not already running
				if ! ps | grep -q "[w]ireguard-fwatchdog\.sh $i"; then
					logger -p user.info "WireGuard: wireguard-fwatchdog $i not running yet"
				else
					logger -p user.info "WireGuard: wireguard-fwatchdog $i already running will be killed first"
					ps | grep "[w]ireguard-fwatchdog\.sh $i" | awk '{print $1}' | xargs kill -9 >/dev/null 2>&1
				fi
				# send tunnelnumber, sleeptime (sec), ping address, reset (1=Yes)
				PINGIP=$($nv get oet${i}_failip)
				[[ -z "$PINGIP" ]] && PINGIP="8.8.8.8"
				sh /usr/bin/wireguard-fwatchdog.sh $i 29 $PINGIP $fset &
			fi
			#restart dnsmasq when the last tunnel has been setup to reread resolv.dnsmasq, due to a bug this does not happen on change of resolv.dnsmasq
			# for now disabled waiting for DNSMasq to be repaired 2.86 works but 287test4 is buggy and does not want to replace a DNS resolver which is already in memory
			#restart=$(nvram get wg_restart_dnsmasq)
			#[[ $i -eq $x && $restart -ge 1 ]] && { sleep $restart; restart dnsmasq; }
			# reload but not restart works of resolv.dnsmasq only if "no-poll" is set
			#[[ $i -eq $x && $restart -ge 1 ]] && { sleep $restart; kill -1 $(pidof dnsmasq); }
		fi
	fi
done
#release lock
release_lock
