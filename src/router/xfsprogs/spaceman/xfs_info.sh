#!/bin/sh -f
# SPDX-License-Identifier: GPL-2.0
#
# Copyright (c) 2000-2001 Silicon Graphics, Inc.  All Rights Reserved.
#

OPTS=""
USAGE="Usage: xfs_info [-V] [-t mtab] [mountpoint|device|file]"

# Try to find a loop device associated with a file.  We only want to return
# one loopdev (multiple loop devices can attach to a single file) so we grab
# the last line and return it if it's actually a block device.
try_find_loop_dev_for_file() {
	local x="$(losetup -O NAME -j "$1" 2> /dev/null | tail -n 1)"
	test -b "$x" && echo "$x"
}

while getopts "t:V" c
do
	case $c in
	t)	OPTS="-t $OPTARG" ;;
	V)	xfs_spaceman -p xfs_info -V
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
	1)
		arg="$1"

		# See if we can map the arg to a loop device
		loopdev="$(try_find_loop_dev_for_file "${arg}")"
		test -n "${loopdev}" && arg="${loopdev}"

		# If we find a mountpoint for the device, do a live query;
		# otherwise try reading the fs with xfs_db.
		if mountpt="$(findmnt -t xfs -f -n -o TARGET "${arg}" 2> /dev/null)"; then
			xfs_spaceman -p xfs_info -c "info" $OPTS "${mountpt}"
			status=$?
		else
			xfs_db -p xfs_info -c "info" $OPTS "${arg}"
			status=$?
		fi
		;;
	*)	echo $USAGE 1>&2
		exit 2
		;;
esac
exit $status
