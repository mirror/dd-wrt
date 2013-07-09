 
#ifdef HAVE_PRIVOXY
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

void start_privoxy(void)
{
	if (!nvram_match("privoxy_enable", "1"))
		return;

	eval("sh" , "/etc/config/privoxy.startup");;
	syslog(LOG_INFO, "Privoxy : privoxy started\n");
	return;
}

void stop_privoxy(void)
{
	stop_process("privoxy", "privoxy");
}
#endif