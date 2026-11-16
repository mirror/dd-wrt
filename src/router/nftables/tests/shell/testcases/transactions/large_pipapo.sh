#!/bin/bash

# load+reload a large, randomly generated concat set

set -e

MAXSIZE=50000
tmpfile=$(mktemp)

cleanup()
{
	# error? Dump the failed set
	test -r "$tmpfile" && cat "$tmpfile"
	rm -f "$tmpfile"
}
trap cleanup EXIT

create_elements() {
	local howmany=$1
	local a=10
	local b=0
	local c=0
	local d=1
	local i=0

	while [ $i -le $howmany ];do
		local step=$((RANDOM%50))
		local port=$(((RANDOM%65535) + 1))

		d=$((d+step))
		if [ $d -ge 256 ];then
			d=1
			c=$((c+1))
		fi

		if [ $c -ge 256 ];then
			b=$((b+1))
			c=0
		fi

		if [ $b -ge 256 ];then
			a=$((a+1))
			b=0
		fi

		echo -n "$a.$b.$c.$d . $port"
		[ $i -lt $howmany ] && echo ", "
		i=$((i+1))
	done

	echo " }"
}

create_set() {
cat - <<EOF
table ip t {
	set s {
		typeof ip daddr . tcp dport
		flags interval
		elements = {
EOF

local size1=$((RANDOM+1))
local size2=$((RANDOM+1))

create_elements $(((size1 * size2) % MAXSIZE))

echo "}"
}

create_ruleset() {
	create_set
cat - <<EOF
	chain input {
		type filter hook input priority 0

		ip daddr . tcp dport @s counter
	}
}
EOF
}

recreate_set() {
	create_set

	# closes table
	echo "}"
}

create_ruleset > "$tmpfile"

# load set
$NFT -f "$tmpfile"

# flush + reload the same, then different sets
reloads=$(((RANDOM%5)+1))
for i in $(seq 1 $reloads); do
	echo reload:
	wc -l $tmpfile
	( echo flush set t s ; cat "$tmpfile" ) | $NFT -f -
	recreate_set > "$tmpfile"
done

rm -f "$tmpfile"
