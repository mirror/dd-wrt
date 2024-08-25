/*
 * chaps.c
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
#include <string.h>
#include <broadcom.h>
#include <cy_conf.h>

/*
 * Example: name:pass:ip:on 
 */

static void show_chaps_table(webs_t wp, char *type, int which)
{
	char word[256];
	char *next, *wordlist;
	char new_user[200], new_pass[200];

	wordlist = nvram_safe_get("pppoeserver_chaps");

	foreach(word, wordlist, next)
	{
		if (which-- == 0) {
			GETENTRYBYIDX(user, word, 0);
			GETENTRYBYIDX(pass, word, 1);
			GETENTRYBYIDX(ip, word, 2);
			GETENTRYBYIDX(enable, word, 3);
			if (!user || !pass || !ip || !enable)
				continue;

			if (!strcmp(type, "user")) {
				httpd_filter_name(user, new_user, sizeof(new_user), GET);
				websWrite(wp, "%s", new_user);
			} else if (!strcmp(type, "pass")) {
				httpd_filter_name(pass, new_pass, sizeof(new_pass), GET);
				websWrite(wp, "%s", new_pass);
			} else if (!strcmp(type, "ip"))
				websWrite(wp, "%s", ip);
			else if (!strcmp(type, "enable")) {
				if (!strcmp(enable, "on"))
					websWrite(wp, "checked=\"checked\"");
			}
			return;
		}
	}
	if (!strcmp(type, "ip"))
		websWrite(wp, "0.0.0.0");
}

EJ_VISIBLE void ej_show_chaps(webs_t wp, int argc, char_t **argv)
{
	int i;
	char *count;
	int c = 0;

	count = nvram_safe_get("pppoeserver_chapsnum");
	if (count == NULL || *(count) == 0) {
		websWrite(
			wp,
			"<tr>\n<td colspan=\"4\" class=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n</tr>\n");
	}
	c = atoi(count);
	if (c <= 0) {
		websWrite(
			wp,
			"<tr>\n<td colspan=\"4\" class=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n</tr>\n");
	}
	for (i = 0; i < c; i++) {
		websWrite(
			wp,
			"<tr><td>\n<input maxlength=\"30\" size=\"30\" name=\"user%d\" onblur=\"valid_name(this,'Name')\" value=\"",
			i);
		show_chaps_table(wp, "user", i);
		websWrite(
			wp,
			"\" /></td>\n<td>\n<input maxlength=\"30\" size=\"30\" name=\"pass%d\" onblur=\"valid_name(this,'Name')\" value=\"",
			i);
		show_chaps_table(wp, "pass", i);
		websWrite(wp, "\" /></td>\n<td>\n<input class=\"num\" maxlength=\"15\" size=\"26\" name=\"ip%d\" value=\"", i);
		show_chaps_table(wp, "ip", i);
		websWrite(wp, "\" /></td>\n<td class=\"center\">\n<input type=\"checkbox\" value=\"on\" name=\"enable%d\" ", i);
		show_chaps_table(wp, "enable", i);
		websWrite(wp, " /></td>\n</tr>\n");
	}
	return;
}
