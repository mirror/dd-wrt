#!/bin/bash

# Do some simple sanity checks on tests:
# - Report tests where reply matches command
# - Report tests with non-ok exit but reply
# - Check for duplicate test commands in *.t files
# - Check for duplicate or stale payload records in *.t.payload* files
# - Check for duplicate or stale json equivalents in *.t.json files

cd $(dirname $0)/../

[[ $1 ]] && tests="$@" || tests="*/*.t"

reportfile=""
report() { # (file, msg)
	[[ "$reportfile" == "$1" ]] || {
		reportfile="$1"
		echo ""
		echo "In $reportfile:"
	}
	shift
	echo "$@"
}

for t in $tests; do
	[[ -f $t ]] || continue

	readarray -t cmdlines <<< $(grep -v -e '^ *[:*#-?]' -e '^ *$' $t)

	cmds=""
	for cmdline in "${cmdlines[@]}"; do
		readarray -t -d ';' cmdparts <<< "$cmdline"
		cmd="${cmdparts[0]}"
		rc="${cmdparts[1]}"
		out="${cmdparts[2]}"

		[[ -n $cmd ]] || continue

		#echo "cmdline: $cmdline"
		#echo "cmd: $cmd"
		#echo "rc: $rc"
		#echo "out: $out"

		[[ "$cmd" != "$out" ]] || \
			report $t "reply matches cmd: $cmd"
		[[ "$rc" != "ok" && "$out" ]] && \
			report $t "output record with non-ok exit: $cmd"

		cmds+="${cmd}\n"
	done

	readarray -t dups <<< $(echo -e "$cmds" | sort | uniq -d)
	for dup in "${dups[@]}"; do
		[[ -n $dup ]] || continue
		report $t "duplicate command: $dup"
	done

	for p in $t.payload* $t.json; do
		[[ -f $p ]] || continue
		[[ $p == *.got ]] && continue
		[[ $p == *.json ]] && t="json" || t="payload"

		pcmds=$(grep '^#' $p)
		readarray -t dups <<< $(echo "$pcmds" | sort | uniq -d)
		readarray -t stales <<< $(echo "$pcmds" | while read hash pcmd; do
			echo -e "$cmds" | grep -qxF "${pcmd}" || echo "# ${pcmd}"
		done)

		for stale in "${stales[@]}"; do
			[[ -n $stale ]] || continue
			report $p "stale $t record: $stale"
		done
		for dup in "${dups[@]}"; do
			[[ -n $dup ]] || continue
			report $p "duplicate $t record: $dup"
		done
	done
done
