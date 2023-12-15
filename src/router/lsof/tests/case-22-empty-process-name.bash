#!/usr/bin/env bash
# See https://github.com/lsof-org/lsof/issues/246
source tests/common.bash

{
    perl -e '$0 = ""; sleep 999' &
    pid=$!
    sleep 1

    output=$($lsof -w -ad cwd -F c -p "$pid")
    echo "lsof output:" >> $report
    echo "$output" >> $report
    kill $pid
    for entry in $output
    do
	if [[ $entry =~ ^p[0-9]+$ ]]; then
	    if [[ $entry != p$pid ]]; then
	        echo "Incorrect pid, expect p$pid, got $entry" >> $report
		exit 1
	    fi
	elif [[ $entry =~ c* ]]; then
	    if [[ $entry =~ cperl* ]]; then
	        echo "The platform does not report changed command name, that's okay" >> $report
	    elif [[ $entry != c ]]; then
	        echo "Process name should be empty, expect c, got $entry" >> $report
		exit 1
	    fi
	fi
    done
    exit 0
} > $report 2>&1
