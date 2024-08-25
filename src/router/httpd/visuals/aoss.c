/*
 * aoss.c
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
#ifdef HAVE_AOSS

#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <broadcom.h>

#if !defined(HAVE_IAS) && !defined(HAVE_BUFFALO)
char *nvram_selget(webs_t wp, char *name)
{
	if (wp->gozila_action) {
		char *buf = GOZILA_GET(wp, name);

		if (buf) {
			return buf;
			//              return sprintf("%s", buf);
		}
	}
	return nvram_safe_get(name);
}
#endif

int aoss_status(void)
{
	if (pidof("aoss") > 0) {
		return 1;
	}
	return 0;
}

void start_aoss(void)
{
	if (!aoss_status()) {
		eval("restart", "aoss");
	}
}

EJ_VISIBLE void ej_isChecked(webs_t wp, int argc, char_t **argv)
{
	fprintf(stderr, "[%s] %s %s\n", argv[0], argv[1], nvram_selget(wp, argv[0]));
	if (!strcmp(nvram_selget(wp, argv[0]), argv[1])) {
		websWrite(wp, " checked");
	}
	return;
}

EJ_VISIBLE void ej_ifnvram_match(webs_t wp, int argc, char_t **argv)
{
	if (!strcmp(nvram_selget(wp, argv[0]), argv[1])) {
		websWrite(wp, "%s", argv[2]);
	}
	return;
}

EJ_VISIBLE void ej_ifnvram_nmatch(webs_t wp, int argc, char_t **argv)
{
	if (strcmp(nvram_selget(wp, argv[0]), argv[1])) {
		websWrite(wp, "%s", argv[2]);
	}
	return;
}

EJ_VISIBLE void ej_ifaoss_possible(webs_t wp, int argc, char_t **argv)
{
#ifdef HAVE_WZRHPAG300NH
	if (!strcmp(argv[0], "yes")) {
		if (!strcmp(nvram_selget(wp, "wlan0_mode"), "ap") || !strcmp(nvram_selget(wp, "wlan0_mode"), "wdsap") ||
		    !strcmp(nvram_selget(wp, "wlan1_mode"), "ap") || !strcmp(nvram_selget(wp, "wlan1_mode"), "wdsap")) {
			websWrite(wp, "%s", argv[1]);
		}
	} else if (!strcmp(argv[0], "no")) {
		if (strcmp(nvram_selget(wp, "wlan0_mode"), "ap") && strcmp(nvram_selget(wp, "wlan0_mode"), "wdsap") &&
		    strcmp(nvram_selget(wp, "wlan1_mode"), "ap") && strcmp(nvram_selget(wp, "wlan1_mode"), "wdsap")) {
			websWrite(wp, "%s", argv[1]);
		}
	}
#else
	if (!strcmp(argv[0], "yes")) {
		if (!strcmp(nvram_selget(wp, "wlan0_mode"), "ap") || !strcmp(nvram_selget(wp, "wlan0_mode"), "wdsap")) {
			websWrite(wp, "%s", argv[1]);
		}
	} else if (!strcmp(argv[0], "no")) {
		if (strcmp(nvram_selget(wp, "wlan0_mode"), "ap") && strcmp(nvram_selget(wp, "wlan0_mode"), "wdsap")) {
			websWrite(wp, "%s", argv[1]);
		}
	}
#endif
	return;
}
#endif
