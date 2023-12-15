#!/bin/bash
source tests/common.bash

if [ -z "$(nc -h 2>&1 | grep '\s\-4')" ]; then
    echo "nc does not support -4 option, skipping" >> $report
    exit 77
fi

nc -l -4 127.0.0.1 10000 > /dev/null < /dev/zero &
server=$!
sleep 1
nc -4 -s 127.0.0.2 -p 9999 127.0.0.1 10000 < /dev/zero  > /dev/null &
client=$!

sleep 1

killBoth()
{
    kill -9 $1
    sleep 1
    kill -9 $2
} 2> /dev/null > /dev/null

fclient=/tmp/${name}-client-$$
$lsof -n -E -P -p $client > $fclient
if ! cat $fclient | grep -q "TCP 127.0.0.2:9999->127.0.0.1:10000 $server,nc,[0-9]\+u (ESTABLISHED)"; then
    echo "failed in client side" >> $report
    cat $fclient >> $report
    killBoth $client $server
    exit 1
fi

fserver=/tmp/${name}-server-$$
$lsof -n -E -P -p $server > $fserver
if ! cat $fserver | grep -q "TCP 127.0.0.1:10000->127.0.0.2:9999\+ $client,nc,[0-9]\+u (ESTABLISHED)"; then
    echo "failed in server side" >> $report
    cat $fserver >> $report
    killBoth $client $server
    exit 1
fi

killBoth $client $server

exit 0
