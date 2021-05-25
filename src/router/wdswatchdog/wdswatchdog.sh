#!/bin/sh

WDS_WATCHDOG_INTERVAL_SEC=$(nvram get wds_watchdog_interval_sec)
WDS_WATCHDOG_IPS=$(nvram get wds_watchdog_ips)
WDS_WATCHDOG_MODE=$(nvram get wds_watchdog_mode)
TAG="WDS_Watchdog[$$]"

logger -t "$TAG" "Started"
while sleep $WDS_WATCHDOG_INTERVAL_SEC
do
  if [ $WDS_WATCHDOG_MODE = "0" ]
  then
    logger -t "$TAG" "Test cycle in mode of any dropped IPs for reboot"
    for ip in $WDS_WATCHDOG_IPS
    do
      if ping -c 1 $ip > /tmp/null
      then
        logger -t "$TAG" "$ip ok"
      else
        logger -t "$TAG" "$ip dropped (1/3)"
        sleep 10
        if ! ping -c 1 $ip > /tmp/null
        then
          logger -t "$TAG" "$ip dropped (2/3)"
          sleep 10
          if ! ping -c 1 $ip > /tmp/null
          then
              logger -t "$TAG" "$ip dropped (3/3), Restarting Router"
              /usr/sbin/nvram commit
              /sbin/reboot &
          fi
        fi
      fi
    done
  else
    logger -t "$TAG" "Test cycle in mode of all dropped IPs for reboot"
    WDS_WATCHDOG_PING_OK=false
    for ip in $WDS_WATCHDOG_IPS
    do
      if ping -c 1 $ip > /tmp/null
      then
        WDS_WATCHDOG_PING_OK=true
        logger -t "$TAG" "$ip ok"
      else
        logger -t "$TAG" "$ip dropped (1/3)"
        sleep 10
        if ping -c 1 $ip > /tmp/null
        then
          WDS_WATCHDOG_PING_OK=true
          logger -t "$TAG" "$ip ok"
        else
          logger -t "$TAG" "$ip dropped (2/3)"
          sleep 10
          if ping -c 1 $ip > /tmp/null
          then
            WDS_WATCHDOG_PING_OK=true
            logger -t "$TAG" "$ip ok"
          else
            logger -t "$TAG" "$ip dropped (3/3)"          
          fi
        fi
      fi
    done
    if ! $WDS_WATCHDOG_PING_OK
    then
      logger -t "$TAG" "None of the IPs $WDS_WATCHDOG_IPS responded ping, Restarting Router"
      /usr/sbin/nvram commit
      /sbin/reboot &
    fi
  fi
done 2>&1
