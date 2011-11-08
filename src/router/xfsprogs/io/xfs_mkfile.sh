#!/bin/sh -f
#
# Copyright (c) 2005 Silicon Graphics, Inc.  All Rights Reserved.
#

OPTS=""
NOBYTES=false
PREALLOC=false
VERBOSE=false
VERSION=false
XFS_IO=`dirname $0`"/xfs_io -p xfs_mkfile"
USAGE="Usage: xfs_mkfile [-npvV] size file..."

while getopts "npvV" c
do
	case $c in
	n)	NOBYTES=true;;
	p)	PREALLOC=true;;
	v)	VERBOSE=true;;
	V)	VERSION=true;;
	\?)	echo $USAGE 1>&2
		exit 2
		;;
	esac
done
$VERSION && $XFS_IO -V

shift `expr $OPTIND - 1`
[ "$1" != "" ] || exit 0
SIZE="$1"
ERRORS=0
shift

while [ "$1" != "" ]
do
	if $VERBOSE ; then
		$PREALLOC && echo "$1 $SIZE bytes pre-allocated"
		$PREALLOC || echo "$1 $SIZE bytes"
	fi
 	if $NOBYTES ; then
		$XFS_IO -ft  -c "truncate $SIZE" -- "$1"
	elif $PREALLOC ; then
		$XFS_IO -ftd -c "resvsp 0 $SIZE" \
			     -c "pwrite -q -S0 -b256k 0 $SIZE" \
			     -c "truncate $SIZE" -- "$1"
	else
		$XFS_IO -ftd -c "pwrite -q -S0 -b256k 0 $SIZE" \
			     -c "truncate $SIZE" -- "$1"
	fi
	[ $? -eq 0 ] || ERRORS=`expr $ERRORS + 1`
	shift
done
exit $ERRORS
