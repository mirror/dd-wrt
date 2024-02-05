#!/bin/sh -f
# SPDX-License-Identifier: GPL-2.0
#
# Copyright (c) 2000-2001 Silicon Graphics, Inc.  All Rights Reserved.
#

status=0
require_offline=""
require_online=""
DB_OPTS=""
REPAIR_OPTS=""
IO_OPTS=""
REPAIR_DEV_OPTS=""
LOG_OPTS=""
USAGE="Usage: xfs_admin [-efjlpuV] [-c 0|1] [-L label] [-O v5_feature] [-r rtdev] [-U uuid] device [logdev]"

while getopts "c:efjlL:O:pr:uU:V" c
do
	case $c in
	c)	REPAIR_OPTS=$REPAIR_OPTS" -c lazycount="$OPTARG
		require_offline=1
		;;
	e)	DB_OPTS=$DB_OPTS" -c 'version extflg'"
		require_offline=1
		;;
	f)	DB_OPTS=$DB_OPTS" -f"
		require_offline=1
		;;
	j)	DB_OPTS=$DB_OPTS" -c 'version log2'"
		require_offline=1
		;;
	l)	DB_OPTS=$DB_OPTS" -r -c label"
		IO_OPTS=$IO_OPTS" -r -c label"
		;;
	L)	DB_OPTS=$DB_OPTS" -c 'label "$OPTARG"'"
		IO_OPTS=$IO_OPTS" -c 'label -s "$OPTARG"'"
		;;
	O)	REPAIR_OPTS=$REPAIR_OPTS" -c $OPTARG"
		require_offline=1
		;;
	p)	DB_OPTS=$DB_OPTS" -c 'version projid32bit'"
		require_offline=1
		;;
	r)	REPAIR_DEV_OPTS=" -r '$OPTARG'"
		require_offline=1
		;;
	u)	DB_OPTS=$DB_OPTS" -r -c uuid"
		IO_OPTS=$IO_OPTS" -r -c fsuuid"
		;;
	U)	DB_OPTS=$DB_OPTS" -c 'uuid "$OPTARG"'"
		require_offline=1
		;;
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
	1|2)
		if mntpt="$(findmnt -t xfs -f -n -o TARGET "$1" 2>/dev/null)"; then
			# filesystem is mounted
			if [ -n "$require_offline" ]; then
				echo "$1: filesystem is mounted."
				exit 2
			fi

			if [ -n "$IO_OPTS" ]; then
				eval xfs_io -p xfs_admin $IO_OPTS "$mntpt"
				exit $?
			fi
		fi

		# filesystem is not mounted
		if [ -n "$require_online" ]; then
			echo "$1: filesystem is not mounted"
			exit 2
		fi

		# Pick up the log device, if present
		if [ -n "$2" ]; then
			LOG_OPTS=" -l '$2'"
		fi

		if [ -n "$DB_OPTS" ]
		then
			eval xfs_db -x -p xfs_admin $LOG_OPTS $DB_OPTS "$1"
			status=$?
		fi
		if [ -n "$REPAIR_OPTS" ]
		then
			echo "Running xfs_repair to upgrade filesystem."
			eval xfs_repair $LOG_OPTS $REPAIR_DEV_OPTS $REPAIR_OPTS "$1"
			status=`expr $? + $status`
		fi
		;;
	*)	echo $USAGE 1>&2
		exit 2
		;;
esac
exit $status
