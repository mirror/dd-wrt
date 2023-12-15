#!/usr/bin/env bash
#
# check that the version numbers are updated
#
source tests/common.bash

expected_version=$(sed '/VN/s/.ds VN \([0-9.a-z]*\)/\1/' ./version)
actual_version=$($lsof -v 2>&1 | sed -ne 's/^ *revision: *\([0-9.a-z]*\)/\1/p')
dist_version=$(sed -ne 's/^\([0-9][0-9.a-z]*\)		.*$/\1/p' 00DIST | tail -1)

if [ "${expected_version}" != "${actual_version}" ]; then
    {
	echo "expected version defined in version file: ${expected_version}"
	echo "lsof executable says: ${actual_version}"
    } > $report
    exit 1
fi

if [ "${expected_version}" != "${dist_version}" ]; then
    {
	echo "expected version defined in version file: ${expected_version}"
	echo "the last entry of 00DIST is: ${dist_version}"
    } > $report
    exit 1
fi

exit 0
