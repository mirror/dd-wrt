#!/bin/bash

# (c) Josua Dietze 2019-2020
#
# Usage: make_string.sh source.tcl > [condensed tcl]

# Converts a Tcl source file into C code suitable
# for using as an embedded script.

input="$1"
rawout="#define RAWSCRIPT \""

while IFS= read -r line
do
	if [ -z "$line" ]; then
		continue
	fi
	clean2="${line//\\/\\\\}"
	clean3="${clean2//\"/\\\"}"
	rawout="$rawout$clean3\\n"
done < <(sed -E 's/^[[:space:]]+|[[:space:]]+$|^[[:space:]]*#.*$//' "$input")

echo "$rawout\""
