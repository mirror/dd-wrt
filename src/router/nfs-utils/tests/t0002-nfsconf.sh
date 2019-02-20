#!/bin/bash

TESTFILES=nfsconf
DUMPER=`realpath ../tools/nfsconf/nfsconftool`

BASEDIR=`dirname "$0"`
pushd $BASEDIR/$TESTFILES

rm -f *.out

TCOUNT=0
TPASS=0

for i in *.conf
do
TNAME=`basename "$i" .conf`

echo "Running test $TNAME"
TCOUNT=$((TCOUNT + 1))

if ! $DUMPER --file "$i" --dump - > "$TNAME.out" 2>&1
then
echo "Error running test $TNAME"
elif ! diff -u "$TNAME.exp" "$TNAME.out"
then
echo "FAIL differences detected in test $TNAME"
else
echo "PASS $TNAME"
TPASS=$((TPASS + 1))
fi

done

echo "nfsconf tests complete. $TPASS of $TCOUNT tests passed"

if [ $TPASS -lt $TCOUNT ]; then
exit 1
fi
