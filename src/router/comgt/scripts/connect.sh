#!/bin/sh
comgt -s -d $1 /etc/comgt/cgatt.comgt
ERR=$?
if [ $ERR = 0 ]
then
	sleep 5
	export COMGTDIAL=${COMGTXDIAL}
	comgt -s -d $2 DIAL
	exit 0
fi
exit 1
