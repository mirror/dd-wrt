#!/bin/sh

for i in $1/*.asp $1/*.svg $1/*.htm $1/*.html $1/*.js $1/*.css; do
	if test -e $i; then
		echo $i
		cp $i $i.tmp
		tr -d '\t\n\r\f' < $i.tmp > $i
		rm $i.tmp
	fi
done
