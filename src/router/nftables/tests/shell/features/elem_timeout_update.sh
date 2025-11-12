#!/bin/bash

# 4201f3938914 ("netfilter: nf_tables: set element timeout update support")

$NFT -f - <<EOF
table ip t {
	set s {
		typeof ip saddr
		timeout 1m
		elements = { 1.2.3.4 }
	}
}
EOF

$NFT add element t s { 1.2.3.4 expires 1ms }

sleep 0.001
$NFT get element t s { 1.2.3.4 }

[ $? -eq 0 ] && exit 111

exit 0
