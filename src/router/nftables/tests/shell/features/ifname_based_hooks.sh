#!/bin/bash

# check if adding a netdev-family chain hooking into a non-existent device is
# accepted or not

RULESET="table netdev t {
	chain c {
		type filter hook ingress priority 0
		devices = { foobar123 }
	}
}"
unshare -n $NFT -f - <<< "$RULESET"
