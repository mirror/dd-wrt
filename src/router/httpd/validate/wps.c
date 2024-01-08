/*
 * wps.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
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
#ifdef HAVE_WPS
#define VALIDSOURCE 1

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else /* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <httpd.h>
#include <errno.h>
#endif /* WEBS */

#include <proto/ethernet.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <dd_defs.h>
#include <cy_conf.h>
// #ifdef EZC_SUPPORT
#include <ezc.h>
// #endif
#include <broadcom.h>
#include <wlutils.h>
#include <netdb.h>
#include <utils.h>
#include <stdarg.h>
#include <sha1.h>

struct variable **variables;

void wps_ap_register(webs_t wp)
{
	char *pin = websGetVar(wp, "wps_ap_pin", NULL);
	if (pin) {
		nvram_set("pincode", pin);
		eval("hostapd_cli", "-i", "wlan0", "wps_ap_pin", "set", pin, "300");
#ifdef HAVE_WZRHPAG300NH
		eval("hostapd_cli", "-i", "wlan1", "wps_ap_pin", "set", pin, "300");
#endif
		nvram_seti("wps_status", 2);
	}
}

void wps_register(webs_t wp)
{
	char *pin = websGetVar(wp, "wps_pin", NULL);
	if (pin) {
		eval("hostapd_cli", "-i", "wlan0", "wps_pin", "any", pin, "300");
#ifdef HAVE_WZRHPAG300NH
		eval("hostapd_cli", "-i", "wlan1", "wps_pin", "any", pin, "300");
#endif
		nvram_seti("wps_status", 3);
	}
}

void wps_forcerelease(webs_t wp)
{
	nvram_seti("wps_forcerelease", 1);
	addAction("wireless");
	nvram_seti("nowebaction", 1);
	service_restart();
}

void wps_configure(webs_t wp)
{
	nvram_seti("wps_status", 1);
	addAction("wireless");
	nvram_seti("nowebaction", 1);
	service_restart();
}
#endif
