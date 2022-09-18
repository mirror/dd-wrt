/*
 * rsync.c
 *
 * Copyright (C) 2005 - 2019 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_RSYNC
#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <broadcom.h>

#include <rsync.h>
#include "fs_common.h"
void show_caption_pp(webs_t wp, const char *class, const char *caption, const char *pre, const char *post);

EJ_VISIBLE void ej_rsync_sharepaths(webs_t wp, int argc, char_t ** argv)
{

	struct fsentry *fs, *current;
	struct rsync_share *cs, *csnext;
	char buffer[64], number[4], perms[16];
	int found, rows = 0;

	fs = getfsentries();
	struct rsync_share *rsyncshares = getrsyncshares();

	// share count var
	for (cs = rsyncshares; cs; cs = cs->next) {
		rows++;
	}
	rows--;
	websWrite(wp, "<input type=\"hidden\" name=\"rsync_shares_count\" id=\"rsync_shares_count\" value=\"%d\">\n", rows);
	rows = 5;

	websWrite(wp, "<input type=\"hidden\" name=\"rsync_shares_count_limit\" id=\"rsync_shares_count_limit\" value=\"%d\">\n", rows);
	rows = 0;

	// table header
	websWrite(wp, "<table id=\"rsync_shares\" class=\"table\" summary=\"rsync share table\">\n");
	show_caption_pp(wp, NULL, "service.samba3_shares", "<thead><tr><th colspan=\"6\">", "</th></tr>\n");
	websWrite(wp, "<tr>\n");
	show_caption_pp(wp, NULL, "service.samba3_share_path", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "service.samba3_share_subdir", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "service.samba3_share_label", "<th>", "</th>\n");
	websWrite(wp, "<th style=\"width: 50px;\">&nbsp;</th>\n");
	websWrite(wp, "</tr></thead><tbody>\n");

	for (cs = rsyncshares; cs; cs = csnext) {

		if (rows == 0) {
			// dummy entry
			sprintf(buffer, "%s", "id=\"rsync_shares_row_template\" style=\"display: none;\"");
			//sprintf(number, '\0');
			number[0] = '\0';
		} else {
			sprintf(buffer, "id=\"rsync_shares_row_%d\"", rows);
			sprintf(number, "_%d", rows);
		}

		websWrite(wp, "		<tr %s>\n", buffer);

		// display filesystems to mount
		found = 0;
		//sprintf( perms, "");
		perms[0] = '\0';
		websWrite(wp, "			<td id=\"n_rsync_mp%s\" style=\"width: 17.816em;\"><select name=\"rsyncshare_mp%s\" id=\"rsyncshare_mp%s\" style=\"width: 100%%;\" >\n", number, number, number);
		websWrite(wp, "				<option value=\"\" rel=\"\">-</option>\n");
		//fprintf(stderr, "[SAMBA] FS %s:%s public:%d\n", cs->label, cs->mp, cs->public );
		for (current = fs; current; current = current->next) {
			if (strcmp(current->fstype, "squashfs")
			    && strcmp(current->fstype, "rootfs")
			    && strcmp(current->fstype, "proc")
			    && strcmp(current->fstype, "sysfs")
			    && strcmp(current->fstype, "sysdebug")
			    && strcmp(current->fstype, "debugfs")
			    && strcmp(current->fstype, "ramfs")
			    && strcmp(current->fstype, "tmpfs")
			    && strcmp(current->fstype, "devpts")
			    && strcmp(current->fstype, "usbfs")) {
				// adjust the rights
				if ( /*rows == 0 || */ !strcmp(current->mp, "")) {
					sprintf(buffer, "%s", "");
				} else if (!strcmp(current->perms, "rw")) {
					sprintf(buffer, "%s", "\"rw\",\"ro\"");
				} else {
					sprintf(buffer, "%s", "\"ro\"");
				}

				if (!strcmp(current->mp, cs->mp)) {
					found = 1;
					sprintf(perms, "%s", current->perms);
				}

				websWrite(wp,
					  "<option value=\"%s\" rel='{\"fstype\":\"%s\",\"perms\":[%s],\"avail\":1}' %s>%s</option>\n",
					  current->mp, current->fstype, buffer, strcmp(current->mp, cs->mp) ? "" : "selected=\"selected\"", current->mp);
			}
		}
		// fs not available -> add stored entry for display
		if (found == 0 && rows > 0) {
			websWrite(wp, "<option value=\"%s\" rel='{\"fstype\":\"\",\"perms\":[\"%s\"],\"avail\":0}' selected>[not available!]</option>\n", cs->mp, cs->mp);
		}
		websWrite(wp, "</select></td>\n");
		websWrite(wp,
			  "				<td style=\"width: 1%%;\"><input type=\"text\" name=\"rsyncshare_subdir%s\" id=\"rsyncshare_subdir%s\" value=\"%s\" style=\"width: 150px;\"/></td>\n",
			  number, number, cs->sd);
		websWrite(wp,
			  "				<td style=\"width: 1%%;\"><input type=\"text\" name=\"rsyncshare_label%s\" id=\"rsyncshare_label%s\" value=\"%s\" style=\"width: 100px;\" /></td>\n",
			  number, number, cs->label);
		websWrite(wp,
			  "					<script type=\"text/javascript\">document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input type=\\\"button\\\" class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" name=\\\"rsyncshare_del%s\\\" style=\\\"width: 100%%;\\\" onclick=\\\"removersyncShare(this);\\\" />\")</script>\n",
			  number);
		websWrite(wp, "</td>\n");
		websWrite(wp, "</tr>\n");

		rows++;
		csnext = cs->next;
		debug_free(cs);
	}

	websWrite(wp, "</tbody></table>\n");

	// add button
	websWrite(wp,
		  "<script type=\"text/javascript\">document.write(\"<div id=\\\"rsync_shares_add\\\" style=\\\"text-align: center;\\\"><input type=\\\"button\\\" class=\\\"button\\\" name=\\\"share_add\\\" value=\\\"\"+nas.shareadd+\"\\\" onclick=\\\"addrsyncShare();\\\" />\")</script></div>");

	for (current = fs; fs; current = fs) {
		fs = current->next;
		debug_free(current);
	}
}

#endif
