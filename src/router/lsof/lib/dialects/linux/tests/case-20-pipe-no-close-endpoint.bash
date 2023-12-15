#!/bin/bash
source tests/common.bash

TARGET=$tcasedir/pipe
if ! [ -x $TARGET ]; then
    echo "target executable ( $TARGET ) is not found" >> $report
    exit 1
fi

{ $TARGET no-close & } | {
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
	    # pipe-no-c 25113 yamato    3r  FIFO   0,12      0t0     616532 pipe 25113,pipe-no-c,4w 25114,pipe-no-c,3r 25114,pipe-no-c,4w
	    echo expected pattern: ".* $parent .* ${fdr}r *FIFO .* pipe ${parent},p[-a-z]*,${fdw}w ${child},p[-a-z]*,${fdr}r ${child},p[-a-z]*,${fdw}w"
	    $lsof +E -p "$parent" |
		grep -q ".* $parent .* ${fdr}r *FIFO .* pipe ${parent},p[-a-z]*,${fdw}w ${child},p[-a-z]*,${fdr}r ${child},p[-a-z]*,${fdw}w"
	} && {
	    # pipe-no-c 25113 yamato    4w  FIFO   0,12      0t0     616532 pipe 25113,pipe-no-c,3r 25114,pipe-no-c,3r 25114,pipe-no-c,4w
	    echo expected pattern: ".* $parent .* ${fdw}w *FIFO .* pipe ${parent},p[-a-z]*,${fdr}r ${child},p[-a-z]*,${fdr}r ${child},p[-a-z]*,${fdw}w"
	    $lsof +E -p "$parent" |
		grep -q ".* $parent .* ${fdw}w *FIFO .* pipe ${parent},p[-a-z]*,${fdr}r ${child},p[-a-z]*,${fdr}r ${child},p[-a-z]*,${fdw}w"
	} && {
	    # pipe-no-c 25114 yamato    3r  FIFO   0,12      0t0     616532 pipe 25113,pipe-no-c,3r 25113,pipe-no-c,4w 25114,pipe-no-c,4w
	    echo expected pattern: ".* $child .* ${fdr}r *FIFO .* pipe ${parent},p[-a-z]*,${fdr}r ${parent},p[-a-z]*,${fdw}w ${child},p[-a-z]*,${fdw}w"
	    $lsof +E -p "$parent" |
		grep -q ".* $child .* ${fdr}r *FIFO .* pipe ${parent},p[-a-z]*,${fdr}r ${parent},p[-a-z]*,${fdw}w ${child},p[-a-z]*,${fdw}w"
	} && {
	    # pipe-no-c 25114 yamato    4w  FIFO   0,12      0t0     616532 pipe 25113,pipe-no-c,3r 25113,pipe-no-c,4w 25114,pipe-no-c,3r
	    echo expected parent: ".* $child .* ${fdw}w *FIFO .* pipe ${parent},p[-a-z]*,${fdr}r ${parent},p[-a-z]*,${fdw}w ${child},p[-a-z]*,${fdr}r"
	    $lsof +E -p "$parent" |
		grep -q ".* $child .* ${fdw}w *FIFO .* pipe ${parent},p[-a-z]*,${fdr}r ${parent},p[-a-z]*,${fdw}w ${child},p[-a-z]*,${fdr}r"
	} && {
	    kill $child
	    exit 0
	}
    } >> $report
    kill $child
    exit 1
}
