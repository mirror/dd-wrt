#!/bin/bash
source tests/common.bash

MQUEUE_MNTPOINT=/tmp/$$

TARGET=$tcasedir/mq_fork
if ! [ -x $TARGET ]; then
    echo "target executable ( $TARGET ) is not found" >> $report
    exit 1
fi

if grep -q mqueue /proc/mounts; then
    :
elif ! [ $(id -u) = 0 ]; then
    echo "root privileged is needed to run $(basename $0 .sh), skipping" >> $report
    exit 77
else
    mkdir -p ${MQUEUE_MNTPOINT}
    if ! mount -t mqueue none ${MQUEUE_MNTPOINT}; then
	echo "failed to mount mqueue file system, skipping"
	exit 77
    fi
fi

umount_mqueue()
{
    if [ -d ${MQUEUE_MNTPOINT} ]; then
	umount ${MQUEUE_MNTPOINT}
	rmdir ${MQUEUE_MNTPOINT}
    fi
}

mqname=/lsof-$name-$$
{ $TARGET $mqname & } | {
    read parent child fd
    if [ -z "$parent" ] || [ -z "$child" ] || [ -z "$fd" ]; then
	echo "unexpected output form target ( $TARGET )" >> $report
	umount_mqueue
	exit 1
    fi
    {
	echo mqname: $mqname
	echo parent: $parent
	echo child:  $child
	echo fd:    $fd
	echo cmdline: "$lsof +E -p "$parent""
	echo
	echo PARENT
	echo
	$lsof +E -p "$parent"
	echo
	echo CHILD
	echo
	$lsof +E -p "$child"
    } >> $report
    {
	{
	    echo From the parent side
	    # mq_fork 18020 yamato    3u  PSXMQ   0,18       80     622461 /xxx 18021,mq_fork,3u
	    echo expected pattern: "mq_fork *$parent .* ${fd}u *PSXMQ .* $mqname $child,mq_fork,${fd}u"
	    $lsof +E -p "$parent" |
		grep -q "mq_fork *${parent} .* ${fd}u *PSXMQ .* ${mqname} ${child},mq_fork,${fd}u"
	} && {
	    echo From the parent side
	    # mq_fork 18021 yamato    3u  PSXMQ   0,18       80     622461 /xxx 18020,mq_fork,3u
	    echo expected pattern: "mq_fork *$child .* ${fd}u *PSXMQ .* $mqname $parent,mq_fork,${fd}u"
	    $lsof +E -p "$parent" |
		grep -q "mq_fork *${child} .* ${fd}u *PSXMQ .* ${mqname} ${parent},mq_fork,${fd}u"
	} && {
	    echo From the child side
	    # mq_fork 18021 yamato    3u  PSXMQ   0,18       80     622461 /xxx 18020,mq_fork,3u
	    echo expected pattern: "mq_fork *$child .* ${fd}u *PSXMQ .* $mqname $parent,mq_fork,${fd}u"
	    $lsof +E -p "$child" |
		grep -q "mq_fork *${child} .* ${fd}u *PSXMQ .* ${mqname} ${parent},mq_fork,${fd}u"
	} && {
	    echo From the child side
	    # mq_fork 18020 yamato    3u  PSXMQ   0,18       80     622461 /xxx 18021,mq_fork,3u
	    echo expected pattern: "mq_fork *$parent .* ${fd}u *PSXMQ .* $mqname $child,mq_fork,${fd}u"
	    $lsof +E -p "$child" |
		grep -q "mq_fork *${parent} .* ${fd}u *PSXMQ .* ${mqname} ${child},mq_fork,${fd}u"
	} && {
	    kill "$child"
	    umount_mqueue
	    exit 0
	}
    } >> $report
    kill "$child"
    umount_mqueue
    exit 1
}
