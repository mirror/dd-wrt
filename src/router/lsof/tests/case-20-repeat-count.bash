#!/usr/bin/env bash
source tests/common.bash
# need gnu grep for -e in solaris
export PATH=/usr/gnu/bin:$PATH
base=$(pwd)

output=$(${lsof} -r 1c1 -p $$)
echo "$output" >> $report
if [ $(echo "$output" | grep -e '=======' | wc -l) != 1 ]; then
    exit 1
fi

output=$(${lsof} -r 1c5 -p $$)
echo "$output" >> $report
if [ $(echo "$output" | grep -e '=======' | wc -l) != 5 ]; then
    exit 1
fi

exit 0
