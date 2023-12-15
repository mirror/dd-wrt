#!/bin/bash
source tests/common.bash

TARGET=$tcasedir/pidfd

{
$TARGET | (
    read pid fd
    if [[ $pid = -1 && $fd = -1 ]]; then
        echo "pidfd is not available on this platform, skipping"
        exit 77
    fi
    line=$($lsof -p $pid -a -d $fd -F pfn| tr '\n' ' ')
    if ! fgrep -q "p${pid} f${fd} n[pidfd:$pid]" <<<"$line"; then
	$lsof -p $pid -a -d $fd -F pfn
	echo
	echo $line
	echo
	r=1
    fi
    kill $pid
    exit $r
)
} >> $report 2>&1
