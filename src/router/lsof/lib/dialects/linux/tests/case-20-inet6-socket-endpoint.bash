#!/bin/bash
source tests/common.bash

nc -l -6 ::1 10000 > /dev/null < /dev/zero 2>> $report &
server=$!
sleep 1
nc -6 -s ::1 -p 9999 ::1 10000 < /dev/zero  > /dev/null 2>> $report &
client=$!
sleep 1

if ! kill -0 $server 2>/dev/null; then
    echo "Maybe ipv6 stack is not available on this system, skipping" >> $report
    exit 77
fi

sleep 1

killBoth()
{
    kill -9 $1
    sleep 1
    kill -9 $2
} 2> /dev/null > /dev/null

fclient=/tmp/${name}-client-$$
$lsof -n -E -P -p $client > $fclient
if ! cat $fclient | grep -q "TCP \[::1\]:9999->\[::1\]:10000 $server,nc,[0-9]\+u (ESTABLISHED)"; then
    echo "failed in client side" >> $report
    cat $fclient >> $report
    killBoth $client $server
    exit 1
fi

fserver=/tmp/${name}-server-$$
$lsof -n -E -P -p $server > $fserver
if ! cat $fserver | grep -q "TCP \[::1\]:10000->\[::1\]:9999\+ $client,nc,[0-9]\+u (ESTABLISHED)"; then
    echo "failed in server side" >> $report
    cat $fserver >> $report
    killBoth $client $server
    exit 1
fi

killBoth $client $server

exit 0
