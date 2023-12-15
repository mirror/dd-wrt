#!/usr/bin/env bash
source tests/common.bash

f=/tmp/lsof-${name}-$$

{
    # should fail if not exists
    $lsof $f > /dev/null
    s=$?
    case $s in
	1)
	    echo "ok: $lsof $f => 1"
	    ;;
	*)
	    echo "unexpected exit status: $s"
	    echo "	cmdline: $lsof $f"
	    exit 1
	    ;;
    esac

    # should fail if no one opens the file
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
	    exit 1
	    ;;
    esac

    # should succeed if some one opens the file
    g=/dev/null
    cat < /dev/zero > $g &
    pid=$!
    $lsof $g > /dev/null
    s=$?
    case $s in
	0)
	    echo "ok: lsof $g => 0"
	    kill $pid
	    ;;
	*)
	    echo "unexpected exit status: $s"
	    echo "	cmdline: $lsof $g"
	    kill $pid
	    exit 1
	    ;;
    esac
} > $report 2>&1


