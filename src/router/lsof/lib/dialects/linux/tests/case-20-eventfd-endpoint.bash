#!/bin/bash
source tests/common.bash

uname -r >> $report
uname -r | sed -ne 's/^\([0-9]\+\)\.\([0-9]\+\)\.\([0-9]\+\).*/\1 \2/p' | {
    read major minor
    if [ "$major" -lt 5 ]; then
	echo "eventfd endpoint features doesn't work on Linux $major, skipping"
	exit 77
    fi
    if [ "$major" -eq 5 -a "$minor" -lt 2 ]; then
	echo "event endpoint features doesn't work on Linux $major.$minor, skipping"
	exit 77
    fi
} >> $report
s=$?
if ! [ $s = 0 ]; then
    exit $s
fi

TARGET=$tcasedir/eventfd
if ! [ -x $TARGET ]; then
    echo "target executable ( $TARGET ) is not found" >> $report
    exit 1
fi

{ $TARGET & } | {
    read parent child fd
    if [ -z "$parent" ] || [ -z "$child" ] || [ -z "$fd" ]; then
	echo "unexpected output form target ( $TARGET )" >> $report
	exit 1
    fi
    {
	echo parent: $parent
	echo child: $child
	echo fd: $fd
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
	    # eventfd 23685 yamato    3u  a_inode   0,13        0    11217 [eventfd:29] 23686,eventfd,3u
	    echo expected pattern: "eventfd *${parent} .* ${fd}u *a_inode .* \[eventfd:[0-9]*\] *${child},eventfd,${fd}u"
	    $lsof +E -p "$parent" |
		grep -q "eventfd *${parent} .* ${fd}u *a_inode .* \[eventfd:[0-9]*\] *${child},eventfd,${fd}u"
	} && {
	    echo From the parent side
	    # eventfd 23686 yamato    3u  a_inode   0,13        0    11217 [eventfd:29] 23685,eventfd,3u
	    echo expected pattern: "eventfd *${child} .* ${fd}u *a_inode .* \[eventfd:[0-9]*\] *${parent},eventfd,${fd}u"
	    $lsof +E -p "$parent" |
		grep -q "eventfd *${child} .* ${fd}u *a_inode .* \[eventfd:[0-9]*\] *${parent},eventfd,${fd}u"

	} && {
	    echo From the child side
	    # eventfd 23685 yamato    3u  a_inode   0,13        0    11217 [eventfd:29] 23686,eventfd,3u
	    echo expected pattern: "eventfd *${parent} .* ${fd}u *a_inode .* \[eventfd:[0-9]*\] *${child},eventfd,${fd}u"
	    $lsof +E -p "$parent" |
		grep -q  "eventfd *${parent} .* ${fd}u *a_inode .* \[eventfd:[0-9]*\] *${child},eventfd,${fd}u"
	} && {
	    echo From the child side
	    # eventfd 23686 yamato    3u  a_inode   0,13        0    11217 [eventfd:29] 23685,eventfd,3u
	    echo expected pattern: "eventfd *${child} .* ${fd}u *a_inode .* \[eventfd:[0-9]*\] *${parent},eventfd,${fd}u"
	    $lsof +E -p "$parent" |
		grep -q  "eventfd *${child} .* ${fd}u *a_inode .* \[eventfd:[0-9]*\] *${parent},eventfd,${fd}u"

	} && {
	    kill "$child"
	    exit 0
	}
    } >> $report
    kill "$child"
    exit 1
}
