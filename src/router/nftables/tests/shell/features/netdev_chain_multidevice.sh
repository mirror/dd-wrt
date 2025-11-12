#!/bin/bash

# d54725cd11a5 ("netfilter: nf_tables: support for multiple devices per netdev hook")
# v5.5-rc1~174^2~312^2~4

trap "ip link del d0; ip link del d1" EXIT

ip link add d0 type dummy
ip link add d1 type dummy

EXPECTED="table netdev filter2 {
        chain Main_Ingress2 {
                type filter hook ingress devices = { \"d0\", \"d1\" } priority -500; policy accept;
        }
}"

$NFT -f - <<< $EXPECTED
