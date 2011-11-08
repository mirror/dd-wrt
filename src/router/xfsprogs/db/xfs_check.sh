#!/bin/sh -f
#
# Copyright (c) 2000-2003 Silicon Graphics, Inc.  All Rights Reserved.
#

OPTS=" "
DBOPTS=" "
USAGE="Usage: xfs_check [-fsvV] [-l logdev] [-i ino]... [-b bno]... special"

while getopts "b:fi:l:stvV" c
do
	case $c in
	s)	OPTS=$OPTS"-s ";;
	t)	OPTS=$OPTS"-t ";;
	v)	OPTS=$OPTS"-v ";;
	i)	OPTS=$OPTS"-i "$OPTARG" ";;
	b)	OPTS=$OPTS"-b "$OPTARG" ";;
	f)	DBOPTS=$DBOPTS" -f";;
	l)	DBOPTS=$DBOPTS" -l "$OPTARG" ";;
	V)	xfs_db -p xfs_check -V
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
	1)	xfs_db$DBOPTS -F -i -p xfs_check -c "check$OPTS" $1
		status=$?
		;;
	*)	echo $USAGE 1>&2
		exit 2
		;;
esac
exit $status
