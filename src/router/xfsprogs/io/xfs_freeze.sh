#!/bin/sh -f
#
# Copyright (c) 2004 Silicon Graphics, Inc.  All Rights Reserved.
#

OPTS=""
USAGE="Usage: xfs_freeze -f | -u <mountpoint>"
DIRNAME=`dirname $0`
VERSION=false
FREEZE=false
THAW=false

while getopts "fuV" c
do
	case $c in
	f)	FREEZE=true;;
	u)	THAW=true;;
	V)	VERSION=true;;
	\?)	echo $USAGE 1>&2
		exit 2
		;;
	esac
done
if $VERSION ; then
	$DIRNAME/xfs_io -p xfs_freeze -V
	exit 0
fi

shift `expr $OPTIND - 1`
if [ "$1" = "" ]; then
	echo $USAGE 1>&2
	exit 2
fi

if $FREEZE ; then
	$DIRNAME/xfs_io -F -r -p xfs_freeze -x -c "freeze" "$1"
	status=$?
	[ $status -ne 0 ] && exit $status
elif $THAW ; then
	$DIRNAME/xfs_io -F -r -p xfs_freeze -x -c "thaw" "$1"
	status=$?
	[ $status -ne 0 ] && exit $status
else
	echo $USAGE 1>&2
	exit 2
fi
exit 0
