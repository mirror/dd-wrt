#!/bin/sh
# Wait until directory is mounted and writable
# Usage: is-mounted path-of-directory  max-wait-time
# default is /jffs 35
# Can also be used to test drive readiness e.g.:
# if is-mounted /jffs 5; then echo "drive ready"; else echo "drive not ready"; fi
DIR=$1
[[ -z "$DIR" ]] && DIR="/jffs"
MAXT=$2
[[ -z "$MAXT" ]] && MAXT=35
i=0
# grep -q "$DIR.*rw" /proc/mounts && echo "$DIR is ready" || echo "$DIR is not ready"
until [[ -f $DIR/mounted ]]; do
	sleep 1
	i=$(($i+1))
	touch "$DIR/mounted" >/dev/null 2>&1
	if [[ $i -gt $MAXT ]]; then
		logger -p user.err -t "$(basename $0)" "$DIR *not* mounted after trying $MAXT sec"
		exit 1
	fi
done 
rm $DIR/mounted >/dev/null 2>&1
logger -p user.info -t "$(basename $0)" "$DIR mounted after $i seconds"
exit 0
