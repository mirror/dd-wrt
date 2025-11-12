#!/bin/bash

# regression check for kernel commit
# cff3bd012a95 ("netfilter: nf_tables: prefer nft_chain_validate")

chains=100

# first create skeleton, linear list
# of 1k jumps, c1 -> c2 .. -> c100.
#
# not caught, commit phase validation doesn't care about
# non-base chains.
(
	echo add table ip t

	for i in $(seq 1 $chains);do
		echo add chain t c$i
	done

	for i in $(seq 1 $((chains-1)) );do
		echo add rule t c$i jump c$((i+1))
	done
) | $NFT -f -

# now link up c0 to c1.  This triggers register-store validation for
# c1. Old algorithm is recursive and will blindly chase the entire
# list of chains created above.  On older kernels, this will cause kernel
# stack overflow/guard page crash.
$NFT -f - <<EOF
add chain t c0 { type filter hook input priority 0; }
add rule t c0 jump c1
EOF

if [ $? -eq 0 ] ; then
        echo "E: loaded bogus ruleset" >&2
        exit 1
fi

$NFT delete table ip t
