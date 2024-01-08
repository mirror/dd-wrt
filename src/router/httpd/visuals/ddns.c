/*
 * ddns.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */
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

EJ_VISIBLE void ej_get_ddns_value(webs_t wp, int argc, char_t **argv)
{
	char *enable = GOZILA_GET(wp, "ddns_enable");
	if (argc == 0 || !enable || !strcmp(enable, "0"))
		return;
	if (!strcmp(enable, "1"))
		websWrite(wp, "%s", nvram_safe_get(argv[0]));
	else
		websWrite(wp, "%s", nvram_nget("%s_%s", argv[0], enable));
	return;
}

EJ_VISIBLE void ej_ddns_checked(webs_t wp, int argc, char_t **argv)
{
	char *enable = GOZILA_GET(wp, "ddns_enable");
	char var[32];
	if (argc < 2 || !enable || !strcmp(enable, "0"))
		return;

	if (!strcmp(enable, "1")) {
		strlcpy(var, argv[0], sizeof(var));
	} else {
		snprintf(var, sizeof(var) - 1, "%s_%s", argv[0], enable);
	}
	if (nvram_match(var, argv[1])) {
		websWrite(wp, "checked=\"checked\"");
	}
	return;
}

EJ_VISIBLE void ej_show_ddns_status(webs_t wp, int argc, char_t **argv)
{
	char buff[512];
	FILE *fp;
	char *enable = websGetVar(wp, "ddns_enable", NULL);

	if (!enable)
		enable = nvram_safe_get("ddns_enable"); // for first time

	if (strcmp(nvram_safe_get("ddns_enable"), enable)) // change
		// service
		websWrite(wp, " ");

	if (nvram_matchi("ddns_enable", 0)) // only for no hidden page
	{
		websWrite(wp, "%s", live_translate(wp, "ddnsm.all_disabled"));
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
		websWrite(wp, "%s", live_translate(wp, "ddnsm.all_connecting"));
		return;
	}

	return;
}
