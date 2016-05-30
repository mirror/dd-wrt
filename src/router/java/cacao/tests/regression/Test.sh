#!/bin/sh

JAVA=$1
TEST=$2
SRCDIR=$3

#
# test which classlibrary was used
# depending on whether classpath or openjdk class library was used we may expect different results from the tests, this may e.g.
# by simple things like intentation when printing stack traces or even more subtle stuff ...
#
$JAVA -XX:+PrintConfig 2>&1 | grep gnu.classpath.boot.library.path: > /dev/null
if [ $? -eq "0" ]; then
IS_CLASSPATH=1
POSTFIX="cp"
fi
$JAVA -XX:+PrintConfig 2>&1 | grep sun.boot.library.path  > /dev/null
if [ $? -eq "0" ]; then
IS_OPENJDK=1
POSTFIX="ojdk"
fi

if [ -z $POSTFIX ]; then
echo "Warning: Could not detect classlibrary the java VM uses, assuming openJDK"
POSTFIX="ojdk"
fi

# mostly classpath and openjdk deliver same results
REFERENCE_OUTPUT=$SRCDIR/$TEST.output
REFERENCE_2OUTPUT=$SRCDIR/$TEST.2output

# if they do not exist, we try the postfixed versions
if [ ! -f $REFERENCE_OUTPUT ]; then
REFERENCE_OUTPUT=$SRCDIR/$TEST.output.$POSTFIX
fi
if [ ! -f $REFERENCE_2OUTPUT ]; then
REFERENCE_2OUTPUT=$SRCDIR/$TEST.2output.$POSTFIX
fi

echo -n "$TEST: "

$JAVA $TEST > $TEST.thisoutputraw 2>&1
result=$?
sed 's/^[[:space:]][[:space:]]*\(at [^:]*\):\(.*\)/   \1)/' < $TEST.thisoutputraw > $TEST.thisoutput && rm -f $TEST.thisoutputraw

if [ $result -eq "0" ]; then
    # no Error returned
    if [ -f $REFERENCE_2OUTPUT ]; then
        # Error should have been returned
        echo "OK, but wrong return value: $?"
        head $TEST.thisoutput
        test -z "$CHECK_NO_FAIL_FAST" && exit 1 || exit
    fi

    cmp -s $REFERENCE_OUTPUT $TEST.thisoutput

    if [ $? -eq "0" ]; then
        echo "OK"
    else
        echo "FAILED"
        diff -u $REFERENCE_OUTPUT $TEST.thisoutput
        test -z "$CHECK_NO_FAIL_FAST" && exit 1
    fi

else
    # Error returned
    if [ ! -f $REFERENCE_2OUTPUT ]; then
        # No Error should have been returned
        echo "FAILED, but wrong return value: $?"
        head $TEST.this2output
        test -z "$CHECK_NO_FAIL_FAST" && exit 1 || exit
    fi

    cmp -s $REFERENCE_2OUTPUT $TEST.thisoutput

    if [ $? -eq "0" ]; then
        echo "OK"
    else
        echo "FAILED"
        diff -u $REFERENCE_2OUTPUT $TEST.thisoutput
        test -z "$CHECK_NO_FAIL_FAST" && exit 1
    fi
fi

rm -f $TEST.thisoutput
