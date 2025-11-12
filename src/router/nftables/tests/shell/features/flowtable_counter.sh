#!/bin/bash

# 53c2b2899af7 ("netfilter: flowtable: add counter support")
# v5.7-rc1~146^2~12^2~16

EXPECTED="table ip filter2 {
	flowtable main_ft2 {
		hook ingress priority filter
		devices = { \"lo\" }
		counter
	}
}"

$NFT -f - <<< $EXPECTED

diff -u <($NFT list ruleset) - <<<"$EXPECTED"
