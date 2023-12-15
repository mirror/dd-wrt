#!/bin/bash
source tests/common.bash

TARGET=$tcasedir/epoll
if ! [ -x $TARGET ]; then
    echo "target executable ( $TARGET ) is not found" >> $report
    exit 1
fi

$TARGET 2>> $report | {
    read pid epfd evfd0 evfd1
    if [[ -z "$pid" || -z "$epfd" || -z "$evfd0" || -z "$evfd1" ]]; then
	echo "unexpected output form target ( $TARGET )" >> $report
	exit 1
    fi
    if ! [ -e "/proc/$pid" ]; then
	echo "the target process dead unexpectedly" >> $report
	exit 1
    fi
    {
	echo pid: $pid
	echo epfd: $epfd
	echo cmdline: "$lsof -p $pid -a -d $epfd"
	$lsof -p $pid -a -d $epfd
	echo done
    } >> $report
    if $lsof -p $pid -a -d $epfd |
	    grep -q "epoll *[0-9]* *.* *${epfd}u *a_inode *[0-9]*,[0-9]* *[0-9]* *[0-9]* *\[eventpoll:${evfd0},${evfd1}\]"; then
	kill $pid
	exit 0
    else
	kill $pid
	exit 1
    fi
}
