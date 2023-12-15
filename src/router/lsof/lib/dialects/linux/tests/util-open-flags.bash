source tests/common.bash

pat=$5
tfile=$6

shift 6

TARGET=$tcasedir/open_with_flags
if ! [ -x $TARGET ]; then
    echo "target executable ( $TARGET ) is not found" >> $report
    exit 1
fi

export LC_ALL=C

log=/tmp/$name-$$.log
$TARGET $tfile "$@" 2>> $log | {
    read pid

    if  [[ -z "$pid" ]]; then
        cat $log >> $report
        # musl prints "Not supported" instead of "Operation not supported"
        if grep -E -q 'open: (Operation not supported|Not supported)' $log; then
            echo "a flag passed to open is not supported on this platform, skipping" >> $report
            exit 77
        fi
        echo "unexpected output from target ( $TARGET )" >> $report
	exit 1
    fi
    if ! [ -e "/proc/$pid" ]; then
        echo "the target process dead unexpectedly" >> $report
        exit 1
    fi

    echo "expected: $pat" >> $report
    echo "lsof output:" >> $report
    output=$($lsof +fg -p $pid)
    echo "$output" >> $report
    echo "$output" | grep -q "$pat"
    result=$?

    kill $pid
    exit $result
}
