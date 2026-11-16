#!/bin/bash

# update payload records in an automated fashion, trying to reduce diff sizes


# scan payloadfile and print record for cmd (if found)
find_payload() { # (payloadfile, cmd)
	local found=false

	readarray -t pl <"$1"
	for l in "${pl[@]}"; do
		if [[ "$l" == "# "* ]]; then
			$found && return
			[[ "$l" == "$2" ]] && found=true
		fi
		$found && echo "$l"
	done
	$found || echo "Warning: Command '$2' not found in '$1'" >&2
}

cd $(dirname $0)/../

# make sure no stray output files get in the way
rm -f */*.got */*.gotgot

# store payload records for later
# clear payload files to force regenerating (but leave them in place)
for pl in */*.payload*; do
	[[ $pl == *.bak ]] && continue # ignore leftover .bak files
	cp "$pl" "${pl}.bak"
	echo >"$pl"
done

# run the testsuite to create .got files
# pass -f to keep going despite missing payloads
./nft-test.py -f

# restore old payload records
for plbak in */*.bak; do
	mv "$plbak" "${plbak%.bak}"
done

# sort created got files to match order in old payload records
for g in ${@:-*/*.payload*.got}; do
	pl=${g%.got}

	[[ -f $g ]] || continue
	[[ -f $pl ]] || continue

	readarray -t ploads <"$g"
	readarray -t cmds <<< $(grep '^# ' $pl)
	for cmd in "${cmds[@]}"; do
		found=false
		for l in "${ploads[@]}"; do
			if [[ "$l" == "# "* ]]; then
				$found && break
				[[ "$l" == "$cmd" ]] && found=true
			fi
			$found && [[ "$l" != "" ]] && echo "$l"
		done
		if ! $found; then
			echo "Warning: Command '$cmd' not found in '$g'" >&2
		else
			echo ""
		fi
	done | head -n -1 >${g}got

	mv "${g}got" "${g}"
done

# overwrite old payload records with new ones
for got in */*.payload*.got; do
	mv "${got}" "${got%.got}"
done
