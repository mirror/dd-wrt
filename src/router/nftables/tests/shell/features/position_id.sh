#!/bin/bash

# 75dd48e2e420 ("netfilter: nf_tables: Support RULE_ID reference in new rule")
# v5.1-rc1~178^2~405^2~27

EXPECTED="table inet t {
	chain c {
		tcp dport 1234 accept
		udp dport 4321 accept
		accept
	}
}"

RULESET="add table inet t
add chain inet t c
add rule inet t c tcp dport 1234 accept
add rule inet t c accept
insert rule inet t c index 1 udp dport 4321 accept
"

$NFT -f - <<< $RULESET

diff -u <($NFT list ruleset) - <<<"$EXPECTED"
