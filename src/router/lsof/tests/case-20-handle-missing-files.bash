#!/usr/bin/env bash
# See https://github.com/lsof-org/lsof/issues/90
source tests/common.bash

msg=$(${lsof} /NO-SUCH-FILE 2>&1)

if [[ "${msg}" == \
	       *': status error on /NO-SUCH-FILE: No such file or directory' ]]; then
    exit 0
else
    {
	echo "unexpected output: "
	echo "${msg}"
    } > $report
    exit 1
fi
