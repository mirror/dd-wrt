#!/bin/bash

# v5.3-rc1~140^2~153^2~8
# 79ebb5bb4e38 ("netfilter: nf_tables: enable set expiration time for set elements")

RULESET="table ip x {
	set y {
		type ipv4_addr
		flags dynamic
		timeout 1h
	}
}"

$NFT -f - <<< $RULESET

$NFT add element ip x y { 1.1.1.1 timeout 1h expires 15m59s }

$NFT list ruleset | grep "expires 15m"
