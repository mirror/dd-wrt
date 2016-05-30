#!/bin/sh

JAVA=$1
CLASS=$2
TESTNAME=$3
TEST=$4
SRCDIR=$5

echo -n "$TESTNAME: "

$JAVA $CLASS > $TEST.thisoutput 2>&1

if [ $? -eq "0" ]; then
    # no Error returned
    if [ -f $SRCDIR/$TEST.2output ]; then
        # Error should have been returned
        echo "OK, but wrong return value: $?"
        head $TEST.thisoutput
        test -z "$CHECK_NO_FAIL_FAST" && exit 1 || exit
    fi

    cmp -s $SRCDIR/$TEST.output $TEST.thisoutput

    if [ $? -eq "0" ]; then
        echo "OK"
    else
        echo "FAILED"
        diff -u $SRCDIR/$TEST.output $TEST.thisoutput
        test -z "$CHECK_NO_FAIL_FAST" && exit 1
    fi

else
    # Error returned
    if [ ! -f $SRCDIR/$TEST.2output ]; then
        # No Error should have been returned
        echo "FAILED, but wrong return value: $?"
        head $TEST.this2output
        test -z "$CHECK_NO_FAIL_FAST" && exit 1 || exit
    fi

    cmp -s $SRCDIR/$TEST.2output $TEST.thisoutput

    if [ $? -eq "0" ]; then
        echo "OK"
    else
        echo "FAILED"
        diff -u $SRCDIR/$TEST.2output $TEST.thisoutput
        test -z "$CHECK_NO_FAIL_FAST" && exit 1
    fi
fi

rm -f $TEST.thisoutput
