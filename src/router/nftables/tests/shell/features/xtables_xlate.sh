#!/bin/bash

# Does nft support translating compat extensions using libxtables?
# Answer a related question first: Do we have a usable iptables-nft available?

iptables-nft --version | grep -q nf_tables || {
	echo "iptables-nft not available or not nft-variant"
	exit 1
}

ns=$(mktemp -u ns-XXXXXX)
trap "ip netns del $ns" EXIT
ip netns add $ns || exit 1

ext_arg="-m comment --comment foobar"
ip netns exec $ns iptables-nft -vv -A FORWARD $ext_arg | \
		grep -q "match name comment" || {
	echo "comment match does not use compat extension?!"
	exit 1
}
ip netns exec $ns $NFT list chain ip filter FORWARD 2>/dev/null | grep -q "foobar"
