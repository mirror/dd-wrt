#!/bin/sh
#

PATH=/usr/local/sbin:/usr/local/bin:/sbin:/bin:/usr/sbin:/usr/bin
DAEMON=/usr/bin/htpdate
NAME=htpdate
DESC="HTTP Time Protocol daemon"

test -x $DAEMON || exit 0

# Include htpdate defaults if available
if [ -f /etc/default/htpdate ] ; then
	. /etc/default/htpdate
fi

case "$1" in
	start)
		echo -n "Starting $DESC: "
		start-stop-daemon --start --quiet --pidfile /var/run/$NAME.pid \
			--exec $DAEMON -- $DAEMON_OPTS
		echo "$NAME."
		;;
	stop)
		echo -n "Stopping $DESC: "
		start-stop-daemon --stop --quiet --pidfile /var/run/$NAME.pid \
			--exec $DAEMON
		rm -f /var/run/$NAME.pid
		echo "$NAME."
		;;
	restart)
   		$0 stop
		sleep 1
   		$0 start
		;;
	*)
		N=/etc/init.d/$NAME
		echo "Usage: $N {start|stop|restart}" >&2
		exit 1
		;;
esac

exit 0

# vi:set ts=4:
