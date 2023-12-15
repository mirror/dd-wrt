#!/bin/bash
source tests/common.bash

TARGET=$tcasedir/pipe
if ! [ -x $TARGET ]; then
    echo "target executable ( $TARGET ) is not found" >> $report
    exit 1
fi

{ $TARGET & } | {
    read parent child fdr fdw;
    if [ -z "$parent" ] || [ -z "$child" ] || [ -z "$fdr" ] || [ -z "$fdw" ]; then
	echo "unexpected output form target ( $TARGET )" >> $report
	exit 1
    fi
    echo parent: $parent >> $report
    echo child:  $child >> $report
    echo fdr:    $fdr >> $report
    echo fdw:    $fdw >> $report
    echo cmdline: "$lsof +E -p "$parent"" >> $report
    $lsof +E -p "$parent" >> $report

    {
	{
	    echo expected pattern: ".* $parent .* ${fdr}r *FIFO .* pipe ${child},p[-a-z]*,${fdw}w"
	    $lsof +E -p "$parent" |
		grep -q ".* $parent .* ${fdr}r *FIFO .* pipe ${child},p[-a-z]*,${fdw}w"
	} && {
	    echo expected parent: ".* $child .* ${fdw}w *FIFO .* pipe ${parent},p[-a-z]*,${fdr}r"
	    $lsof +E -p "$parent" |
		grep -q ".* $child .* ${fdw}w *FIFO .* pipe ${parent},p[-a-z]*,${fdr}r"
	} && {
	    kill "$child"
	    exit 0
	}
    } >> $report
    kill "$child"
    exit 1
}
