#!/bin/sh -f
#
# Copyright (c) 2000-2001 Silicon Graphics, Inc.  All Rights Reserved.
#

OPTS=""
USAGE="Usage: xfs_info [-V] [-t mtab] mountpoint"

while getopts "t:V" c
do
	case $c in
	t)	OPTS="-t $OPTARG" ;;
	V)	xfs_growfs -p xfs_info -V
		status=$?
		exit $status
		;;
	*)	echo $USAGE 1>&2
		exit 2
		;;
	esac
done
set -- extra "$@"
shift $OPTIND
case $# in
	1)	xfs_growfs -p xfs_info -n $OPTS "$1"
		status=$?
		;;
	*)	echo $USAGE 1>&2
		exit 2
		;;
esac
exit $status
