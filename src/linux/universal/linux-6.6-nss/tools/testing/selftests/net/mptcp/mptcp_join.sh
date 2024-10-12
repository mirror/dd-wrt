#!/bin/bash
# SPDX-License-Identifier: GPL-2.0

# Double quotes to prevent globbing and word splitting is recommended in new
# code but we accept it, especially because there were too many before having
# address all other issues detected by shellcheck.
#shellcheck disable=SC2086

# ShellCheck incorrectly believes that most of the code here is unreachable
# because it's invoked by variable name, see how the "tests" array is used
#shellcheck disable=SC2317

. "$(dirname "${0}")/mptcp_lib.sh"

ret=0
sin=""
sinfail=""
sout=""
cin=""
cinfail=""
cinsent=""
tmpfile=""
cout=""
err=""
capout=""
ns1=""
ns2=""
ksft_skip=4
iptables="iptables"
ip6tables="ip6tables"
timeout_poll=30
timeout_test=$((timeout_poll * 2 + 1))
capture=0
checksum=0
ip_mptcp=0
check_invert=0
validate_checksum=0
init=0
evts_ns1=""
evts_ns2=""
evts_ns1_pid=0
evts_ns2_pid=0
last_test_failed=0
last_test_skipped=0
last_test_ignored=1

declare -A all_tests
declare -a only_tests_ids
declare -a only_tests_names
declare -A failed_tests
TEST_COUNT=0
TEST_NAME=""
nr_blank=6

# These var are used only in some tests, make sure they are not already set
unset FAILING_LINKS
unset test_linkfail
unset addr_nr_ns1
unset addr_nr_ns2
unset sflags
unset fastclose
unset fullmesh
unset speed

# generated using "nfbpf_compile '(ip && (ip[54] & 0xf0) == 0x30) ||
#				  (ip6 && (ip6[74] & 0xf0) == 0x30)'"
CBPF_MPTCP_SUBOPTION_ADD_ADDR="14,
			       48 0 0 0,
			       84 0 0 240,
			       21 0 3 64,
			       48 0 0 54,
			       84 0 0 240,
			       21 6 7 48,
			       48 0 0 0,
			       84 0 0 240,
			       21 0 4 96,
			       48 0 0 74,
			       84 0 0 240,
			       21 0 1 48,
			       6 0 0 65535,
			       6 0 0 0"

init_partial()
{
	capout=$(mktemp)

	local sec rndh
	sec=$(date +%s)
	rndh=$(printf %x $sec)-$(mktemp -u XXXXXX)

	ns1="ns1-$rndh"
	ns2="ns2-$rndh"

	local netns
	for netns in "$ns1" "$ns2"; do
		ip netns add $netns || exit $ksft_skip
		ip -net $netns link set lo up
		ip netns exec $netns sysctl -q net.mptcp.enabled=1
		ip netns exec $netns sysctl -q net.mptcp.pm_type=0 2>/dev/null || true
		ip netns exec $netns sysctl -q net.ipv4.conf.all.rp_filter=0
		ip netns exec $netns sysctl -q net.ipv4.conf.default.rp_filter=0
		if [ $checksum -eq 1 ]; then
			ip netns exec $netns sysctl -q net.mptcp.checksum_enabled=1
		fi
	done

	check_invert=0
	validate_checksum=$checksum

	#  ns1         ns2
	# ns1eth1    ns2eth1
	# ns1eth2    ns2eth2
	# ns1eth3    ns2eth3
	# ns1eth4    ns2eth4

	local i
	for i in $(seq 1 4); do
		ip link add ns1eth$i netns "$ns1" type veth peer name ns2eth$i netns "$ns2"
		ip -net "$ns1" addr add 10.0.$i.1/24 dev ns1eth$i
		ip -net "$ns1" addr add dead:beef:$i::1/64 dev ns1eth$i nodad
		ip -net "$ns1" link set ns1eth$i up

		ip -net "$ns2" addr add 10.0.$i.2/24 dev ns2eth$i
		ip -net "$ns2" addr add dead:beef:$i::2/64 dev ns2eth$i nodad
		ip -net "$ns2" link set ns2eth$i up

		# let $ns2 reach any $ns1 address from any interface
		ip -net "$ns2" route add default via 10.0.$i.1 dev ns2eth$i metric 10$i
		ip -net "$ns2" route add default via dead:beef:$i::1 dev ns2eth$i metric 10$i
	done
}

init_shapers()
{
	local i
	for i in $(seq 1 4); do
		tc -n $ns1 qdisc add dev ns1eth$i root netem rate 20mbit delay 1ms
		tc -n $ns2 qdisc add dev ns2eth$i root netem rate 20mbit delay 1ms
	done
}

cleanup_partial()
{
	rm -f "$capout"

	local netns
	for netns in "$ns1" "$ns2"; do
		ip netns del $netns
		rm -f /tmp/$netns.{nstat,out}
	done
}

check_tools()
{
	mptcp_lib_check_mptcp
	mptcp_lib_check_kallsyms

	if ! ip -Version &> /dev/null; then
		echo "SKIP: Could not run test without ip tool"
		exit $ksft_skip
	fi

	if ! ss -h | grep -q MPTCP; then
		echo "SKIP: ss tool does not support MPTCP"
		exit $ksft_skip
	fi

	# Use the legacy version if available to support old kernel versions
	if iptables-legacy -V &> /dev/null; then
		iptables="iptables-legacy"
		ip6tables="ip6tables-legacy"
	elif ! iptables -V &> /dev/null; then
		echo "SKIP: Could not run all tests without iptables tool"
		exit $ksft_skip
	elif ! ip6tables -V &> /dev/null; then
		echo "SKIP: Could not run all tests without ip6tables tool"
		exit $ksft_skip
	fi
}

init() {
	init=1

	check_tools

	sin=$(mktemp)
	sout=$(mktemp)
	cin=$(mktemp)
	cinsent=$(mktemp)
	cout=$(mktemp)
	err=$(mktemp)
	evts_ns1=$(mktemp)
	evts_ns2=$(mktemp)

	trap cleanup EXIT

	make_file "$cin" "client" 1 >/dev/null
	make_file "$sin" "server" 1 >/dev/null
}

cleanup()
{
	rm -f "$cin" "$cout" "$sinfail"
	rm -f "$sin" "$sout" "$cinsent" "$cinfail"
	rm -f "$tmpfile"
	rm -rf $evts_ns1 $evts_ns2
	rm -f "$err"
	cleanup_partial
}

print_title()
{
	printf "%03u %s\n" "${TEST_COUNT}" "${TEST_NAME}"
}

print_check()
{
	printf "%-${nr_blank}s%-36s" " " "${*}"
}

print_info()
{
	# It can be empty, no need to print anything then
	[ -z "${1}" ] && return

	mptcp_lib_print_info "      Info: ${*}"
}

print_ok()
{
	mptcp_lib_print_ok "[ ok ]${1:+ ${*}}"
}

print_fail()
{
	mptcp_lib_print_err "[fail]${1:+ ${*}}"
}

print_skip()
{
	mptcp_lib_print_warn "[skip]${1:+ ${*}}"
}

# [ $1: fail msg ]
mark_as_skipped()
{
	local msg="${1:-"Feature not supported"}"

	mptcp_lib_fail_if_expected_feature "${msg}"

	print_check "${msg}"
	print_skip

	last_test_skipped=1
}

# $@: condition
continue_if()
{
	if ! "${@}"; then
		mark_as_skipped
		return 1
	fi
}

skip_test()
{
	if [ "${#only_tests_ids[@]}" -eq 0 ] && [ "${#only_tests_names[@]}" -eq 0 ]; then
		return 1
	fi

	local i
	for i in "${only_tests_ids[@]}"; do
		if [ "${TEST_COUNT}" -eq "${i}" ]; then
			return 1
		fi
	done
	for i in "${only_tests_names[@]}"; do
		if [ "${TEST_NAME}" = "${i}" ]; then
			return 1
		fi
	done

	return 0
}

append_prev_results()
{
	if [ ${last_test_failed} -eq 1 ]; then
		mptcp_lib_result_fail "${TEST_NAME}"
	elif [ ${last_test_skipped} -eq 1 ]; then
		mptcp_lib_result_skip "${TEST_NAME}"
	elif [ ${last_test_ignored} -ne 1 ]; then
		mptcp_lib_result_pass "${TEST_NAME}"
	fi

	last_test_failed=0
	last_test_skipped=0
	last_test_ignored=0
}

# $1: test name
reset()
{
	append_prev_results

	TEST_NAME="${1}"

	TEST_COUNT=$((TEST_COUNT+1))

	if skip_test; then
		last_test_ignored=1
		return 1
	fi

	print_title

	if [ "${init}" != "1" ]; then
		init
	else
		cleanup_partial
	fi

	init_partial

	return 0
}

# $1: test name ; $2: counter to check
reset_check_counter()
{
	reset "${1}" || return 1

	local counter="${2}"

	if ! nstat -asz "${counter}" | grep -wq "${counter}"; then
		mark_as_skipped "counter '${counter}' is not available"
		return 1
	fi
}

# $1: test name
reset_with_cookies()
{
	reset "${1}" || return 1

	local netns
	for netns in "$ns1" "$ns2"; do
		ip netns exec $netns sysctl -q net.ipv4.tcp_syncookies=2
	done
}

# $1: test name
reset_with_add_addr_timeout()
{
	local ip="${2:-4}"
	local tables

	reset "${1}" || return 1

	tables="${iptables}"
	if [ $ip -eq 6 ]; then
		tables="${ip6tables}"
	fi

	ip netns exec $ns1 sysctl -q net.mptcp.add_addr_timeout=1

	if ! ip netns exec $ns2 $tables -A OUTPUT -p tcp \
			-m tcp --tcp-option 30 \
			-m bpf --bytecode \
			"$CBPF_MPTCP_SUBOPTION_ADD_ADDR" \
			-j DROP; then
		mark_as_skipped "unable to set the 'add addr' rule"
		return 1
	fi
}

# $1: test name
reset_with_checksum()
{
	local ns1_enable=$1
	local ns2_enable=$2

	reset "checksum test ${1} ${2}" || return 1

	ip netns exec $ns1 sysctl -q net.mptcp.checksum_enabled=$ns1_enable
	ip netns exec $ns2 sysctl -q net.mptcp.checksum_enabled=$ns2_enable

	validate_checksum=1
}

reset_with_allow_join_id0()
{
	local ns1_enable=$2
	local ns2_enable=$3

	reset "${1}" || return 1

	ip netns exec $ns1 sysctl -q net.mptcp.allow_join_initial_addr_port=$ns1_enable
	ip netns exec $ns2 sysctl -q net.mptcp.allow_join_initial_addr_port=$ns2_enable
}

# Modify TCP payload without corrupting the TCP packet
#
# This rule inverts a 8-bit word at byte offset 148 for the 2nd TCP ACK packets
# carrying enough data.
# Once it is done, the TCP Checksum field is updated so the packet is still
# considered as valid at the TCP level.
# Because the MPTCP checksum, covering the TCP options and data, has not been
# updated, the modification will be detected and an MP_FAIL will be emitted:
# what we want to validate here without corrupting "random" MPTCP options.
#
# To avoid having tc producing this pr_info() message for each TCP ACK packets
# not carrying enough data:
#
#     tc action pedit offset 162 out of bounds
#
# Netfilter is used to mark packets with enough data.
setup_fail_rules()
{
	check_invert=1
	validate_checksum=1
	local i="$1"
	local ip="${2:-4}"
	local tables

	tables="${iptables}"
	if [ $ip -eq 6 ]; then
		tables="${ip6tables}"
	fi

	ip netns exec $ns2 $tables \
		-t mangle \
		-A OUTPUT \
		-o ns2eth$i \
		-p tcp \
		-m length --length 150:9999 \
		-m statistic --mode nth --packet 1 --every 99999 \
		-j MARK --set-mark 42 || return ${ksft_skip}

	tc -n $ns2 qdisc add dev ns2eth$i clsact || return ${ksft_skip}
	tc -n $ns2 filter add dev ns2eth$i egress \
		protocol ip prio 1000 \
		handle 42 fw \
		action pedit munge offset 148 u8 invert \
		pipe csum tcp \
		index 100 || return ${ksft_skip}
}

reset_with_fail()
{
	reset_check_counter "${1}" "MPTcpExtInfiniteMapTx" || return 1
	shift

	ip netns exec $ns1 sysctl -q net.mptcp.checksum_enabled=1
	ip netns exec $ns2 sysctl -q net.mptcp.checksum_enabled=1

	local rc=0
	setup_fail_rules "${@}" || rc=$?

	if [ ${rc} -eq ${ksft_skip} ]; then
		mark_as_skipped "unable to set the 'fail' rules"
		return 1
	fi
}

start_events()
{
	mptcp_lib_events "${ns1}" "${evts_ns1}" evts_ns1_pid
	mptcp_lib_events "${ns2}" "${evts_ns2}" evts_ns2_pid
}

reset_with_events()
{
	reset "${1}" || return 1

	start_events
}

reset_with_tcp_filter()
{
	reset "${1}" || return 1
	shift

	local ns="${!1}"
	local src="${2}"
	local target="${3}"
	local chain="${4:-INPUT}"

	if ! ip netns exec "${ns}" ${iptables} \
			-A "${chain}" \
			-s "${src}" \
			-p tcp \
			-j "${target}"; then
		mark_as_skipped "unable to set the filter rules"
		return 1
	fi
}

# $1: err msg
fail_test()
{
	ret=1

	print_fail "${@}"

	# just in case a test is marked twice as failed
	if [ ${last_test_failed} -eq 0 ]; then
		failed_tests[${TEST_COUNT}]="${TEST_NAME}"
		dump_stats
		last_test_failed=1
	fi
}

get_failed_tests_ids()
{
	# sorted
	local i
	for i in "${!failed_tests[@]}"; do
		echo "${i}"
	done | sort -n
}

print_file_err()
{
	ls -l "$1" 1>&2
	echo -n "Trailing bytes are: "
	tail -c 27 "$1"
}

check_transfer()
{
	local in=$1
	local out=$2
	local what=$3
	local bytes=$4
	local i a b

	local line
	if [ -n "$bytes" ]; then
		local out_size
		# when truncating we must check the size explicitly
		out_size=$(wc -c $out | awk '{print $1}')
		if [ $out_size -ne $bytes ]; then
			fail_test "$what output file has wrong size ($out_size, $bytes)"
			return 1
		fi

		# note: BusyBox's "cmp" command doesn't support --bytes
		tmpfile=$(mktemp)
		head --bytes="$bytes" "$in" > "$tmpfile"
		mv "$tmpfile" "$in"
		head --bytes="$bytes" "$out" > "$tmpfile"
		mv "$tmpfile" "$out"
		tmpfile=""
	fi
	cmp -l "$in" "$out" | while read -r i a b; do
		local sum=$((0${a} + 0${b}))
		if [ $check_invert -eq 0 ] || [ $sum -ne $((0xff)) ]; then
			fail_test "$what does not match (in, out):"
			print_file_err "$in"
			print_file_err "$out"

			return 1
		else
			print_info "$what has inverted byte at ${i}"
		fi
	done

	return 0
}

do_ping()
{
	local listener_ns="$1"
	local connector_ns="$2"
	local connect_addr="$3"

	if ! ip netns exec ${connector_ns} ping -q -c 1 $connect_addr >/dev/null; then
		fail_test "$listener_ns -> $connect_addr connectivity"
	fi
}

link_failure()
{
	local ns="$1"

	if [ -z "$FAILING_LINKS" ]; then
		l=$((RANDOM%4))
		FAILING_LINKS=$((l+1))
	fi

	local l
	for l in $FAILING_LINKS; do
		local veth="ns1eth$l"
		ip -net "$ns" link set "$veth" down
	done
}

# $1: ns, $2: port
wait_local_port_listen()
{
	local listener_ns="${1}"
	local port="${2}"

	local port_hex
	port_hex="$(printf "%04X" "${port}")"

	local i
	for i in $(seq 10); do
		ip netns exec "${listener_ns}" cat /proc/net/tcp* | \
			awk "BEGIN {rc=1} {if (\$2 ~ /:${port_hex}\$/ && \$4 ~ /0A/) {rc=0; exit}} END {exit rc}" &&
			break
		sleep 0.1
	done
}

rm_addr_count()
{
	mptcp_lib_get_counter "${1}" "MPTcpExtRmAddr"
}

# $1: ns, $2: old rm_addr counter in $ns
wait_rm_addr()
{
	local ns="${1}"
	local old_cnt="${2}"
	local cnt

	local i
	for i in $(seq 10); do
		cnt=$(rm_addr_count ${ns})
		[ "$cnt" = "${old_cnt}" ] || break
		sleep 0.1
	done
}

rm_sf_count()
{
	mptcp_lib_get_counter "${1}" "MPTcpExtRmSubflow"
}

# $1: ns, $2: old rm_sf counter in $ns
wait_rm_sf()
{
	local ns="${1}"
	local old_cnt="${2}"
	local cnt

	local i
	for i in $(seq 10); do
		cnt=$(rm_sf_count ${ns})
		[ "$cnt" = "${old_cnt}" ] || break
		sleep 0.1
	done
}

wait_mpj()
{
	local ns="${1}"
	local cnt old_cnt

	old_cnt=$(mptcp_lib_get_counter ${ns} "MPTcpExtMPJoinAckRx")

	local i
	for i in $(seq 10); do
		cnt=$(mptcp_lib_get_counter ${ns} "MPTcpExtMPJoinAckRx")
		[ "$cnt" = "${old_cnt}" ] || break
		sleep 0.1
	done
}

kill_events_pids()
{
	mptcp_lib_kill_wait $evts_ns1_pid
	evts_ns1_pid=0
	mptcp_lib_kill_wait $evts_ns2_pid
	evts_ns2_pid=0
}

pm_nl_set_limits()
{
	local ns=$1
	local addrs=$2
	local subflows=$3

	if [ $ip_mptcp -eq 1 ]; then
		ip -n $ns mptcp limits set add_addr_accepted $addrs subflows $subflows
	else
		ip netns exec $ns ./pm_nl_ctl limits $addrs $subflows
	fi
}

pm_nl_add_endpoint()
{
	local ns=$1
	local addr=$2
	local flags _flags
	local port _port
	local dev _dev
	local id _id
	local nr=2

	local p
	for p in "${@}"
	do
		if [ $p = "flags" ]; then
			eval _flags=\$"$nr"
			[ -n "$_flags" ]; flags="flags $_flags"
		fi
		if [ $p = "dev" ]; then
			eval _dev=\$"$nr"
			[ -n "$_dev" ]; dev="dev $_dev"
		fi
		if [ $p = "id" ]; then
			eval _id=\$"$nr"
			[ -n "$_id" ]; id="id $_id"
		fi
		if [ $p = "port" ]; then
			eval _port=\$"$nr"
			[ -n "$_port" ]; port="port $_port"
		fi

		nr=$((nr + 1))
	done

	if [ $ip_mptcp -eq 1 ]; then
		ip -n $ns mptcp endpoint add $addr ${_flags//","/" "} $dev $id $port
	else
		ip netns exec $ns ./pm_nl_ctl add $addr $flags $dev $id $port
	fi
}

pm_nl_del_endpoint()
{
	local ns=$1
	local id=$2
	local addr=$3

	if [ $ip_mptcp -eq 1 ]; then
		[ $id -ne 0 ] && addr=''
		ip -n $ns mptcp endpoint delete id $id $addr
	else
		ip netns exec $ns ./pm_nl_ctl del $id $addr
	fi
}

pm_nl_flush_endpoint()
{
	local ns=$1

	if [ $ip_mptcp -eq 1 ]; then
		ip -n $ns mptcp endpoint flush
	else
		ip netns exec $ns ./pm_nl_ctl flush
	fi
}

pm_nl_show_endpoints()
{
	local ns=$1

	if [ $ip_mptcp -eq 1 ]; then
		ip -n $ns mptcp endpoint show
	else
		ip netns exec $ns ./pm_nl_ctl dump
	fi
}

pm_nl_change_endpoint()
{
	local ns=$1
	local id=$2
	local flags=$3

	if [ $ip_mptcp -eq 1 ]; then
		ip -n $ns mptcp endpoint change id $id ${flags//","/" "}
	else
		ip netns exec $ns ./pm_nl_ctl set id $id flags $flags
	fi
}

pm_nl_check_endpoint()
{
	local line expected_line
	local msg="$1"
	local ns=$2
	local addr=$3
	local _flags=""
	local flags
	local _port
	local port
	local dev
	local _id
	local id

	print_check "${msg}"

	shift 3
	while [ -n "$1" ]; do
		if [ $1 = "flags" ]; then
			_flags=$2
			[ -n "$_flags" ]; flags="flags $_flags"
			shift
		elif [ $1 = "dev" ]; then
			[ -n "$2" ]; dev="dev $2"
			shift
		elif [ $1 = "id" ]; then
			_id=$2
			[ -n "$_id" ]; id="id $_id"
			shift
		elif [ $1 = "port" ]; then
			_port=$2
			[ -n "$_port" ]; port=" port $_port"
			shift
		fi

		shift
	done

	if [ -z "$id" ]; then
		fail_test "bad test - missing endpoint id"
		return
	fi

	if [ $ip_mptcp -eq 1 ]; then
		# get line and trim trailing whitespace
		line=$(ip -n $ns mptcp endpoint show $id)
		line="${line% }"
		# the dump order is: address id flags port dev
		[ -n "$addr" ] && expected_line="$addr"
		expected_line+=" $id"
		[ -n "$_flags" ] && expected_line+=" ${_flags//","/" "}"
		[ -n "$dev" ] && expected_line+=" $dev"
		[ -n "$port" ] && expected_line+=" $port"
	else
		line=$(ip netns exec $ns ./pm_nl_ctl get $_id)
		# the dump order is: id flags dev address port
		expected_line="$id"
		[ -n "$flags" ] && expected_line+=" $flags"
		[ -n "$dev" ] && expected_line+=" $dev"
		[ -n "$addr" ] && expected_line+=" $addr"
		[ -n "$_port" ] && expected_line+=" $_port"
	fi
	if [ "$line" = "$expected_line" ]; then
		print_ok
	else
		fail_test "expected '$expected_line' found '$line'"
	fi
}

pm_nl_set_endpoint()
{
	local listener_ns="$1"
	local connector_ns="$2"
	local connect_addr="$3"

	local addr_nr_ns1=${addr_nr_ns1:-0}
	local addr_nr_ns2=${addr_nr_ns2:-0}
	local sflags=${sflags:-""}
	local fullmesh=${fullmesh:-""}

	local flags="subflow"
	if [ -n "${fullmesh}" ]; then
		flags="${flags},fullmesh"
		addr_nr_ns2=${fullmesh}
	fi

	# let the mptcp subflow be established in background before
	# do endpoint manipulation
	if [ $addr_nr_ns1 != "0" ] || [ $addr_nr_ns2 != "0" ]; then
		sleep 1
	fi

	if [ $addr_nr_ns1 -gt 0 ]; then
		local counter=2
		local add_nr_ns1=${addr_nr_ns1}
		local id=10
		while [ $add_nr_ns1 -gt 0 ]; do
			local addr
			if mptcp_lib_is_v6 "${connect_addr}"; then
				addr="dead:beef:$counter::1"
			else
				addr="10.0.$counter.1"
			fi
			pm_nl_add_endpoint $ns1 $addr flags signal
			counter=$((counter + 1))
			add_nr_ns1=$((add_nr_ns1 - 1))
			id=$((id + 1))
		done
	elif [ $addr_nr_ns1 -lt 0 ]; then
		local rm_nr_ns1=$((-addr_nr_ns1))
		if [ $rm_nr_ns1 -lt 8 ]; then
			local counter=0
			local line
			pm_nl_show_endpoints ${listener_ns} | while read -r line; do
				# shellcheck disable=SC2206 # we do want to split per word
				local arr=($line)
				local nr=0

				local i
				for i in "${arr[@]}"; do
					if [ $i = "id" ]; then
						if [ $counter -eq $rm_nr_ns1 ]; then
							break
						fi
						id=${arr[$nr+1]}
						rm_addr=$(rm_addr_count ${connector_ns})
						pm_nl_del_endpoint ${listener_ns} $id
						wait_rm_addr ${connector_ns} ${rm_addr}
						counter=$((counter + 1))
					fi
					nr=$((nr + 1))
				done
			done
		elif [ $rm_nr_ns1 -eq 8 ]; then
			pm_nl_flush_endpoint ${listener_ns}
		elif [ $rm_nr_ns1 -eq 9 ]; then
			pm_nl_del_endpoint ${listener_ns} 0 ${connect_addr}
		fi
	fi

	# if newly added endpoints must be deleted, give the background msk
	# some time to created them
	[ $addr_nr_ns1 -gt 0 ] && [ $addr_nr_ns2 -lt 0 ] && sleep 1

	if [ $addr_nr_ns2 -gt 0 ]; then
		local add_nr_ns2=${addr_nr_ns2}
		local counter=3
		local id=20
		while [ $add_nr_ns2 -gt 0 ]; do
			local addr
			if mptcp_lib_is_v6 "${connect_addr}"; then
				addr="dead:beef:$counter::2"
			else
				addr="10.0.$counter.2"
			fi
			pm_nl_add_endpoint $ns2 $addr flags $flags
			counter=$((counter + 1))
			add_nr_ns2=$((add_nr_ns2 - 1))
			id=$((id + 1))
		done
	elif [ $addr_nr_ns2 -lt 0 ]; then
		local rm_nr_ns2=$((-addr_nr_ns2))
		if [ $rm_nr_ns2 -lt 8 ]; then
			local counter=0
			local line
			pm_nl_show_endpoints ${connector_ns} | while read -r line; do
				# shellcheck disable=SC2206 # we do want to split per word
				local arr=($line)
				local nr=0

				local i
				for i in "${arr[@]}"; do
					if [ $i = "id" ]; then
						if [ $counter -eq $rm_nr_ns2 ]; then
							break
						fi
						local id rm_addr
						# rm_addr are serialized, allow the previous one to
						# complete
						id=${arr[$nr+1]}
						rm_addr=$(rm_addr_count ${listener_ns})
						pm_nl_del_endpoint ${connector_ns} $id
						wait_rm_addr ${listener_ns} ${rm_addr}
						counter=$((counter + 1))
					fi
					nr=$((nr + 1))
				done
			done
		elif [ $rm_nr_ns2 -eq 8 ]; then
			pm_nl_flush_endpoint ${connector_ns}
		elif [ $rm_nr_ns2 -eq 9 ]; then
			local addr
			if mptcp_lib_is_v6 "${connect_addr}"; then
				addr="dead:beef:1::2"
			else
				addr="10.0.1.2"
			fi
			pm_nl_del_endpoint ${connector_ns} 0 $addr
		fi
	fi

	if [ -n "${sflags}" ]; then
		sleep 1

		local netns
		for netns in "$ns1" "$ns2"; do
			local line
			pm_nl_show_endpoints $netns | while read -r line; do
				# shellcheck disable=SC2206 # we do want to split per word
				local arr=($line)
				local nr=0
				local id

				local i
				for i in "${arr[@]}"; do
					if [ $i = "id" ]; then
						id=${arr[$nr+1]}
					fi
					nr=$((nr + 1))
				done
				pm_nl_change_endpoint $netns $id $sflags
			done
		done
	fi
}

do_transfer()
{
	local listener_ns="$1"
	local connector_ns="$2"
	local cl_proto="$3"
	local srv_proto="$4"
	local connect_addr="$5"

	local port=$((10000 + TEST_COUNT - 1))
	local cappid
	local FAILING_LINKS=${FAILING_LINKS:-""}
	local fastclose=${fastclose:-""}
	local speed=${speed:-"fast"}

	:> "$cout"
	:> "$sout"
	:> "$capout"

	if [ $capture -eq 1 ]; then
		local capuser
		if [ -z $SUDO_USER ] ; then
			capuser=""
		else
			capuser="-Z $SUDO_USER"
		fi

		capfile=$(printf "mp_join-%02u-%s.pcap" "$TEST_COUNT" "${listener_ns}")

		echo "Capturing traffic for test $TEST_COUNT into $capfile"
		ip netns exec ${listener_ns} tcpdump -i any -s 65535 -B 32768 $capuser -w $capfile > "$capout" 2>&1 &
		cappid=$!

		sleep 1
	fi

	NSTAT_HISTORY=/tmp/${listener_ns}.nstat ip netns exec ${listener_ns} \
		nstat -n
	NSTAT_HISTORY=/tmp/${connector_ns}.nstat ip netns exec ${connector_ns} \
		nstat -n

	local extra_args
	if [ $speed = "fast" ]; then
		extra_args="-j"
	elif [ $speed = "slow" ]; then
		extra_args="-r 50"
	elif [ $speed -gt 0 ]; then
		extra_args="-r ${speed}"
	fi

	local extra_cl_args=""
	local extra_srv_args=""
	local trunc_size=""
	if [ -n "${fastclose}" ]; then
		if [ ${test_linkfail} -le 1 ]; then
			fail_test "fastclose tests need test_linkfail argument"
			return 1
		fi

		# disconnect
		trunc_size=${test_linkfail}
		local side=${fastclose}

		if [ ${side} = "client" ]; then
			extra_cl_args="-f ${test_linkfail}"
			extra_srv_args="-f -1"
		elif [ ${side} = "server" ]; then
			extra_srv_args="-f ${test_linkfail}"
			extra_cl_args="-f -1"
		else
			fail_test "wrong/unknown fastclose spec ${side}"
			return 1
		fi
	fi

	extra_srv_args="$extra_args $extra_srv_args"
	if [ "$test_linkfail" -gt 1 ];then
		timeout ${timeout_test} \
			ip netns exec ${listener_ns} \
				./mptcp_connect -t ${timeout_poll} -l -p $port -s ${srv_proto} \
					$extra_srv_args "::" < "$sinfail" > "$sout" &
	else
		timeout ${timeout_test} \
			ip netns exec ${listener_ns} \
				./mptcp_connect -t ${timeout_poll} -l -p $port -s ${srv_proto} \
					$extra_srv_args "::" < "$sin" > "$sout" &
	fi
	local spid=$!

	wait_local_port_listen "${listener_ns}" "${port}"

	extra_cl_args="$extra_args $extra_cl_args"
	if [ "$test_linkfail" -eq 0 ];then
		timeout ${timeout_test} \
			ip netns exec ${connector_ns} \
				./mptcp_connect -t ${timeout_poll} -p $port -s ${cl_proto} \
					$extra_cl_args $connect_addr < "$cin" > "$cout" &
	elif [ "$test_linkfail" -eq 1 ] || [ "$test_linkfail" -eq 2 ];then
		( cat "$cinfail" ; sleep 2; link_failure $listener_ns ; cat "$cinfail" ) | \
			tee "$cinsent" | \
			timeout ${timeout_test} \
				ip netns exec ${connector_ns} \
					./mptcp_connect -t ${timeout_poll} -p $port -s ${cl_proto} \
						$extra_cl_args $connect_addr > "$cout" &
	else
		tee "$cinsent" < "$cinfail" | \
			timeout ${timeout_test} \
				ip netns exec ${connector_ns} \
					./mptcp_connect -t ${timeout_poll} -p $port -s ${cl_proto} \
						$extra_cl_args $connect_addr > "$cout" &
	fi
	local cpid=$!

	pm_nl_set_endpoint $listener_ns $connector_ns $connect_addr

	wait $cpid
	local retc=$?
	wait $spid
	local rets=$?

	if [ $capture -eq 1 ]; then
	    sleep 1
	    kill $cappid
	fi

	NSTAT_HISTORY=/tmp/${listener_ns}.nstat ip netns exec ${listener_ns} \
		nstat | grep Tcp > /tmp/${listener_ns}.out
	NSTAT_HISTORY=/tmp/${connector_ns}.nstat ip netns exec ${connector_ns} \
		nstat | grep Tcp > /tmp/${connector_ns}.out

	if [ ${rets} -ne 0 ] || [ ${retc} -ne 0 ]; then
		fail_test "client exit code $retc, server $rets"
		echo -e "\nnetns ${listener_ns} socket stat for ${port}:" 1>&2
		ip netns exec ${listener_ns} ss -Menita 1>&2 -o "sport = :$port"
		cat /tmp/${listener_ns}.out
		echo -e "\nnetns ${connector_ns} socket stat for ${port}:" 1>&2
		ip netns exec ${connector_ns} ss -Menita 1>&2 -o "dport = :$port"
		cat /tmp/${connector_ns}.out

		cat "$capout"
		return 1
	fi

	if [ "$test_linkfail" -gt 1 ];then
		check_transfer $sinfail $cout "file received by client" $trunc_size
	else
		check_transfer $sin $cout "file received by client" $trunc_size
	fi
	retc=$?
	if [ "$test_linkfail" -eq 0 ];then
		check_transfer $cin $sout "file received by server" $trunc_size
	else
		check_transfer $cinsent $sout "file received by server" $trunc_size
	fi
	rets=$?

	if [ $retc -eq 0 ] && [ $rets -eq 0 ];then
		cat "$capout"
		return 0
	fi

	cat "$capout"
	return 1
}

make_file()
{
	local name=$1
	local who=$2
	local size=$3

	dd if=/dev/urandom of="$name" bs=1024 count=$size 2> /dev/null
	echo -e "\nMPTCP_TEST_FILE_END_MARKER" >> "$name"

	print_info "Test file (size $size KB) for $who"
}

run_tests()
{
	local listener_ns="$1"
	local connector_ns="$2"
	local connect_addr="$3"

	local size
	local test_linkfail=${test_linkfail:-0}

	# The values above 2 are reused to make test files
	# with the given sizes (KB)
	if [ "$test_linkfail" -gt 2 ]; then
		size=$test_linkfail

		if [ -z "$cinfail" ]; then
			cinfail=$(mktemp)
		fi
		make_file "$cinfail" "client" $size
	# create the input file for the failure test when
	# the first failure test run
	elif [ "$test_linkfail" -ne 0 ] && [ -z "$cinfail" ]; then
		# the client file must be considerably larger
		# of the maximum expected cwin value, or the
		# link utilization will be not predicable
		size=$((RANDOM%2))
		size=$((size+1))
		size=$((size*8192))
		size=$((size + ( RANDOM % 8192) ))

		cinfail=$(mktemp)
		make_file "$cinfail" "client" $size
	fi

	if [ "$test_linkfail" -gt 2 ]; then
		size=$test_linkfail

		if [ -z "$sinfail" ]; then
			sinfail=$(mktemp)
		fi
		make_file "$sinfail" "server" $size
	elif [ "$test_linkfail" -eq 2 ] && [ -z "$sinfail" ]; then
		size=$((RANDOM%16))
		size=$((size+1))
		size=$((size*2048))

		sinfail=$(mktemp)
		make_file "$sinfail" "server" $size
	fi

	do_transfer ${listener_ns} ${connector_ns} MPTCP MPTCP ${connect_addr}
}

dump_stats()
{
	echo Server ns stats
	ip netns exec $ns1 nstat -as | grep Tcp
	echo Client ns stats
	ip netns exec $ns2 nstat -as | grep Tcp
}

chk_csum_nr()
{
	local csum_ns1=${1:-0}
	local csum_ns2=${2:-0}
	local count
	local extra_msg=""
	local allow_multi_errors_ns1=0
	local allow_multi_errors_ns2=0

	if [[ "${csum_ns1}" = "+"* ]]; then
		allow_multi_errors_ns1=1
		csum_ns1=${csum_ns1:1}
	fi
	if [[ "${csum_ns2}" = "+"* ]]; then
		allow_multi_errors_ns2=1
		csum_ns2=${csum_ns2:1}
	fi

	print_check "sum"
	count=$(mptcp_lib_get_counter ${ns1} "MPTcpExtDataCsumErr")
	if [ -n "$count" ] && [ "$count" != "$csum_ns1" ]; then
		extra_msg+=" ns1=$count"
	fi
	if [ -z "$count" ]; then
		print_skip
	elif { [ "$count" != $csum_ns1 ] && [ $allow_multi_errors_ns1 -eq 0 ]; } ||
	     { [ "$count" -lt $csum_ns1 ] && [ $allow_multi_errors_ns1 -eq 1 ]; }; then
		fail_test "got $count data checksum error[s] expected $csum_ns1"
	else
		print_ok
	fi
	print_check "csum"
	count=$(mptcp_lib_get_counter ${ns2} "MPTcpExtDataCsumErr")
	if [ -n "$count" ] && [ "$count" != "$csum_ns2" ]; then
		extra_msg+=" ns2=$count"
	fi
	if [ -z "$count" ]; then
		print_skip
	elif { [ "$count" != $csum_ns2 ] && [ $allow_multi_errors_ns2 -eq 0 ]; } ||
	     { [ "$count" -lt $csum_ns2 ] && [ $allow_multi_errors_ns2 -eq 1 ]; }; then
		fail_test "got $count data checksum error[s] expected $csum_ns2"
	else
		print_ok
	fi

	print_info "$extra_msg"
}

chk_fail_nr()
{
	local fail_tx=$1
	local fail_rx=$2
	local ns_invert=${3:-""}
	local count
	local ns_tx=$ns1
	local ns_rx=$ns2
	local extra_msg=""
	local allow_tx_lost=0
	local allow_rx_lost=0

	if [[ $ns_invert = "invert" ]]; then
		ns_tx=$ns2
		ns_rx=$ns1
		extra_msg="invert"
	fi

	if [[ "${fail_tx}" = "-"* ]]; then
		allow_tx_lost=1
		fail_tx=${fail_tx:1}
	fi
	if [[ "${fail_rx}" = "-"* ]]; then
		allow_rx_lost=1
		fail_rx=${fail_rx:1}
	fi

	print_check "ftx"
	count=$(mptcp_lib_get_counter ${ns_tx} "MPTcpExtMPFailTx")
	if [ -n "$count" ] && [ "$count" != "$fail_tx" ]; then
		extra_msg+=",tx=$count"
	fi
	if [ -z "$count" ]; then
		print_skip
	elif { [ "$count" != "$fail_tx" ] && [ $allow_tx_lost -eq 0 ]; } ||
	     { [ "$count" -gt "$fail_tx" ] && [ $allow_tx_lost -eq 1 ]; }; then
		fail_test "got $count MP_FAIL[s] TX expected $fail_tx"
	else
		print_ok
	fi

	print_check "failrx"
	count=$(mptcp_lib_get_counter ${ns_rx} "MPTcpExtMPFailRx")
	if [ -n "$count" ] && [ "$count" != "$fail_rx" ]; then
		extra_msg+=",rx=$count"
	fi
	if [ -z "$count" ]; then
		print_skip
	elif { [ "$count" != "$fail_rx" ] && [ $allow_rx_lost -eq 0 ]; } ||
	     { [ "$count" -gt "$fail_rx" ] && [ $allow_rx_lost -eq 1 ]; }; then
		fail_test "got $count MP_FAIL[s] RX expected $fail_rx"
	else
		print_ok
	fi

	print_info "$extra_msg"
}

chk_fclose_nr()
{
	local fclose_tx=$1
	local fclose_rx=$2
	local ns_invert=$3
	local count
	local ns_tx=$ns2
	local ns_rx=$ns1
	local extra_msg=""

	if [[ $ns_invert = "invert" ]]; then
		ns_tx=$ns1
		ns_rx=$ns2
		extra_msg="invert"
	fi

	print_check "ctx"
	count=$(mptcp_lib_get_counter ${ns_tx} "MPTcpExtMPFastcloseTx")
	if [ -z "$count" ]; then
		print_skip
	elif [ "$count" != "$fclose_tx" ]; then
		extra_msg+=",tx=$count"
		fail_test "got $count MP_FASTCLOSE[s] TX expected $fclose_tx"
	else
		print_ok
	fi

	print_check "fclzrx"
	count=$(mptcp_lib_get_counter ${ns_rx} "MPTcpExtMPFastcloseRx")
	if [ -z "$count" ]; then
		print_skip
	elif [ "$count" != "$fclose_rx" ]; then
		extra_msg+=",rx=$count"
		fail_test "got $count MP_FASTCLOSE[s] RX expected $fclose_rx"
	else
		print_ok
	fi

	print_info "$extra_msg"
}

chk_rst_nr()
{
	local rst_tx=$1
	local rst_rx=$2
	local ns_invert=${3:-""}
	local count
	local ns_tx=$ns1
	local ns_rx=$ns2
	local extra_msg=""

	if [[ $ns_invert = "invert" ]]; then
		ns_tx=$ns2
		ns_rx=$ns1
		extra_msg="invert"
	fi

	print_check "rtx"
	count=$(mptcp_lib_get_counter ${ns_tx} "MPTcpExtMPRstTx")
	if [ -z "$count" ]; then
		print_skip
	# accept more rst than expected except if we don't expect any
	elif { [ $rst_tx -ne 0 ] && [ $count -lt $rst_tx ]; } ||
	     { [ $rst_tx -eq 0 ] && [ $count -ne 0 ]; }; then
		fail_test "got $count MP_RST[s] TX expected $rst_tx"
	else
		print_ok
	fi

	print_check "rstrx"
	count=$(mptcp_lib_get_counter ${ns_rx} "MPTcpExtMPRstRx")
	if [ -z "$count" ]; then
		print_skip
	# accept more rst than expected except if we don't expect any
	elif { [ $rst_rx -ne 0 ] && [ $count -lt $rst_rx ]; } ||
	     { [ $rst_rx -eq 0 ] && [ $count -ne 0 ]; }; then
		fail_test "got $count MP_RST[s] RX expected $rst_rx"
	else
		print_ok
	fi

	print_info "$extra_msg"
}

chk_infi_nr()
{
	local infi_tx=$1
	local infi_rx=$2
	local count

	print_check "itx"
	count=$(mptcp_lib_get_counter ${ns2} "MPTcpExtInfiniteMapTx")
	if [ -z "$count" ]; then
		print_skip
	elif [ "$count" != "$infi_tx" ]; then
		fail_test "got $count infinite map[s] TX expected $infi_tx"
	else
		print_ok
	fi

	print_check "infirx"
	count=$(mptcp_lib_get_counter ${ns1} "MPTcpExtInfiniteMapRx")
	if [ -z "$count" ]; then
		print_skip
	elif [ "$count" != "$infi_rx" ]; then
		fail_test "got $count infinite map[s] RX expected $infi_rx"
	else
		print_ok
	fi
}

chk_join_nr()
{
	local syn_nr=$1
	local syn_ack_nr=$2
	local ack_nr=$3
	local csum_ns1=${4:-0}
	local csum_ns2=${5:-0}
	local fail_nr=${6:-0}
	local rst_nr=${7:-0}
	local infi_nr=${8:-0}
	local corrupted_pkts=${9:-0}
	local count
	local with_cookie

	if [ "${corrupted_pkts}" -gt 0 ]; then
		print_info "${corrupted_pkts} corrupted pkts"
	fi

	print_check "syn"
	count=$(mptcp_lib_get_counter ${ns1} "MPTcpExtMPJoinSynRx")
	if [ -z "$count" ]; then
		print_skip
	elif [ "$count" != "$syn_nr" ]; then
		fail_test "got $count JOIN[s] syn expected $syn_nr"
	else
		print_ok
	fi

	print_check "synack"
	with_cookie=$(ip netns exec $ns2 sysctl -n net.ipv4.tcp_syncookies)
	count=$(mptcp_lib_get_counter ${ns2} "MPTcpExtMPJoinSynAckRx")
	if [ -z "$count" ]; then
		print_skip
	elif [ "$count" != "$syn_ack_nr" ]; then
		# simult connections exceeding the limit with cookie enabled could go up to
		# synack validation as the conn limit can be enforced reliably only after
		# the subflow creation
		if [ "$with_cookie" = 2 ] && [ "$count" -gt "$syn_ack_nr" ] && [ "$count" -le "$syn_nr" ]; then
			print_ok
		else
			fail_test "got $count JOIN[s] synack expected $syn_ack_nr"
		fi
	else
		print_ok
	fi

	print_check "ack"
	count=$(mptcp_lib_get_counter ${ns1} "MPTcpExtMPJoinAckRx")
	if [ -z "$count" ]; then
		print_skip
	elif [ "$count" != "$ack_nr" ]; then
		fail_test "got $count JOIN[s] ack expected $ack_nr"
	else
		print_ok
	fi
	if [ $validate_checksum -eq 1 ]; then
		chk_csum_nr $csum_ns1 $csum_ns2
		chk_fail_nr $fail_nr $fail_nr
		chk_rst_nr $rst_nr $rst_nr
		chk_infi_nr $infi_nr $infi_nr
	fi
}

# a negative value for 'stale_max' means no upper bound:
# for bidirectional transfer, if one peer sleep for a while
# - as these tests do - we can have a quite high number of
# stale/recover conversions, proportional to
# sleep duration/ MPTCP-level RTX interval.
chk_stale_nr()
{
	local ns=$1
	local stale_min=$2
	local stale_max=$3
	local stale_delta=$4
	local dump_stats
	local stale_nr
	local recover_nr

	print_check "stale"

	stale_nr=$(mptcp_lib_get_counter ${ns} "MPTcpExtSubflowStale")
	recover_nr=$(mptcp_lib_get_counter ${ns} "MPTcpExtSubflowRecover")
	if [ -z "$stale_nr" ] || [ -z "$recover_nr" ]; then
		print_skip
	elif [ $stale_nr -lt $stale_min ] ||
	   { [ $stale_max -gt 0 ] && [ $stale_nr -gt $stale_max ]; } ||
	   [ $((stale_nr - recover_nr)) -ne $stale_delta ]; then
		fail_test "got $stale_nr stale[s] $recover_nr recover[s], " \
		     " expected stale in range [$stale_min..$stale_max]," \
		     " stale-recover delta $stale_delta"
		dump_stats=1
	else
		print_ok
	fi

	if [ "${dump_stats}" = 1 ]; then
		echo $ns stats
		ip netns exec $ns ip -s link show
		ip netns exec $ns nstat -as | grep MPTcp
	fi
}

chk_add_nr()
{
	local add_nr=$1
	local echo_nr=$2
	local port_nr=${3:-0}
	local ns_invert=${4:-""}
	local syn_nr=$port_nr
	local syn_ack_nr=$port_nr
	local ack_nr=$port_nr
	local mis_syn_nr=0
	local mis_ack_nr=0
	local ns_tx=$ns1
	local ns_rx=$ns2
	local extra_msg=""
	local count
	local timeout

	if [[ $ns_invert = "invert" ]]; then
		ns_tx=$ns2
		ns_rx=$ns1
		extra_msg="invert"
	fi

	timeout=$(ip netns exec ${ns_tx} sysctl -n net.mptcp.add_addr_timeout)

	print_check "add"
	count=$(mptcp_lib_get_counter ${ns_rx} "MPTcpExtAddAddr")
	if [ -z "$count" ]; then
		print_skip
	# if the test configured a short timeout tolerate greater then expected
	# add addrs options, due to retransmissions
	elif [ "$count" != "$add_nr" ] && { [ "$timeout" -gt 1 ] || [ "$count" -lt "$add_nr" ]; }; then
		fail_test "got $count ADD_ADDR[s] expected $add_nr"
	else
		print_ok
	fi

	print_check "echo"
	count=$(mptcp_lib_get_counter ${ns_tx} "MPTcpExtEchoAdd")
	if [ -z "$count" ]; then
		print_skip
	elif [ "$count" != "$echo_nr" ]; then
		fail_test "got $count ADD_ADDR echo[s] expected $echo_nr"
	else
		print_ok
	fi

	if [ $port_nr -gt 0 ]; then
		print_check "pt"
		count=$(mptcp_lib_get_counter ${ns_rx} "MPTcpExtPortAdd")
		if [ -z "$count" ]; then
			print_skip
		elif [ "$count" != "$port_nr" ]; then
			fail_test "got $count ADD_ADDR[s] with a port-number expected $port_nr"
		else
			print_ok
		fi

		print_check "syn"
		count=$(mptcp_lib_get_counter ${ns_tx} "MPTcpExtMPJoinPortSynRx")
		if [ -z "$count" ]; then
			print_skip
		elif [ "$count" != "$syn_nr" ]; then
			fail_test "got $count JOIN[s] syn with a different \
				   port-number expected $syn_nr"
		else
			print_ok
		fi

		print_check "synack"
		count=$(mptcp_lib_get_counter ${ns_rx} "MPTcpExtMPJoinPortSynAckRx")
		if [ -z "$count" ]; then
			print_skip
		elif [ "$count" != "$syn_ack_nr" ]; then
			fail_test "got $count JOIN[s] synack with a different \
				   port-number expected $syn_ack_nr"
		else
			print_ok
		fi

		print_check "ack"
		count=$(mptcp_lib_get_counter ${ns_tx} "MPTcpExtMPJoinPortAckRx")
		if [ -z "$count" ]; then
			print_skip
		elif [ "$count" != "$ack_nr" ]; then
			fail_test "got $count JOIN[s] ack with a different \
				   port-number expected $ack_nr"
		else
			print_ok
		fi

		print_check "syn"
		count=$(mptcp_lib_get_counter ${ns_tx} "MPTcpExtMismatchPortSynRx")
		if [ -z "$count" ]; then
			print_skip
		elif [ "$count" != "$mis_syn_nr" ]; then
			fail_test "got $count JOIN[s] syn with a mismatched \
				   port-number expected $mis_syn_nr"
		else
			print_ok
		fi

		print_check "ack"
		count=$(mptcp_lib_get_counter ${ns_tx} "MPTcpExtMismatchPortAckRx")
		if [ -z "$count" ]; then
			print_skip
		elif [ "$count" != "$mis_ack_nr" ]; then
			fail_test "got $count JOIN[s] ack with a mismatched \
				   port-number expected $mis_ack_nr"
		else
			print_ok
		fi
	fi

	print_info "$extra_msg"
}

chk_add_tx_nr()
{
	local add_tx_nr=$1
	local echo_tx_nr=$2
	local timeout
	local count

	timeout=$(ip netns exec $ns1 sysctl -n net.mptcp.add_addr_timeout)

	print_check "add TX"
	count=$(mptcp_lib_get_counter ${ns1} "MPTcpExtAddAddrTx")
	if [ -z "$count" ]; then
		print_skip
	# if the test configured a short timeout tolerate greater then expected
	# add addrs options, due to retransmissions
	elif [ "$count" != "$add_tx_nr" ] && { [ "$timeout" -gt 1 ] || [ "$count" -lt "$add_tx_nr" ]; }; then
		fail_test "got $count ADD_ADDR[s] TX, expected $add_tx_nr"
	else
		print_ok
	fi

	print_check "echo TX"
	count=$(mptcp_lib_get_counter ${ns2} "MPTcpExtEchoAddTx")
	if [ -z "$count" ]; then
		print_skip
	elif [ "$count" != "$echo_tx_nr" ]; then
		fail_test "got $count ADD_ADDR echo[s] TX, expected $echo_tx_nr"
	else
		print_ok
	fi
}

chk_rm_nr()
{
	local rm_addr_nr=$1
	local rm_subflow_nr=$2
	local invert
	local simult
	local count
	local addr_ns=$ns1
	local subflow_ns=$ns2
	local extra_msg=""

	shift 2
	while [ -n "$1" ]; do
		[ "$1" = "invert" ] && invert=true
		[ "$1" = "simult" ] && simult=true
		shift
	done

	if [ -z $invert ]; then
		addr_ns=$ns1
		subflow_ns=$ns2
	elif [ $invert = "true" ]; then
		addr_ns=$ns2
		subflow_ns=$ns1
		extra_msg="invert"
	fi

	print_check "rm"
	count=$(mptcp_lib_get_counter ${addr_ns} "MPTcpExtRmAddr")
	if [ -z "$count" ]; then
		print_skip
	elif [ "$count" != "$rm_addr_nr" ]; then
		fail_test "got $count RM_ADDR[s] expected $rm_addr_nr"
	else
		print_ok
	fi

	print_check "rmsf"
	count=$(mptcp_lib_get_counter ${subflow_ns} "MPTcpExtRmSubflow")
	if [ -z "$count" ]; then
		print_skip
	elif [ -n "$simult" ]; then
		local cnt suffix

		cnt=$(mptcp_lib_get_counter ${addr_ns} "MPTcpExtRmSubflow")

		# in case of simult flush, the subflow removal count on each side is
		# unreliable
		count=$((count + cnt))
		if [ "$count" != "$rm_subflow_nr" ]; then
			suffix="$count in [$rm_subflow_nr:$((rm_subflow_nr*2))]"
			extra_msg+=" simult"
		fi
		if [ $count -ge "$rm_subflow_nr" ] && \
		   [ "$count" -le "$((rm_subflow_nr *2 ))" ]; then
			print_ok "$suffix"
		else
			fail_test "got $count RM_SUBFLOW[s] expected in range [$rm_subflow_nr:$((rm_subflow_nr*2))]"
		fi
	elif [ "$count" != "$rm_subflow_nr" ]; then
		fail_test "got $count RM_SUBFLOW[s] expected $rm_subflow_nr"
	else
		print_ok
	fi

	print_info "$extra_msg"
}

chk_rm_tx_nr()
{
	local rm_addr_tx_nr=$1

	print_check "rm TX"
	count=$(mptcp_lib_get_counter ${ns2} "MPTcpExtRmAddrTx")
	if [ -z "$count" ]; then
		print_skip
	elif [ "$count" != "$rm_addr_tx_nr" ]; then
		fail_test "got $count RM_ADDR[s] expected $rm_addr_tx_nr"
	else
		print_ok
	fi
}

chk_prio_nr()
{
	local mp_prio_nr_tx=$1
	local mp_prio_nr_rx=$2
	local mpj_syn=$3
	local mpj_syn_ack=$4
	local count

	print_check "ptx"
	count=$(mptcp_lib_get_counter ${ns1} "MPTcpExtMPPrioTx")
	if [ -z "$count" ]; then
		print_skip
	elif [ "$count" != "$mp_prio_nr_tx" ]; then
		fail_test "got $count MP_PRIO[s] TX expected $mp_prio_nr_tx"
	else
		print_ok
	fi

	print_check "prx"
	count=$(mptcp_lib_get_counter ${ns1} "MPTcpExtMPPrioRx")
	if [ -z "$count" ]; then
		print_skip
	elif [ "$count" != "$mp_prio_nr_rx" ]; then
		fail_test "got $count MP_PRIO[s] RX expected $mp_prio_nr_rx"
	else
		print_ok
	fi

	print_check "syn backup"
	count=$(mptcp_lib_get_counter ${ns1} "MPTcpExtMPJoinSynBackupRx")
	if [ -z "$count" ]; then
		print_skip
	elif [ "$count" != "$mpj_syn" ]; then
		fail_test "got $count JOIN[s] syn with Backup expected $mpj_syn"
	else
		print_ok
	fi

	print_check "synack backup"
	count=$(mptcp_lib_get_counter ${ns2} "MPTcpExtMPJoinSynAckBackupRx")
	if [ -z "$count" ]; then
		print_skip
	elif [ "$count" != "$mpj_syn_ack" ]; then
		fail_test "got $count JOIN[s] synack with Backup expected $mpj_syn_ack"
	else
		print_ok
	fi
}

chk_subflow_nr()
{
	local msg="$1"
	local subflow_nr=$2
	local cnt1
	local cnt2
	local dump_stats

	print_check "${msg}"

	cnt1=$(ss -N $ns1 -tOni | grep -c token)
	cnt2=$(ss -N $ns2 -tOni | grep -c token)
	if [ "$cnt1" != "$subflow_nr" ] || [ "$cnt2" != "$subflow_nr" ]; then
		fail_test "got $cnt1:$cnt2 subflows expected $subflow_nr"
		dump_stats=1
	else
		print_ok
	fi

	if [ "${dump_stats}" = 1 ]; then
		ss -N $ns1 -tOni
		ss -N $ns1 -tOni | grep token
		ip -n $ns1 mptcp endpoint
	fi
}

chk_mptcp_info()
{
	local info1=$1
	local exp1=$2
	local info2=$3
	local exp2=$4
	local cnt1
	local cnt2
	local dump_stats

	print_check "mptcp_info ${info1:0:15}=$exp1:$exp2"

	cnt1=$(ss -N $ns1 -inmHM | mptcp_lib_get_info_value "$info1" "$info1")
	cnt2=$(ss -N $ns2 -inmHM | mptcp_lib_get_info_value "$info2" "$info2")
	# 'ss' only display active connections and counters that are not 0.
	[ -z "$cnt1" ] && cnt1=0
	[ -z "$cnt2" ] && cnt2=0

	if [ "$cnt1" != "$exp1" ] || [ "$cnt2" != "$exp2" ]; then
		fail_test "got $cnt1:$cnt2 $info1:$info2 expected $exp1:$exp2"
		dump_stats=1
	else
		print_ok
	fi

	if [ "$dump_stats" = 1 ]; then
		ss -N $ns1 -inmHM
		ss -N $ns2 -inmHM
	fi
}

# $1: subflows in ns1 ; $2: subflows in ns2
# number of all subflows, including the initial subflow.
chk_subflows_total()
{
	local cnt1
	local cnt2
	local info="subflows_total"
	local dump_stats

	# if subflows_total counter is supported, use it:
	if [ -n "$(ss -N $ns1 -inmHM | mptcp_lib_get_info_value $info $info)" ]; then
		chk_mptcp_info $info $1 $info $2
		return
	fi

	print_check "$info $1:$2"

	# if not, count the TCP connections that are in fact MPTCP subflows
	cnt1=$(ss -N $ns1 -ti state established state syn-sent state syn-recv |
	       grep -c tcp-ulp-mptcp)
	cnt2=$(ss -N $ns2 -ti state established state syn-sent state syn-recv |
	       grep -c tcp-ulp-mptcp)

	if [ "$1" != "$cnt1" ] || [ "$2" != "$cnt2" ]; then
		fail_test "got subflows $cnt1:$cnt2 expected $1:$2"
		dump_stats=1
	else
		print_ok
	fi

	if [ "$dump_stats" = 1 ]; then
		ss -N $ns1 -ti
		ss -N $ns2 -ti
	fi
}

chk_link_usage()
{
	local ns=$1
	local link=$2
	local out=$3
	local expected_rate=$4

	local tx_link tx_total
	tx_link=$(ip netns exec $ns cat /sys/class/net/$link/statistics/tx_bytes)
	tx_total=$(stat --format=%s $out)
	local tx_rate=$((tx_link * 100 / tx_total))
	local tolerance=5

	print_check "link usage"
	if [ $tx_rate -lt $((expected_rate - tolerance)) ] || \
	   [ $tx_rate -gt $((expected_rate + tolerance)) ]; then
		fail_test "got $tx_rate% usage, expected $expected_rate%"
	else
		print_ok
	fi
}

wait_attempt_fail()
{
	local timeout_ms=$((timeout_poll * 1000))
	local time=0
	local ns=$1

	while [ $time -lt $timeout_ms ]; do
		local cnt

		cnt=$(mptcp_lib_get_counter ${ns} "TcpAttemptFails")

		[ "$cnt" = 1 ] && return 1
		time=$((time + 100))
		sleep 0.1
	done
	return 1
}

set_userspace_pm()
{
	local ns=$1

	ip netns exec $ns sysctl -q net.mptcp.pm_type=1
}

subflows_tests()
{
	if reset "no JOIN"; then
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
	fi

	# subflow limited by client
	if reset "single subflow, limited by client"; then
		pm_nl_set_limits $ns1 0 0
		pm_nl_set_limits $ns2 0 0
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
	fi

	# subflow limited by server
	if reset "single subflow, limited by server"; then
		pm_nl_set_limits $ns1 0 0
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 0
	fi

	# subflow
	if reset "single subflow"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
	fi

	# multiple subflows
	if reset "multiple subflows"; then
		pm_nl_set_limits $ns1 0 2
		pm_nl_set_limits $ns2 0 2
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		pm_nl_add_endpoint $ns2 10.0.2.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
	fi

	# multiple subflows limited by server
	if reset "multiple subflows, limited by server"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 2
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		pm_nl_add_endpoint $ns2 10.0.2.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 1
	fi

	# single subflow, dev
	if reset "single subflow, dev"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow dev ns2eth3
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
	fi
}

subflows_error_tests()
{
	# If a single subflow is configured, and matches the MPC src
	# address, no additional subflow should be created
	if reset "no MPC reuse with single endpoint"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 10.0.1.2 flags subflow
		speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
	fi

	# multiple subflows, with subflow creation error
	if reset_with_tcp_filter "multi subflows, with failing subflow" ns1 10.0.3.2 REJECT &&
	   continue_if mptcp_lib_kallsyms_has "mptcp_pm_subflow_check_next$"; then
		pm_nl_set_limits $ns1 0 2
		pm_nl_set_limits $ns2 0 2
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		pm_nl_add_endpoint $ns2 10.0.2.2 flags subflow
		speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
	fi

	# multiple subflows, with subflow timeout on MPJ
	if reset_with_tcp_filter "multi subflows, with subflow timeout" ns1 10.0.3.2 DROP &&
	   continue_if mptcp_lib_kallsyms_has "mptcp_pm_subflow_check_next$"; then
		pm_nl_set_limits $ns1 0 2
		pm_nl_set_limits $ns2 0 2
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		pm_nl_add_endpoint $ns2 10.0.2.2 flags subflow
		speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
	fi

	# multiple subflows, check that the endpoint corresponding to
	# closed subflow (due to reset) is not reused if additional
	# subflows are added later
	if reset_with_tcp_filter "multi subflows, fair usage on close" ns1 10.0.3.2 REJECT &&
	   continue_if mptcp_lib_kallsyms_has "mptcp_pm_subflow_check_next$"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		speed=slow \
			run_tests $ns1 $ns2 10.0.1.1 &

		# mpj subflow will be in TW after the reset
		wait_attempt_fail $ns2
		pm_nl_add_endpoint $ns2 10.0.2.2 flags subflow
		wait

		# additional subflow could be created only if the PM select
		# the later endpoint, skipping the already used one
		chk_join_nr 1 1 1
	fi
}

signal_address_tests()
{
	# add_address, unused
	if reset "unused signal address"; then
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
		chk_add_tx_nr 1 1
		chk_add_nr 1 1
	fi

	# accept and use add_addr
	if reset "signal address"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 1 1
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 1 1
	fi

	# accept and use add_addr with an additional subflow
	# note: signal address in server ns and local addresses in client ns must
	# belong to different subnets or one of the listed local address could be
	# used for 'add_addr' subflow
	if reset "subflow and signal"; then
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		pm_nl_set_limits $ns1 0 2
		pm_nl_set_limits $ns2 1 2
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
		chk_add_nr 1 1
	fi

	# uncommon: subflow and signal flags on the same endpoint
	# or because the user wrongly picked both, but still expects the client
	# to create additional subflows
	if reset "subflow and signal together"; then
		pm_nl_set_limits $ns1 0 2
		pm_nl_set_limits $ns2 0 2
		pm_nl_add_endpoint $ns2 10.0.3.2 flags signal,subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 1 1 0 invert  # only initiated by ns2
		chk_add_nr 0 0 0         # none initiated by ns1
		chk_rst_nr 0 0 invert    # no RST sent by the client
		chk_rst_nr 0 0           # no RST sent by the server
	fi

	# accept and use add_addr with additional subflows
	if reset "multiple subflows and signal"; then
		pm_nl_set_limits $ns1 0 3
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		pm_nl_set_limits $ns2 1 3
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		pm_nl_add_endpoint $ns2 10.0.4.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 3 3 3
		chk_add_nr 1 1
	fi

	# signal addresses
	if reset "signal addresses"; then
		pm_nl_set_limits $ns1 3 3
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		pm_nl_add_endpoint $ns1 10.0.3.1 flags signal
		pm_nl_add_endpoint $ns1 10.0.4.1 flags signal
		pm_nl_set_limits $ns2 3 3
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 3 3 3
		chk_add_nr 3 3
	fi

	# signal invalid addresses
	if reset "signal invalid addresses"; then
		pm_nl_set_limits $ns1 3 3
		pm_nl_add_endpoint $ns1 10.0.12.1 flags signal
		pm_nl_add_endpoint $ns1 10.0.3.1 flags signal
		pm_nl_add_endpoint $ns1 10.0.14.1 flags signal
		pm_nl_set_limits $ns2 3 3
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 3 3
	fi

	# signal addresses race test
	if reset "signal addresses race test"; then
		pm_nl_set_limits $ns1 4 4
		pm_nl_set_limits $ns2 4 4
		pm_nl_add_endpoint $ns1 10.0.1.1 flags signal
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		pm_nl_add_endpoint $ns1 10.0.3.1 flags signal
		pm_nl_add_endpoint $ns1 10.0.4.1 flags signal
		pm_nl_add_endpoint $ns2 10.0.1.2 flags signal
		pm_nl_add_endpoint $ns2 10.0.2.2 flags signal
		pm_nl_add_endpoint $ns2 10.0.3.2 flags signal
		pm_nl_add_endpoint $ns2 10.0.4.2 flags signal

		# the peer could possibly miss some addr notification, allow retransmission
		ip netns exec $ns1 sysctl -q net.mptcp.add_addr_timeout=1
		speed=slow \
			run_tests $ns1 $ns2 10.0.1.1

		# It is not directly linked to the commit introducing this
		# symbol but for the parent one which is linked anyway.
		if ! mptcp_lib_kallsyms_has "mptcp_pm_subflow_check_next$"; then
			chk_join_nr 3 3 2
			chk_add_nr 4 4
		else
			chk_join_nr 3 3 3
			# the server will not signal the address terminating
			# the MPC subflow
			chk_add_nr 3 3
		fi
	fi
}

link_failure_tests()
{
	# accept and use add_addr with additional subflows and link loss
	if reset "multiple flows, signal, link failure"; then
		# without any b/w limit each veth could spool the packets and get
		# them acked at xmit time, so that the corresponding subflow will
		# have almost always no outstanding pkts, the scheduler will pick
		# always the first subflow and we will have hard time testing
		# active backup and link switch-over.
		# Let's set some arbitrary (low) virtual link limits.
		init_shapers
		pm_nl_set_limits $ns1 0 3
		pm_nl_add_endpoint $ns1 10.0.2.1 dev ns1eth2 flags signal
		pm_nl_set_limits $ns2 1 3
		pm_nl_add_endpoint $ns2 10.0.3.2 dev ns2eth3 flags subflow
		pm_nl_add_endpoint $ns2 10.0.4.2 dev ns2eth4 flags subflow
		test_linkfail=1 \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 3 3 3
		chk_add_nr 1 1
		chk_stale_nr $ns2 1 5 1
	fi

	# accept and use add_addr with additional subflows and link loss
	# for bidirectional transfer
	if reset "multi flows, signal, bidi, link fail"; then
		init_shapers
		pm_nl_set_limits $ns1 0 3
		pm_nl_add_endpoint $ns1 10.0.2.1 dev ns1eth2 flags signal
		pm_nl_set_limits $ns2 1 3
		pm_nl_add_endpoint $ns2 10.0.3.2 dev ns2eth3 flags subflow
		pm_nl_add_endpoint $ns2 10.0.4.2 dev ns2eth4 flags subflow
		test_linkfail=2 \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 3 3 3
		chk_add_nr 1 1
		chk_stale_nr $ns2 1 -1 1
	fi

	# 2 subflows plus 1 backup subflow with a lossy link, backup
	# will never be used
	if reset "backup subflow unused, link failure"; then
		init_shapers
		pm_nl_set_limits $ns1 0 2
		pm_nl_add_endpoint $ns1 10.0.2.1 dev ns1eth2 flags signal
		pm_nl_set_limits $ns2 1 2
		pm_nl_add_endpoint $ns2 10.0.3.2 dev ns2eth3 flags subflow,backup
		FAILING_LINKS="1" test_linkfail=1 \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
		chk_add_nr 1 1
		chk_link_usage $ns2 ns2eth3 $cinsent 0
	fi

	# 2 lossy links after half transfer, backup will get half of
	# the traffic
	if reset "backup flow used, multi links fail"; then
		init_shapers
		pm_nl_set_limits $ns1 0 2
		pm_nl_add_endpoint $ns1 10.0.2.1 dev ns1eth2 flags signal
		pm_nl_set_limits $ns2 1 2
		pm_nl_add_endpoint $ns2 10.0.3.2 dev ns2eth3 flags subflow,backup
		FAILING_LINKS="1 2" test_linkfail=1 \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
		chk_add_nr 1 1
		chk_stale_nr $ns2 2 4 2
		chk_link_usage $ns2 ns2eth3 $cinsent 50
	fi

	# use a backup subflow with the first subflow on a lossy link
	# for bidirectional transfer
	if reset "backup flow used, bidi, link failure"; then
		init_shapers
		pm_nl_set_limits $ns1 0 2
		pm_nl_add_endpoint $ns1 10.0.2.1 dev ns1eth2 flags signal
		pm_nl_set_limits $ns2 1 3
		pm_nl_add_endpoint $ns2 10.0.3.2 dev ns2eth3 flags subflow,backup
		FAILING_LINKS="1 2" test_linkfail=2 \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
		chk_add_nr 1 1
		chk_stale_nr $ns2 1 -1 2
		chk_link_usage $ns2 ns2eth3 $cinsent 50
	fi
}

add_addr_timeout_tests()
{
	# add_addr timeout
	if reset_with_add_addr_timeout "signal address, ADD_ADDR timeout"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 1 1
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_tx_nr 4 4
		chk_add_nr 4 0
	fi

	# add_addr timeout IPv6
	if reset_with_add_addr_timeout "signal address, ADD_ADDR6 timeout" 6; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 1 1
		pm_nl_add_endpoint $ns1 dead:beef:2::1 flags signal
		speed=slow \
			run_tests $ns1 $ns2 dead:beef:1::1
		chk_join_nr 1 1 1
		chk_add_nr 4 0
	fi

	# signal addresses timeout
	if reset_with_add_addr_timeout "signal addresses, ADD_ADDR timeout"; then
		pm_nl_set_limits $ns1 2 2
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		pm_nl_add_endpoint $ns1 10.0.3.1 flags signal
		pm_nl_set_limits $ns2 2 2
		speed=10 \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
		chk_add_nr 8 0
	fi

	# signal invalid addresses timeout
	if reset_with_add_addr_timeout "invalid address, ADD_ADDR timeout"; then
		pm_nl_set_limits $ns1 2 2
		pm_nl_add_endpoint $ns1 10.0.12.1 flags signal
		pm_nl_add_endpoint $ns1 10.0.3.1 flags signal
		pm_nl_set_limits $ns2 2 2
		speed=10 \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 8 0
	fi
}

remove_tests()
{
	# single subflow, remove
	if reset "remove single subflow"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		addr_nr_ns2=-1 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_rm_tx_nr 1
		chk_rm_nr 1 1
		chk_rst_nr 0 0
	fi

	# multiple subflows, remove
	if reset "remove multiple subflows"; then
		pm_nl_set_limits $ns1 0 2
		pm_nl_set_limits $ns2 0 2
		pm_nl_add_endpoint $ns2 10.0.2.2 flags subflow
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		addr_nr_ns2=-2 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
		chk_rm_nr 2 2
		chk_rst_nr 0 0
	fi

	# single address, remove
	if reset "remove single address"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		pm_nl_set_limits $ns2 1 1
		addr_nr_ns1=-1 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 1 1
		chk_rm_nr 1 1 invert
		chk_rst_nr 0 0
	fi

	# subflow and signal, remove
	if reset "remove subflow and signal"; then
		pm_nl_set_limits $ns1 0 2
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		pm_nl_set_limits $ns2 1 2
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		addr_nr_ns1=-1 addr_nr_ns2=-1 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
		chk_add_nr 1 1
		chk_rm_nr 1 1
		chk_rst_nr 0 0
	fi

	# subflows and signal, remove
	if reset "remove subflows and signal"; then
		pm_nl_set_limits $ns1 0 3
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		pm_nl_set_limits $ns2 1 3
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		pm_nl_add_endpoint $ns2 10.0.4.2 flags subflow
		addr_nr_ns1=-1 addr_nr_ns2=-2 speed=10 \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 3 3 3
		chk_add_nr 1 1
		chk_rm_nr 2 2
		chk_rst_nr 0 0
	fi

	# addresses remove
	if reset "remove addresses"; then
		pm_nl_set_limits $ns1 3 3
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal id 250
		pm_nl_add_endpoint $ns1 10.0.3.1 flags signal
		pm_nl_add_endpoint $ns1 10.0.4.1 flags signal
		pm_nl_set_limits $ns2 3 3
		addr_nr_ns1=-3 speed=10 \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 3 3 3
		chk_add_nr 3 3
		chk_rm_nr 3 3 invert
		chk_rst_nr 0 0
	fi

	# invalid addresses remove
	if reset "remove invalid addresses"; then
		pm_nl_set_limits $ns1 3 3
		pm_nl_add_endpoint $ns1 10.0.12.1 flags signal
		# broadcast IP: no packet for this address will be received on ns1
		pm_nl_add_endpoint $ns1 224.0.0.1 flags signal
		pm_nl_add_endpoint $ns1 10.0.3.1 flags signal
		pm_nl_set_limits $ns2 2 2
		addr_nr_ns1=-3 speed=10 \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 3 3
		chk_rm_nr 3 1 invert
		chk_rst_nr 0 0
	fi

	# subflows and signal, flush
	if reset "flush subflows and signal"; then
		pm_nl_set_limits $ns1 0 3
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		pm_nl_set_limits $ns2 1 3
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		pm_nl_add_endpoint $ns2 10.0.4.2 flags subflow
		addr_nr_ns1=-8 addr_nr_ns2=-8 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 3 3 3
		chk_add_nr 1 1
		chk_rm_nr 1 3 invert simult
		chk_rst_nr 0 0
	fi

	# subflows flush
	if reset "flush subflows"; then
		pm_nl_set_limits $ns1 3 3
		pm_nl_set_limits $ns2 3 3
		pm_nl_add_endpoint $ns2 10.0.2.2 flags subflow id 150
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		pm_nl_add_endpoint $ns2 10.0.4.2 flags subflow
		addr_nr_ns1=-8 addr_nr_ns2=-8 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 3 3 3

		if mptcp_lib_kversion_ge 5.18; then
			chk_rm_tx_nr 0
			chk_rm_nr 0 3 simult
		else
			chk_rm_nr 3 3
		fi
		chk_rst_nr 0 0
	fi

	# addresses flush
	if reset "flush addresses"; then
		pm_nl_set_limits $ns1 3 3
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal id 250
		pm_nl_add_endpoint $ns1 10.0.3.1 flags signal
		pm_nl_add_endpoint $ns1 10.0.4.1 flags signal
		pm_nl_set_limits $ns2 3 3
		addr_nr_ns1=-8 addr_nr_ns2=-8 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 3 3 3
		chk_add_nr 3 3
		chk_rm_nr 3 3 invert simult
		chk_rst_nr 0 0
	fi

	# invalid addresses flush
	if reset "flush invalid addresses"; then
		pm_nl_set_limits $ns1 3 3
		pm_nl_add_endpoint $ns1 10.0.12.1 flags signal
		pm_nl_add_endpoint $ns1 10.0.3.1 flags signal
		pm_nl_add_endpoint $ns1 10.0.14.1 flags signal
		pm_nl_set_limits $ns2 3 3
		addr_nr_ns1=-8 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 3 3
		chk_rm_nr 3 1 invert
		chk_rst_nr 0 0
	fi

	# remove id 0 subflow
	if reset "remove id 0 subflow"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		addr_nr_ns2=-9 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_rm_nr 1 1
		chk_rst_nr 0 0
	fi

	# remove id 0 address
	if reset "remove id 0 address"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		pm_nl_set_limits $ns2 1 1
		addr_nr_ns1=-9 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 1 1
		chk_rm_nr 1 1 invert
		chk_rst_nr 0 0 invert
	fi
}

add_tests()
{
	# add single subflow
	if reset "add single subflow"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		addr_nr_ns2=1 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
	fi

	# add signal address
	if reset "add signal address"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 1 1
		addr_nr_ns1=1 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 1 1
	fi

	# add multiple subflows
	if reset "add multiple subflows"; then
		pm_nl_set_limits $ns1 0 2
		pm_nl_set_limits $ns2 0 2
		addr_nr_ns2=2 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
	fi

	# add multiple subflows IPv6
	if reset "add multiple subflows IPv6"; then
		pm_nl_set_limits $ns1 0 2
		pm_nl_set_limits $ns2 0 2
		addr_nr_ns2=2 speed=slow \
			run_tests $ns1 $ns2 dead:beef:1::1
		chk_join_nr 2 2 2
	fi

	# add multiple addresses IPv6
	if reset "add multiple addresses IPv6"; then
		pm_nl_set_limits $ns1 0 2
		pm_nl_set_limits $ns2 2 2
		addr_nr_ns1=2 speed=slow \
			run_tests $ns1 $ns2 dead:beef:1::1
		chk_join_nr 2 2 2
		chk_add_nr 2 2
	fi
}

ipv6_tests()
{
	# subflow IPv6
	if reset "single subflow IPv6"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 dead:beef:3::2 dev ns2eth3 flags subflow
		speed=slow \
			run_tests $ns1 $ns2 dead:beef:1::1
		chk_join_nr 1 1 1
	fi

	# add_address, unused IPv6
	if reset "unused signal address IPv6"; then
		pm_nl_add_endpoint $ns1 dead:beef:2::1 flags signal
		speed=slow \
			run_tests $ns1 $ns2 dead:beef:1::1
		chk_join_nr 0 0 0
		chk_add_nr 1 1
	fi

	# signal address IPv6
	if reset "single address IPv6"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_add_endpoint $ns1 dead:beef:2::1 flags signal
		pm_nl_set_limits $ns2 1 1
		speed=slow \
			run_tests $ns1 $ns2 dead:beef:1::1
		chk_join_nr 1 1 1
		chk_add_nr 1 1
	fi

	# single address IPv6, remove
	if reset "remove single address IPv6"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_add_endpoint $ns1 dead:beef:2::1 flags signal
		pm_nl_set_limits $ns2 1 1
		addr_nr_ns1=-1 speed=slow \
			run_tests $ns1 $ns2 dead:beef:1::1
		chk_join_nr 1 1 1
		chk_add_nr 1 1
		chk_rm_nr 1 1 invert
	fi

	# subflow and signal IPv6, remove
	if reset "remove subflow and signal IPv6"; then
		pm_nl_set_limits $ns1 0 2
		pm_nl_add_endpoint $ns1 dead:beef:2::1 flags signal
		pm_nl_set_limits $ns2 1 2
		pm_nl_add_endpoint $ns2 dead:beef:3::2 dev ns2eth3 flags subflow
		addr_nr_ns1=-1 addr_nr_ns2=-1 speed=slow \
			run_tests $ns1 $ns2 dead:beef:1::1
		chk_join_nr 2 2 2
		chk_add_nr 1 1
		chk_rm_nr 1 1
	fi
}

v4mapped_tests()
{
	# subflow IPv4-mapped to IPv4-mapped
	if reset "single subflow IPv4-mapped"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 "::ffff:10.0.3.2" flags subflow
		run_tests $ns1 $ns2 "::ffff:10.0.1.1"
		chk_join_nr 1 1 1
	fi

	# signal address IPv4-mapped with IPv4-mapped sk
	if reset "signal address IPv4-mapped"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 1 1
		pm_nl_add_endpoint $ns1 "::ffff:10.0.2.1" flags signal
		run_tests $ns1 $ns2 "::ffff:10.0.1.1"
		chk_join_nr 1 1 1
		chk_add_nr 1 1
	fi

	# subflow v4-map-v6
	if reset "single subflow v4-map-v6"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		run_tests $ns1 $ns2 "::ffff:10.0.1.1"
		chk_join_nr 1 1 1
	fi

	# signal address v4-map-v6
	if reset "signal address v4-map-v6"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 1 1
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		run_tests $ns1 $ns2 "::ffff:10.0.1.1"
		chk_join_nr 1 1 1
		chk_add_nr 1 1
	fi

	# subflow v6-map-v4
	if reset "single subflow v6-map-v4"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 "::ffff:10.0.3.2" flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
	fi

	# signal address v6-map-v4
	if reset "signal address v6-map-v4"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 1 1
		pm_nl_add_endpoint $ns1 "::ffff:10.0.2.1" flags signal
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 1 1
	fi

	# no subflow IPv6 to v4 address
	if reset "no JOIN with diff families v4-v6"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 dead:beef:2::2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
	fi

	# no subflow IPv6 to v4 address even if v6 has a valid v4 at the end
	if reset "no JOIN with diff families v4-v6-2"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 dead:beef:2::10.0.3.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
	fi

	# no subflow IPv4 to v6 address, no need to slow down too then
	if reset "no JOIN with diff families v6-v4"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		run_tests $ns1 $ns2 dead:beef:1::1
		chk_join_nr 0 0 0
	fi
}

mixed_tests()
{
	if reset "IPv4 sockets do not use IPv6 addresses" &&
	   continue_if mptcp_lib_kversion_ge 6.3; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 1 1
		pm_nl_add_endpoint $ns1 dead:beef:2::1 flags signal
		speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
	fi

	# Need an IPv6 mptcp socket to allow subflows of both families
	if reset "simult IPv4 and IPv6 subflows" &&
	   continue_if mptcp_lib_kversion_ge 6.3; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 1 1
		pm_nl_add_endpoint $ns1 10.0.1.1 flags signal
		speed=slow \
			run_tests $ns1 $ns2 dead:beef:2::1
		chk_join_nr 1 1 1
	fi

	# cross families subflows will not be created even in fullmesh mode
	if reset "simult IPv4 and IPv6 subflows, fullmesh 1x1" &&
	   continue_if mptcp_lib_kversion_ge 6.3; then
		pm_nl_set_limits $ns1 0 4
		pm_nl_set_limits $ns2 1 4
		pm_nl_add_endpoint $ns2 dead:beef:2::2 flags subflow,fullmesh
		pm_nl_add_endpoint $ns1 10.0.1.1 flags signal
		speed=slow \
			run_tests $ns1 $ns2 dead:beef:2::1
		chk_join_nr 1 1 1
	fi

	# fullmesh still tries to create all the possibly subflows with
	# matching family
	if reset "simult IPv4 and IPv6 subflows, fullmesh 2x2" &&
	   continue_if mptcp_lib_kversion_ge 6.3; then
		pm_nl_set_limits $ns1 0 4
		pm_nl_set_limits $ns2 2 4
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		pm_nl_add_endpoint $ns1 dead:beef:2::1 flags signal
		fullmesh=1 speed=slow \
			run_tests $ns1 $ns2 dead:beef:1::1
		chk_join_nr 4 4 4
	fi
}

backup_tests()
{
	# single subflow, backup
	if reset "single subflow, backup" &&
	   continue_if mptcp_lib_kallsyms_has "subflow_rebuild_header$"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow,backup
		sflags=nobackup speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_prio_nr 0 1 1 0
	fi

	# single address, backup
	if reset "single address, backup" &&
	   continue_if mptcp_lib_kallsyms_has "subflow_rebuild_header$"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal,backup
		pm_nl_set_limits $ns2 1 1
		sflags=nobackup speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 1 1
		chk_prio_nr 1 0 0 1
	fi

	# single address, switch to backup
	if reset "single address, switch to backup" &&
	   continue_if mptcp_lib_kallsyms_has "subflow_rebuild_header$"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		pm_nl_set_limits $ns2 1 1
		sflags=backup speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 1 1
		chk_prio_nr 1 1 0 0
	fi

	# single address with port, backup
	if reset "single address with port, backup" &&
	   continue_if mptcp_lib_kallsyms_has "subflow_rebuild_header$"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal,backup port 10100
		pm_nl_set_limits $ns2 1 1
		sflags=nobackup speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 1 1
		chk_prio_nr 1 0 0 1
	fi

	if reset "mpc backup" &&
	   continue_if mptcp_lib_kallsyms_doesnt_have "T mptcp_subflow_send_ack$"; then
		pm_nl_add_endpoint $ns2 10.0.1.2 flags subflow,backup
		speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
		chk_prio_nr 0 1 0 0
	fi

	if reset "mpc backup both sides" &&
	   continue_if mptcp_lib_kallsyms_doesnt_have "T mptcp_subflow_send_ack$"; then
		pm_nl_set_limits $ns1 0 2
		pm_nl_set_limits $ns2 1 2
		pm_nl_add_endpoint $ns1 10.0.1.1 flags signal,backup
		pm_nl_add_endpoint $ns2 10.0.1.2 flags subflow,backup

		# 10.0.2.2 (non-backup) -> 10.0.1.1 (backup)
		pm_nl_add_endpoint $ns2 10.0.2.2 flags subflow
		# 10.0.1.2 (backup) -> 10.0.2.1 (non-backup)
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		ip -net "$ns2" route add 10.0.2.1 via 10.0.1.1 dev ns2eth1 # force this path

		speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
		chk_prio_nr 1 1 1 1
	fi

	if reset "mpc switch to backup" &&
	   continue_if mptcp_lib_kallsyms_doesnt_have "T mptcp_subflow_send_ack$"; then
		pm_nl_add_endpoint $ns2 10.0.1.2 flags subflow
		sflags=backup speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
		chk_prio_nr 0 1 0 0
	fi

	if reset "mpc switch to backup both sides" &&
	   continue_if mptcp_lib_kallsyms_doesnt_have "T mptcp_subflow_send_ack$"; then
		pm_nl_add_endpoint $ns1 10.0.1.1 flags subflow
		pm_nl_add_endpoint $ns2 10.0.1.2 flags subflow
		sflags=backup speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
		chk_prio_nr 1 1 0 0
	fi
}

verify_listener_events()
{
	local evt=$1
	local e_type=$2
	local e_family=$3
	local e_saddr=$4
	local e_sport=$5
	local type
	local family
	local saddr
	local sport
	local name

	if [ $e_type = $MPTCP_LIB_EVENT_LISTENER_CREATED ]; then
		name="LISTENER_CREATED"
	elif [ $e_type = $MPTCP_LIB_EVENT_LISTENER_CLOSED ]; then
		name="LISTENER_CLOSED "
	else
		name="$e_type"
	fi

	print_check "$name $e_saddr:$e_sport"

	if ! mptcp_lib_kallsyms_has "mptcp_event_pm_listener$"; then
		print_skip "event not supported"
		return
	fi

	type=$(mptcp_lib_evts_get_info type "$evt" "$e_type")
	family=$(mptcp_lib_evts_get_info family "$evt" "$e_type")
	sport=$(mptcp_lib_evts_get_info sport "$evt" "$e_type")
	if [ $family ] && [ $family = $AF_INET6 ]; then
		saddr=$(mptcp_lib_evts_get_info saddr6 "$evt" "$e_type")
	else
		saddr=$(mptcp_lib_evts_get_info saddr4 "$evt" "$e_type")
	fi

	if [ $type ] && [ $type = $e_type ] &&
	   [ $family ] && [ $family = $e_family ] &&
	   [ $saddr ] && [ $saddr = $e_saddr ] &&
	   [ $sport ] && [ $sport = $e_sport ]; then
		print_ok
		return 0
	fi
	fail_test "$e_type:$type $e_family:$family $e_saddr:$saddr $e_sport:$sport"
}

add_addr_ports_tests()
{
	# signal address with port
	if reset "signal address with port"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 1 1
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal port 10100
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 1 1 1
	fi

	# subflow and signal with port
	if reset "subflow and signal with port"; then
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal port 10100
		pm_nl_set_limits $ns1 0 2
		pm_nl_set_limits $ns2 1 2
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
		chk_add_nr 1 1 1
	fi

	# single address with port, remove
	# pm listener events
	if reset_with_events "remove single address with port"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal port 10100
		pm_nl_set_limits $ns2 1 1
		addr_nr_ns1=-1 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 1 1 1
		chk_rm_nr 1 1 invert

		verify_listener_events $evts_ns1 $MPTCP_LIB_EVENT_LISTENER_CREATED \
				       $MPTCP_LIB_AF_INET 10.0.2.1 10100
		verify_listener_events $evts_ns1 $MPTCP_LIB_EVENT_LISTENER_CLOSED \
				       $MPTCP_LIB_AF_INET 10.0.2.1 10100
		kill_events_pids
	fi

	# subflow and signal with port, remove
	if reset "remove subflow and signal with port"; then
		pm_nl_set_limits $ns1 0 2
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal port 10100
		pm_nl_set_limits $ns2 1 2
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		addr_nr_ns1=-1 addr_nr_ns2=-1 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
		chk_add_nr 1 1 1
		chk_rm_nr 1 1
	fi

	# subflows and signal with port, flush
	if reset "flush subflows and signal with port"; then
		pm_nl_set_limits $ns1 0 3
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal port 10100
		pm_nl_set_limits $ns2 1 3
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		pm_nl_add_endpoint $ns2 10.0.4.2 flags subflow
		addr_nr_ns1=-8 addr_nr_ns2=-2 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 3 3 3
		chk_add_nr 1 1
		chk_rm_nr 1 3 invert simult
	fi

	# multiple addresses with port
	if reset "multiple addresses with port"; then
		pm_nl_set_limits $ns1 2 2
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal port 10100
		pm_nl_add_endpoint $ns1 10.0.3.1 flags signal port 10100
		pm_nl_set_limits $ns2 2 2
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
		chk_add_nr 2 2 2
	fi

	# multiple addresses with ports
	if reset "multiple addresses with ports"; then
		pm_nl_set_limits $ns1 2 2
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal port 10100
		pm_nl_add_endpoint $ns1 10.0.3.1 flags signal port 10101
		pm_nl_set_limits $ns2 2 2
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
		chk_add_nr 2 2 2
	fi
}

syncookies_tests()
{
	# single subflow, syncookies
	if reset_with_cookies "single subflow with syn cookies"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
	fi

	# multiple subflows with syn cookies
	if reset_with_cookies "multiple subflows with syn cookies"; then
		pm_nl_set_limits $ns1 0 2
		pm_nl_set_limits $ns2 0 2
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		pm_nl_add_endpoint $ns2 10.0.2.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
	fi

	# multiple subflows limited by server
	if reset_with_cookies "subflows limited by server w cookies"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 2
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		pm_nl_add_endpoint $ns2 10.0.2.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 1 1
	fi

	# test signal address with cookies
	if reset_with_cookies "signal address with syn cookies"; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 1 1
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 1 1
	fi

	# test cookie with subflow and signal
	if reset_with_cookies "subflow and signal w cookies"; then
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		pm_nl_set_limits $ns1 0 2
		pm_nl_set_limits $ns2 1 2
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
		chk_add_nr 1 1
	fi

	# accept and use add_addr with additional subflows
	if reset_with_cookies "subflows and signal w. cookies"; then
		pm_nl_set_limits $ns1 0 3
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		pm_nl_set_limits $ns2 1 3
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		pm_nl_add_endpoint $ns2 10.0.4.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 3 3 3
		chk_add_nr 1 1
	fi
}

checksum_tests()
{
	# checksum test 0 0
	if reset_with_checksum 0 0; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
	fi

	# checksum test 1 1
	if reset_with_checksum 1 1; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
	fi

	# checksum test 0 1
	if reset_with_checksum 0 1; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
	fi

	# checksum test 1 0
	if reset_with_checksum 1 0; then
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
	fi
}

deny_join_id0_tests()
{
	# subflow allow join id0 ns1
	if reset_with_allow_join_id0 "single subflow allow join id0 ns1" 1 0; then
		pm_nl_set_limits $ns1 1 1
		pm_nl_set_limits $ns2 1 1
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
	fi

	# subflow allow join id0 ns2
	if reset_with_allow_join_id0 "single subflow allow join id0 ns2" 0 1; then
		pm_nl_set_limits $ns1 1 1
		pm_nl_set_limits $ns2 1 1
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
	fi

	# signal address allow join id0 ns1
	# ADD_ADDRs are not affected by allow_join_id0 value.
	if reset_with_allow_join_id0 "signal address allow join id0 ns1" 1 0; then
		pm_nl_set_limits $ns1 1 1
		pm_nl_set_limits $ns2 1 1
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 1 1
	fi

	# signal address allow join id0 ns2
	# ADD_ADDRs are not affected by allow_join_id0 value.
	if reset_with_allow_join_id0 "signal address allow join id0 ns2" 0 1; then
		pm_nl_set_limits $ns1 1 1
		pm_nl_set_limits $ns2 1 1
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
		chk_add_nr 1 1
	fi

	# subflow and address allow join id0 ns1
	if reset_with_allow_join_id0 "subflow and address allow join id0 1" 1 0; then
		pm_nl_set_limits $ns1 2 2
		pm_nl_set_limits $ns2 2 2
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
	fi

	# subflow and address allow join id0 ns2
	if reset_with_allow_join_id0 "subflow and address allow join id0 2" 0 1; then
		pm_nl_set_limits $ns1 2 2
		pm_nl_set_limits $ns2 2 2
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1
	fi
}

fullmesh_tests()
{
	# fullmesh 1
	# 2 fullmesh addrs in ns2, added before the connection,
	# 1 non-fullmesh addr in ns1, added during the connection.
	if reset "fullmesh test 2x1"; then
		pm_nl_set_limits $ns1 0 4
		pm_nl_set_limits $ns2 1 4
		pm_nl_add_endpoint $ns2 10.0.2.2 flags subflow,fullmesh
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow,fullmesh
		addr_nr_ns1=1 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 4 4 4
		chk_add_nr 1 1
	fi

	# fullmesh 2
	# 1 non-fullmesh addr in ns1, added before the connection,
	# 1 fullmesh addr in ns2, added during the connection.
	if reset "fullmesh test 1x1"; then
		pm_nl_set_limits $ns1 1 3
		pm_nl_set_limits $ns2 1 3
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		if mptcp_lib_kallsyms_has "mptcp_pm_subflow_check_next$"; then
			pm_nl_add_endpoint $ns2 10.0.1.2 flags subflow,fullmesh
		fi
		fullmesh=1 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 3 3 3
		chk_add_nr 1 1
	fi

	# fullmesh 3
	# 1 non-fullmesh addr in ns1, added before the connection,
	# 2 fullmesh addrs in ns2, added during the connection.
	if reset "fullmesh test 1x2"; then
		pm_nl_set_limits $ns1 2 5
		pm_nl_set_limits $ns2 1 5
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		fullmesh=2 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 5 5 5
		chk_add_nr 1 1
	fi

	# fullmesh 4
	# 1 non-fullmesh addr in ns1, added before the connection,
	# 2 fullmesh addrs in ns2, added during the connection,
	# limit max_subflows to 4.
	if reset "fullmesh test 1x2, limited"; then
		pm_nl_set_limits $ns1 2 4
		pm_nl_set_limits $ns2 1 4
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		fullmesh=2 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 4 4 4
		chk_add_nr 1 1
	fi

	# set fullmesh flag
	if reset "set fullmesh flag test" &&
	   continue_if mptcp_lib_kversion_ge 5.18; then
		pm_nl_set_limits $ns1 4 4
		pm_nl_add_endpoint $ns1 10.0.2.1 flags subflow
		pm_nl_set_limits $ns2 4 4
		addr_nr_ns2=1 sflags=fullmesh speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
		chk_rm_nr 0 1
	fi

	# set nofullmesh flag
	if reset "set nofullmesh flag test" &&
	   continue_if mptcp_lib_kversion_ge 5.18; then
		pm_nl_set_limits $ns1 4 4
		pm_nl_add_endpoint $ns1 10.0.2.1 flags subflow,fullmesh
		pm_nl_set_limits $ns2 4 4
		fullmesh=1 sflags=nofullmesh speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
		chk_rm_nr 0 1
	fi

	# set backup,fullmesh flags
	if reset "set backup,fullmesh flags test" &&
	   continue_if mptcp_lib_kversion_ge 5.18; then
		pm_nl_set_limits $ns1 4 4
		pm_nl_add_endpoint $ns1 10.0.2.1 flags subflow
		pm_nl_set_limits $ns2 4 4
		addr_nr_ns2=1 sflags=backup,fullmesh speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
		chk_prio_nr 0 1 1 0
		chk_rm_nr 0 1
	fi

	# set nobackup,nofullmesh flags
	if reset "set nobackup,nofullmesh flags test" &&
	   continue_if mptcp_lib_kversion_ge 5.18; then
		pm_nl_set_limits $ns1 4 4
		pm_nl_set_limits $ns2 4 4
		pm_nl_add_endpoint $ns2 10.0.2.2 flags subflow,backup,fullmesh
		sflags=nobackup,nofullmesh speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 2 2 2
		chk_prio_nr 0 1 1 0
		chk_rm_nr 0 1
	fi
}

fastclose_tests()
{
	if reset_check_counter "fastclose test" "MPTcpExtMPFastcloseTx"; then
		test_linkfail=1024 fastclose=client \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
		chk_fclose_nr 1 1
		chk_rst_nr 1 1 invert
	fi

	if reset_check_counter "fastclose server test" "MPTcpExtMPFastcloseRx"; then
		test_linkfail=1024 fastclose=server \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0 0 0 0 1
		chk_fclose_nr 1 1 invert
		chk_rst_nr 1 1
	fi
}

pedit_action_pkts()
{
	tc -n $ns2 -j -s action show action pedit index 100 | \
		mptcp_lib_get_info_value \"packets\" packets
}

fail_tests()
{
	# single subflow
	if reset_with_fail "Infinite map" 1; then
		MPTCP_LIB_SUBTEST_FLAKY=1
		test_linkfail=128 \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0 +1 +0 1 0 1 "$(pedit_action_pkts)"
		chk_fail_nr 1 -1 invert
	fi

	# multiple subflows
	if reset_with_fail "MP_FAIL MP_RST" 2; then
		MPTCP_LIB_SUBTEST_FLAKY=1
		tc -n $ns2 qdisc add dev ns2eth1 root netem rate 1mbit delay 5ms
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 10.0.2.2 dev ns2eth2 flags subflow
		test_linkfail=1024 \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 1 1 0 1 1 0 "$(pedit_action_pkts)"
	fi
}

# $1: ns ; $2: addr ; $3: id
userspace_pm_add_addr()
{
	local evts=$evts_ns1
	local tk

	[ "$1" == "$ns2" ] && evts=$evts_ns2
	tk=$(mptcp_lib_evts_get_info token "$evts")

	ip netns exec $1 ./pm_nl_ctl ann $2 token $tk id $3
	sleep 1
}

# $1: ns ; $2: id
userspace_pm_rm_addr()
{
	local evts=$evts_ns1
	local tk
	local cnt

	[ "$1" == "$ns2" ] && evts=$evts_ns2
	tk=$(mptcp_lib_evts_get_info token "$evts")

	cnt=$(rm_addr_count ${1})
	ip netns exec $1 ./pm_nl_ctl rem token $tk id $2
	wait_rm_addr $1 "${cnt}"
}

# $1: ns ; $2: addr ; $3: id
userspace_pm_add_sf()
{
	local evts=$evts_ns1
	local tk da dp

	[ "$1" == "$ns2" ] && evts=$evts_ns2
	tk=$(mptcp_lib_evts_get_info token "$evts")
	da=$(mptcp_lib_evts_get_info daddr4 "$evts")
	dp=$(mptcp_lib_evts_get_info dport "$evts")

	ip netns exec $1 ./pm_nl_ctl csf lip $2 lid $3 \
				rip $da rport $dp token $tk
	sleep 1
}

# $1: ns ; $2: addr $3: event type
userspace_pm_rm_sf()
{
	local evts=$evts_ns1
	local t=${3:-1}
	local ip
	local tk da dp sp
	local cnt

	[ "$1" == "$ns2" ] && evts=$evts_ns2
	[ -n "$(mptcp_lib_evts_get_info "saddr4" "$evts" $t)" ] && ip=4
	[ -n "$(mptcp_lib_evts_get_info "saddr6" "$evts" $t)" ] && ip=6
	tk=$(mptcp_lib_evts_get_info token "$evts")
	da=$(mptcp_lib_evts_get_info "daddr$ip" "$evts" $t $2)
	dp=$(mptcp_lib_evts_get_info dport "$evts" $t $2)
	sp=$(mptcp_lib_evts_get_info sport "$evts" $t $2)

	cnt=$(rm_sf_count ${1})
	ip netns exec $1 ./pm_nl_ctl dsf lip $2 lport $sp \
				rip $da rport $dp token $tk
	wait_rm_sf $1 "${cnt}"
}

check_output()
{
	local cmd="$1"
	local expected="$2"
	local msg="$3"
	local rc=0

	mptcp_lib_check_output "${err}" "${cmd}" "${expected}" || rc=${?}
	if [ ${rc} -eq 2 ]; then
		fail_test "fail to check output # error ${rc}"
	elif [ ${rc} -eq 0 ]; then
		print_ok
	elif [ ${rc} -eq 1 ]; then
		fail_test "fail to check output # different output"
	fi
}

# $1: ns
userspace_pm_dump()
{
	local evts=$evts_ns1
	local tk

	[ "$1" == "$ns2" ] && evts=$evts_ns2
	tk=$(mptcp_lib_evts_get_info token "$evts")

	ip netns exec $1 ./pm_nl_ctl dump token $tk
}

# $1: ns ; $2: id
userspace_pm_get_addr()
{
	local evts=$evts_ns1
	local tk

	[ "$1" == "$ns2" ] && evts=$evts_ns2
	tk=$(mptcp_lib_evts_get_info token "$evts")

	ip netns exec $1 ./pm_nl_ctl get $2 token $tk
}

userspace_pm_chk_dump_addr()
{
	local ns="${1}"
	local exp="${2}"
	local check="${3}"

	print_check "dump addrs ${check}"

	if false && mptcp_lib_kallsyms_has "mptcp_userspace_pm_dump_addr$"; then
		check_output "userspace_pm_dump ${ns}" "${exp}"
	else
		print_skip
	fi
}

userspace_pm_chk_get_addr()
{
	local ns="${1}"
	local id="${2}"
	local exp="${3}"

	print_check "get id ${id} addr"

	if false && mptcp_lib_kallsyms_has "mptcp_userspace_pm_get_addr$"; then
		check_output "userspace_pm_get_addr ${ns} ${id}" "${exp}"
	else
		print_skip
	fi
}

# $1: ns ; $2: event type ; $3: count
chk_evt_nr()
{
	local ns=${1}
	local evt_name="${2}"
	local exp="${3}"

	local evts="${evts_ns1}"
	local evt="${!evt_name}"
	local count

	evt_name="${evt_name:16}" # without MPTCP_LIB_EVENT_
	[ "${ns}" == "ns2" ] && evts="${evts_ns2}"

	print_check "event ${ns} ${evt_name} (${exp})"

	if [[ "${evt_name}" = "LISTENER_"* ]] &&
	   ! mptcp_lib_kallsyms_has "mptcp_event_pm_listener$"; then
		print_skip "event not supported"
		return
	fi

	count=$(grep -cw "type:${evt}" "${evts}")
	if [ "${count}" != "${exp}" ]; then
		fail_test "got ${count} events, expected ${exp}"
	else
		print_ok
	fi
}

userspace_tests()
{
	# userspace pm type prevents add_addr
	if reset "userspace pm type prevents add_addr" &&
	   continue_if mptcp_lib_has_file '/proc/sys/net/mptcp/pm_type'; then
		set_userspace_pm $ns1
		pm_nl_set_limits $ns1 0 2
		pm_nl_set_limits $ns2 0 2
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
		chk_add_nr 0 0
	fi

	# userspace pm type does not echo add_addr without daemon
	if reset "userspace pm no echo w/o daemon" &&
	   continue_if mptcp_lib_has_file '/proc/sys/net/mptcp/pm_type'; then
		set_userspace_pm $ns2
		pm_nl_set_limits $ns1 0 2
		pm_nl_set_limits $ns2 0 2
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
		chk_add_nr 1 0
	fi

	# userspace pm type rejects join
	if reset "userspace pm type rejects join" &&
	   continue_if mptcp_lib_has_file '/proc/sys/net/mptcp/pm_type'; then
		set_userspace_pm $ns1
		pm_nl_set_limits $ns1 1 1
		pm_nl_set_limits $ns2 1 1
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 0
	fi

	# userspace pm type does not send join
	if reset "userspace pm type does not send join" &&
	   continue_if mptcp_lib_has_file '/proc/sys/net/mptcp/pm_type'; then
		set_userspace_pm $ns2
		pm_nl_set_limits $ns1 1 1
		pm_nl_set_limits $ns2 1 1
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
	fi

	# userspace pm type prevents mp_prio
	if reset "userspace pm type prevents mp_prio" &&
	   continue_if mptcp_lib_has_file '/proc/sys/net/mptcp/pm_type'; then
		set_userspace_pm $ns1
		pm_nl_set_limits $ns1 1 1
		pm_nl_set_limits $ns2 1 1
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		sflags=backup speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 1 1 0
		chk_prio_nr 0 0 0 0
	fi

	# userspace pm type prevents rm_addr
	if reset "userspace pm type prevents rm_addr" &&
	   continue_if mptcp_lib_has_file '/proc/sys/net/mptcp/pm_type'; then
		set_userspace_pm $ns1
		set_userspace_pm $ns2
		pm_nl_set_limits $ns1 0 1
		pm_nl_set_limits $ns2 0 1
		pm_nl_add_endpoint $ns2 10.0.3.2 flags subflow
		addr_nr_ns2=-1 speed=slow \
			run_tests $ns1 $ns2 10.0.1.1
		chk_join_nr 0 0 0
		chk_rm_nr 0 0
	fi

	# userspace pm add & remove address
	if reset_with_events "userspace pm add & remove address" &&
	   continue_if mptcp_lib_has_file '/proc/sys/net/mptcp/pm_type'; then
		set_userspace_pm $ns1
		pm_nl_set_limits $ns2 2 2
		speed=5 \
			run_tests $ns1 $ns2 10.0.1.1 &
		local tests_pid=$!
		wait_mpj $ns1
		userspace_pm_add_addr $ns1 10.0.2.1 10
		userspace_pm_add_addr $ns1 10.0.3.1 20
		chk_join_nr 2 2 2
		chk_add_nr 2 2
		chk_mptcp_info subflows 2 subflows 2
		chk_subflows_total 3 3
		chk_mptcp_info add_addr_signal 2 add_addr_accepted 2
		userspace_pm_chk_dump_addr "${ns1}" \
			$'id 10 flags signal 10.0.2.1\nid 20 flags signal 10.0.3.1' \
			"signal"
		userspace_pm_chk_get_addr "${ns1}" "10" "id 10 flags signal 10.0.2.1"
		userspace_pm_chk_get_addr "${ns1}" "20" "id 20 flags signal 10.0.3.1"
		userspace_pm_rm_sf $ns1 "::ffff:10.0.2.1" $MPTCP_LIB_EVENT_SUB_ESTABLISHED
		userspace_pm_chk_dump_addr "${ns1}" \
			"id 20 flags signal 10.0.3.1" "after rm_sf 10"
		userspace_pm_rm_addr $ns1 20
		userspace_pm_chk_dump_addr "${ns1}" "" "after rm_addr 20"
		chk_rm_nr 1 1 invert
		chk_mptcp_info subflows 0 subflows 0
		chk_subflows_total 1 1
		kill_events_pids
		mptcp_lib_kill_wait $tests_pid
	fi

	# userspace pm create destroy subflow
	if reset_with_events "userspace pm create destroy subflow" &&
	   continue_if mptcp_lib_has_file '/proc/sys/net/mptcp/pm_type'; then
		set_userspace_pm $ns2
		pm_nl_set_limits $ns1 0 1
		speed=5 \
			run_tests $ns1 $ns2 10.0.1.1 &
		local tests_pid=$!
		wait_mpj $ns2
		userspace_pm_add_sf $ns2 10.0.3.2 20
		chk_join_nr 1 1 1
		chk_mptcp_info subflows 1 subflows 1
		chk_subflows_total 2 2
		userspace_pm_chk_dump_addr "${ns2}" \
			"id 20 flags subflow 10.0.3.2" \
			"subflow"
		userspace_pm_chk_get_addr "${ns2}" "20" "id 20 flags subflow 10.0.3.2"
		userspace_pm_rm_sf $ns2 10.0.3.2 $MPTCP_LIB_EVENT_SUB_ESTABLISHED
		userspace_pm_chk_dump_addr "${ns2}" \
			"" \
			"after rm_sf 20"
		chk_rm_nr 0 1
		chk_mptcp_info subflows 0 subflows 0
		chk_subflows_total 1 1
		kill_events_pids
		mptcp_lib_kill_wait $tests_pid
	fi

	# userspace pm create id 0 subflow
	if reset_with_events "userspace pm create id 0 subflow" &&
	   continue_if mptcp_lib_has_file '/proc/sys/net/mptcp/pm_type'; then
		set_userspace_pm $ns2
		pm_nl_set_limits $ns1 0 1
		speed=5 \
			run_tests $ns1 $ns2 10.0.1.1 &
		local tests_pid=$!
		wait_mpj $ns2
		chk_mptcp_info subflows 0 subflows 0
		chk_subflows_total 1 1
		userspace_pm_add_sf $ns2 10.0.3.2 0
		userspace_pm_chk_dump_addr "${ns2}" \
			"id 0 flags subflow 10.0.3.2" "id 0 subflow"
		chk_join_nr 1 1 1
		chk_mptcp_info subflows 1 subflows 1
		chk_subflows_total 2 2
		kill_events_pids
		mptcp_lib_kill_wait $tests_pid
	fi
}

endpoint_tests()
{
	# subflow_rebuild_header is needed to support the implicit flag
	# userspace pm type prevents add_addr
	if reset "implicit EP" &&
	   mptcp_lib_kallsyms_has "subflow_rebuild_header$"; then
		pm_nl_set_limits $ns1 2 2
		pm_nl_set_limits $ns2 2 2
		pm_nl_add_endpoint $ns1 10.0.2.1 flags signal
		speed=slow \
			run_tests $ns1 $ns2 10.0.1.1 &
		local tests_pid=$!

		wait_mpj $ns1
		pm_nl_check_endpoint "creation" \
			$ns2 10.0.2.2 id 1 flags implicit
		chk_mptcp_info subflows 1 subflows 1
		chk_mptcp_info add_addr_signal 1 add_addr_accepted 1

		pm_nl_add_endpoint $ns2 10.0.2.2 id 33 2>/dev/null
		pm_nl_check_endpoint "ID change is prevented" \
			$ns2 10.0.2.2 id 1 flags implicit

		pm_nl_add_endpoint $ns2 10.0.2.2 flags signal
		pm_nl_check_endpoint "modif is allowed" \
			$ns2 10.0.2.2 id 1 flags signal
		mptcp_lib_kill_wait $tests_pid
	fi

	if reset_with_tcp_filter "delete and re-add" ns2 10.0.3.2 REJECT OUTPUT &&
	   mptcp_lib_kallsyms_has "subflow_rebuild_header$"; then
		start_events
		pm_nl_set_limits $ns1 0 3
		pm_nl_set_limits $ns2 0 3
		pm_nl_add_endpoint $ns2 10.0.1.2 id 1 dev ns2eth1 flags subflow
		pm_nl_add_endpoint $ns2 10.0.2.2 id 2 dev ns2eth2 flags subflow
		test_linkfail=4 speed=5 \
			run_tests $ns1 $ns2 10.0.1.1 &
		local tests_pid=$!

		wait_mpj $ns2
		pm_nl_check_endpoint "creation" \
			$ns2 10.0.2.2 id 2 flags subflow dev ns2eth2
		chk_subflow_nr "before delete id 2" 2
		chk_mptcp_info subflows 1 subflows 1

		pm_nl_del_endpoint $ns2 2 10.0.2.2
		sleep 0.5
		chk_subflow_nr "after delete id 2" 1
		chk_mptcp_info subflows 0 subflows 0

		pm_nl_add_endpoint $ns2 10.0.2.2 id 2 dev ns2eth2 flags subflow
		wait_mpj $ns2
		chk_subflow_nr "after re-add id 2" 2
		chk_mptcp_info subflows 1 subflows 1

		pm_nl_add_endpoint $ns2 10.0.3.2 id 3 flags subflow
		wait_attempt_fail $ns2
		chk_subflow_nr "after new reject" 2
		chk_mptcp_info subflows 1 subflows 1

		ip netns exec "${ns2}" ${iptables} -D OUTPUT -s "10.0.3.2" -p tcp -j REJECT
		pm_nl_del_endpoint $ns2 3 10.0.3.2
		pm_nl_add_endpoint $ns2 10.0.3.2 id 3 flags subflow
		wait_mpj $ns2
		chk_subflow_nr "after no reject" 3
		chk_mptcp_info subflows 2 subflows 2

		local i
		for i in $(seq 3); do
			pm_nl_del_endpoint $ns2 1 10.0.1.2
			sleep 0.5
			chk_subflow_nr "after delete id 0 ($i)" 2
			chk_mptcp_info subflows 2 subflows 2 # only decr for additional sf

			pm_nl_add_endpoint $ns2 10.0.1.2 id 1 dev ns2eth1 flags subflow
			wait_mpj $ns2
			chk_subflow_nr "after re-add id 0 ($i)" 3
			chk_mptcp_info subflows 3 subflows 3
		done

		mptcp_lib_kill_wait $tests_pid

		kill_events_pids
		chk_evt_nr ns1 MPTCP_LIB_EVENT_LISTENER_CREATED 1
		chk_evt_nr ns1 MPTCP_LIB_EVENT_CREATED 1
		chk_evt_nr ns1 MPTCP_LIB_EVENT_ESTABLISHED 1
		chk_evt_nr ns1 MPTCP_LIB_EVENT_ANNOUNCED 0
		chk_evt_nr ns1 MPTCP_LIB_EVENT_REMOVED 4
		chk_evt_nr ns1 MPTCP_LIB_EVENT_SUB_ESTABLISHED 6
		chk_evt_nr ns1 MPTCP_LIB_EVENT_SUB_CLOSED 4

		chk_evt_nr ns2 MPTCP_LIB_EVENT_CREATED 1
		chk_evt_nr ns2 MPTCP_LIB_EVENT_ESTABLISHED 1
		chk_evt_nr ns2 MPTCP_LIB_EVENT_ANNOUNCED 0
		chk_evt_nr ns2 MPTCP_LIB_EVENT_REMOVED 0
		chk_evt_nr ns2 MPTCP_LIB_EVENT_SUB_ESTABLISHED 6
		chk_evt_nr ns2 MPTCP_LIB_EVENT_SUB_CLOSED 5 # one has been closed before estab

		chk_join_nr 6 6 6
		chk_rm_nr 4 4
	fi

	# remove and re-add
	if reset_with_events "delete re-add signal" &&
	   mptcp_lib_kallsyms_has "subflow_rebuild_header$"; then
		pm_nl_set_limits $ns1 0 3
		pm_nl_set_limits $ns2 3 3
		pm_nl_add_endpoint $ns1 10.0.2.1 id 1 flags signal
		# broadcast IP: no packet for this address will be received on ns1
		pm_nl_add_endpoint $ns1 224.0.0.1 id 2 flags signal
		pm_nl_add_endpoint $ns1 10.0.1.1 id 42 flags signal
		test_linkfail=4 speed=5 \
			run_tests $ns1 $ns2 10.0.1.1 &
		local tests_pid=$!

		wait_mpj $ns2
		pm_nl_check_endpoint "creation" \
			$ns1 10.0.2.1 id 1 flags signal
		chk_subflow_nr "before delete" 2
		chk_mptcp_info subflows 1 subflows 1

		pm_nl_del_endpoint $ns1 1 10.0.2.1
		pm_nl_del_endpoint $ns1 2 224.0.0.1
		sleep 0.5
		chk_subflow_nr "after delete" 1
		chk_mptcp_info subflows 0 subflows 0

		pm_nl_add_endpoint $ns1 10.0.2.1 id 1 flags signal
		pm_nl_add_endpoint $ns1 10.0.3.1 id 2 flags signal
		wait_mpj $ns2
		chk_subflow_nr "after re-add" 3
		chk_mptcp_info subflows 2 subflows 2

		pm_nl_del_endpoint $ns1 42 10.0.1.1
		sleep 0.5
		chk_subflow_nr "after delete ID 0" 2
		chk_mptcp_info subflows 2 subflows 2

		pm_nl_add_endpoint $ns1 10.0.1.1 id 99 flags signal
		wait_mpj $ns2
		chk_subflow_nr "after re-add ID 0" 3
		chk_mptcp_info subflows 3 subflows 3

		pm_nl_del_endpoint $ns1 99 10.0.1.1
		sleep 0.5
		chk_subflow_nr "after re-delete ID 0" 2
		chk_mptcp_info subflows 2 subflows 2

		pm_nl_add_endpoint $ns1 10.0.1.1 id 88 flags signal
		wait_mpj $ns2
		chk_subflow_nr "after re-re-add ID 0" 3
		chk_mptcp_info subflows 3 subflows 3
		mptcp_lib_kill_wait $tests_pid

		kill_events_pids
		chk_evt_nr ns1 MPTCP_LIB_EVENT_LISTENER_CREATED 1
		chk_evt_nr ns1 MPTCP_LIB_EVENT_CREATED 1
		chk_evt_nr ns1 MPTCP_LIB_EVENT_ESTABLISHED 1
		chk_evt_nr ns1 MPTCP_LIB_EVENT_ANNOUNCED 0
		chk_evt_nr ns1 MPTCP_LIB_EVENT_REMOVED 0
		chk_evt_nr ns1 MPTCP_LIB_EVENT_SUB_ESTABLISHED 5
		chk_evt_nr ns1 MPTCP_LIB_EVENT_SUB_CLOSED 3

		chk_evt_nr ns2 MPTCP_LIB_EVENT_CREATED 1
		chk_evt_nr ns2 MPTCP_LIB_EVENT_ESTABLISHED 1
		chk_evt_nr ns2 MPTCP_LIB_EVENT_ANNOUNCED 6
		chk_evt_nr ns2 MPTCP_LIB_EVENT_REMOVED 4
		chk_evt_nr ns2 MPTCP_LIB_EVENT_SUB_ESTABLISHED 5
		chk_evt_nr ns2 MPTCP_LIB_EVENT_SUB_CLOSED 3

		chk_join_nr 5 5 5
		chk_add_nr 6 6
		chk_rm_nr 4 3 invert
	fi

	# flush and re-add
	if reset_with_tcp_filter "flush re-add" ns2 10.0.3.2 REJECT OUTPUT &&
	   mptcp_lib_kallsyms_has "subflow_rebuild_header$"; then
		pm_nl_set_limits $ns1 0 2
		pm_nl_set_limits $ns2 1 2
		# broadcast IP: no packet for this address will be received on ns1
		pm_nl_add_endpoint $ns1 224.0.0.1 id 2 flags signal
		pm_nl_add_endpoint $ns2 10.0.3.2 id 3 flags subflow
		test_linkfail=4 speed=20 \
			run_tests $ns1 $ns2 10.0.1.1 &
		local tests_pid=$!

		wait_attempt_fail $ns2
		chk_subflow_nr "before flush" 1
		chk_mptcp_info subflows 0 subflows 0

		pm_nl_flush_endpoint $ns2
		pm_nl_flush_endpoint $ns1
		wait_rm_addr $ns2 0
		ip netns exec "${ns2}" ${iptables} -D OUTPUT -s "10.0.3.2" -p tcp -j REJECT
		pm_nl_add_endpoint $ns2 10.0.3.2 id 3 flags subflow
		wait_mpj $ns2
		pm_nl_add_endpoint $ns1 10.0.3.1 id 2 flags signal
		wait_mpj $ns2
		mptcp_lib_kill_wait $tests_pid

		chk_join_nr 2 2 2
		chk_add_nr 2 2
		chk_rm_nr 1 0 invert
	fi
}

# [$1: error message]
usage()
{
	if [ -n "${1}" ]; then
		echo "${1}"
		ret=1
	fi

	echo "mptcp_join usage:"

	local key
	for key in "${!all_tests[@]}"; do
		echo "  -${key} ${all_tests[${key}]}"
	done

	echo "  -c capture pcap files"
	echo "  -C enable data checksum"
	echo "  -i use ip mptcp"
	echo "  -h help"

	echo "[test ids|names]"

	exit ${ret}
}


# Use a "simple" array to force an specific order we cannot have with an associative one
all_tests_sorted=(
	f@subflows_tests
	e@subflows_error_tests
	s@signal_address_tests
	l@link_failure_tests
	t@add_addr_timeout_tests
	r@remove_tests
	a@add_tests
	6@ipv6_tests
	4@v4mapped_tests
	M@mixed_tests
	b@backup_tests
	p@add_addr_ports_tests
	k@syncookies_tests
	S@checksum_tests
	d@deny_join_id0_tests
	m@fullmesh_tests
	z@fastclose_tests
	F@fail_tests
	u@userspace_tests
	I@endpoint_tests
)

all_tests_args=""
all_tests_names=()
for subtests in "${all_tests_sorted[@]}"; do
	key="${subtests%@*}"
	value="${subtests#*@}"

	all_tests_args+="${key}"
	all_tests_names+=("${value}")
	all_tests[${key}]="${value}"
done

tests=()
while getopts "${all_tests_args}cCih" opt; do
	case $opt in
		["${all_tests_args}"])
			tests+=("${all_tests[${opt}]}")
			;;
		c)
			capture=1
			;;
		C)
			checksum=1
			;;
		i)
			ip_mptcp=1
			;;
		h)
			usage
			;;
		*)
			usage "Unknown option: -${opt}"
			;;
	esac
done

shift $((OPTIND - 1))

for arg in "${@}"; do
	if [[ "${arg}" =~ ^[0-9]+$ ]]; then
		only_tests_ids+=("${arg}")
	else
		only_tests_names+=("${arg}")
	fi
done

if [ ${#tests[@]} -eq 0 ]; then
	tests=("${all_tests_names[@]}")
fi

for subtests in "${tests[@]}"; do
	"${subtests}"
done

if [ ${ret} -ne 0 ]; then
	echo
	echo "${#failed_tests[@]} failure(s) has(ve) been detected:"
	for i in $(get_failed_tests_ids); do
		echo -e "\t- ${i}: ${failed_tests[${i}]}"
	done
	echo
fi

append_prev_results
mptcp_lib_result_print_all_tap

exit $ret
