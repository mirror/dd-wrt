#!/bin/sh -f
#
# Copyright (c) 2000-2001 Silicon Graphics, Inc.  All Rights Reserved.
#

status=0
DB_OPTS=""
REPAIR_OPTS=""
USAGE="Usage: xfs_admin [-efjlpuV] [-c 0|1] [-L label] [-U uuid] device"

while getopts "efjlpuc:L:U:V" c
do
	case $c in
	c)	REPAIR_OPTS=$REPAIR_OPTS" -c lazycount="$OPTARG;;
	e)	DB_OPTS=$DB_OPTS" -c 'version extflg'";;
	f)	DB_OPTS=$DB_OPTS" -f";;
	j)	DB_OPTS=$DB_OPTS" -c 'version log2'";;
	l)	DB_OPTS=$DB_OPTS" -r -c label";;
	L)	DB_OPTS=$DB_OPTS" -c 'label "$OPTARG"'";;
	p)	DB_OPTS=$DB_OPTS" -c 'version projid32bit'";;
	u)	DB_OPTS=$DB_OPTS" -r -c uuid";;
	U)	DB_OPTS=$DB_OPTS" -c 'uuid "$OPTARG"'";;
	V)	xfs_db -p xfs_admin -V
		status=$?
		exit $status
		;;
	\?)	echo $USAGE 1>&2
		exit 2
		;;
	esac
done
set -- extra $@
shift $OPTIND
case $# in
	1)	if [ -n "$DB_OPTS" ]
		then
			eval xfs_db -x -p xfs_admin $DB_OPTS $1
			status=$?
		fi
		if [ -n "$REPAIR_OPTS" ]
		then
			# Hide normal repair output which is sent to stderr
			# assuming the filesystem is fine when a user is
			# running xfs_admin.
			# Ideally, we need to improve the output behaviour
			# of repair for this purpose (say a "quiet" mode).
			eval xfs_repair $REPAIR_OPTS $1 2> /dev/null
			status=`expr $? + $status`
			if [ $status -ne 0 ]
			then
				echo "Conversion failed, is the filesystem unmounted?"
			fi
		fi
		;;
	*)	echo $USAGE 1>&2
		exit 2
		;;
esac
exit $status
