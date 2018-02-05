#!/bin/sh -f
#
# Copyright (c) 2006 Silicon Graphics, Inc.  All Rights Reserved.
#

AUTO=false
while getopts ":aApy" c
do
	case $c in
	a|A|p|y)	AUTO=true;;
	esac
done
eval DEV=\${$#}
if [ ! -e $DEV ]; then
	echo "$0: $DEV does not exist"
	exit 8
fi
if $AUTO; then
	echo "$0: XFS file system."
else
	echo "If you wish to check the consistency of an XFS filesystem or"
	echo "repair a damaged filesystem, see xfs_repair(8)."
fi
exit 0
