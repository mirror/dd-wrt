#!/bin/sh -f
#
# Copyright (c) 2003 Silicon Graphics, Inc.  All Rights Reserved.
#

OPTS=""
VERSION=false
USAGE="Usage: xfs_bmap [-adlpvV] [-n nx] file..."
DIRNAME=`dirname $0`

while getopts "adln:pvV" c
do
	case $c in
	a)	OPTS=$OPTS" -a";;
	d)	OPTS=$OPTS" -d";;
	l)	OPTS=$OPTS" -l";;
	n)	OPTS=$OPTS" -n "$OPTARG;;
	p)	OPTS=$OPTS" -p";;
	v)	OPTS=$OPTS" -v";;
	V)	VERSION=true;;
	\?)	echo $USAGE 1>&2
		exit 2
		;;
	esac
done
$VERSION && $DIRNAME/xfs_io -p xfs_bmap -V

shift `expr $OPTIND - 1`

while [ "$1" != "" ]
do
	$DIRNAME/xfs_io -r -p xfs_bmap -c "bmap $OPTS" "$1"
	status=$?
	[ $status -ne 0 ] && exit $status
	shift
done
exit 0
