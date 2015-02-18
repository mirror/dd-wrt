#!/bin/sh

for i in $2/*.asp $2/*.htm $2/*.html $2/*.js $2/*.css; do
	if test -e $i; then
		echo $i
		$1/tools/removewhitespace $i
	fi
done
