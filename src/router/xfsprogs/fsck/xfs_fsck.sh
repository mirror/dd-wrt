#!/bin/sh -f
#
# Copyright (c) 2006 Silicon Graphics, Inc.  All Rights Reserved.
#

NAME=$0

# get the right return code for fsck
repair2fsck_code() {
	case $1 in
	0)  return 0 # everything is ok
		;;
	1)  echo "$NAME error: xfs_repair could not fix the filesystem." 1>&2
		return 4 # errors left uncorrected
		;;
	2)  echo "$NAME error: The filesystem log is dirty, mount it to recover" \
		     "the log. If that fails, refer to the section DIRTY LOGS in the" \
		     "xfs_repair manual page." 1>&2
		return 4 # dirty log, don't do anything and let the user solve it
		;;
	4)  return 1 # The fs has been fixed
		;;
	127)
		echo "$NAME error: xfs_repair was not found!" 1>&2
		return 4
		;;
	*)  echo "$NAME error: An unknown return code from xfs_repair '$1'" 1>&2
		return 4 # something went wrong with xfs_repair
	esac
}

AUTO=false
FORCE=false
REPAIR=false
while getopts ":aApyf" c
do
	case $c in
	a|A|p)		AUTO=true;;
	y)		REPAIR=true;;
	f)      	FORCE=true;;
	esac
done
eval DEV=\${$#}
if [ ! -e $DEV ]; then
	echo "$0: $DEV does not exist"
	exit 8
fi

# The flag -f is added by systemd/init scripts when /forcefsck file is present
# or fsck.mode=force is used during boot; an unclean shutdown won't trigger
# this check, user has to explicitly require a forced fsck.
# But first of all, test if it is a non-interactive session.
# Invoking xfs_repair via fsck.xfs is only intended to happen via initscripts.
# Normal administrative filesystem repairs should always invoke xfs_repair
# directly.
#
# Use multiple methods to capture most of the cases:
# The case for *i* and -n "$PS1" are commonly suggested in bash manual
# and the -t 0 test checks stdin
case $- in
	*i*) FORCE=false ;;
esac
if [ -n "$PS1" -o -t 0 ]; then
	FORCE=false
fi

if $FORCE; then
	xfs_repair -e $DEV
	error=$?
	if [ $error -eq 2 ] && [ $REPAIR = true ]; then
		echo "Replaying log for $DEV"
		mkdir -p /tmp/repair_mnt || exit 1
		for x in $(cat /proc/cmdline); do
			case $x in
				root=*)
					ROOT="${x#root=}"
				;;
				rootflags=*)
					ROOTFLAGS="-o ${x#rootflags=}"
				;;
			esac
		done
		test -b "$ROOT" || ROOT=$(blkid -t "$ROOT" -o device)
		if [ $(basename $DEV) = $(basename $ROOT) ]; then
			mount $DEV /tmp/repair_mnt $ROOTFLAGS || exit 1
		else
			mount $DEV /tmp/repair_mnt || exit 1
		fi
		umount /tmp/repair_mnt
		xfs_repair -e $DEV
		error=$?
		rm -d /tmp/repair_mnt
	fi
	repair2fsck_code $error
	exit $?
fi

if $AUTO; then
	echo "$0: XFS file system."
else
	echo "If you wish to check the consistency of an XFS filesystem or"
	echo "repair a damaged filesystem, see xfs_repair(8)."
fi
exit 0
