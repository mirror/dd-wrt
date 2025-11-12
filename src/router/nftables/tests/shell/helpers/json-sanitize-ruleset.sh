#!/bin/bash -e

die() {
	printf "%s\n" "$*"
	exit 1
}

do_sed() {
	# Normalize the "version"/"release_name", otherwise we have to
	# regenerate the JSON output upon new release.
	#
	# Also, "handle" are not stable. Normalize them 0.
	sed \
		-e '1s/^\({"nftables": \[{"metainfo": {"version": "\)[0-9.]\+\(", "release_name": "\)[^"]\+\(", "\)/\1VERSION\2RELEASE_NAME\3/' \
		-e '1s/"handle": [0-9]\+\>/"handle": 0/g' \
		"$@"
}

if [ "$#" = 0 ] ; then
	do_sed
	exit $?
fi

for f ; do
	test -f "$f" || die "$0: file \"$f\" does not exist"
done

for f ; do
	do_sed -i "$f" || die "$0: \`sed -i\` failed for \"$f\""
done
