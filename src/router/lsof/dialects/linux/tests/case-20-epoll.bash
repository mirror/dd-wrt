#!/bin/bash

lsof=$1
report=$2
tdir=$3

TARGET=$tdir/epoll
if ! [ -x $TARGET ]; then
    echo "target executable ( $TARGET ) is not found" >> $report
    exit 1
fi

$TARGET | {
    read pid epfd
    if [[ -z "$pid" || -z "$epfd" ]]; then
	echo "unexpected output form target ( $TARGET )" >> $report
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
	    grep -q "epoll *[0-9]* *.* *${epfd}u *a_inode *[0-9]*,[0-9]* *[0-9]* *[0-9]* *\[eventpoll:0,1,2\]"; then
	kill $pid
	exit 0
    else
	kill $pid
	exit 1
    fi
}
