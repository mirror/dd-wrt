#!/bin/bash

# contains utility functions commonly used in tests.

assert_pass()
{
	local ret=$?
	if [ $ret != 0 ]; then
		echo "FAIL: ${@}"
		if type -t assert_failout; then
			assert_failout
		fi
		exit 1
	else
		echo "PASS: ${@}"
	fi
}
assert_fail()
{
	local ret=$?
	if [ $ret == 0 ]; then
		echo "FAIL: ${@}"
		if type -t assert_failout; then
			assert_failout
		fi
		exit 1
	else
		echo "PASS: ${@}"
	fi
}

wait_local_port_listen()
{
	local listener_ns="${1}"
	local port="${2}"
	local protocol="${3}"
	local pattern
	local i

	pattern=":$(printf "%04X" "${port}") "

	# for tcp protocol additionally check the socket state
	[ ${protocol} = "tcp" ] && pattern="${pattern}0A"
	for i in $(seq 10); do
		if ip netns exec "${listener_ns}" awk '{print $2" "$4}' \
		   /proc/net/"${protocol}"* | grep -q "${pattern}"; then
			break
		fi
		sleep 0.1
	done
}
