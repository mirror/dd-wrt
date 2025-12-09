#!/bin/bash

# convenience script to check afl reproducers. Many reproducers are
# variations of the same bug/root cause, so this allows to check if
# we get same assertion.

pts=$(readlink /proc/self/fd/0)

files=$(find tests/afl++/out/*/crashes/ -type f -name 'id:0000*')

TMP=""

cleanup()
{
	rm -rf "$TMP"
}

trap cleanup EXIT

[ -z "$files" ] && exit 0

TMP=$(mktemp -d)

prompt_del()
{
	local f="$1"

	read yn < "$pts"
	if [ "$yn" = "y" ];then
		echo delete
		rm -f "$f"
	elif [ "$yn" = "n" ]; then
		echo kept.
	fi
}

filter_asan()
{
	# retrain the backtrace only.
	# else check_dup_output won't detect duplicates due to PID
	# and register dump.
	grep '#' "$TMP/output" > "$TMP/asan_bt"
	[ -s "$TMP/asan_bt" ] && mv "$TMP/asan_bt" "$TMP/output"
}

check_dup_output()
{
	local f="$1"
	local sha=""

	if [ ! -s "$TMP/output" ]; then
		return 0
	fi

	sha=$(sha256sum "$TMP/output" | cut -f 1 -d " ")

	if [ -f "$TMP/$sha.output" ]; then
		local dup=$(cat "$TMP/$sha".filename)
		echo "Duplicate output, identical splat seen from $dup"

		local s1=$(du -sb "$dup"| cut -f 1)
		local s2=$(du -sb "$f"| cut -f 1)

		# keep the smaller file.
		if [ "$s2" -lt "$s1" ];then
			echo "$f" > "$TMP/$sha".filename
			f="$dup"
		fi

		echo "Delete $f?"
		prompt_del "$f"
		return 1
	fi

	# New output.
	mv "$TMP/output" "$TMP/$sha.output"
	echo "$f" > "$TMP/$sha.filename"
	return 0
}

check_input()
{
	local NFT="$1"
	local f="$2"

	if [ ! -x "$NFT" ] ;then
		return 1
	fi

	for arg in "" "-j"; do
		"$NFT" --check $arg -f - < "$f" > "$TMP/output" 2>&1
		local rv=$?

		if grep AddressSanitizer "$TMP/output"; then
			echo "ASAN: \"$NFT $arg -f $f\" exited with $rv"
			filter_asan "$TMP/output"
			check_dup_output "$f"
			return 0
		fi

		[ $rv -eq 0 ] && continue
		[ $rv -eq 1 ] && continue

		echo "\"$NFT $arg -f $f\" exited with $rv"
		check_dup_output "$f"
		return 0
	done

	return 1
}

for f in $files;do
	check_input src/nft-asan "$f" && continue
	check_input src/nft "$f" && continue

	echo "$f did not trigger a splat.  Delete?"
	prompt_del "$f"
done
