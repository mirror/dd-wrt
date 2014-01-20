/*
 * index.c
 *
 * Copyright (C) 2005 - 2013 Sebastian Gottschall <gottschall@dd-wrt.com>
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

#include <broadcom.h>
#include <cymac.h>

void ej_show_index_setting(webs_t wp, int argc, char_t ** argv)
{
	char *type;

	type = GOZILA_GET(wp, "wan_proto");
	if (type == NULL)
		type = nvram_safe_get("wan_proto");
#ifdef HAVE_DSL_CPE_CONTROL
	do_ej(NULL, "index_atm.asp", wp, NULL);
#endif
	char ejname[32];
	snprintf(ejname, 31, "index_%s.asp", type);
	do_ej(NULL, ejname, wp, NULL);
}

void ej_get_wl_max_channel(webs_t wp, int argc, char_t ** argv)
{

	websWrite(wp, "%s", WL_MAX_CHANNEL);
}

void ej_get_wl_domain(webs_t wp, int argc, char_t ** argv)
{

#if COUNTRY == EUROPE
	websWrite(wp, "ETSI");
#elif COUNTRY == JAPAN
	websWrite(wp, "JP");
#else
	websWrite(wp, "US");
#endif
}

void ej_get_clone_mac(webs_t wp, int argc, char_t ** argv)
{
	char *c;
	int mac, which;
	int dofree = 0;

	ejArgs(argc, argv, "%d", &which);

	if (nvram_match("clone_wan_mac", "1"))
		c = nvram_safe_get("http_client_mac");
	else {
		if (nvram_match("def_hwaddr", "00:00:00:00:00:00")) {
			if (nvram_match("port_swap", "1"))
				c = strdup(nvram_safe_get("et1macaddr"));
			else
				c = strdup(nvram_safe_get("et0macaddr"));
			if (c) {
				MAC_ADD(c);
				dofree = 1;
			}
		} else
			c = nvram_safe_get("def_hwaddr");
	}

	if (c) {
		mac = get_single_mac(c, which);
		websWrite(wp, "%02X", mac);
		if (dofree)
			free(c);
	} else
		websWrite(wp, "00");
}

void macclone_onload(webs_t wp, char *arg)
{

	if (nvram_match("clone_wan_mac", "1"))
		websWrite(wp, arg);

	return;
}

void ej_atmsettings(webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_DSL_CPE_CONTROL
	char ejname[32];
	snprintf(ejname, 31, "index_%s_atm.asp", argv[0]);
	do_ej(NULL, ejname, wp, NULL);
#endif
}
