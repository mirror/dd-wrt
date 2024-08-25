/*
 * emf.c
 *
 * Copyright (C) 2014 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
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
#ifdef HAVE_EMF
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <wlutils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

void start_emf(void)
{
#if 0
	int i;
	int cnt = get_wl_instances();
	char tmp[256];
	for (i = 0; i < cnt; i++) {
		if (nvram_nmatch("1", "wl%d_wmf_bss_enable", i))
			eval("emf", "start", getBridge(get_wl_instance_name(i), tmp));
//              else
//                      eval("emf", "stop", getBridge(get_wl_instance_name(i), tmp));

		char *next;
		char var[80];
		char *vifs = nvram_nget("wl%d_vifs", i);
		if (vifs != NULL) {
			foreach(var, vifs, next) {
				if (nvram_nmatch("1", "%s_wmf_bss_enable", var))
					eval("emf", "start", getBridge(var, tmp));
//                              else
//                                      eval("emf", "stop", getBridge(var, tmp));

			}
		}
	}
#endif
	return;
}

void stop_emf(void)
{
#if 0
	int i;
	int cnt = get_wl_instances();
	char tmp[256];
	for (i = 0; i < cnt; i++) {
		char *next;
		char var[80];
		char *vifs = nvram_nget("wl%d_vifs", i);
		if (vifs != NULL) {
			foreach(var, vifs, next) {
				eval("emf", "stop", getBridge(var, tmp));
			}
		}

		eval("emf", "stop", getBridge(get_wl_instance_name(i), tmp));
	}
	return;
#endif
}
#endif
