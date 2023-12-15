#!/bin/bash
source tests/common.bash

if ! [ -r "/proc/self/stack" ]; then
    echo "this platform doesn't allow to access /proc/\$PID/stack, skipping" >> $report
    exit 77
fi

if [ -z "$(nc -h 2>&1 | grep '\-U')" ]; then
    echo "nc does not support unix socket, skipping" >> $report
    exit 77
fi

ux=/tmp/$name-$$.sock
nc -l -U $ux > /dev/null < /dev/zero &
server=$!

killBoth()
{
    kill -9 $1
    sleep 1
    kill -9 $2
} 2> /dev/null > /dev/null

waitForSyscall()
{
    local pid=$1
    local pat=$2
    local niterations=$3
    local i

    for i in $(seq 0 $niterations); do
	sleep 1
	if grep -q "$pat" /proc/$pid/stack; then
	    break
	fi
    done
}

waitForSyscall $server 'select\|poll' 10

fserver=/tmp/${name}-server-$$-before
$lsof -n -Ts -P -U -a -p $server > $fserver
# nc        22512 yamato    3u  unix 0x000000008f6993b8      0t0     470697 /tmp/a type=STREAM (LISTEN)
if ! cat $fserver | grep -q "^.* unix 0x[0-9a-f]\+ \+0t0 \+[0-9]\+ $ux type=STREAM (LISTEN)"; then
    echo "failed in server side (before connecting)" >> $report
    cat $fserver >> $report
    kill -9 $server
    rm $ux
    exit 1
fi

nc -U $ux < /dev/zero  > /dev/null &
client=$!
sleep 1
fserver=/tmp/${name}-server-$$-after
$lsof -n -Ts -P -U -a -p $server > $fserver
# nc      22512 yamato    4u  unix 0x00000000deffde05      0t0 472699 /tmp/a type=STREAM (CONNECTED)
if ! cat $fserver | grep -q "^.* unix 0x[0-9a-f]\+ \+0t0 \+[0-9]\+ $ux type=STREAM (CONNECTED)"; then
    echo "failed in server side (after connecting)" >> $report
    cat $fserver >> $report
    killBoth $client $server
    rm $ux
    exit 1
fi

fclient=/tmp/${name}-client-$$
$lsof -n -Ts -P -U -a -p $client -FT | grep ^TST > $fclient
# TST=CONNECTED
if ! cat $fclient | grep -q "^TST=CONNECTED"; then
    echo "failed in client side" >> $report
    cat $fclient >> $report
    killBoth $client $server
    rm $ux
    exit 1
fi

killBoth $client $server
rm $ux

exit 0
