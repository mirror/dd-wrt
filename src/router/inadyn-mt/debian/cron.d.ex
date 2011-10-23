#
# Regular cron jobs for the inadyn-mt package
#
0 4	* * *	root	[ -x /usr/bin/inadyn-mt_maintenance ] && /usr/bin/inadyn-mt_maintenance
