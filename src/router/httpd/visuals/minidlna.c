#ifdef HAVE_MINIDLNA
#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <math.h>

#include <broadcom.h>
#include <cymac.h>

#include <dlna.h>
#include "fs_common.h"

void ej_dlna_sharepaths(webs_t wp, int argc, char_t ** argv)
{

	struct fsentry *fs, *current;
	struct dlna_share *cs, *csnext;
	char buffer[64], number[4], perms[16];
	int found, rows = 0;

	fs = getfsentries();
	struct dlna_share *dlnashares = getdlnashares();

	// share count var
	for (cs = dlnashares; cs; cs = cs->next) {
		rows++;
	}
	rows--;
	websWrite(wp, "	<input type=\"hidden\" name=\"dlna_shares_count\" id=\"dlna_shares_count\" value=\"%d\">\n", rows);
	rows = 5;

	websWrite(wp, "	<input type=\"hidden\" name=\"dlna_shares_count_limit\" id=\"dlna_shares_count_limit\" value=\"%d\">\n", rows);
	rows = 0;

	// table header
	websWrite(wp, "	<table id=\"dlna_shares\" class=\"table center\" summary=\"dlna share table\">\n");
	websWrite(wp, "		<tr><th colspan=\"5\"><script type=\"text/javascript\">Capture(service.samba3_shares)</script></th></tr>\n");
	websWrite(wp, "		<tr>\n");
	websWrite(wp, "			<th><script type=\"text/javascript\">Capture(service.samba3_share_path)</script></th>\n");
	websWrite(wp, "			<th><script type=\"text/javascript\">Capture(service.dlna_type_audio)</script></th>\n");
	websWrite(wp, "			<th><script type=\"text/javascript\">Capture(service.dlna_type_video)</script></th>\n");
	websWrite(wp, "			<th><script type=\"text/javascript\">Capture(service.dlna_type_images)</script></th>\n");
	websWrite(wp, "			<th style=\"width: 50px;\">&nbsp;</th>\n");
	websWrite(wp, "		</tr>\n");

	for (cs = dlnashares; cs; cs = csnext) {

		if (rows == 0) {
			// dummy entry
			sprintf(buffer, "%s", "id=\"dlna_shares_row_template\" style=\"display: none;\"");
			//sprintf(number, '\0');
			number[0] = '\0';
		} else {
			sprintf(buffer, "id=\"dlna_shares_row_%d\"", rows);
			sprintf(number, "_%d", rows);
		}

		websWrite(wp, "		<tr %s>\n", buffer);

		// display filesystems to mount
		found = 0;
		//sprintf( perms, "");
		perms[0] = '\0';
		websWrite(wp,
			  "			<td id=\"n_dlna_mp%s\" style=\"width: 17.816em;\"><select name=\"dlnashare_mp%s\" id=\"dlnashare_mp%s\" style=\"width: 100%%;\" onchange=\"setDlnaShareAccessOptions(this);\">\n",
			  number, number, number);
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
					  "				<option value=\"%s\" rel='{\"fstype\":\"%s\",\"perms\":[%s],\"avail\":1}' %s>%s</option>\n",
					  current->mp, current->fstype, buffer, strcmp(current->mp, cs->mp) ? "" : "selected=\"selected\"", current->mp);
			}
		}
		// fs not available -> add stored entry for display
		if (found == 0 && rows > 0) {
			websWrite(wp, "				<option value=\"%s\" rel='{\"fstype\":\"\",\"perms\":[\"%s\"],\"avail\":0}' selected>[not available!]</option>\n", cs->mp, cs->mp);
		}
		websWrite(wp, "				</select></td>\n");
		websWrite(wp,
			  "				<td style=\"width: 25px; text-align: center;\"><input type=\"checkbox\" name=\"dlnashare_audio%s\" id=\"dlnashare_audio%s\" value=\"1\" %s></td>\n",
			  number, number, cs->types & TYPE_AUDIO ? "checked" : "");
		websWrite(wp,
			  "				<td style=\"width: 25px; text-align: center;\"><input type=\"checkbox\" name=\"dlnashare_video%s\" id=\"dlnashare_video%s\" value=\"1\" %s></td>\n",
			  number, number, cs->types & TYPE_VIDEO ? "checked" : "");
		websWrite(wp,
			  "				<td style=\"width: 25px; text-align: center;\"><input type=\"checkbox\" name=\"dlnashare_images%s\" id=\"dlnashare_images%s\" value=\"1\" %s></td>\n",
			  number, number, cs->types & TYPE_IMAGES ? "checked" : "");

		websWrite(wp, "				<td style=\"width: 50px; text-align: center;\">\n");
		websWrite(wp, "					<input type=\"button\" class=\"button\" name=\"dlnashare_del%s\" value=\"Remove\"  style=\"width: 100%%;\" onclick=\"removeDlnaShare(this);\">\n", number);
		websWrite(wp, "				</td>\n");
		websWrite(wp, "			</tr>\n");

		rows++;
		csnext = cs->next;
		free(cs);
	}

	websWrite(wp, "		</table>\n");

	// add button
	websWrite(wp, "<div id=\"dlna_shares_add\" style=\"text-align: center;\"><input type=\"button\" class=\"button\" name=\"share_add\" value=\"Add Share\" onclick=\"addDlnaShare();\" /></div>");

	for (current = fs; fs; current = fs) {
		fs = current->next;
		free(current);
	}
}

#endif
