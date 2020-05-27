#!/bin/sh
nv=/usr/sbin/nvram

i=$1

#make sure WG/router is up and ready
sleep 35

peers=$((`$nv get oet${i}_peers` - 1))
for p in `seq 0 $peers`
	do
		# oet${i}_aip_rten${p} #nvram variable to allow routing of Allowed IP's, 1=add route
		if [[ $($nv get oet${i}_aip_rten${p}) -eq 1 ]] 
		then 
			for aip in $($nv get oet${i}_aip${p} | sed "s/,/ /g") ; do
				# check if PBR is set then skip default gateway
				if [ ! -z "$($nv get oet${i}_pbr | sed '/^[[:blank:]]*#/d')" ] && [ $aip = 0.0.0.0/1 -o $aip = 128.0.0.0/1 -o $aip = 0.0.0.0/0 ] ; then
					continue
				fi
				 #ip route del $aip >/dev/null 2>&1
				logger -p user.info "route $aip added via oet${i}" #debug
				ip route add $aip dev oet${i} 
			done
		fi			
	done
# end add routes

#egc PBR
let "TID=20+$i"  #table number for WireGuard PBR
while ip rule delete from 0/0 to 0/0 table $TID; do true; done
ip route flush table $TID 2>&1 > /dev/null
if [ ! -z "$($nv get oet${i}_pbr | sed '/^[[:blank:]]*#/d')" ]; then
	for aip in $($nv get oet${i}_pbr | sed "s/,/ /g") ; do
	#for aip in $($nv get oet${i}_pbr | sed '/^[[:blank:]]*#/d;s/#.*//') ; do
		if [ ${aip:0:1} = "#" ]; then
			continue
		fi
		ip rule add table $TID from $aip
	done
	#add default route to alternate table
	ip route add default dev oet${i} table $TID
	#add local routes to alternate table
	ip route show | grep -Ev '^default |^0.0.0.0/1 |^128.0.0.0/1 ' | while read route; do
		ip route add $route table $TID
	done
# else  #clean up
	# while ip rule delete from 0/0 to 0/0 table $TID; do true; done
	# ip route flush table $TID 2>&1 > /dev/null
fi
# end PBR

ip route flush cache
