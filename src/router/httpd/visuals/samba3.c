/*
 * samba3.c
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
#ifdef HAVE_NAS_SERVER

#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <broadcom.h>

#include <samba3.h>
#include "fs_common.h"
void show_caption_pp(webs_t wp, const char *class, const char *caption, const char *pre, const char *post);
void show_caption_simple(webs_t wp, const char *caption);
void show_caption(webs_t wp, const char *class, const char *caption, const char *ext);

EJ_VISIBLE void ej_samba3_sharepaths(webs_t wp, int argc, char_t **argv)
{
	struct fsentry *fs, *current;
	struct samba3_share *cs, *csnext;
	char buffer[64], number[16], perms[16];
	int found, rows = 0;

	fs = getfsentries();
	struct samba3_share *samba3shares = getsamba3shares();

	// share count var
	for (cs = samba3shares; cs; cs = cs->next) {
		rows++;
	}
	rows--;
	websWrite(wp, "<input type=\"hidden\" name=\"samba_shares_count\" id=\"samba_shares_count\" value=\"%d\">\n", rows);
	rows = 5;

	websWrite(wp, "<input type=\"hidden\" name=\"samba_shares_count_limit\" id=\"samba_shares_count_limit\" value=\"%d\">\n",
		  rows);
	rows = 0;

	// table header
	websWrite(wp, "<table id=\"samba_shares\" class=\"table\" summary=\"samba share table\">\n");
	show_caption_pp(wp, NULL, "service.samba3_shares", "<tbody><tr><th colspan=\"6\">", "</th></tr>\n<tr>\n");
	show_caption_pp(wp, NULL, "service.samba3_share_path", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "service.samba3_share_subdir", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "service.samba3_share_label", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "service.samba3_share_public", "<th>", "</th>\n");
	show_caption_pp(wp, NULL, "service.samba3_share_access", "<th>", "</th>\n");
	websWrite(wp,
		  "<th class=\"center\" width=\"10%%\" ><script type=\"text/javascript\">Capture(share.actiontbl)</script></th>\n");

	for (cs = samba3shares; cs; cs = csnext) {
		if (rows == 0) {
			// dummy entry
			sprintf(buffer, "%s", "id=\"samba_shares_row_template\" style=\"display: none;\"");
			//sprintf(number, '\0');
			number[0] = '\0';
		} else {
			sprintf(buffer, "id=\"samba_shares_row_%d\"", rows);
			sprintf(number, "_%d", rows);
		}

		websWrite(wp, "<tr %s>\n", buffer);

		// display filesystems to mount
		found = 0;
		//sprintf( perms, "");
		perms[0] = '\0';
		websWrite(
			wp,
			"<td id=\"n_share_mp%s\" style=\"width: 17.816em;\"><select name=\"smbshare_mp%s\" id=\"smbshare_mp%s\" style=\"width: 100%%;\" onchange=\"setSambaShareAccessOptions(this);\">\n",
			number, number, number);
		websWrite(wp, "<option value=\"\" rel=\"\">-</option>\n");
		//fprintf(stderr, "[SAMBA] FS %s:%s public:%d\n", cs->label, cs->mp, cs->public );
		for (current = fs; current; current = current->next) {
			if (strcmp(current->fstype, "squashfs") && strcmp(current->fstype, "rootfs") &&
			    strcmp(current->fstype, "proc") && strcmp(current->fstype, "sysfs") &&
			    strcmp(current->fstype, "sysdebug") && strcmp(current->fstype, "debugfs") &&
			    strcmp(current->fstype, "ramfs") && strcmp(current->fstype, "tmpfs") &&
			    strcmp(current->fstype, "devpts") && strcmp(current->fstype, "usbfs")) {
				// adjust the rights
				if (/*rows == 0 || */ !strcmp(current->mp, "")) {
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

				websWrite(
					wp,
					"<option value=\"%s\" rel='{\"fstype\":\"%s\",\"perms\":[%s],\"avail\":1}' %s>%s</option>\n",
					current->mp, current->fstype, buffer,
					strcmp(current->mp, cs->mp) ? "" : "selected=\"selected\"", current->mp);
			}
		}
		// fs not available -> add stored entry for display
		if (found == 0 && rows > 0) {
			websWrite(
				wp,
				"<option value=\"%s\" rel='{\"fstype\":\"\",\"perms\":[\"%s\"],\"avail\":0}' selected>%s [not available!]</option>\n",
				cs->mp, cs->access_perms, cs->mp);
			sprintf(perms, "%s", cs->access_perms);
		}
		websWrite(wp, "</select></td>\n");
		websWrite(
			wp,
			"<td style=\"width: 1%%;\"><input type=\"text\" name=\"smbshare_subdir%s\" id=\"smbshare_subdir%s\" value=\"%s\" style=\"width: 120px;\" onChange=\"updateSambaUserShare(this);\" /></td>\n",
			number, number, cs->sd);
		websWrite(
			wp,
			"<td style=\"width: 1%%;\"><input type=\"text\" name=\"smbshare_label%s\" id=\"smbshare_label%s\" value=\"%s\" style=\"width: 100px;\" onChange=\"updateSambaUserShare(this);\" /></td>\n",
			number, number, cs->label);
		websWrite(
			wp,
			"<td style=\"width: 25px; vertical-align: middle;\" class=\"center\"><input type=\"checkbox\" name=\"smbshare_public%s\" id=\"smbshare_public%s\" value=\"1\" %s></td>\n",
			number, number, cs->public == 1 ? "checked" : "");
		websWrite(wp, "<td>\n");
		websWrite(wp,
			  "<select name=\"smbshare_access_perms%s\" id=\"smbshare_access_perms%s\" style=\"width: 100%%;\"%s>\n",
			  number, number, !strcmp(perms, "") ? " disabled" : "");
		if (rows == 0 || strcmp(perms, "")) {
			websWrite(wp, "<option value=\"rw\"%s>", !strcmp(cs->access_perms, "rw") ? " selected" : "");
			show_caption(wp, NULL, "nas.perm_rw", "</option>\n");
			websWrite(wp, "<option value=\"ro\"%s>", !strcmp(cs->access_perms, "ro") ? " selected" : "");
			show_caption(wp, NULL, "nas.perm_ro", "</option>\n");
		}
		websWrite(wp, "</select>\n");
		websWrite(wp, "<input type=\"hidden\" name=\"smbshare_access_perms_prev_%d\" value=\"%s\">\n", rows,
			  cs->access_perms);
		websWrite(wp, "</td>\n");
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" name=\\\"smbshare_del%s\\\" onclick=\\\"removeSambaShare(this);\\\" />\");\n//]]>\n</script>\n",
			number);
		websWrite(wp, "</td>\n");
		websWrite(wp, "</tr>\n");

		rows++;
		csnext = cs->next;
		debug_free(cs);
	}

	websWrite(wp, "</tbody></table><br />\n");

	// add button
	websWrite(
		wp,
		"<script type=\"text/javascript\">document.write(\"<div id=\\\"samba_shares_add\\\" class=\\\"center\\\"><input type=\\\"button\\\" class=\\\"button\\\" name=\\\"share_add\\\" value=\\\"\"+nas.shareadd+\"\\\" onclick=\\\"addSambaShare();\\\" />\");</script></div>");

	for (current = fs; fs; current = fs) {
		fs = current->next;
		debug_free(current);
	}
}

EJ_VISIBLE void ej_samba3_users(webs_t wp, int argc, char_t **argv)
{
	struct samba3_share *cs, *csnext;
	struct samba3_shareuser *csu, *csunext;
	struct samba3_user *samba3users, *cu, *cunext;
	char buffer[64], number[16];
	int rows = 0, usershares = 0;

	samba3users = getsamba3users();
	struct samba3_share *samba3shares = getsamba3shares();

	// share count var
	for (cu = samba3users; cu; cu = cu->next) {
		rows++;
	}
	rows--;
	websWrite(wp, "<input type=\"hidden\" name=\"samba_users_count\" id=\"samba_users_count\" value=\"%d\">\n", rows);
	rows = 10;

	websWrite(wp, "<input type=\"hidden\" name=\"samba_users_count_limit\" id=\"samba_users_count_limit\" value=\"%d\">\n",
		  rows);
	rows = 0;

	// table header
	websWrite(wp, "<table id=\"samba_users\" class=\"table\" summary=\"samba user table\">\n");

	show_caption_pp(wp, NULL, "service.samba3_users", "<tbody><tr><th colspan=\"6\">", "</th></tr>\n");
	websWrite(wp, "<tr>\n");
	websWrite(wp, "<th><script type=\"text/javascript\">Capture(nas.uname);</script></th>\n");
	show_caption_pp(wp, NULL, "nas.pwd", "<th style=\"width:200px;\">", "</th>\n");
	show_caption_pp(wp, NULL, "service.samba3_user_shares", "<th width=\"20%%\">", "</th>\n");
	websWrite(wp, "<th class=\"center\">Samba</th>\n");
	websWrite(wp, "<th class=\"center\">FTP</th>\n");

	websWrite(wp,
		  "<th class=\"center\" width=\"10%%\"><script type=\"text/javascript\">Capture(share.actiontbl)</script></th>\n");
	websWrite(wp, "</tr>\n");

	for (cu = samba3users; cu; cu = cunext) {
		if (rows == 0) {
			// dummy entry
			sprintf(buffer, "%s", "id=\"n_smbuser_template\" style=\"display: none;\"");
			//sprintf(number, '\0');
			number[0] = '\0';
		} else {
			sprintf(buffer, "id=\"samba_users_row_%d\"", rows);
			sprintf(number, "_%d", rows);
		}

		websWrite(wp, "<tr %s>\n", buffer);

		websWrite(wp, "<td id=\"n_smbuser_user\" width=\"1%%\">\n");
		websWrite(
			wp,
			"<input type=\"text\" name=\"smbuser_username%s\" autocomplete=\"new-password\" value=\"%s\" size=\"20\">\n",
			number, cu->username);
		websWrite(wp, "</td>\n");

		websWrite(wp, "<td id=\"n_smbuser_pass\">\n");
		websWrite(
			wp,
			"<input type=\"password\" autocomplete=\"new-password\" name=\"smbuser_password%s\" id=\"smbuser_password%s\" value=\"%s\" size=\"20\">&nbsp;\n",
			number, number, cu->password);
		//websWrite(wp, "                               <div style=\"float: left;padding-top: 2px;\">\n");
		websWrite(
			wp,
			"<input type=\"checkbox\" name=\"smbuser_password_unmask%s\" value=\"0\" onclick=\"setElementMask('smbuser_password' + this.name.substr(23, this.name.length - 23), this.checked);\" />",
			number, number);
		show_caption_simple(wp, "share.unmask");
		//websWrite(wp, "                               </div>\n");
		websWrite(wp, "</td>\n");

		//fprintf( stderr, "[USERS] %s:%s\n", cu->username, cu->password );
		if (rows == 0) {
			websWrite(wp, "<td id=\"n_smbuser_shareaccess\">\n");
			websWrite(
				wp,
				"<div id=\"n_smbuser_share\"><input type=\"checkbox\" value=\"1\">&nbsp;<span>&nbsp;</span></div>\n");
			websWrite(wp, "</td>\n");
			websWrite(wp, "<td style=\"width: 25px;\" class=\"center\">\n");
			websWrite(wp, "<input type=\"checkbox\" name=\"smbuser_samba%s\" value=\"1\">\n", number);
			websWrite(wp, "</td>\n");
			websWrite(wp, "<td style=\"width: 25px;\" class=\"center\">\n");
			websWrite(wp, "<input type=\"checkbox\" name=\"smbuser_ftp%s\" value=\"1\">\n", number);
			websWrite(wp, "</td>\n");
		} else {
			websWrite(wp, "<td id=\"n_smbuser_shareaccess\">\n");
			usershares = 0;
			for (cs = samba3shares; cs; cs = cs->next) {
				buffer[0] = '\0';
				for (csu = cs->users; csu; csu = csu->next) {
					if (!strcmp(csu->username, cu->username)) {
						//fprintf( stderr, "[USERSHARES] %s: %s\n", cs->label, csu->username );
						sprintf(buffer, " checked");
					}
				}
				if (usershares > 0) {
					websWrite(
						wp,
						"<div id=\"n_smbuser_share\"><input type=\"checkbox\" name=\"smbshare_%d_user_%d\"%s value=\"1\">&nbsp;<span>%s</span></div>\n",
						usershares, rows, buffer, cs->label);
				}
				usershares++;
			}
			websWrite(wp, "</td>\n");
			websWrite(wp, "<td style=\"width: 25px;\" class=\"center\">\n");
			websWrite(wp, "<input type=\"checkbox\" name=\"smbuser_samba%s\" value=\"1\" %s>\n", number,
				  cu->sharetype & SHARETYPE_SAMBA ? "checked" : "");
			websWrite(wp, "</td>\n");

			websWrite(wp, "<td style=\"width: 25px;\" class=\"center\">\n");
			websWrite(wp, "<input type=\"checkbox\" name=\"smbuser_ftp%s\" value=\"1\" %s>\n", number,
				  cu->sharetype & SHARETYPE_FTP ? "checked" : "");
			websWrite(wp, "</td>\n");
		}

		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" name=\\\"smbuser_del%s\\\" onclick=\\\"removeTableEntry('samba_users', this);\\\" />\");\n//]]>\n</script>\n",
			number);

		websWrite(wp, "</td>\n");

		websWrite(wp, "</tr>\n");
		rows++;

		cunext = cu->next;
		debug_free(cu);
	}
	for (cs = samba3shares; cs; cs = csnext) {
		for (csu = cs->users; csu; csu = csunext) {
			csunext = csu->next;
			debug_free(csu);
		}
		csnext = cs->next;
		debug_free(cs);
	}

	websWrite(wp, "</tbody></table><br />\n");

	// add button
	websWrite(
		wp,
		"<script type=\"text/javascript\">document.write(\"<div id=\\\"samba_users_add\\\" class=\\\"center\\\"><input type=\\\"button\\\" class=\\\"button\\\" name=\\\"user_add\\\" value=\\\"\"+nas.useradd+\"\\\" onclick=\\\"addSambaUser();\\\" />\");</script></div>");

	// free memory
}
#endif
