
#ifdef HAVE_TRANSMISSION
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

void start_transmission(void)
{
	if (!nvram_match("transmission_enable", "1"))
		return;
	
	
	eval("mkdir", "-p", nvram_safe_get("transmission_dir"));
	
	eval("transmissiond", "--config-dir", nvram_safe_get("transmission_dir"));
	syslog(LOG_INFO, "transmission : transmissiond started\n");
	return;
}

void stop_transmission(void)
{
	stop_process("transmissiond", "transmissiond");
}
#endif
