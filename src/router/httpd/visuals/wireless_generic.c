/*
 * wireless_generic.c
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
static char *UPTIME(int uptime, char *str, size_t len)
{
	int days, minutes;
	char str2[64] = { 0 };
	bzero(str, len);
	bzero(str2, 64);
	days = uptime / (60 * 60 * 24);
	if (days)
		snprintf(str2, sizeof(str2), "%d day%s, ", days,
			 (days == 1 ? "" : "s"));
	minutes = uptime / 60;
	if (*(str2))
		snprintf(str, len, "%s %d:%02d:%02d", str2, (minutes / 60) % 24,
			 minutes % 60, uptime % 60);
	else
		snprintf(str, len, "%d:%02d:%02d", (minutes / 60) % 24,
			 minutes % 60, uptime % 60);
	return str;
}

static int assoc_count[16];

static void assoc_count_prefix(webs_t wp, char *prefix)
{
	int count = getdevicecount();
	int i;
	char *next;
	char var[32];
	char *select = websGetVar(wp, "wifi_display", NULL);
	if (!select)
		select = nvram_safe_get("wifi_display");
	if (!*(select)) {
		websWrite(wp, "0");
		return;
	}
	if (count < 1) {
		websWrite(wp, "0");
		return;
	}
	char s[128];
	strcpy(s, select);
	char *p = strstr(s, ".sta");
	if (p)
		*p = 0;
	int idx = 0;
	// get index for interface selection
	for (i = 0; i < count; i++) {
		sprintf(var, "%s%d", prefix, i);
		if (!strcmp(var, s))
			goto done;
		idx++;
		char *names = nvram_nget("%s%d_vifs", prefix, i);
		foreach(var, names, next)
		{
			if (!strcmp(var, s))
				goto done;
			idx++;
		}
	}
	websWrite(wp, "0");
	return;
done:;
	websWrite(wp, "%d", assoc_count[idx]);
}

#ifndef HAVE_ATH9K
EJ_VISIBLE void ej_get_busy(webs_t wp, int argc, char_t **argv)
{
}

EJ_VISIBLE void ej_get_active(webs_t wp, int argc, char_t **argv)
{
}

EJ_VISIBLE void ej_show_busy(webs_t wp, int argc, char_t **argv)
{
}
#endif
