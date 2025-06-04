#!/usr/bin/env bash
[ -n "$CROSS_COMPILE" ] || {
	echo "The variable CROSS must be set to point to the cross-compiler prefix"
	exit 1
}

MODULE="$1"

[ "$#" -ne 1 ] && {
	echo "Usage: $0 <module>"
	exit 1
}

ARGS=
if [ -n "$KEEP_SYMBOLS" ]; then
	ARGS="-X --strip-debug"
else
	ARGS="-x -G __this_module --strip-unneeded"
fi

${CROSS_COMPILE}objcopy \
	-R .comment \
	-R .pdr \
	-R .mdebug.abi32 \
	-R .note.gnu.build-id \
	-R .gnu.attributes \
	-R .reginfo \
	$ARGS \
	"$MODULE" "$MODULE.tmp"

[ -n "$NO_RENAME" ] && {
	mv "${MODULE}.tmp" "$MODULE"
	exit 0
}

${CROSS_COMPILE}nm "$MODULE.tmp" | awk '
BEGIN {
	n = 0
}

$3 && $2 ~ /[brtd]/ && $3 !~ /\$LC/ && !def[$3] {
	print "--redefine-sym "$3"=_"n;
	n = n + 1
	def[$3] = 1
}
' > "$MODULE.tmp1"

${CROSS_COMPILE}objcopy `cat ${MODULE}.tmp1` ${MODULE}.tmp ${MODULE}.out
mv "${MODULE}.out" "${MODULE}"
rm -f "${MODULE}".t*
