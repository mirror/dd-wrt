#!/bin/bash

# 65038428b2c6 ("netfilter: nf_tables: allow to specify stateful expression in set definition")
# v5.7-rc1~146^2~12^2~25

# NFT_SET_EXPR to detect kernel feature only available since
# b4e70d8dd9ea ("netfilter: nftables: add set expression flags")
# v5.11-rc3~39^2^2

EXPECTED="table ip x {
	set y {
		typeof ip saddr
		counter
	}
}"

$NFT -f - <<< $EXPECTED

diff -u <($NFT list ruleset) - <<<"$EXPECTED"
