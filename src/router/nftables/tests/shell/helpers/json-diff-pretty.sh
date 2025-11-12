#!/bin/bash -e

BASEDIR="$(dirname "$0")"

[ $# -eq 2 ] || (echo "$0: expects two JSON files as arguments" ; exit 1)

FILE1="$1"
FILE2="$2"

pretty()
{
	"$BASEDIR/json-pretty.sh" < "$1" 2>&1 || :
}

echo "Cmd: \"$0\" \"$FILE1\" \"$FILE2\""
diff -u "$FILE1" "$FILE2" 2>&1 || :
diff -u <(pretty "$FILE1") <(pretty "$FILE2") 2>&1 || :
