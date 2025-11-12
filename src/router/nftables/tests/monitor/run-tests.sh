#!/bin/bash

nft=${NFT:-$(dirname $0)/../../src/nft}
debug=false
test_json=false

mydiff() {
	diff -w -I '^# ' "$@"
}

err() {
	echo "$*" >&2
}

die() {
	err "$*"
	exit 1
}

if [ "$(id -u)" != "0" ] ; then
	die "this requires root!"
fi

testdir=$(mktemp -d)
if [ ! -d $testdir ]; then
	die "Failed to create test directory"
fi
trap 'rm -rf $testdir; $nft flush ruleset' EXIT

command_file=$(mktemp -p $testdir)
output_file=$(mktemp -p $testdir)

cmd_append() {
	echo "$*" >>$command_file
}
monitor_output_append() {
	[[ "$*" == '-' ]] && {
		cat $command_file >>$output_file
		return
	}
	echo "$*" >>$output_file
}
echo_output_append() {
	# this is a bit tricky: for replace commands, nft prints a delete
	# command - so in case there is a replace command in $command_file,
	# just assume any other commands in the same file are sane
	grep -q '^replace' $command_file >/dev/null 2>&1 && {
		monitor_output_append "$*"
		return
	}
	[[ "$*" == '-' ]] && {
		grep '^\(add\|replace\|insert\)' $command_file >>$output_file
		return
	}
	[[ "$*" =~ ^add|replace|insert ]] && echo "$*" >>$output_file
}
json_output_filter() { # (filename)
	# unify handle values
	sed -i -e 's/\("handle":\) [0-9][0-9]*/\1 0/g' "$1"
}
monitor_run_test() {
	monitor_output=$(mktemp -p $testdir)
	monitor_args=""
	$test_json && monitor_args="vm json"
	local rc=0

	$nft -nn monitor $monitor_args >$monitor_output &
	monitor_pid=$!

	sleep 0.5

	$debug && {
		echo "command file:"
		cat $command_file
	}
	$nft -f - <$command_file || {
		err "nft command failed!"
		rc=1
	}
	sleep 0.5
	kill $monitor_pid
	wait >/dev/null 2>&1
	$test_json && json_output_filter $monitor_output
	mydiff -q $monitor_output $output_file >/dev/null 2>&1
	if [[ $rc == 0 && $? != 0 ]]; then
		err "monitor output differs!"
		mydiff -u $output_file $monitor_output >&2
		rc=1
	fi
	rm $command_file
	rm $output_file
	touch $command_file
	touch $output_file
	return $rc
}

echo_run_test() {
	echo_output=$(mktemp -p $testdir)
	local rc=0

	$debug && {
		echo "command file:"
		cat $command_file
	}
	$nft -nn -e -f - <$command_file >$echo_output || {
		err "nft command failed!"
		rc=1
	}
	mydiff -q $echo_output $output_file >/dev/null 2>&1
	if [[ $rc == 0 && $? != 0 ]]; then
		err "echo output differs!"
		mydiff -u $output_file $echo_output >&2
		rc=1
	fi
	rm $command_file
	rm $output_file
	touch $command_file
	touch $output_file
	return $rc
}

netns=true
for arg in "$@"; do
	[[ "$arg" == "--no-netns" ]] && netns=false
done
if $netns; then
	exec unshare -n $0 --no-netns "$@"
fi

testcases=""
while [ -n "$1" ]; do
	case "$1" in
	-d|--debug)
		debug=true
		shift
		;;
	-j|--json)
		test_json=true
		shift
		;;
	--no-netns)
		shift
		;;
	-H|--host)
		nft=nft
		shift
		;;
	*.t)
		testcases+=" $1"
		shift
		;;
	*)
		echo "unknown option '$1'"
		;&
	-h|--help)
		echo "Usage: $(basename $0) [-j|--json] [-d|--debug] [testcase ...]"
		exit 1
		;;
	esac
done

if $test_json; then
	variants="monitor"
else
	variants="monitor echo"
fi

rc=0
for variant in $variants; do
	run_test=${variant}_run_test
	output_append=${variant}_output_append

	for testcase in ${testcases:-testcases/*.t}; do
		filename=$(basename $testcase)
		echo "$variant: running tests from file $filename"
		rc_start=$rc

		# files are like this:
		#
		# I add table ip t
		# O add table ip t
		# I add chain ip t c
		# O add chain ip t c

		$nft flush ruleset

		input_complete=false
		while read dir line; do
			case $dir in
			I)
				$input_complete && {
					$run_test
					let "rc += $?"
				}
				input_complete=false
				cmd_append "$line"
				;;
			O)
				input_complete=true
				$test_json || $output_append "$line"
				;;
			J)
				input_complete=true
				$test_json && $output_append "$line"
				;;
			'#'|'')
				# ignore comments and empty lines
				;;
			esac
		done <$testcase
		$input_complete && {
			$run_test
			let "rc += $?"
		}

		let "rc_diff = rc - rc_start"
		[[ $rc_diff -ne 0 ]] && \
			echo "$variant: $rc_diff tests from file $filename failed"
	done
done
exit $rc
