#!/bin/sh

WDS_WATCHDOG_INTERVAL_SEC=$(nvram get wds_watchdog_interval_sec)
WDS_WATCHDOG_IPS=$(nvram get wds_watchdog_ips)
WDS_WATCHDOG_MODE=$(nvram get wds_watchdog_mode)
WDS_WATCHDOG_TIMEOUT=$(nvram get wds_watchdog_timeout)
TAG="WDS_Watchdog[$$]"

logger -t "$TAG" "Started"
while sleep $WDS_WATCHDOG_INTERVAL_SEC
do
	if [ $WDS_WATCHDOG_MODE = "0" ]
	then
		logger -t "$TAG" "Test cycle in mode of any dropped IPs for reboot"
		DROPPED=false
		for ip in $WDS_WATCHDOG_IPS
		do
			if ping -c 1 -W $WDS_WATCHDOG_TIMEOUT $ip > /tmp/null
			then
				#logger -t "$TAG" "$ip ok"
				:
			else
				logger -t "$TAG" "$ip dropped (1/3)"
				DROPPED=true
			fi 
		done
		if $DROPPED
		then
			DROPPED=false
			sleep 10
			for ip in $WDS_WATCHDOG_IPS
			do
				if ping -c 1 -W $WDS_WATCHDOG_TIMEOUT $ip > /tmp/null
				then
					#logger -t "$TAG" "$ip ok"
					:
				else
					logger -t "$TAG" "$ip dropped (2/3)"
					DROPPED=true
				fi 
			done
		fi
		if $DROPPED
		then
			DROPPED=false
			sleep 10
			for ip in $WDS_WATCHDOG_IPS
			do
				if ping -c 1 -W $WDS_WATCHDOG_TIMEOUT $ip > /tmp/null
				then
					#logger -t "$TAG" "$ip ok"
					:
				else
					logger -t "$TAG" "$ip dropped (3/3), Restarting Router"
					/sbin/reboot &
				fi 
			done
		fi
	else
		logger -t "$TAG" "Test cycle in mode of all dropped IPs for reboot"
		DROPPED=true
		for ip in $WDS_WATCHDOG_IPS
		do
			if ping -c 1 -W $WDS_WATCHDOG_TIMEOUT $ip > /tmp/null
			then
				DROPPED=false
				#logger -t "$TAG" "$ip ok"
				:
			else
				logger -t "$TAG" "$ip dropped (1/3)"
			fi 
		done
		if $DROPPED
		then
			DROPPED=true
			sleep 10
			for ip in $WDS_WATCHDOG_IPS
			do
				if ping -c 1 -W $WDS_WATCHDOG_TIMEOUT $ip > /tmp/null
				then
					DROPPED=false
					#logger -t "$TAG" "$ip ok"
					:
				else
					logger -t "$TAG" "$ip dropped (2/3)"
				fi 
			done
		fi
		if $DROPPED
		then
			DROPPED=true
			sleep 10
			for ip in $WDS_WATCHDOG_IPS
			do
				if ping -c 1 -W $WDS_WATCHDOG_TIMEOUT $ip > /tmp/null
				then
					DROPPED=false
					#logger -t "$TAG" "$ip ok"
					:
				else
					logger -t "$TAG" "$ip dropped (3/3)"          
				fi 
			done
		fi
		if $DROPPED
		then
			logger -t "$TAG" "None of the IPs $WDS_WATCHDOG_IPS responded ping, Restarting Router"
			/sbin/reboot &
		fi
	fi
done 2>&1
