#!/bin/bash
source tests/common.bash

uname -r >> $report
uname -r | sed -ne 's/^\([0-9]\+\)\.\([0-9]\+\)\.\([0-9]\+\).*/\1 \2/p' | {
    read major minor
    if [ "$major" -lt 4 ]; then
	echo "pty endpoint features doesn't work on Linux $major, skipping"
	exit 77
    fi
    if [ "$major" -eq 4 -a "$minor" -lt 13 ]; then
	echo "pty endpoint features doesn't work on Linux $major.$minor, skipping"
	exit 77
    fi
} >> $report
s=$?
if ! [ $s = 0 ]; then
    exit $s
fi

TARGET=$tcasedir/pty
if ! [ -x $TARGET ]; then
    echo "target executable ( $TARGET ) is not found" >> $report
    exit 1
fi

{ $TARGET & } | {
    read parent child fdm fds names;
    if [ -z "$parent" ] || [ -z "$child" ] || [ -z "$fdm" ] || [ -z "$fds" ] || [ -z "$names" ]; then
	echo "unexpected output form target ( $TARGET )" >> $report
	exit 1
    fi
    {
	echo parent: $parent
	echo child:  $child
	echo fdm:    $fdm
	echo fds:    $fds
	echo nams:   $names
	echo cmdline: "$lsof +E -p $parent"
    } >> $report
    $lsof +E -p "$parent" >> $report
    {
	{
	    # pty     17592 yamato    3r   CHR    5,2      0t0       1129 /dev/ptmx ->/dev/pts/16 17592,pty,4r 17593,pty,3r
	    echo expected pattern: "pty *$parent .* ${fdm}r *CHR .* /dev/(pts/)?ptmx ->/dev/pts/$names ($parent,pty,${fds}r $child,pty,${fdm}r)|($child,pty,${fdm}r $parent,pty,${fds}r)"
	    $lsof +E -p "$parent" |
		grep -E -q "pty *$parent .* ${fdm}r *CHR .* /dev/(pts/)?ptmx ->/dev/pts/$names ($parent,pty,${fds}r $child,pty,${fdm}r)|($child,pty,${fdm}r $parent,pty,${fds}r)"
	} && {
	    # pty     17592 yamato    4r   CHR 136,16      0t0         19 /dev/pts/16 17592,pty,3r
	    echo expected pattern: "pty *$parent .* ${fds}r *CHR .* /dev/pts/$names $parent,pty,${fdm}r"
	    $lsof +E -p "$parent" |
		grep -E -q "pty *$parent .* ${fds}r *CHR .* /dev/pts/$names $parent,pty,${fdm}r"
	} && {
	    # pty     17593 yamato    3r   CHR 136,16      0t0         19 /dev/pts/16 17592,pty,3r
	    echo expected pattern: "pty *$child .* ${fdm}r *CHR .* /dev/pts/$names $parent,pty,${fdm}r"
	    $lsof +E -p "$parent" |
		grep -E -q "pty *$child .* ${fdm}r *CHR .* /dev/pts/$names $parent,pty,${fdm}r"
	} && {
	    # pty     17592 yamato    3r   CHR    5,2      0t0       1129 /dev/ptmx ->/dev/pts/16 17592,pty,4r 17593,pty,3r
	    echo expected pattern: "pty *$parent .* ${fdm}r *CHR .* /dev/(pts/)?ptmx ->/dev/pts/$names ($parent,pty,${fds}r $child,pty,${fdm}r)|($child,pty,${fdm}r $parent,pty,${fds}r)"
	    $lsof +E -p "$child" |
		grep -E -q "pty *$parent .* ${fdm}r *CHR .* /dev/(pts/)?ptmx ->/dev/pts/$names ($parent,pty,${fds}r $child,pty,${fdm}r)|($child,pty,${fdm}r $parent,pty,${fds}r)"
	} && {
	    # pty     17593 yamato    3r   CHR 136,16      0t0         19 /dev/pts/16 17592,pty,3r
	    echo expected pattern: "pty *$child .* ${fdm}r *CHR .* /dev/pts/$names $parent,pty,${fdm}r"
	    $lsof +E -p "$child" |
		grep -E -q "pty *$child .* ${fdm}r *CHR .* /dev/pts/$names $parent,pty,${fdm}r"
	} && {
	    kill "$child"
	    exit 0
	}
    } >> $report
    kill "$child"
    exit 1
}
