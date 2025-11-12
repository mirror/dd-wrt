#!/bin/bash

# d62d0ba97b58 ("netfilter: nf_tables: Introduce stateful object update operation")
# v5.4-rc1~131^2~59^2~2

set -e
$NFT add table test-ip
$NFT add quota test-ip traffic-quota 25 mbytes
$NFT add quota test-ip traffic-quota 50 mbytes

EXPECTED="table ip test-ip {
	quota traffic-quota {
		50 mbytes
	}
}"

GET="$($NFT list ruleset)"
if [ "$EXPECTED" != "$GET" ] ; then
	diff -u <(echo "$EXPECTED") <(echo "$GET")
	exit 1
fi
