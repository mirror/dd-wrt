#!/bin/bash
source tests/common.bash

if [ -z "$(nc -h 2>&1 | grep '\-U')" ]; then
    echo "nc does not support unix socket, skipping" >> $report
    exit 77
fi

ux=/tmp/$name-$$.sock
nc -l -U $ux > /dev/null < /dev/zero &
server=$!
sleep 1
nc -U $ux < /dev/zero  > /dev/null &
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
if ! cat $fclient | grep -q "^.* unix 0x[0-9a-f]\+ \+0t0 \+[0-9]\+ type=STREAM ->INO=[0-9]\+ $server,nc,[0-9]\+u"; then
    echo "failed in client side" >> $report
    cat $fclient >> $report
    killBoth $client $server
    exit 1
fi

fserver=/tmp/${name}-server-$$
$lsof -n -E -P -p $server > $fserver
if ! cat $fserver | grep -q "^.* unix 0x[0-9a-f]\+ \+0t0 \+[0-9]\+ $ux type=STREAM ->INO=[0-9]\+ $client,nc,[0-9]\+u"; then
    echo "failed in server side" >> $report
    cat $fserver >> $report
    killBoth $client $server
    exit 1
fi

killBoth $client $server

exit 0
