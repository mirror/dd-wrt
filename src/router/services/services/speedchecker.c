
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
#include <prevision.h>

#define SCVERSION "1.1"

void start_speedchecker_init(void)
{
	char uuid[37];
	char change = 0;
	if (!nvram_get("speedchecker_uuid")) {
		if (getUUID(uuid)) {
			nvram_set("speedchecker_uuid", uuid);
			change = 1;
		}
	}

	if (!nvram_get("speedchecker_uuid2")) {
		if (getUUID(uuid)) {
			nvram_set("speedchecker_uuid2", uuid);
			change = 1;
		}
	}
	if (change)
		nvram_commit();

}

void start_speedchecker(void)
{
	start_speedchecker_init();
	if (nvram_matchi("speedchecker_enable", 1)) {
		sysprintf("SCC_JID=\"%s@xmpp.speedcheckerapi.com/%s|%s|ddwrt|%s|\" SCC_SRV=\"xmpp.speedcheckerapi.com\" SCC_STATS_IF=%s SCC_RNAME=\"%s\" scc &\n",	//
			  nvram_safe_get("speedchecker_uuid"),	//
			  SCVERSION,	//
			  PSVN_REVISION,	//
			  nvram_safe_get("os_version"),	//
			  get_wan_face(),	//
			  nvram_safe_get("DD_BOARD"));
		syslog(LOG_INFO, "speedchecker : client started\n");
	}

	return;
}

void stop_speedchecker(void)
{
	stop_process("scc", "speedchecker");
	return;
}
#endif
