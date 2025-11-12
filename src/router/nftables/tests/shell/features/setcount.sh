#!/bin/bash

$NFT -f - <<EOF
table ip t {
	set s {
		type ipv4_addr
		size 2
		elements = { 1.2.3.4 }
	}
}
EOF

$NFT list set ip t s | grep -q 'size 2	# count 1'
