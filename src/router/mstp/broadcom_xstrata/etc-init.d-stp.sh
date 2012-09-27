#!/bin/sh
#
# Copyright (C) 2012 by Phybridge Inc
#
PATH=/sbin:/usr/sbin:/bin:/usr/bin
# source our service control routines
[ -r /etc/polre/svc-ctl ] && . /etc/polre/svc-ctl
# spanning tree service
SVC=stp

MODPATH=/sbin
DAEMON=mstpd
DAEMONARGS=

# bridge ID
BRID=br0
# lowest
BRPRIO=0

GBPORTS="GbE1 GbE2"
GBCOST=4

start() {
    [ "$VERBOSE" != no ] ||  echo -n "Starting Spanning Tree: "
    bcmexp rcload /root/stp.soc
    # launch userland mstpd daemon
    [ -f $MODPATH/$DAEMON ] || exit 1
    start-stop-daemon -S -q --exec $MODPATH/$DAEMON -- $DAEMONARGS
    # create the bridge, add the ports, adjust the costs
    brctl addbr $BRID  || (echo "nope, already running"; exit 0)
    brctl setbridgeprio $BRID $BRPRIO
    brctl addif $BRID $GBPORTS
    for port in $GBPORTS; do brctl setpathcost $BRID $port $GBCOST; done
    
    # turn it on & bring interface up
    brctl stp $BRID on
    ifconfig $BRID up

    /sbin/mstpctl setforcevers br0 stp
    
    [ "$VERBOSE" != no ] && echo "OK"
}

stop() {
    # undo what start did
    mstpctl delbridge br0
    ifconfig $BRID down
    brctl stp $BRID off
    brctl delif $BRID $STKPORTS $GBPORTS
    brctl delbr $BRID
    start-stop-daemon -K -q -p /var/run/$DAEMON.pid
}

restart() {
    stop
    sleep 1
    start
}

status() {
	brctl show
	brctl showstp $BRID
	brctl showmacs $BRID
}

case "$1" in
  start)
   if enabled
   then
		start
	else
		[ "$VERBOSE" != no ] && echo "$SVC disabled" && exit 0
	fi
	;;
  stop)
	stop
	;;
  restart|reload)
	restart
	;;
  enable)
  	enable
  	;;
  status)
  	status
  	;;
  disable)
	disable
    stop
	;;
	*)
	echo $"Usage: $0 {start|stop|restart|enable|disable|status}"
	exit 1
esac

exit $?

