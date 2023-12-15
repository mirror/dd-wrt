#!/bin/bash
source tests/common.bash

if [[ $(id -u) != 0 ]]; then
    echo "root privileged is needed to run $(basename $0. sh)" >> "${report}"
    exit 77
fi

#
# Derrived from the issue #102 opened by @zhrf2020.
#

v6addr=abcd:ef10:ffff:ffff:ffff:ffff:ffff:ff62
port=9999
dev=lo

if ! ip -6 address add "${v6addr}" dev "${dev}" 2>> "${report}"; then
    echo "failed to add ipv6 address "${v6addr}" to ${dev}, skipping" >> "${report}"
    exit 77
fi

ip -6 address show >> "${report}"

nc -6 -l "${v6addr}" "${port}" 2>> "${report}" &
pid=$!

sleep 1

expectation="n[${v6addr}]:$port"
result=1
if "${lsof}" -p "${pid}" -a -d fd -P -n -F n \
    | tee -a "${report}" \
    | fgrep -q "$expectation"; then
    result=0
fi

nc -6 "${v6addr}" "${port}" < /dev/null > /dev/null 2>> "${report}"
sleep 1

ip -6 address delete "${v6addr}" dev "${dev}" 2>> "${report}"

if [[ "${result}" != 0 ]]; then
    echo "failed to find \"$expectation\" in the outpuf of lsof" >> "${report}"
fi

exit "${result}"
