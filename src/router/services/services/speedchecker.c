
#ifdef HAVE_SPEEDCHECKER
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

void start_speedchecker(void)
{
	if (!nvram_matchi("speedchecker_enable", 1)) {
		return;
	} else {
		eval("sh", "/etc/config/speedchecker.startup");
		syslog(LOG_INFO, "speedchecker : client started\n");
	}

	return;
}

void stop_speedchecker(void)
{

	if (pidof("scc") > 0) {
		eval("kill", "-9", pidof("scc"));
	}

	return;
}
#endif
