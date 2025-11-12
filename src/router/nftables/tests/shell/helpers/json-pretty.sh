#!/bin/bash -e

exec_pretty() {
	# The output of this command must be stable (and `jq` and python
	# fallback must generate the same output.

	if command -v jq &>/dev/null ; then
		# If we have, use `jq`
		exec jq
	fi

	# Fallback to python.
	exec python -c '
import json
import sys

parsed = json.load(sys.stdin)
print(json.dumps(parsed, indent=2))
'
}

[ "$#" -le 1 ] || { echo "At most one argument supported" ; exit 1 ; }

if [ "$#" -eq 1 ] ; then
	# One argument passed. This must be a JSON file.
	[ -f "$1" ] || { echo "File \"$1\" does not exist" ; exit 1 ; }
	exec_pretty < "$1"
fi

exec_pretty
