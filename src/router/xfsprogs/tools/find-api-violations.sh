#!/bin/bash
# SPDX-License-Identifier: GPL-2.0

# Find libxfs API violations -- calls to functions defined in libxfs/*.c that
# don't use the libxfs wrappers; or failing to negate the integer return
# values.

# NOTE: This script doesn't look for API violations in function parameters.

tool_dirs="copy db estimate fsck fsr growfs io logprint mdrestore mkfs quota repair rtcp scrub"

# Calls to xfs_* functions in libxfs/*.c without the libxfs_ prefix
find_possible_api_calls() {
	grep -rn '[-[:space:],(]xfs_[a-z_]*(' $tool_dirs | sed -e 's/^.*\(xfs_[a-z_]*\)(.*$/\1/g' | sort | uniq
}

check_if_api_calls() {
	while read f; do grep "^$f(" libxfs/*.c; done | sed -e 's/^.*:xfs_/xfs_/g' -e 's/.$//g'
}

# Generate a grep search expression for troublesome API call sites.
# " foo(", ",foo(", "-foo(", and "(foo(" are examples.
grep_pattern() {
	sed -e 's/^/[[:space:],-\\(]/g' -e 's/$/(/g'
}

find_libxfs_violations() {
	grep -r -n -f <(find_possible_api_calls | check_if_api_calls | grep_pattern) $tool_dirs
}

# libxfs calls without negated error codes
find_errcode_violations() {
	grep -r -n 'err.* = libxfs' $tool_dirs
}

# Find xfs_* calls that are in the libxfs definition list
find_possible_libxfs_api_calls() {
	grep '#define[[:space:]]*xfs' libxfs/libxfs_api_defs.h | awk '{print $2}'
}

find_libxfs_api_violations() {
	grep -r -n -f <(find_possible_libxfs_api_calls | grep_pattern) $tool_dirs
}

(find_libxfs_violations ; find_errcode_violations ; find_libxfs_api_violations) | sort -g -t ':' -k 2 | sort -g -t ':' -k 1 | uniq
