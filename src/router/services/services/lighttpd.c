
#ifdef HAVE_LIGHTTPD
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <signal.h>
#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <services.h>

void start_lighttpd(void)
{
	if (!nvram_match("lighttpd_enable", "1"))
		return;
	
	eval("lighttpd", "-f /etc/lighttpd.conf");
	syslog(LOG_INFO, "lighttpd : lighttpd started\n");
	return;
}

void stop_lighttpd(void)
{
	syslog(LOG_INFO, "lighttpd : lighttpd stopped\n");
	stop_process("lighttpd", "lighttpd");
}
#endif
