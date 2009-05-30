#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>

#include <broadcom.h>

#include <stdio.h>

#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// #include "libbb.h"

void ej_show_ddns_status(webs_t wp, int argc, char_t ** argv)
{
	char buff[512];
	FILE *fp;
	char *enable = websGetVar(wp, "ddns_enable", NULL);

	if (!enable)
		enable = nvram_safe_get("ddns_enable");	// for first time

	if (strcmp(nvram_safe_get("ddns_enable"), enable))	// change
		// service
		websWrite(wp, " ");

	if (nvram_match("ddns_enable", "0"))	// only for no hidden page
	{
		websWrite(wp, "%s", live_translate("ddnsm.all_disabled"));
		return;
	}

	/*
	 * if (!check_wan_link (0)) { websWrite (wp, "<script
	 * type=\"text/javascript\">Capture(ddnsm.all_noip)</script>"); return; } 
	 */

	if ((fp = fopen("/tmp/ddns/ddns.log", "r"))) {
		/*
		 * Just dump the log file onto the web page 
		 */
		while (fgets(buff, sizeof(buff), fp)) {
			websWrite(wp, "%s <br />", buff);
		}
		fclose(fp);
	} else {
		websWrite(wp, "%s", live_translate("ddnsm.all_connecting"));
		return;
	}

	return;
}
