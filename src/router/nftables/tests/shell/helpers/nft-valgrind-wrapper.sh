#!/bin/bash -e

SUFFIX="$(date "+%H%M%S.%6N").$$"

rc=0
libtool \
	--mode=execute \
	valgrind \
		--log-file="$NFT_TEST_TESTTMPDIR/valgrind.$SUFFIX.%p.log" \
		--trace-children=yes \
		--leak-check=full \
		--show-leak-kinds=all \
		--num-callers=100 \
		--error-exitcode=122 \
		--vgdb-prefix="$_NFT_TEST_VALGRIND_VGDB_PREFIX-$SUFFIX" \
		$NFT_TEST_VALGRIND_OPTS \
		"$NFT_REAL" \
		"$@" \
	|| rc=$?

if [ "$rc" -eq 122 ] ; then
	shopt -s nullglob
	FILES=( "$NFT_TEST_TESTTMPDIR/valgrind.$SUFFIX."*".log" )
	shopt -u nullglob
	(
		printf '%s\n' "args: $*"
		printf '%s\n' "${FILES[*]}"
	) >> "$NFT_TEST_TESTTMPDIR/rc-failed-valgrind"
fi

exit $rc
