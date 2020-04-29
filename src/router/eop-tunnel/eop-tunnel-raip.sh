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
					 #ip route del $aip >/dev/null 2>&1
					logger -p user.info "route $aip added via oet${i}" #debug
					ip route add $aip dev oet${i} 
				done
			fi			
		done
ip route flush cache
# end add routes

