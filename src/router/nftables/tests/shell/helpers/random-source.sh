#!/bin/bash

# Commands like `sort` and `shuf` have a "--random-source" argument, for
# generating a stable, reproducible output. However, they require an input
# that provides sufficiently many bytes (depending on the input).
#
# This script generates a stream that can be used like
#
#     shuf --random-source=<($0 "$seed")

seed=""
for a; do
	seed="$seed${#a}:$a\n"
done

if command -v openssl &>/dev/null ; then
	# We have openssl. Use it.
	# https://www.gnu.org/software/coreutils/manual/html_node/Random-sources.html#Random-sources
	#
	# Note that we don't care that different installations/architectures generate the
	# same output.
	openssl enc -aes-256-ctr -pass "pass:$seed" -nosalt </dev/zero 2>/dev/null
else
	# Hack something. It's much slower.
	idx=0
	while : ; do
		idx="$((idx++))"
		seed="$(sha256sum <<<"$idx.$seed")"
		echo ">>>$seed" >> a
		seed="${seed%% *}"
		LANG=C awk -v s="$seed" 'BEGIN{
			for (i=1; i <= length(s); i+=2) {
				xchar = substr(s, i, 2);
				decnum = strtonum("0x"xchar);
				printf("%c", decnum);
			}
		}' || break
	done
fi
exit 0
