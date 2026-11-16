#!/bin/bash

unset LANGUAGE
export LANG=C
export LC_ALL=C

GREEN=""
YELLOW=""
RED=""
RESET=""
if [ -z "$NO_COLOR" ] ; then
	if [ -n "$CLICOLOR_FORCE" ] || [[ -t 1 ]] ; then
		# See https://bixense.com/clicolors/ . We only check isatty() on
		# file descriptor 1, to decide whether colorizing happens (although,
		# we might also colorize on other places/FDs).
		GREEN=$'\e[32m'
		YELLOW=$'\e[33m'
		RED=$'\e[31m'
		RESET=$'\e[0m'
	fi
fi

array_contains() {
	local needle="$1"
	local a
	shift
	for a; do
		[ "$a" = "$needle" ] && return 0
	done
	return 1
}

array_remove_first() {
	local _varname="$1"
	local _needle="$2"
	local _result=()
	local _a

	eval "local _input=( \"\${$_varname[@]}\" )"
	for _a in "${_input[@]}" ; do
		if [ -n "${_needle+x}" -a "$_needle" = "$_a" ] ; then
			unset _needle
		else
			_result+=("$_a")
		fi
	done
	eval "$_varname="'( "${_result[@]}" )'
}

colorize_keywords() {
	local out_variable="$1"
	local color="$2"
	local val="$3"
	local val2
	shift 3

	printf -v val2 '%q' "$val"
	array_contains "$val" "$@" && val2="$color$val2$RESET"
	printf -v "$out_variable" '%s' "$val2"
}

strtonum() {
	local s="$1"
	local n
	local n2

	re='^[[:space:]]*([0-9]+)[[:space:]]*$'
	if [[ "$s" =~ $re ]] ; then
		n="${BASH_REMATCH[1]}"
		if [ "$(( n + 0 ))" = "$n" ] ; then
			echo "$n"
			return 0
		fi
	fi
	re='^[[:space:]]*0x([0-9a-fA-F]+)[[:space:]]*$'
	if [[ "$s" =~ $re ]] ; then
		n="${BASH_REMATCH[1]}"
		n2="$(( 16#$n + 0 ))"
		if [ "$n2" = "$(printf '%d' "0x$n" 2>/dev/null)" ] ; then
			echo "$n2"
			return 0
		fi
	fi
	return 1
}

_msg() {
	local level="$1"
	shift

	if [ "$level" = E ] ; then
		printf '%s\n' "$RED$level$RESET: $*"
	elif [ "$level" = W ] ; then
		printf '%s\n' "$YELLOW$level$RESET: $*"
	else
		printf '%s\n' "$level: $*"
	fi
	if [ "$level" = E ] ; then
		exit 99
	fi
}

msg_error() {
	_msg E "$@"
}

msg_warn() {
	_msg W "$@"
}

msg_info() {
	_msg I "$@"
}

align_text() {
	local _OUT_VARNAME="$1"
	local _LEFT_OR_RIGHT="$2"
	local _INDENT="$3"
	shift 3
	local _text="$*"
	local _text_plain
	local _text_align
	local _text_result
	local _i

	# This function is needed, because "$text" might contain color escape
	# sequences. A plain `printf '%12s' "$text"` will not align properly.

	# strip escape sequences
	_text_plain="${_text//$'\e['[0-9]m/}"
	_text_plain="${_text_plain//$'\e['[0-9][0-9]m/}"

	_text_align=""
	for (( _i = "${#_text_plain}" ; "$_i" < "$_INDENT" ; _i++ )) ; do
		_text_align="$_text_align "
	done

	if [ "$_LEFT_OR_RIGHT" = left ] ; then
		_text_result="$(printf "%s$_text_align-" "$_text")"
	else
		_text_result="$(printf "$_text_align%s-" "$_text")"
	fi
	_text_result="${_text_result%-}"

	eval "$_OUT_VARNAME=\"\$_text_result\""
}

bool_n() {
	case "$1" in
		n|N|no|No|NO|0|false|False|FALSE)
			printf n
			;;
		*)
			printf y
			;;
	esac
}

bool_y() {
	case "$1" in
		y|Y|yes|Yes|YES|1|true|True|TRUE)
			printf y
			;;
		*)
			printf n
			;;
	esac
}

usage() {
	echo " $0 [OPTIONS] [TESTS...]"
	echo
	echo "OPTIONS:"
	echo " -h|--help       : Print usage."
	echo " -L|--list-tests : List test names and quit."
	echo " -v              : Sets VERBOSE=y."
	echo " -g              : Sets DUMPGEN=y."
	echo " -V              : Sets VALGRIND=y."
	echo " -K              : Sets KMEMLEAK=y."
	echo " -R|--without-realroot : Sets NFT_TEST_HAS_REALROOT=n."
	echo " -U|--no-unshare : Sets NFT_TEST_UNSHARE_CMD=\"\"."
	echo " -k|--keep-logs  : Sets NFT_TEST_KEEP_LOGS=y."
	echo " -x              : Sets NFT_TEST_VERBOSE_TEST=y."
	echo " -s|--sequential : Sets NFT_TEST_JOBS=0, which also enables global cleanups."
	echo "                   Also sets NFT_TEST_SHUFFLE_TESTS=n if left unspecified."
	echo " -Q|--quick      : Sets NFT_TEST_SKIP_slow=y."
	echo " -S|--setup-host : Modify the host to run as rootless. Otherwise, some tests will be"
	echo "                   skipped. Basically, this bumps /proc/sys/net/core/{wmem_max,rmem_max}."
	echo "                   Must run as root and this option must be specified alone."
	echo " --              : Separate options from tests."
	echo " [TESTS...]      : Other options are treated as test names,"
	echo "                   that is, executables that are run by the runner."
	echo
	echo "ENVIRONMENT VARIABLES:"
	echo " NFT=<CMD>     : Path to nft executable. Will be called as \`\$NFT [...]\` so"
	echo "                 it can be a command with parameters. Note that in this mode quoting"
	echo "                 does not work, so the usage is limited and the command cannot contain"
	echo "                 spaces."
	echo " NFT_REAL=<CMD> : Real nft comand. Usually this is just the same as \$NFT,"
	echo "                 however, you may set NFT='valgrind nft' and NFT_REAL to the real command."
	echo " VERBOSE=*|y   : Enable verbose output."
	echo " NFT_TEST_VERBOSE_TEST=*|y: if true, enable verbose output for tests. For bash scripts, this means"
	echo "                 to pass \"-x\" to the interpreter."
	echo " DUMPGEN=*|y|all : Regenerate dump files \".{nft,json-nft,nodump}\". \"DUMPGEN=y\" only regenerates existing"
	echo "                 files, unless the test has no files (then all three files are generated, and you need to"
	echo "                 choose which to keep). With \"DUMPGEN=all\" all 3 files are regenerated, regardless"
	echo "                 whether they already exist."
	echo " VALGRIND=*|y  : Run \$NFT in valgrind."
	echo " KMEMLEAK=*|y  : Check for kernel memleaks."
	echo " NFT_TEST_HAS_REALROOT=*|y : To indicate whether the test has real root permissions."
	echo "                 Usually, you don't need this and it gets autodetected."
	echo "                 You might want to set it, if you know better than the"
	echo "                 \`id -u\` check, whether the user is root in the main namespace."
	echo "                 Note that without real root, certain tests may not work,"
	echo "                 e.g. due to limited /proc/sys/net/core/{wmem_max,rmem_max}."
	echo "                 Checks that cannot pass in such environment should check for"
	echo "                 [ \"\$NFT_TEST_HAS_REALROOT\" != y ] and skip gracefully."
	echo " NFT_TEST_HAS_SOCKET_LIMITS=*|n : some tests will fail if /proc/sys/net/core/{wmem_max,rmem_max} is"
	echo "                 too small. When running as real root, then test can override those limits. However,"
	echo "                 with rootless the test would fail. Tests will check for [ "\$NFT_TEST_HAS_SOCKET_LIMITS" = y ]"
	echo "                 and skip. You may set NFT_TEST_HAS_SOCKET_LIMITS=n if you ensure those limits are"
	echo "                 suitable to run the test rootless. Otherwise will be autodetected."
	echo "                 Set /proc/sys/net/core/{wmem_max,rmem_max} to at least 4MB to get them to pass automatically."
	echo " NFT_TEST_UNSHARE_CMD=cmd : when set, this is the command line for an unshare"
	echo "                 command, which is used to sandbox each test invocation. By"
	echo "                 setting it to empty, no unsharing is done."
	echo "                 By default it is unset, in which case it's autodetected as"
	echo "                 \`unshare -f -p\` (for root) or as \`unshare -f -p --mount-proc -U --map-root-user -n\`"
	echo "                 for non-root."
	echo "                 When setting this, you may also want to set NFT_TEST_HAS_UNSHARED=,"
	echo "                 NFT_TEST_HAS_REALROOT= and NFT_TEST_HAS_UNSHARED_MOUNT= accordingly."
	echo " NFT_TEST_HAS_UNSHARED=*|y : To indicate to the test whether the test run will be unshared."
	echo "                 Test may consider this."
	echo "                 This is only honored when \$NFT_TEST_UNSHARE_CMD= is set. Otherwise it's detected."
	echo " NFT_TEST_HAS_UNSHARED_MOUNT=*|y : To indicate to the test whether the test run will have a private"
	echo "                 mount namespace."
	echo "                 This is only honored when \$NFT_TEST_UNSHARE_CMD= is set. Otherwise it's detected."
	echo " NFT_TEST_KEEP_LOGS=*|y: Keep the temp directory. On success, it will be deleted by default."
	echo " NFT_TEST_JOBS=<NUM}>: number of jobs for parallel execution. Defaults to \"\$(nproc)*1.5\" for parallel run."
	echo "                 Setting this to \"0\" or \"1\", means to run jobs sequentially."
	echo "                 Setting this to \"0\" means also to perform global cleanups between tests (remove"
	echo "                 kernel modules)."
	echo "                 Parallel jobs requires unshare and are disabled with NFT_TEST_UNSHARE_CMD=\"\"."
	echo " NFT_TEST_FAIL_ON_SKIP=*|y: if any jobs are skipped, exit with error."
	echo " NFT_TEST_RANDOM_SEED=<SEED>: The test runner will export the environment variable NFT_TEST_RANDOM_SEED"
	echo "                 set to a random number. This can be used as a stable seed for tests to randomize behavior."
	echo "                 Set this to a fixed value to get reproducible behavior."
	echo " NFT_TEST_SHUFFLE_TESTS=*|n|y: control whether to randomly shuffle the order of tests. By default, if"
	echo "                 tests are specified explicitly, they are not shuffled while they are shuffled when"
	echo "                 all tests are run. The shuffling is based on NFT_TEST_RANDOM_SEED."
	echo " TMPDIR=<PATH> : select a different base directory for the result data."
	echo
	echo " NFT_TEST_HAVE_<FEATURE>=*|y: Some tests requires certain features or will be skipped."
	echo "                 The features are autodetected, but you can force it by setting the variable."
	echo "                 Supported <FEATURE>s are: ${_HAVE_OPTS[@]}."
	echo " NFT_TEST_SKIP_<OPTION>=*|y: if set, certain tests are skipped."
	echo "                 Supported <OPTION>s are: ${_SKIP_OPTS[@]}."
	echo " NFT_TEST_EXCLUDES=\"TEST1 TEST2 ...\": Specify tests to be excluded from running"
}

NFT_TEST_BASEDIR="$(dirname "$0")"

# Export the base directory. It may be used by tests.
export NFT_TEST_BASEDIR

_HAVE_OPTS=()
shopt -s nullglob
F=( "$NFT_TEST_BASEDIR/features/"*.nft "$NFT_TEST_BASEDIR/features/"*.sh )
shopt -u nullglob
for file in "${F[@]}"; do
	feat="${file##*/}"
	feat="${feat%.*}"
	re="^[a-z_0-9]+$"
	if [[ "$feat" =~ $re ]] && ! array_contains "$feat" "${_HAVE_OPTS[@]}" && [[ "$file" != *.sh || -x "$file" ]] ; then
		_HAVE_OPTS+=( "$feat" )
	else
		msg_warn "Ignore feature file \"$file\""
	fi
done
_HAVE_OPTS=( $(printf '%s\n' "${_HAVE_OPTS[@]}" | sort) )

for KEY in $(compgen -v | grep '^NFT_TEST_HAVE_' | sort) ; do
	if ! array_contains "${KEY#NFT_TEST_HAVE_}" "${_HAVE_OPTS[@]}" ; then
		unset "$KEY"
	fi
done

_SKIP_OPTS=( slow )
for KEY in $(compgen -v | grep '^NFT_TEST_SKIP_' | sort) ; do
	if ! array_contains "${KEY#NFT_TEST_SKIP_}" "${_SKIP_OPTS[@]}" ; then
		unset "$KEY"
	fi
done

_NFT_TEST_JOBS_DEFAULT="$(nproc)"
[ "$_NFT_TEST_JOBS_DEFAULT" -gt 0 ] 2>/dev/null || _NFT_TEST_JOBS_DEFAULT=1
_NFT_TEST_JOBS_DEFAULT="$(( _NFT_TEST_JOBS_DEFAULT + (_NFT_TEST_JOBS_DEFAULT + 1) / 2 ))"

VERBOSE="$(bool_y "$VERBOSE")"
NFT_TEST_VERBOSE_TEST="$(bool_y "$NFT_TEST_VERBOSE_TEST")"
if [ "$DUMPGEN" != "all" ] ; then
	DUMPGEN="$(bool_y "$DUMPGEN")"
fi
VALGRIND="$(bool_y "$VALGRIND")"
KMEMLEAK="$(bool_y "$KMEMLEAK")"
NFT_TEST_KEEP_LOGS="$(bool_y "$NFT_TEST_KEEP_LOGS")"
NFT_TEST_HAS_REALROOT="$NFT_TEST_HAS_REALROOT"
NFT_TEST_JOBS="${NFT_TEST_JOBS:-$_NFT_TEST_JOBS_DEFAULT}"
NFT_TEST_FAIL_ON_SKIP="$(bool_y "$NFT_TEST_FAIL_ON_SKIP")"
NFT_TEST_RANDOM_SEED="$NFT_TEST_RANDOM_SEED"
NFT_TEST_SHUFFLE_TESTS="$NFT_TEST_SHUFFLE_TESTS"
NFT_TEST_SKIP_slow="$(bool_y "$NFT_TEST_SKIP_slow")"
DO_LIST_TESTS=

if [ -z "$NFT_TEST_RANDOM_SEED" ] ; then
	# Choose a random value.
	n="$SRANDOM"
	[ -z "$n" ] && n="$RANDOM"
else
	# Parse as number.
	n="$(strtonum "$NFT_TEST_RANDOM_SEED")"
	if [ -z "$n" ] ; then
		# If not a number, pick a hash based on the SHA-sum of the seed.
		n="$(printf "%d" "0x$(sha256sum <<<"NFT_TEST_RANDOM_SEED:$NFT_TEST_RANDOM_SEED" | sed -n '1 { s/^\(........\).*/\1/p }')")"
	fi
fi
# Limit a 31 bit decimal so tests can rely on this being in a certain
# restricted form.
NFT_TEST_RANDOM_SEED="$(( $n % 0x80000000 ))"
export NFT_TEST_RANDOM_SEED

TESTS=()

SETUP_HOST=
SETUP_HOST_OTHER=

ARGV_ORIG=( "$@" )

while [ $# -gt 0 ] ; do
	A="$1"
	shift
	case "$A" in
		-S|--setup-host)
			;;
		*)
			SETUP_HOST_OTHER=y
			;;
	esac
	case "$A" in
		-S|--setup-host)
			SETUP_HOST="$A"
			;;
		-v)
			VERBOSE=y
			;;
		-x)
			NFT_TEST_VERBOSE_TEST=y
			;;
		-g)
			DUMPGEN=y
			;;
		-V)
			VALGRIND=y
			;;
		-K)
			KMEMLEAK=y
			;;
		-h|--help)
			usage
			exit 0
			;;
		-k|--keep-logs)
			NFT_TEST_KEEP_LOGS=y
			;;
		-L|--list-tests)
			DO_LIST_TESTS=y
			;;
		-R|--without-realroot)
			NFT_TEST_HAS_REALROOT=n
			;;
		-U|--no-unshare)
			NFT_TEST_UNSHARE_CMD=
			;;
		-s|--sequential)
			NFT_TEST_JOBS=0
			if [ -z "$NFT_TEST_SHUFFLE_TESTS" ] ; then
				NFT_TEST_SHUFFLE_TESTS=n
			fi
			;;
		-Q|--quick)
			NFT_TEST_SKIP_slow=y
			;;
		--)
			TESTS+=( "$@" )
			shift $#
			;;
		*)
			TESTS+=( "$A" )
			;;
	esac
done

sysctl_bump() {
	local sysctl="$1"
	local val="$2"
	local cur;

	cur="$(cat "$sysctl" 2>/dev/null)" || :
	if [ -n "$cur" -a "$cur" -ge "$val" ] ; then
		echo "# Skip: echo $val > $sysctl (current value $cur)"
		return 0
	fi
	echo "    echo $val > $sysctl (previous value $cur)"
	echo "$val" > "$sysctl"
}

setup_host() {
	echo "Setting up host for running as rootless (requires root)."
	sysctl_bump /proc/sys/net/core/rmem_max $((4000*1024)) || return $?
	sysctl_bump /proc/sys/net/core/wmem_max $((4000*1024)) || return $?
}

if [ -n "$SETUP_HOST" ] ; then
	if [ "$SETUP_HOST_OTHER" = y ] ; then
		msg_error "The $SETUP_HOST option must be specified alone."
	fi
	setup_host
	exit $?
fi

find_tests() {
	find "$1" -type f -executable | sort
}

if [ "${#TESTS[@]}" -eq 0 ] ; then
	d="$NFT_TEST_BASEDIR/testcases/"
	d="${d#./}"
	TESTS=( $(find_tests "$d") )
	test "${#TESTS[@]}" -gt 0 || msg_error "Could not find tests"
	if [ -z "$NFT_TEST_SHUFFLE_TESTS" ] ; then
		NFT_TEST_SHUFFLE_TESTS=y
	fi
fi

TESTSOLD=( "${TESTS[@]}" )
TESTS=()
for t in "${TESTSOLD[@]}" ; do
	if [ -f "$t" -a -x "$t" ] ; then
		TESTS+=( "$t" )
	elif [ -d "$t" ] ; then
		TESTS+=( $(find_tests "$t") )
	else
		msg_error "Unknown test \"$t\""
	fi
done

NFT_TEST_SHUFFLE_TESTS="$(bool_y "$NFT_TEST_SHUFFLE_TESTS")"

if [ -n "$NFT_TEST_EXCLUDES" ]; then
	TESTS=( $(printf '%s\n' "${TESTS[@]}" |
		grep -vE "$(printf '%s|' $NFT_TEST_EXCLUDES | sed 's/|$//')") )
fi

if [ "$DO_LIST_TESTS" = y ] ; then
	printf '%s\n' "${TESTS[@]}"
	exit 0
fi

START_TIME="$(cut -d ' ' -f1 /proc/uptime)"

_TMPDIR="${TMPDIR:-/tmp}"

# Export the orignal TMPDIR for the tests. "test-wrapper.sh" sets TMPDIR to
# NFT_TEST_TESTTMPDIR, so that temporary files are placed along side the
# test data. In some cases, we may want to know the original TMPDIR.
export NFT_TEST_TMPDIR_ORIG="$_TMPDIR"

if [ "$NFT_TEST_HAS_REALROOT" = "" ] ; then
	# The caller didn't set NFT_TEST_HAS_REALROOT and didn't specify
	# -R/--without-root option. Autodetect it based on `id -u`.
	export NFT_TEST_HAS_REALROOT="$(test "$(id -u)" = "0" && echo y || echo n)"
else
	NFT_TEST_HAS_REALROOT="$(bool_y "$NFT_TEST_HAS_REALROOT")"
fi
export NFT_TEST_HAS_REALROOT

if [ "$NFT_TEST_HAS_SOCKET_LIMITS" = "" ] ; then
	if [ "$NFT_TEST_HAS_REALROOT" = y ] ; then
		NFT_TEST_HAS_SOCKET_LIMITS=n
	elif [ "$(cat /proc/sys/net/core/wmem_max 2>/dev/null)" -ge $((4000*1024)) ] 2>/dev/null && \
	     [ "$(cat /proc/sys/net/core/rmem_max 2>/dev/null)" -ge $((4000*1024)) ] 2>/dev/null ; then
		NFT_TEST_HAS_SOCKET_LIMITS=n
	else
		NFT_TEST_HAS_SOCKET_LIMITS=y
	fi
else
	NFT_TEST_HAS_SOCKET_LIMITS="$(bool_n "$NFT_TEST_HAS_SOCKET_LIMITS")"
fi
export NFT_TEST_HAS_SOCKET_LIMITS

detect_unshare() {
	if ! $1 true &>/dev/null ; then
		return 1
	fi
	NFT_TEST_UNSHARE_CMD="$1"
	return 0
}

if [ -n "${NFT_TEST_UNSHARE_CMD+x}" ] ; then
	# User overrides the unshare command.
	if ! detect_unshare "$NFT_TEST_UNSHARE_CMD" ; then
		msg_error "Cannot unshare via NFT_TEST_UNSHARE_CMD=$(printf '%q' "$NFT_TEST_UNSHARE_CMD")"
	fi
	if [ -z "${NFT_TEST_HAS_UNSHARED+x}" ] ; then
		# Autodetect NFT_TEST_HAS_UNSHARED based one whether
		# $NFT_TEST_UNSHARE_CMD is set.
		if [ -n "$NFT_TEST_UNSHARE_CMD" ] ; then
			NFT_TEST_HAS_UNSHARED="y"
		else
			NFT_TEST_HAS_UNSHARED="n"
		fi
	else
		NFT_TEST_HAS_UNSHARED="$(bool_y "$NFT_TEST_HAS_UNSHARED")"
	fi
	if [ -z "${NFT_TEST_HAS_UNSHARED_MOUNT+x}" ] ; then
		NFT_TEST_HAS_UNSHARED_MOUNT=n
		if [ "$NFT_TEST_HAS_UNSHARED" == y ] ; then
			case "$NFT_TEST_UNSHARE_CMD" in
				unshare*-m*|unshare*--mount-proc*)
					NFT_TEST_HAS_UNSHARED_MOUNT=y
					;;
			esac
		fi
	else
		NFT_TEST_HAS_UNSHARED_MOUNT="$(bool_y "$NFT_TEST_HAS_UNSHARED_MOUNT")"
	fi
else
	NFT_TEST_HAS_UNSHARED_MOUNT=n
	if [ "$NFT_TEST_HAS_REALROOT" = y ] ; then
		# We appear to have real root. So try to unshare
		# without a separate USERNS. CLONE_NEWUSER will break
		# tests that are limited by
		# /proc/sys/net/core/{wmem_max,rmem_max}. With real
		# root, we want to test that.
		if detect_unshare "unshare -f -n -m" ; then
			NFT_TEST_HAS_UNSHARED_MOUNT=y
		else
			detect_unshare "unshare -f -n" ||
			detect_unshare "unshare -f -p -m --mount-proc -U --map-root-user -n" ||
			detect_unshare "unshare -f -U --map-root-user -n"
		fi
	else
		if detect_unshare "unshare -f -p -m --mount-proc -U --map-root-user -n" ; then
			NFT_TEST_HAS_UNSHARED_MOUNT=y
		else
			detect_unshare "unshare -f -U --map-root-user -n"
		fi
	fi
	if [ -z "$NFT_TEST_UNSHARE_CMD" ] ; then
		msg_error "Unshare does not work. Run as root with -U/--no-unshare or set NFT_TEST_UNSHARE_CMD"
	fi
	NFT_TEST_HAS_UNSHARED=y
fi
# If tests wish, they can know whether they are unshared via this variable.
export NFT_TEST_HAS_UNSHARED
export NFT_TEST_HAS_UNSHARED_MOUNT

# normalize the jobs number to be an integer.
case "$NFT_TEST_JOBS" in
	''|*[!0-9]*) NFT_TEST_JOBS=_NFT_TEST_JOBS_DEFAULT ;;
esac
if [ -z "$NFT_TEST_UNSHARE_CMD" -a "$NFT_TEST_JOBS" -gt 1 ] ; then
	NFT_TEST_JOBS=1
fi

[ -z "$NFT" ] && NFT="$NFT_TEST_BASEDIR/../../src/nft"
${NFT} > /dev/null 2>&1
ret=$?
if [ ${ret} -eq 126 ] || [ ${ret} -eq 127 ]; then
	msg_error "cannot execute nft command: $NFT"
fi

NFT_REAL="${NFT_REAL-$NFT}"

feature_probe()
{
	local with_path="$NFT_TEST_BASEDIR/features/$1"

	if [ -r "$with_path.nft" ] ; then
		$NFT_TEST_UNSHARE_CMD "$NFT_REAL" --check -f "$with_path.nft" &>/dev/null
		return $?
	fi

	if [ -x "$with_path.sh" ] ; then
		NFT="$NFT_REAL" $NFT_TEST_UNSHARE_CMD "$with_path.sh" &>/dev/null
		return $?
	fi

	return 1
}

for feat in "${_HAVE_OPTS[@]}" ; do
	var="NFT_TEST_HAVE_$feat"
	if [ -z "${!var+x}" ] ; then
		val='y'
		feature_probe "$feat" || val='n'
	else
		val="$(bool_n "${!var}")"
	fi
	eval "export $var=$val"
	if [ "$NFT_TEST_HAS_UNSHARED" != y ] ; then
		$NFT flush ruleset
	fi
done

if [ "$NFT_TEST_JOBS" -eq 0 ] ; then
	MODPROBE="$(which modprobe)"
	if [ ! -x "$MODPROBE" ] ; then
		msg_error "no modprobe binary found"
	fi
fi

DIFF="$(which diff)"
if [ ! -x "$DIFF" ] ; then
	DIFF=true
fi

JOBS_PIDLIST_ARR=()
declare -A JOBS_PIDLIST

_NFT_TEST_VALGRIND_VGDB_PREFIX=

cleanup_on_exit() {
	pids_search=''
	for pid in "${JOBS_PIDLIST_ARR[@]}" ; do
		kill -- "-$pid" &>/dev/null
		pids_search="$pids_search\\|\\<$pid\\>"
	done
	if [ -n "$pids_search" ] ; then
		pids_search="${pids_search:2}"
		for i in {1..100}; do
			ps xh -o pgrp | grep -q "$pids_search" || break
			sleep 0.01
		done
	fi
	if [ "$NFT_TEST_KEEP_LOGS" != y -a -n "$NFT_TEST_TMPDIR" ] ; then
		rm -rf "$NFT_TEST_TMPDIR"
	fi
	if [ -n "$_NFT_TEST_VALGRIND_VGDB_PREFIX" ] ; then
		rm -rf "$_NFT_TEST_VALGRIND_VGDB_PREFIX"* &>/dev/null
	fi
}

trap 'exit 130' SIGINT
trap 'exit 143' SIGTERM
trap 'rc=$?; cleanup_on_exit; exit $rc' EXIT

TIMESTAMP=$(date '+%Y%m%d-%H%M%S.%3N')
NFT_TEST_TMPDIR="$(mktemp --tmpdir="$_TMPDIR" -d "nft-test.$TIMESTAMP$NFT_TEST_TMPDIR_TAG.XXXXXX")" ||
	msg_error "Failure to create temp directory in \"$_TMPDIR\""
chmod 755 "$NFT_TEST_TMPDIR"

exec &> >(tee "$NFT_TEST_TMPDIR/test.log")

msg_info "conf: NFT=$(printf '%q' "$NFT")"
msg_info "conf: NFT_REAL=$(printf '%q' "$NFT_REAL")"
msg_info "conf: VERBOSE=$(printf '%q' "$VERBOSE")"
msg_info "conf: NFT_TEST_VERBOSE_TEST=$(printf '%q' "$NFT_TEST_VERBOSE_TEST")"
msg_info "conf: DUMPGEN=$(printf '%q' "$DUMPGEN")"
msg_info "conf: VALGRIND=$(printf '%q' "$VALGRIND")"
msg_info "conf: KMEMLEAK=$(printf '%q' "$KMEMLEAK")"
msg_info "conf: NFT_TEST_HAS_REALROOT=$(printf '%q' "$NFT_TEST_HAS_REALROOT")"
colorize_keywords value "$YELLOW" "$NFT_TEST_HAS_SOCKET_LIMITS" y
msg_info "conf: NFT_TEST_HAS_SOCKET_LIMITS=$value"
msg_info "conf: NFT_TEST_UNSHARE_CMD=$(printf '%q' "$NFT_TEST_UNSHARE_CMD")"
msg_info "conf: NFT_TEST_HAS_UNSHARED=$(printf '%q' "$NFT_TEST_HAS_UNSHARED")"
msg_info "conf: NFT_TEST_HAS_UNSHARED_MOUNT=$(printf '%q' "$NFT_TEST_HAS_UNSHARED_MOUNT")"
msg_info "conf: NFT_TEST_KEEP_LOGS=$(printf '%q' "$NFT_TEST_KEEP_LOGS")"
msg_info "conf: NFT_TEST_JOBS=$NFT_TEST_JOBS"
msg_info "conf: NFT_TEST_FAIL_ON_SKIP=$NFT_TEST_FAIL_ON_SKIP"
msg_info "conf: NFT_TEST_RANDOM_SEED=$NFT_TEST_RANDOM_SEED"
msg_info "conf: NFT_TEST_SHUFFLE_TESTS=$NFT_TEST_SHUFFLE_TESTS"
msg_info "conf: TMPDIR=$(printf '%q' "$_TMPDIR")"
echo
for KEY in $(compgen -v | grep '^NFT_TEST_SKIP_' | sort) ; do
	colorize_keywords value "$YELLOW" "${!KEY}" y
	msg_info "conf: $KEY=$value"
	export "$KEY"
done
for KEY in $(compgen -v | grep '^NFT_TEST_HAVE_' | sort) ; do
	colorize_keywords value "$YELLOW" "${!KEY}" n
	msg_info "conf: $KEY=$value"
	export "$KEY"
done

NFT_TEST_LATEST="$_TMPDIR/nft-test.latest.$USER"

ln -snf "$NFT_TEST_TMPDIR" "$NFT_TEST_LATEST"

# export the tmp directory for tests. They may use it, but create distinct
# files! On success, it will be deleted on EXIT. See also "--keep-logs"
export NFT_TEST_TMPDIR

echo
msg_info "info: NFT_TEST_BASEDIR=$(printf '%q' "$NFT_TEST_BASEDIR")"
msg_info "info: NFT_TEST_TMPDIR=$(printf '%q' "$NFT_TEST_TMPDIR")"

if [ "$VALGRIND" == "y" ]; then
	NFT="$NFT_TEST_BASEDIR/helpers/nft-valgrind-wrapper.sh"
	msg_info "info: NFT=$(printf '%q' "$NFT")"
	_NFT_TEST_VALGRIND_VGDB_PREFIX="$NFT_TEST_TMPDIR_ORIG/vgdb-pipe-nft-test-$TIMESTAMP.$$.$RANDOM"
	export _NFT_TEST_VALGRIND_VGDB_PREFIX
fi

kernel_cleanup() {
	if [ "$NFT_TEST_JOBS" -ne 0 ] ; then
		# When we run jobs in parallel (even with only one "parallel"
		# job via `NFT_TEST_JOBS=1`), we skip such global cleanups.
		return
	fi
	if [ "$NFT_TEST_HAS_UNSHARED" != y ] ; then
		$NFT flush ruleset
	fi
	$MODPROBE -raq \
	nft_reject_ipv4 nft_reject_bridge nft_reject_ipv6 nft_reject \
	nft_redir_ipv4 nft_redir_ipv6 nft_redir \
	nft_dup_ipv4 nft_dup_ipv6 nft_dup nft_nat \
	nft_masq_ipv4 nft_masq_ipv6 nft_masq \
	nft_exthdr nft_payload nft_cmp nft_range \
	nft_quota nft_queue nft_numgen nft_osf nft_socket nft_tproxy \
	nft_meta nft_meta_bridge nft_counter nft_log nft_limit \
	nft_fib nft_fib_ipv4 nft_fib_ipv6 nft_fib_inet \
	nft_hash nft_ct nft_compat nft_rt nft_objref \
	nft_set_hash nft_set_rbtree nft_set_bitmap \
	nft_synproxy nft_connlimit \
	nft_chain_nat \
	nft_chain_route_ipv4 nft_chain_route_ipv6 \
	nft_dup_netdev nft_fwd_netdev \
	nft_reject nft_reject_inet nft_reject_netdev \
	nf_tables_set nf_tables \
	nf_flow_table nf_flow_table_ipv4 nf_flow_tables_ipv6 \
	nf_flow_table_inet nft_flow_offload \
	nft_xfrm
}

echo ""
ok=0
skipped=0
failed=0

kmem_runs=0
kmemleak_found=0

check_kmemleak_force()
{
	test -f /sys/kernel/debug/kmemleak || return 0

	echo scan > /sys/kernel/debug/kmemleak

	lines=$(grep "unreferenced object" /sys/kernel/debug/kmemleak | wc -l)
	if [ $lines -ne $kmemleak_found ];then
		msg_warn "[FAILED]	kmemleak detected $lines memory leaks"
		kmemleak_found=$lines
	fi

	if [ $lines -ne 0 ];then
		return 1
	fi

	return 0
}

check_kmemleak()
{
	test -f /sys/kernel/debug/kmemleak || return

	if [ "$KMEMLEAK" == "y" ] ; then
		check_kmemleak_force
		return
	fi

	kmem_runs=$((kmem_runs + 1))
	if [ $((kmem_runs % 30)) -eq 0 ]; then
		# scan slows tests down quite a bit, hence
		# do this only for every 30th test file by
		# default.
		check_kmemleak_force
	fi
}

read kernel_tainted < /proc/sys/kernel/tainted
if [ "$kernel_tainted" -ne 0 ] ; then
	msg_warn "kernel is tainted ($kernel_tainted)"
	echo
fi

print_test_header() {
	local msglevel="$1"
	local testfile="$2"
	local testidx_completed="$3"
	local status="$4"
	local text
	local s_idx

	s_idx="${#TESTS[@]}"
	align_text text right "${#s_idx}" "$testidx_completed"
	s_idx="$text/${#TESTS[@]}"

	align_text text left 12 "[$status]"
	_msg "$msglevel" "$text $s_idx $testfile"
}

print_test_result() {
	local NFT_TEST_TESTTMPDIR="$1"
	local testfile="$2"
	local rc_got="$3"

	local result_msg_level="I"
	local result_msg_files=( "$NFT_TEST_TESTTMPDIR/testout.log" "$NFT_TEST_TESTTMPDIR/ruleset-diff" )
	local result_msg_status

	if [ "$rc_got" -eq 0 ] ; then
		((ok++))
		result_msg_status="${GREEN}OK$RESET"
	elif [ "$rc_got" -eq 77 ] ; then
		((skipped++))
		result_msg_status="${YELLOW}SKIPPED$RESET"
	else
		((failed++))
		result_msg_level="W"
		if [ "$rc_got" -eq 121 ] ; then
			result_msg_status="CHK DUMP"
		elif [ "$rc_got" -eq 122 ] ; then
			result_msg_status="VALGRIND"
		elif [ "$rc_got" -eq 123 ] ; then
			result_msg_status="TAINTED"
		elif [ "$rc_got" -eq 124 ] ; then
			result_msg_status="DUMP FAIL"
		else
			result_msg_status="FAILED"
		fi
		result_msg_status="$RED$result_msg_status$RESET"
		result_msg_files=( "$NFT_TEST_TESTTMPDIR/testout.log" )
	fi

	print_test_header "$result_msg_level" "$testfile" "$((ok + skipped + failed))" "$result_msg_status"

	if [ "$VERBOSE" = "y" ] ; then
		local f

		for f in "${result_msg_files[@]}"; do
			if [ -s "$f" ] ; then
				cat "$f"
			fi
		done

		if [ "$rc_got" -ne 0 ] ; then
			msg_info "check \"$NFT_TEST_TESTTMPDIR\""
		fi
	fi
}

declare -A JOBS_TEMPDIR

job_start() {
	local testfile="$1"
	local testidx="$2"

	if [ "$NFT_TEST_JOBS" -le 1 ] && [[ -t 1 ]]; then
		print_test_header I "$testfile" "$testidx" "EXECUTING"
	fi

	NFT_TEST_TESTTMPDIR="${JOBS_TEMPDIR["$testfile"]}" \
	NFT="$NFT" \
	NFT_REAL="$NFT_REAL" \
	DIFF="$DIFF" \
	DUMPGEN="$DUMPGEN" \
	NFT_TEST_VERBOSE_TEST="$NFT_TEST_VERBOSE_TEST" \
	$NFT_TEST_UNSHARE_CMD "$NFT_TEST_BASEDIR/helpers/test-wrapper.sh" "$testfile"
	local rc_got=$?

	if [ "$NFT_TEST_JOBS" -le 1 ] && [[ -t 1 ]]; then
		echo -en "\033[1A\033[K" # clean the [EXECUTING] foobar line
	fi

	return "$rc_got"
}

# `wait -p` is only supported since bash 5.1
WAIT_SUPPORTS_P=1
[ "${BASH_VERSINFO[0]}" -le 4 -o \( "${BASH_VERSINFO[0]}" -eq 5 -a "${BASH_VERSINFO[1]}" -eq 0 \) ] && WAIT_SUPPORTS_P=0

job_wait()
{
	local num_jobs="$1"
	local JOBCOMPLETED
	local rc_got

	while [ "${#JOBS_PIDLIST_ARR[@]}" -gt 0 -a "${#JOBS_PIDLIST_ARR[@]}" -ge "$num_jobs" ] ; do
		if [ "$WAIT_SUPPORTS_P" = 1 ] ; then
			wait -n -p JOBCOMPLETED
			rc_got="$?"
			array_remove_first JOBS_PIDLIST_ARR "$JOBCOMPLETED"
		else
			# Without `wait -p` support, we need to explicitly wait
			# for a PID. That reduces parallelism.
			JOBCOMPLETED="${JOBS_PIDLIST_ARR[0]}"
			JOBS_PIDLIST_ARR=( "${JOBS_PIDLIST_ARR[@]:1}" )
			wait -n "$JOBCOMPLETED"
			rc_got="$?"
		fi

		local testfile2="${JOBS_PIDLIST[$JOBCOMPLETED]}"
		unset JOBS_PIDLIST[$JOBCOMPLETED]
		print_test_result "${JOBS_TEMPDIR["$testfile2"]}" "$testfile2" "$rc_got"
		check_kmemleak
	done
}

if [ "$NFT_TEST_SHUFFLE_TESTS" = y ] ; then
	TESTS=( $(printf '%s\n' "${TESTS[@]}" | shuf --random-source=<("$NFT_TEST_BASEDIR/helpers/random-source.sh" "nft-test-shuffle-tests" "$NFT_TEST_RANDOM_SEED") ) )
fi

TESTIDX=0
for testfile in "${TESTS[@]}" ; do
	job_wait "$NFT_TEST_JOBS"

	kernel_cleanup

	((TESTIDX++))

	NFT_TEST_TESTTMPDIR="$NFT_TEST_TMPDIR/test-${testfile//\//-}.$TESTIDX"
	mkdir "$NFT_TEST_TESTTMPDIR"
	chmod 755 "$NFT_TEST_TESTTMPDIR"
	JOBS_TEMPDIR["$testfile"]="$NFT_TEST_TESTTMPDIR"

	[[ -o monitor ]] && set_old_state='set -m' || set_old_state='set +m'
	set -m
	( job_start "$testfile" "$TESTIDX" ) &
	pid=$!
	eval "$set_old_state"
	JOBS_PIDLIST[$pid]="$testfile"
	JOBS_PIDLIST_ARR+=( "$pid" )
done

job_wait 0

echo ""

# kmemleak may report suspected leaks
# that get free'd after all, so always do
# a check after all test cases
# have completed and reset the counter
# so another warning gets emitted.
kmemleak_found=0
check_kmemleak_force

failed_total="$failed"
if [ "$NFT_TEST_FAIL_ON_SKIP" = y ] ; then
	failed_total="$((failed_total + skipped))"
fi

if [ "$failed_total" -gt 0 ] ; then
	RR="$RED"
elif [ "$skipped" -gt 0 ] ; then
	RR="$YELLOW"
else
	RR="$GREEN"
fi
msg_info "${RR}results$RESET: [OK] $GREEN$ok$RESET [SKIPPED] $YELLOW$skipped$RESET [FAILED] $RED$failed$RESET [TOTAL] $((ok+skipped+failed))"

kernel_cleanup

#    ( \
#        for d in /tmp/nft-test.latest.*/test-*/ ; do \
#               printf '%10.2f  %s\n' \
#                      "$(sed '1!d' "$d/times")" \
#                      "$(cat "$d/name")" ; \
#        done \
#        | sort -n \
#        | awk '{print $0; s+=$1} END{printf("%10.2f\n", s)}' ; \
#        printf '%10.2f wall time\n' "$(sed '1!d' /tmp/nft-test.latest.*/times)" \
#    )
END_TIME="$(cut -d ' ' -f1 /proc/uptime)"
WALL_TIME="$(awk -v start="$START_TIME" -v end="$END_TIME" "BEGIN { print(end - start) }")"
printf "%s\n" "$WALL_TIME" "$START_TIME" "$END_TIME" > "$NFT_TEST_TMPDIR/times"

if [ "$failed_total" -gt 0 -o "$NFT_TEST_KEEP_LOGS" = y ] ; then
	msg_info "check the temp directory \"$NFT_TEST_TMPDIR\" (\"$NFT_TEST_LATEST\")"
	msg_info "   ls -lad \"$NFT_TEST_LATEST\"/*/*"
	msg_info "   grep -R ^ \"$NFT_TEST_LATEST\"/"
	NFT_TEST_TMPDIR=
fi

if [ "$failed" -gt 0 ] ; then
	exit 1
elif [ "$NFT_TEST_FAIL_ON_SKIP" = y -a "$skipped" -gt 0 ] ; then
	msg_info "some tests were skipped. Fail due to NFT_TEST_FAIL_ON_SKIP=y"
	exit 1
elif [ "$ok" -eq 0 -a "$skipped" -gt 0 ] ; then
	exit 77
else
	exit 0
fi
