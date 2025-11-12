#!/bin/bash

set -e

rnd=$(mktemp -u XXXXXXXX)
ns="nft-atomic-$rnd"
pid1=""
pid2=""
duration=8

cleanup()
{
	kill "$pid1" "$pid2"
	ip netns del "$ns"
}

trap cleanup EXIT

ip netns add "$ns" || exit 111
ip -net "$ns" link set lo up

ip netns exec "$ns" ping 127.0.0.1 -q -c 1

ip netns exec "$ns" $NFT -f - <<EOF
table ip t {
	set s {
		type ipv4_addr
		elements = { 127.0.0.1 }
	}

	chain input {
		type filter hook input priority 0; policy accept;
		ip protocol icmp counter
	}

	chain output {
		type filter hook output priority 0; policy accept;
		ip protocol icmp ip daddr @s drop
	}
}
EOF

ip netns exec "$ns" ping -f 127.0.0.1 &
pid1=$!
ip netns exec "$ns" ping -f 127.0.0.1 &
pid2=$!

time_now=$(date +%s)
time_stop=$((time_now + duration))
repl=0

while [ $time_now -lt $time_stop ]; do
ip netns exec "$ns" $NFT -f - <<EOF
flush chain ip t output
table ip t {
	chain output {
		type filter hook output priority 0; policy accept;
		ip protocol icmp ip daddr @s drop
	}
}
EOF
	repl=$((repl+1))

	# do at least 100 replaces and stop after $duration seconds.
	if [ $((repl % 101)) -eq 100 ];then
		time_now=$(date +%s)
	fi
done

# must match, all icmp packets dropped in output.
ip netns exec "$ns" $NFT list chain ip t input | grep "counter packets 0"

echo "Completed $repl chain replacements"
