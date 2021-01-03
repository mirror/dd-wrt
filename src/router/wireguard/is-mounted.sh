#!/bin/sh
# wait until directory is mounted and writable
# usage: is-mounted /name-of directory
# default is /jffs
(
DIR=$1
[[ -z "$DIR" ]] && DIR="/jffs"
i=0
# grep -q "$DIR.*rw" /proc/mounts && echo "$DIR is ready" || echo "$DIR is not ready"
until [[ -f $DIR/mounted ]]; do
	sleep 1
	i=$(($i+1))
	touch "$DIR/mounted" >/dev/null 2>&1
	if [[ $i -gt 35 ]]; then
		echo "$DIR not mounted after 35 sec"
		exit
	fi
done 
rm $DIR/mounted >/dev/null 2>&1
echo "$DIR mounted after $i seconds"
)2>&1 | logger -t "$(basename $0): "
