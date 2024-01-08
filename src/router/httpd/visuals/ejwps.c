/*
 * ejwps.c
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
#ifdef HAVE_WPS

#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <broadcom.h>

EJ_VISIBLE void ej_get_wpsstatus(webs_t wp, int argc, char_t **argv)
{
	if (nvram_matchi("wps_status", 0))
		websWrite(wp, "Unconfigured");
	else if (nvram_matchi("wps_status", 1))
		websWrite(wp, "Configured");
	else if (nvram_matchi("wps_status", 2))
		websWrite(wp, "Waiting for Enrollee");
	else if (nvram_matchi("wps_status", 3))
		websWrite(wp, "Waiting for Registrar");
	else
		websWrite(wp, "Configured");
	return;
}

EJ_VISIBLE void ej_get_wpsconfigure(webs_t wp, int argc, char_t **argv)
{
	if (nvram_matchi("wps_status", 0))
		websWrite(wp, "<input class=\"button\" type=\"button\" value=\"%s\" onclick=\"to_configure(this.form);\" />",
			  live_translate(wp, "aoss.configure"));
	else
		websWrite(wp, "<input class=\"button\" type=\"button\" value=\"%s\" onclick=\"to_forcerelease(this.form);\" />",
			  live_translate(wp, "aoss.release"));
}
#endif
