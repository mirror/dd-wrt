#!/usr/bin/env bash
source tests/common.bash

echo "inclusion test" >> $report
while read line; do
    if [[ $line =~ ^f[^0-9].* ]]; then
	echo "Unexpectedly, a named file descriptor is included: "
	echo "${line}"
	echo
	echo "## whole output for debugging (-d fd -F fd): "
	${lsof} -p $$ -a -d fd -F fd
	echo "## whole output for debugging (-d fd): "
	${lsof} -p $$ -a -d fd
	echo "## whole output for debugging (no -d): "
	${lsof} -p $$
	exit 1
    fi
done < <(${lsof} -p $$ -a -d fd -F fd) >> $report

echo "exclusion test" >> $report
while read line; do
    if [[ $line =~ ^f[0-9]+ ]]; then
	echo "Unexpectedly, a numbered file descriptor is included: "
	echo "${line}"
	echo "## whole output for debugging (-d fd -F fd): "
	${lsof} -p $$ -a -d '^fd' -F fd
	echo "## whole output for debugging (-d ^fd): "
	${lsof} -p $$ -a -d '^fd'
	echo "## whole output for debugging (no -d): "
	${lsof} -p $$
	exit 1
    fi
done < <(${lsof} -p $$ -a -d "^fd" -F fd) >> $report

exit 0
