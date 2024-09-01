/*
 * overclocking.c
 *
 * Copyright (C) 2005 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>
#include <time.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/statfs.h>
#include <sys/sysinfo.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <broadcom.h>

#include <wlutils.h>
#include <bcmparams.h>
#include <dirent.h>
#include <netdb.h>
#include <utils.h>
#include <wlutils.h>
#include <bcmnvram.h>

#ifdef HAVE_OVERCLOCKING
#ifdef HAVE_HABANERO
static unsigned int qca4019_clocks[] = { 48, 200, 384, 413, 448, 500, 512, 537, 565, 597, 632, 672, 716, 768, 823, 896, 0 };
#endif
#ifdef HAVE_ALPINE
static unsigned int alpine_clocks[] = {
	533, 800, 1200, 1400, 1500, 1600, 1700, 1800, 1900, 2000, 0
}; //i tested up to 2200, but it hard on the limit
#else
static unsigned int type2_clocks[7] = { 200, 240, 252, 264, 300, 330, 0 };
static unsigned int type3_clocks[3] = { 150, 200, 0 };
static unsigned int type4_clocks[10] = { 192, 200, 216, 228, 240, 252, 264, 280, 300, 0 };
static unsigned int type7_clocks[10] = { 183, 187, 198, 200, 216, 225, 233, 237, 250, 0 };
static unsigned int type8_clocks[9] = { 200, 300, 400, 500, 600, 632, 650, 662, 0 };

static unsigned int type10_clocks[9] = { 200, 266, 300, 333, 400, 480, 500, 533, 0 };

#ifdef HAVE_NORTHSTAR
static unsigned int ns_type11_clocks[4] = { 600, 800, 900, 0 };
static unsigned int ns_type10_clocks[4] = { 600, 800, 1000, 0 };
static unsigned int ns_type9_clocks[3] = { 600, 800, 0 };
static unsigned int ns_type8_clocks[6] = { 600, 800, 1000, 1200, 1400, 0 };
static unsigned int ns_type7_clocks[4] = { 600, 800, 1000, 0 };
#endif
#endif
#endif


static unsigned int ipq6018_clocks[] = { 864000, 1056000, 1320000, 1440000, 15120000, 1800000,0};
static unsigned int ipq807x_clocks[] = { 1017600, 1382400, 1651200, 1843200, 1920000, 2208000,0};

EJ_VISIBLE void ej_show_clocks(webs_t wp, int argc, char_t **argv)
{
	int rev = cpu_plltype();
	unsigned int *c;
	char *oclk = nvram_safe_get("overclocking");
	int div = 1;
#if defined(HAVE_ALPINE)
	if (!*oclk) {
		oclk = "1700";
		nvram_set("clkfreq", "1700");
	}
	c = alpine_clocks;
#elif defined(HAVE_IPQ6018)
	div = 1000;
	int brand = getRouterBrand();
	char *defclock = "2208000";

	case ROUTER_LINKSYS_MR7350:
		defclock = "1440000";
		break;
	case ROUTER_LINKSYS_MX4200V1:
	case ROUTER_LINKSYS_MX4200V2:
	case ROUTER_LINKSYS_MX4300:
		defclock = "1324000";
		break;
	}
	if (!*oclk) {
		oclk = defclock;
		nvram_set("clkfreq", defclock);
	}
	c = alpine_clocks;
#elif defined(HAVE_HABANERO)
	if (!*oclk) {
		oclk = "716";
		nvram_set("clkfreq", "716");
	}
	c = qca4019_clocks;
#elif defined(HAVE_NORTHSTAR)
	switch (rev) {
	case 11:
		c = ns_type11_clocks;
		break;
	case 10:
		c = ns_type10_clocks;
		break;
	case 9:
		c = ns_type9_clocks;
		break;
	case 8:
		c = ns_type8_clocks;
		break;
	case 7:
		c = ns_type7_clocks;
		break;
	default:
		show_caption(wp, NULL, "management.clock_support", "\n");
		return;
	}
#else
	switch (rev) {
	case 2:
		c = type2_clocks;
		break;
	case 3:
		c = type3_clocks;
		break;
	case 4:
		c = type4_clocks;
		break;
	case 7:
		c = type7_clocks;
		break;
	case 8:
		c = type8_clocks;
		break;
	case 10:
		c = type10_clocks;
		break;
	default:
		show_caption(wp, NULL, "management.clock_support", "</div>\n");
		return;
	}
#endif

	int cclk = atoi(oclk);

	int i = 0;
	int in_clock_array = 0;

	//check if cpu clock list contains current clkfreq
	while (c[i] != 0) {
		if (c[i] == cclk) {
			in_clock_array = 1;
		}
		i++;
	}

	if (in_clock_array && nvram_exists("clkfreq")) {
		show_caption(wp, "label", "management.clock_frq", NULL);
		websWrite(wp, "<select name=\"overclocking\">\n");
		i = 0;
		while (c[i] != 0) {
			websWrite(wp, "<option value=\"%d\" %s >%d ", c[i], c[i] == cclk ? "selected=\"selected\"" : "", c[i]/div);
			show_caption(wp, NULL, "wl_basic.mhz", "</option>\n");
			i++;
		}
		websWrite(wp, "</select>\n</div>\n");
	} else {
		show_caption(wp, NULL, "management.clock_support", "</div>\n");
		fprintf(stderr, "CPU frequency list for rev: %d does not contain current clkfreq: %d.", rev, cclk);
	}
}
