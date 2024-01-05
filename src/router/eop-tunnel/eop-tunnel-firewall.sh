#!/bin/sh
[[ ! -z $(nvram get wg_debug) ]] && { DEBUG=1; set -x; }
{
nv=/usr/sbin/nvram
ipt=/usr/sbin/iptables
ip6t=/usr/sbin/ip6tables
ACCEPT="ACCEPT"
DROP="DROP"
REJECT="REJECT"
if [[ $($nv get log_enable) -eq 1 ]]; then
	if [[ $($nv get log_level) -eq 1 ]]; then
		DROP="logdrop"
		REJECT="logreject"
	fi
	if [[ $($nv get log_level) -eq 2 ]]; then
		ACCEPT="logaccept"
		DROP="logdrop"
		REJECT="logreject"
	fi
fi
tunnels=$($nv get oet_tunnels)
#WAN_IF="$(ip route | awk '/^default/{print $NF}')"
WAN_IF=$(get_wanface)
FW_STATE="-m state --state NEW"
ipv6_en=$($nv get ipv6_enable)
wan_ipaddr=$($nv get wan_ipaddr)
[[ $($nv get wan_proto) = "disabled" ]] && { IN_IF="-i br0"; logger -p user.info "WireGuard Killswitch for WAP on br0 only!, oet${i}"; } || IN_IF=""
LOCK="/tmp/oet-fw.lock"
#acquire_lock() { logger -p user.info "WireGuard acquiring $LOCK for firewall $$ "; while ! mkdir $LOCK >/dev/null 2>&1; do sleep 2; done; logger -p user.info "WireGuard $LOCK acquired for firewall $$ "; }
acquire_lock() { 
	logger -p user.info "WireGuard acquiring $LOCK for firewall $$ "
	local SLEEPCT=0
	local MAXTIME=30
	while ! mkdir $LOCK >/dev/null 2>&1; do
		sleep 2
		SLEEPCT=$((SLEEPCT+2))
		if [[ $SLEEPCT -gt $MAXTIME && $MAXTIME -ne 0 ]]; then
			logger -p user.err "WireGuard ERROR max. waiting $SLEEPCT sec. for locking firewall $$!"
			break
		fi
	done 
	logger -p user.info "WireGuard $LOCK acquired for $$ " 
}
release_lock() { rmdir $LOCK >/dev/null 2>&1; logger -p user.info "WireGuard released $LOCK for firewall $$ "; }
trap '{ release_lock; logger -p user.info "WireGuard script $0 running on oet${i} fatal error"; exit 1; }' SIGHUP SIGINT SIGTERM
acquire_lock
for i in $(seq 1 $tunnels); do
	if [[ -e "/tmp/oet/pid/f${i}.pid" ]]; then
		FW_CHAIN="pbr-oet${i}"
		WGDELRT="/tmp/wgdelrt_oet${i}"
		{
		$ipt -D FORWARD -i oet${i} -j $ACCEPT
		$ipt -D INPUT -i oet${i} -j $ACCEPT
		$ipt -D INPUT -p 47 -s `$nv get oet${i}_rem` -j $ACCEPT
		$ipt -D INPUT -p 97 -s `$nv get oet${i}_rem` -j $ACCEPT
		$ipt -D INPUT -p udp --sport oet${i}_srcmin:oet${i}_srcmax  -j $ACCEPT
		$ipt -D FORWARD $IN_IF -o $WAN_IF -j $FW_CHAIN
		$ipt -F $FW_CHAIN
		$ipt -X $FW_CHAIN
		if [[ $ipv6_en -eq 1 ]]; then
			$ip6t -D FORWARD $IN_IF -o $WAN_IF -j $FW_CHAIN
			$ip6t -F $FW_CHAIN
			$ip6t -X $FW_CHAIN
		fi
		$ipt -t mangle -D INPUT -p udp -m udp --dport `$nv get oet${i}_port` -j WGOBFS --key `$nv get oet${i}_obfkey` --unobfs
		$ipt -t mangle -D OUTPUT -p udp -m udp --sport `$nv get oet${i}_port` -j WGOBFS --key `$nv get oet${i}_obfkey` --obfs
		peers=$(($($nv get oet${i}_peers) - 1))
		for p in $(seq 0 $peers); do
			$ipt -t mangle -D INPUT -p udp -m udp --sport `$nv get oet${i}_peerport${p}` -j WGOBFS --key `$nv get oet${i}_obfkey${p}` --unobfs >/dev/null 2>&1
			$ipt -t mangle -D OUTPUT -p udp -m udp --dport `$nv get oet${i}_peerport${p}` -j WGOBFS --key `$nv get oet${i}_obfkey${p}` --obfs >/dev/null 2>&1
		done
		if [[ -f "$WGDELRT" ]]; then
			(while read route; do $route; done < $WGDELRT)
			rm $WGDELRT
		fi
		# remove because FW sets it by default
		$ipt -D FORWARD -i oet${i} $FW_STATE -j $ACCEPT
		$ip6t -D FORWARD -i oet${i} -m conntrack --ctstate NEW -j $ACCEPT
		rm -f /tmp/oet/pid/f${i}.pid
		} >/dev/null 2>&1
	fi
done
for i in $(seq 1 $tunnels); do
	if [[ $($nv get oet${i}_en) -eq 1 ]]; then
		if [ $($nv get oet${i}_proto) -eq 1 ]; then
			$ipt -I INPUT -p 47 -s `$nv get oet${i}_rem` -j $ACCEPT >/dev/null 2>&1
		fi
		if [ $($nv get oet${i}_proto) -eq 0 ]; then
			$ipt -I INPUT -p 97 -s `$nv get oet${i}_rem` -j $ACCEPT >/dev/null 2>&1
		fi
		if [ $($nv get oet${i}_proto) -eq 3 ]; then
			$ipt -I INPUT -p udp --sport oet${i}_srcmin:oet${i}_srcmax  -j $ACCEPT >/dev/null 2>&1
		fi
		if [[ `$nv get oet${i}_bridged` -ne 1 && `$nv get oet${i}_proto` -ne 2 && `$nv get oet${i}_proto` -ne 3 ]]; then
			$ipt -I FORWARD -i oet${i} -j $ACCEPT >/dev/null 2>&1
			$ipt -I INPUT -i oet${i} -j $ACCEPT >/dev/null 2>&1
		fi
		if [[ `$nv get oet${i}_bridged` -ne 1 && `$nv get oet${i}_proto` -eq 3 ]]; then
			$ipt -I FORWARD -i vxlan${i} -j $ACCEPT >/dev/null 2>&1
			$ipt -I INPUT -i vxlan${i} -j $ACCEPT >/dev/null 2>&1
		fi
		if [ $($nv get oet${i}_proto) -eq 2 ] && [ $($nv get oet${i}_obf) -eq 1 ]; then
			insmod xt_WGOBFS
			$ipt -t mangle -I INPUT -p udp -m udp --dport `$nv get oet${i}_port` -j WGOBFS --key `$nv get oet${i}_obfkey` --unobfs >/dev/null 2>&1
			$ipt -t mangle -I OUTPUT -p udp -m udp --sport `$nv get oet${i}_port` -j WGOBFS --key `$nv get oet${i}_obfkey` --obfs >/dev/null 2>&1
		fi
		if [ $($nv get oet${i}_proto) -eq 2 ]; then
			peers=$(($($nv get oet${i}_peers) - 1))
			for p in $(seq 0 $peers); do
				if [ $($nv get oet${i}_obf${p}) -eq 1 ]; then
					insmod xt_WGOBFS
					$ipt -t mangle -I INPUT -p udp -m udp --sport `$nv get oet${i}_peerport${p}` -j WGOBFS --key `$nv get oet${i}_obfkey${p}` --unobfs >/dev/null 2>&1
					$ipt -t mangle -I OUTPUT -p udp -m udp --dport `$nv get oet${i}_peerport${p}` -j WGOBFS --key `$nv get oet${i}_obfkey${p}` --obfs >/dev/null 2>&1
				fi
			done
		fi
		if [ $($nv get oet${i}_proto) -eq 2 ] && [[ $($nv get oet${i}_failgrp) -ne 1 || $($nv get oet${i}_failstate) -eq 2 ]]; then
			WGDELRT="/tmp/wgdelrt_oet${i}"
			if [ $($nv get oet${i}_mit) -eq 1 ]; then
				insmod xt_addrtype >/dev/null 2>&1
				echo "$ipt -t raw -D PREROUTING ! -i oet${i} -d $(getipmask oet${i}) -m addrtype ! --src-type LOCAL -j DROP" >> $WGDELRT
				$ipt -t raw -I PREROUTING ! -i oet${i} -d $(getipmask oet${i}) -m addrtype ! --src-type LOCAL -j DROP
				#$ipt -t raw -I PREROUTING ! -i oet${i} -d $(getipmask oet${i}) -j DROP
			fi
			#add NAT out for WG interface
			if [[ $($nv get oet${i}_natout) -eq 1 ]]; then
				for addrmask in $($nv get oet${i}_ipaddrmask | sed "s/,/ /g") ; do
					addr=${addrmask%/*}
					ip=""
					case $addr in
					  *.*) #IPv4
						ip=${ipt};;
					  *:*) #IPv6
						ip=${ip6t};;
					  *) #no match
						logger -p user.err "WireGuard NAT ERROR invalid [$addr] for oet${i}"
						;;
					esac
					echo "$ip -t nat -D POSTROUTING -o oet${i} -j SNAT --to $addr" >> $WGDELRT
					$ip -t nat -I POSTROUTING -o oet${i} -j SNAT --to $addr >/dev/null 2>&1
					logger -p user.info "WireGuard NAT via oet${i} for $addr enabled"
				done
			fi
			#allow internet access via WAN
			if [[ "$($nv get oet${i}_wanac)" != "0" ]]; then
				for addrmask in $($nv get oet${i}_ipaddrmask | sed "s/,/ /g") ; do
					case $addrmask in
					  *.*) #IPv4
						echo "$ipt -D POSTROUTING -t nat -s $addrmask -o $WAN_IF -j SNAT --to $wan_ipaddr" >> $WGDELRT
						$ipt -t nat -I POSTROUTING -s $addrmask -o $WAN_IF -j SNAT --to $wan_ipaddr
						logger -p user.info "WireGuard IPv4 internet access for $addrmask enabled"
						;;
					  *:*) #IPv6
						echo "$ip6t -D POSTROUTING -t nat -s $addrmask -o $WAN_IF -j MASQUERADE" >> $WGDELRT
						$ip6t -t nat -I POSTROUTING -s $addrmask -o $WAN_IF -j MASQUERADE
						logger -p user.info "WireGuard IPv6 internet access for $addrmask enabled"
						;;
					esac
				done
			fi
			#add NAT via br0 to allow seamless LAN access
			if [[ $($nv get oet${i}_lanac) -eq 1 ]]; then
				for addrmask in $($nv get oet${i}_ipaddrmask | sed "s/,/ /g") ; do
					case $addrmask in
					  *.*) #IPv4
						echo "$ipt -t nat -D POSTROUTING -o br+ -s $addrmask -j MASQUERADE" >> $WGDELRT
						$ipt -t nat -I POSTROUTING -o br+ -s $addrmask -j MASQUERADE
						;;
					 # *:*) #IPv6 Not sure if we need this
						#echo "$ip6t -t nat -D POSTROUTING -o br+ -s $addrmask -j MASQUERADE" >> $WGDELRT
						#$ip6t -t nat -I POSTROUTING -o br+ -s $addrmask -j MASQUERADE
						#;;
					esac
				done
			fi
			#split DNS tunnel for PBR
			if [[ ! -z "$($nv get oet${i}_dns | sed '/^[[:blank:]]*#/d')" ]] && [[ $($nv get oet${i}_spbr) -ne 0 && $($nv get oet${i}_dnspbr) -eq 1 ]]; then
				logger -p user.info "WireGuard Split DNS tunnel for PBR"
				#[[ $($nv get oet${i}_spbr) -eq 1 ]] && { dnsserver=$($nv get oet${i}_dns); dnsserver=${dnsserver%%,*}; } || { dnsserver=$($nv get wan_dns); dnsserver=${dnsserver%% *}; }
				dns4=$($nv get oet${i}_dns4)
				dns6=$($nv get oet${i}_dns6)
				echo $($nv get oet${i}_spbr_ip), | while read -d ',' pbrip; do
					pbrip=$(eval echo $pbrip)
						case $pbrip in
						 "#"*)
							continue
							;;
						 *[0-9].*)
							sourcepbr="-s"
							ipxt=ip4t
							;;
						 *[0-9a-fA-F]:*)
							sourcepbr="-s"
							ipxt=ip6t
							;;
						 iif*)
							sourcepbr="-i"
							#pbrip=${pbrip#* }
							pbrip="${pbrip#iif }"
							ipxt=ipxt
							;;
						 *)
							logger -p user.err "WireGuard: ERROR invalid entry: $pbrip in Split DNS"
							;;
						esac
					if [[ $dns4 != "0" ]] && [[ $ipxt = ip4t || $ipxt = ipxt ]]; then
						echo "$ipt -t nat -D PREROUTING -p udp $sourcepbr $pbrip --dport 53 -j DNAT --to $dns4" >> $WGDELRT
						echo "$ipt -t nat -D PREROUTING -p tcp $sourcepbr $pbrip --dport 53 -j DNAT --to $dns4" >> $WGDELRT
						$ipt -t nat -I PREROUTING -p udp $sourcepbr $pbrip --dport 53 -j DNAT --to $dns4
						$ipt -t nat -I PREROUTING -p tcp $sourcepbr $pbrip --dport 53 -j DNAT --to $dns4
					fi
					if [[ $ipv6_en -eq 1 && $dns6 != "0" ]] && [[ $ipxt = ip6t || $ipxt = ipxt ]]; then
						echo "$ip6t -t nat -D PREROUTING -p udp $sourcepbr $pbrip --dport 53 -j DNAT --to $dns6" >> $WGDELRT
						echo "$ip6t -t nat -D PREROUTING -p tcp $sourcepbr $pbrip --dport 53 -j DNAT --to $dns6" >> $WGDELRT
						$ip6t -t nat -I PREROUTING -p udp $sourcepbr $pbrip --dport 53 -j DNAT --to $dns6
						$ip6t -t nat -I PREROUTING -p tcp $sourcepbr $pbrip --dport 53 -j DNAT --to $dns6
					fi
				done
			fi
			#killswitch
			if [[ $($nv get oet${i}_killswitch) -eq 1 ]]; then
				#For WAP or Bridge, use br0 as IN_IF
				runchain=0
				makechain () {
					if [[ $runchain -eq 0 ]]; then
						FW_CHAIN="pbr-oet${i}"
						$ipt -N $FW_CHAIN
						$ipt -I FORWARD $IN_IF -o $WAN_IF -j $FW_CHAIN
						if [[ $ipv6_en -eq 1 ]]; then
							$ip6t -N $FW_CHAIN
							$ip6t -I FORWARD $IN_IF -o $WAN_IF -j $FW_CHAIN
						fi
						runchain=1
					fi
				}
				if [[ $($nv get oet${i}_spbr) -ne 1 ]]; then
					logger -p user.info "WireGuard Killswitch activated for all clients!"
					echo "$ipt -D FORWARD $IN_IF -o $WAN_IF -j $REJECT" >> $WGDELRT
					$ipt -I FORWARD $IN_IF -o $WAN_IF -j $REJECT
					if [[ $ipv6_en -eq 1 ]]; then
						echo "$ip6t -D FORWARD $IN_IF -o $WAN_IF -j $REJECT" >> $WGDELRT
						$ip6t -I FORWARD $IN_IF -o $WAN_IF -j $REJECT
					fi
					#todo restart SFE/CTF to cut existing connections
					#cat /proc/net/ip_conntrack_flush 2>&1
					#cat /proc/sys/net/netfilter/nf_conntrack_flush 2>&1
					# Escape Destination routing via the WAN
					if [[ $($nv get oet${i}_dpbr) -eq 2 ]]; then
						makechain
						WGDPBRIP="/tmp/wgdpbrip_oet${i}"
						MAXTIME=10
						SLEEPCT=0
						#wait for WGDPBRIP alternatively parse oet${i}_dpbr_ip again
						while [[ ! -s $WGDPBRIP ]]; do
							sleep 2
							SLEEPCT=$((SLEEPCT+2))
							if [[ $SLEEPCT -gt $MAXTIME && $MAXTIME -ne 0 ]]; then
								break
							fi
						done
						logger -p user.info "WireGuard waited $SLEEPCT sec. for $WGDPBRIP"
						sleep 2
						while read dpbrip; do
								if [[ "$dpbrip" != "${dpbrip#*[0-9].[0-9]}" ]]; then
									$ipt -D $FW_CHAIN -d $dpbrip -j ACCEPT >/dev/null 2>&1
									$ipt -A $FW_CHAIN -d $dpbrip -j ACCEPT >/dev/null 2>&1
								else
									$ip6t -D $FW_CHAIN -d $dpbrip -j ACCEPT >/dev/null 2>&1
									$ip6t -A $FW_CHAIN -d $dpbrip -j ACCEPT >/dev/null 2>&1
								fi
						done < $WGDPBRIP
					fi
				fi
				if [[ $($nv get oet${i}_spbr) -eq 2 ]]; then  # alternatively use -ne 0 so that block is alos set but should not be necessary as PBR table with prohibited default takes care of this
					#PBR killswitch
					logger -p user.info "WireGuard firewall on PBR activated for oet${i}"
					makechain
					echo $($nv get oet${i}_spbr_ip), | while read -d ',' pbrip; do	# added "," so that last entry is read
						pbrip=$(eval echo $pbrip)
						case $pbrip in
						 "#"*)
							continue
							;;
						 *[0-9].*)
							sourcepbr="-s"
							ipxt=ip4t
							;;
						 *[0-9a-fA-F]:*)
							sourcepbr="-s"
							ipxt=ip6t
							;;
						 iif*)
							sourcepbr="-i"
							#pbrip=${pbrip#* }
							pbrip="${pbrip#iif }"
							ipxt=ipxt
							;;
						 *port*)
							sourcepbr="port"
							pbrip="--$pbrip"
							ipxt=ipxt
							;;
						 *)
							logger -p user.err "WireGuard: ERROR invalid entry: $pbrip in PBR Killswitch"
							;;
						esac
						if [[ $ipxt = ip4t || $ipxt = ipxt ]]; then
							if [[ $sourcepbr = "port" ]];then
								$ipt -A $FW_CHAIN -p tcp $pbrip -j ACCEPT >/dev/null 2>&1
								$ipt -A $FW_CHAIN -p udp $pbrip -j ACCEPT >/dev/null 2>&1
							else
								$ipt -A $FW_CHAIN $sourcepbr $pbrip -j ACCEPT >/dev/null 2>&1
							fi
						fi
						if [[ $ipv6_en -eq 1 ]] && [[ $ipxt = ip6t || $ipxt = ipxt ]]; then
							if [[ $sourcepbr = "port" ]];then
								$ip6t -A $FW_CHAIN -p tcp $pbrip -j ACCEPT >/dev/null 2>&1
								$ip6t -A $FW_CHAIN -p udp $pbrip -j ACCEPT >/dev/null 2>&1
							else
								$ip6t -A $FW_CHAIN $sourcepbr $pbrip -j ACCEPT >/dev/null 2>&1
							fi
						fi
					done
				fi
			fi
			#end kill switch
			#New Inbound opening of firewall for site-to-site
			if [[ $($nv get oet${i}_firewallin) -eq 0 ]]; then
				echo "$ipt -D INPUT -i $WAN_IF -p udp --dport $($nv get oet${i}_port) -j $ACCEPT" >> $WGDELRT
				echo "$ipt -D FORWARD -i oet${i} $FW_STATE -j $ACCEPT" >> $WGDELRT
				echo "$ipt -D INPUT -i oet${i} $FW_STATE -j $ACCEPT" >> $WGDELRT
				$ipt -I INPUT -i $WAN_IF -p udp --dport $($nv get oet${i}_port) -j $ACCEPT >/dev/null 2>&1
				$ipt -I FORWARD -i oet${i} $FW_STATE -j $ACCEPT >/dev/null 2>&1
				$ipt -I INPUT -i oet${i} $FW_STATE -j $ACCEPT >/dev/null 2>&1
				#For IPv6
				if [[ $ipv6_en -eq 1 ]]; then
					echo "$ip6t -D INPUT -i $WAN_IF -p udp --dport $($nv get oet${i}_port) -j $ACCEPT" >> $WGDELRT
					echo "$ip6t -D FORWARD -i oet${i} $FW_STATE -j $ACCEPT" >> $WGDELRT
					echo "$ip6t -D INPUT -i oet${i} $FW_STATE -j $ACCEPT" >> $WGDELRT
					$ip6t -I INPUT -i $WAN_IF -p udp --dport $($nv get oet${i}_port) -j $ACCEPT >/dev/null 2>&1
					$ip6t -I FORWARD -i oet${i} $FW_STATE -j $ACCEPT >/dev/null 2>&1
					$ip6t -I INPUT -i oet${i} $FW_STATE -j $ACCEPT >/dev/null 2>&1
				fi
				logger -p user.info "WireGuard Inbound Firewall deactivated on oet${i}"
			fi
			#end inbound firewall
			# todo make escape rules for destination based routing on by default
			# use wgdpbrip_oetx but wait till file is made
		fi
		echo enable > /tmp/oet/pid/f${i}.pid
	fi
done
release_lock
# consider re-executing .rc_firewall so that those rules will be last
# ( [[ -f /tmp/.rc_firewall ]] && sh /tmp/.rc-firewall & ) >/dev/null 2>&1
} 2>&1 | logger $([ ${DEBUG+x} ] && echo '-p user.debug') \
    -t $(echo $(basename $0) | grep -Eo '^.{0,23}')[$$] &

