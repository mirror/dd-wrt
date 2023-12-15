#!/usr/bin/env bash
# See https://github.com/lsof-org/lsof/issues/128
source tests/common.bash

lsof0="$lsof"
lsof=

f=/tmp/lsof-${name}-$$
r=0

{
    lsof="$lsof0"
    $lsof $f > /dev/null
    s=$?
    case $s in
	1)
	    echo "ok: $lsof $f => 1"
	    ;;
	*)
	    echo "unexpected exit status: $s"
	    echo "	cmdline: $lsof $f"
	    r=1
	    ;;
    esac

    lsof="$lsof0 -Q"
    $lsof $f > /dev/null
    s=$?
    case $s in
	0)
	    echo "ok: $lsof $f => 0"
	    ;;
	*)
	    echo "unexpected exit status: $s"
	    echo "	cmdline: $lsof $f"
	    r=1
	    ;;
    esac

    lsof="$lsof0"
    touch $f
    $lsof $f > /dev/null
    s=$?
    case $s in
	1)
	    echo "ok: touch $f; $lsof $f => 1"
	    rm $f
	    ;;
	*)
	    echo "unexpected exit status: $2"
	    echo "	cmdline: $lsof $f"
	    rm $f
	    r=1
	    ;;
    esac

    lsof="$lsof0 -Q"
    touch $f
    $lsof $f > /dev/null
    s=$?
    case $s in
	0)
	    echo "ok: touch $f; $lsof $f => 1"
	    rm $f
	    ;;
	*)
	    echo "unexpected exit status: $2"
	    echo "	cmdline: $lsof $f"
	    rm $f
	    r=1
	    ;;
    esac

    g=/dev/null
    cat < /dev/zero > $g &
    pid=$!

    lsof="$lsof"
    $lsof $g > /dev/null
    s=$?
    case $s in
	0)
	    echo "ok: lsof $g => 0"
	    ;;
	*)
	    echo "unexpected exit status: $s"
	    echo "	cmdline: $lsof $g"
	    r=1
	    ;;
    esac

    lsof="$lsof -Q"
    $lsof $g > /dev/null
    s=$?
    case $s in
	0)
	    echo "ok: lsof $g => 0"
	    ;;
	*)
	    echo "unexpected exit status: $s"
	    echo "	cmdline: $lsof $g"
	    r=1
	    ;;
    esac

    kill $pid
    exit $r
} > $report 2>&1
