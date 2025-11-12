#!/bin/bash -e

# This wrapper wraps the invocation of the test. It is called by run-tests.sh,
# and already in the unshared namespace.
#
# For some printf debugging, you can also patch this file.

rc_dump=0

array_contains() {
	local needle="$1"
	local a
	shift
	for a; do
		[ "$a" = "$needle" ] && return 0
	done
	return 1
}

show_file() {
	local filename="$1"
	shift
	local msg="$*"

	printf '%s\n>>>>\n' "$msg"
	cat "$filename"
	printf "<<<<\n"
}

diff_check_setcount() {
	local dumpfile="$1"
	local after="$2"

	if $DIFF -u "$dumpfile" "$after" &> "$NFT_TEST_TESTTMPDIR/ruleset-diff" ; then
		rm -f "$NFT_TEST_TESTTMPDIR/ruleset-diff"
		return
	fi

	if [ $NFT_TEST_HAVE_setcount = n ];then
		# old kernel or nft binary, expect "size 42", not "size 42	# count 1".
		sed s/.\#\ count\ .\*//g "$dumpfile" > "$NFT_TEST_TESTTMPDIR/ruleset-diff-postprocess"

		if $DIFF -u "$NFT_TEST_TESTTMPDIR/ruleset-diff-postprocess" "$after" > /dev/null ; then
			rm -f "$NFT_TEST_TESTTMPDIR/ruleset-diff" "$NFT_TEST_TESTTMPDIR/ruleset-diff-postprocess"
			return
		fi
	fi

	show_file "$NFT_TEST_TESTTMPDIR/ruleset-diff" "Failed \`$DIFF -u \"$dumpfile\" \"$after\"\`" >> "$NFT_TEST_TESTTMPDIR/rc-failed-dump"
	rc_dump=1
}

json_pretty() {
	"$NFT_TEST_BASEDIR/helpers/json-pretty.sh" "$@" 2>&1 || :
}

TEST="$1"
TESTBASE="$(basename "$TEST")"
TESTDIR="$(dirname "$TEST")"

START_TIME="$(cut -d ' ' -f1 /proc/uptime)"

export TMPDIR="$NFT_TEST_TESTTMPDIR"
export NFT_TEST_LIBRARY_FILE="$NFT_TEST_BASEDIR/helpers/lib.sh"

CLEANUP_UMOUNT_VAR_RUN=n

cleanup() {
	if [ "$CLEANUP_UMOUNT_VAR_RUN" = y ] ; then
		umount "/var/run" &>/dev/null || :
	fi
}

trap cleanup EXIT

printf '%s\n' "$TEST" > "$NFT_TEST_TESTTMPDIR/name"

read tainted_before < /proc/sys/kernel/tainted

if [ "$NFT_TEST_HAS_UNSHARED_MOUNT" = y ] ; then
	# We have a private mount namespace. We will mount /var/run/ as a tmpfs.
	#
	# The main purpose is so that we can create /var/run/netns, which is
	# required for `ip netns add` to work.  When running as rootless, this
	# is necessary to get such tests to pass. When running rootful, it's
	# still useful to not touch the "real" /var/run/netns of the system.
	#
	# Note that this also hides everything that might reside in /var/run.
	# That is desirable, as tests should not depend on content there (or if
	# they do, we need to explicitly handle it as appropriate).
	if mount -t tmpfs --make-private tmpfs "/var/run" 2>/dev/null ; then
		CLEANUP_UMOUNT_VAR_RUN=y
	fi
	mkdir -p /var/run/netns
fi

TEST_TAGS_PARSED=0
ensure_TEST_TAGS() {
	if [ "$TEST_TAGS_PARSED" = 0 ] ; then
		TEST_TAGS_PARSED=1
		TEST_TAGS=( $(sed -n '1,10 { s/^.*\<\(NFT_TEST_REQUIRES\|NFT_TEST_SKIP\)\>\s*(\s*\(NFT_TEST_SKIP_[a-zA-Z0-9_]\+\|NFT_TEST_HAVE_[a-zA-Z0-9_]\+\)\s*).*$/\1(\2)/p }' "$1" 2>/dev/null || : ) )
	fi
}

rc_test=0

if [ "$rc_test" -eq 0 ] ; then
	for KEY in $(compgen -v | grep '^NFT_TEST_HAVE_') ; do
		if [ "${!KEY}" != n ]; then
			continue
		fi
		ensure_TEST_TAGS "$TEST"
		if array_contains "NFT_TEST_REQUIRES($KEY)" "${TEST_TAGS[@]}" ; then
			echo "Test skipped due to $KEY=n (test has \"NFT_TEST_REQUIRES($KEY)\" tag)" >> "$NFT_TEST_TESTTMPDIR/testout.log"
			rc_test=77
			break
		fi
	done
fi

if [ "$rc_test" -eq 0 ] ; then
	for KEY in $(compgen -v | grep '^NFT_TEST_SKIP_') ; do
		if [ "${!KEY}" != y ]; then
			continue
		fi
		ensure_TEST_TAGS "$TEST"
		if array_contains "NFT_TEST_SKIP($KEY)" "${TEST_TAGS[@]}" ; then
			echo "Test skipped due to $KEY=y (test has \"NFT_TEST_SKIP($KEY)\" tag)" >> "$NFT_TEST_TESTTMPDIR/testout.log"
			rc_test=77
			break
		fi
	done
fi

if [ "$rc_test" -eq 0 ] ; then
	CMD=( "$TEST" )
	if [ "$NFT_TEST_VERBOSE_TEST" = y ] ; then
		X="$(sed -n '1 s/^#!\(\/bin\/bash\>.*$\)/\1/p' "$TEST" 2>/dev/null)"
		if [ -n "$X" ] ; then
			# Note that kernel parses the shebang differently and does not
			# word splitting for the arguments. We do split the arguments here
			# which would matter if there are spaces. For our tests, there
			# are either no arguments or only one argument without space. So
			# this is good enough.
			CMD=( $X -x "$TEST" )
		fi
	fi
	printf "Command: $(printf '%q ' "${CMD[@]}")\n" &>> "$NFT_TEST_TESTTMPDIR/testout.log"
	"${CMD[@]}" &>> "$NFT_TEST_TESTTMPDIR/testout.log" || rc_test=$?
fi

rc_chkdump=0
rc=0
$NFT list ruleset > "$NFT_TEST_TESTTMPDIR/ruleset-after" 2> "$NFT_TEST_TESTTMPDIR/chkdump" || rc=$?
if [ "$rc" -ne 0 -o -s "$NFT_TEST_TESTTMPDIR/chkdump" ] ; then
	show_file "$NFT_TEST_TESTTMPDIR/chkdump" "Command \`$NFT list ruleset\` failed" >> "$NFT_TEST_TESTTMPDIR/rc-failed-chkdump"
	rc_chkdump=1
fi
if [ "$NFT_TEST_HAVE_json" != n ] ; then
	rc=0
	$NFT -j list ruleset > "$NFT_TEST_TESTTMPDIR/ruleset-after.json" 2> "$NFT_TEST_TESTTMPDIR/chkdump" || rc=$?

	# Workaround known bug in stmt_print_json(), due to
	# "chain_stmt_ops.json" being NULL. This spams stderr.
	sed -i '/^warning: stmt ops chain have no json callback$/d' "$NFT_TEST_TESTTMPDIR/chkdump"

	if [ "$rc" -ne 0 -o -s "$NFT_TEST_TESTTMPDIR/chkdump" ] ; then
		show_file "$NFT_TEST_TESTTMPDIR/chkdump" "Command \`$NFT -j list ruleset\` failed" >> "$NFT_TEST_TESTTMPDIR/rc-failed-chkdump"
		rc_chkdump=1
	fi
	# JSON output needs normalization/sanitization, otherwise it's not stable.
	"$NFT_TEST_BASEDIR/helpers/json-sanitize-ruleset.sh" "$NFT_TEST_TESTTMPDIR/ruleset-after.json"
	json_pretty "$NFT_TEST_TESTTMPDIR/ruleset-after.json" > "$NFT_TEST_TESTTMPDIR/ruleset-after.json-pretty"
fi

read tainted_after < /proc/sys/kernel/tainted

DUMPPATH="$TESTDIR/dumps"
DUMPFILE="$DUMPPATH/$TESTBASE.nft"
JDUMPFILE="$DUMPPATH/$TESTBASE.json-nft"
NODUMPFILE="$DUMPPATH/$TESTBASE.nodump"

# The caller can request a re-geneating of the .nft, .nodump, .json-nft dump files
# by setting DUMPGEN=y. In that case, only the existing files will be regenerated
# (unless all three files are missing, in which case all of them are generated).
#
# By setting DUMPGEN=all, all 3 files are always regenerated.
dump_written=n
if [ "$rc_test" -eq 0 -a '(' "$DUMPGEN" = all -o "$DUMPGEN" = y ')' ] ; then
	dump_written=y
	if [ ! -d "$DUMPPATH" ] ; then
		mkdir "$DUMPPATH"
	fi
	if [ "$DUMPGEN" = all ] ; then
		gen_nodumpfile=y
		gen_dumpfile=y
		gen_jdumpfile=y
	else
		# by default, only regenerate the files that we already have on disk.
		gen_nodumpfile=n
		gen_dumpfile=n
		gen_jdumpfile=n
		test -f "$DUMPFILE"  && gen_dumpfile=y
		test -f "$JDUMPFILE" && gen_jdumpfile=y
		test -f "$NODUMPFILE" && gen_nodumpfile=y
		if [ "$gen_dumpfile" != y -a "$gen_jdumpfile" != y -a "$gen_nodumpfile" != y ] ; then
			# Except, if no files exist. Them generate all files.
			gen_dumpfile=y
			gen_jdumpfile=y
			gen_nodumpfile=y
		fi
	fi
	if [ "$gen_nodumpfile" = y ] ; then
		: > "$NODUMPFILE"
	fi
	if [ "$gen_dumpfile" = y ] ; then
		cat "$NFT_TEST_TESTTMPDIR/ruleset-after" > "$DUMPFILE"
	fi
	if [ "$NFT_TEST_HAVE_json" != n -a "$gen_jdumpfile" = y ] ; then
		cat "$NFT_TEST_TESTTMPDIR/ruleset-after.json-pretty" > "$JDUMPFILE"
	fi
fi

if [ "$rc_test" -ne 77 -a "$dump_written" != y ] ; then
	if [ -f "$DUMPFILE" ] ; then
		diff_check_setcount "$DUMPFILE" "$NFT_TEST_TESTTMPDIR/ruleset-after"
	fi
	if [ "$NFT_TEST_HAVE_json" != n -a -f "$JDUMPFILE" ] ; then
		if ! $DIFF -u "$JDUMPFILE" "$NFT_TEST_TESTTMPDIR/ruleset-after.json-pretty" &> "$NFT_TEST_TESTTMPDIR/ruleset-diff.json" ; then
			show_file "$NFT_TEST_TESTTMPDIR/ruleset-diff.json" "Failed \`$DIFF -u \"$JDUMPFILE\" \"$NFT_TEST_TESTTMPDIR/ruleset-after.json-pretty\"\`" >> "$NFT_TEST_TESTTMPDIR/rc-failed-dump"
			rc_dump=1
		else
			rm -f "$NFT_TEST_TESTTMPDIR/ruleset-diff.json"
		fi
	fi
fi

# check that a flush after the test succeeds. We anyway need a clean ruleset
# for the `nft --check` next.
rc=0
$NFT flush ruleset &> "$NFT_TEST_TESTTMPDIR/chkdump" || rc=1
if [ "$rc" = 1 -o -s "$NFT_TEST_TESTTMPDIR/chkdump" ] ; then
	show_file "$NFT_TEST_TESTTMPDIR/chkdump" "Command \`$NFT flush ruleset\` failed" >> "$NFT_TEST_TESTTMPDIR/rc-failed-chkdump"
	rc_chkdump=1
fi
# Check that `nft [-j] list ruleset | nft [-j] --check -f -` works.
fail=n
$NFT --check -f "$NFT_TEST_TESTTMPDIR/ruleset-after" &> "$NFT_TEST_TESTTMPDIR/chkdump" || fail=y
test -s "$NFT_TEST_TESTTMPDIR/chkdump" && fail=y
if [ "$fail" = y ] ; then
	show_file "$NFT_TEST_TESTTMPDIR/chkdump" "Command \`$NFT --check -f \"$NFT_TEST_TESTTMPDIR/ruleset-after\"\` failed" >> "$NFT_TEST_TESTTMPDIR/rc-failed-chkdump"
	rc_chkdump=1
fi
if [ -f "$DUMPFILE" ] && ! cmp "$DUMPFILE" "$NFT_TEST_TESTTMPDIR/ruleset-after" &>/dev/null ; then
	# Also check the $DUMPFILE to hit possibly new code paths. This
	# is useful to see crashes and with ASAN/valgrind.
	$NFT --check -f "$DUMPFILE" &>/dev/null || :
fi
if [ "$NFT_TEST_HAVE_json" != n ] ; then
	if [ ! -f "$JDUMPFILE" ] ; then
		# Optimally, `nft -j list ruleset | nft -j --check -f -` never
		# fails.  However, there are known issues where this doesn't
		# work, and we cannot assert hard against that. It's those
		# tests that don't have a .json-nft file.
		#
		# This should be fixed, every test should have a .json-nft
		# file, and this workaround removed.
		$NFT -j --check -f "$NFT_TEST_TESTTMPDIR/ruleset-after.json" &>/dev/null || :
		$NFT -j --check -f "$NFT_TEST_TESTTMPDIR/ruleset-after.json-pretty" &>/dev/null || :
	else
		fail=n
		$NFT -j --check -f "$NFT_TEST_TESTTMPDIR/ruleset-after.json" &> "$NFT_TEST_TESTTMPDIR/chkdump" || fail=y
		test -s "$NFT_TEST_TESTTMPDIR/chkdump" && fail=y
		if [ "$fail" = y ] ; then
			show_file "$NFT_TEST_TESTTMPDIR/chkdump" "Command \`$NFT -j --check -f \"$NFT_TEST_TESTTMPDIR/ruleset-after.json\"\` failed" >> "$NFT_TEST_TESTTMPDIR/rc-failed-chkdump"
			rc_chkdump=1
		fi
		fail=n
		$NFT -j --check -f "$NFT_TEST_TESTTMPDIR/ruleset-after.json-pretty" &> "$NFT_TEST_TESTTMPDIR/chkdump" || fail=y
		test -s "$NFT_TEST_TESTTMPDIR/chkdump" && fail=y
		if [ "$fail" = y ] ; then
			show_file "$NFT_TEST_TESTTMPDIR/chkdump" "Command \`$NFT -j --check -f \"$NFT_TEST_TESTTMPDIR/ruleset-after.json-pretty\"\` failed" >> "$NFT_TEST_TESTTMPDIR/rc-failed-chkdump"
			rc_chkdump=1
		fi
	fi
	if [ -f "$JDUMPFILE" ] \
	     && ! cmp "$JDUMPFILE" "$NFT_TEST_TESTTMPDIR/ruleset-after.json" &>/dev/null \
	     && ! cmp "$JDUMPFILE" "$NFT_TEST_TESTTMPDIR/ruleset-after.json-pretty" &>/dev/null ; \
	then
		$NFT -j --check -f "$JDUMPFILE" &>/dev/null || :
	fi
fi
rm -f "$NFT_TEST_TESTTMPDIR/chkdump"

rc_valgrind=0
[ -f "$NFT_TEST_TESTTMPDIR/rc-failed-valgrind" ] && rc_valgrind=1

rc_tainted=0
if [ "$tainted_before" != "$tainted_after" ] ; then
	echo "$tainted_after" > "$NFT_TEST_TESTTMPDIR/rc-failed-tainted"
	rc_tainted=1
fi

if [ "$rc_valgrind" -ne 0 ] ; then
	rc_exit=122
elif [ "$rc_tainted" -ne 0 ] ; then
	rc_exit=123
elif [ "$rc_test" -ge 118 -a "$rc_test" -le 124 ] ; then
	# Special exit codes are reserved. Coerce them.
	rc_exit=125
elif [ "$rc_test" -ne 0 ] ; then
	rc_exit="$rc_test"
elif [ "$rc_dump" -ne 0 ] ; then
	rc_exit=124
elif [ "$rc_chkdump" -ne 0 ] ; then
	rc_exit=121
else
	rc_exit=0
fi


# We always write the real exit code of the test ($rc_test) to one of the files
# rc-{ok,skipped,failed}, depending on which it is.
#
# Note that there might be other rc-failed-{dump,tainted,valgrind} files with
# additional errors. Note that if such files exist, the overall state will
# always be failed too (and an "rc-failed" file exists).
#
# On failure, we also write the combined "$rc_exit" code from "test-wrapper.sh"
# to "rc-failed-exit" file.
#
# This means, failed tests will have a "rc-failed" file, and additional
# "rc-failed-*" files exist for further information.
if [ "$rc_exit" -eq 0 ] ; then
	RC_FILENAME="rc-ok"
elif [ "$rc_exit" -eq 77 ] ; then
	RC_FILENAME="rc-skipped"
else
	RC_FILENAME="rc-failed"
	echo "$rc_exit" > "$NFT_TEST_TESTTMPDIR/rc-failed-exit"
fi
echo "$rc_test" > "$NFT_TEST_TESTTMPDIR/$RC_FILENAME"

END_TIME="$(cut -d ' ' -f1 /proc/uptime)"
WALL_TIME="$(awk -v start="$START_TIME" -v end="$END_TIME" "BEGIN { print(end - start) }")"
printf "%s\n" "$WALL_TIME" "$START_TIME" "$END_TIME" > "$NFT_TEST_TESTTMPDIR/times"

exit "$rc_exit"
