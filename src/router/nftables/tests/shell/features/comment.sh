#!/bin/bash

# 002f21765320 ("netfilter: nf_tables: add userdata attributes to nft_chain")
# v5.10-rc1~107^2~60^2~5

EXPECTED="table ip x {
	chain y {
		comment \"test\"
	}
}"

$NFT -f - <<< $EXPECTED

diff -u <($NFT list ruleset) - <<<"$EXPECTED"
