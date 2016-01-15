#define VISUALSOURCE 1
/*
 * dd-wrt.c
 *
 * Copyright (C) 2005 - 2007 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
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
#include <netinet/in.h>
#include <arpa/inet.h>
#include <broadcom.h>
#include <cymac.h>
#include <wlutils.h>
#include <bcmparams.h>
#include <dirent.h>
#include <netdb.h>
#include <utils.h>
#include <wlutils.h>
#include <bcmnvram.h>
#include <pokerdb.h>

void ej_show_poker_users(webs_t wp, int argc, char_t ** argv)
{
	websWrite(wp, "<table class=\"table\" summary=\"Pokerspieler\">\n");

	websWrite(wp, "<tr>\n\
  					<th>Nutzer ID</th>\n\
  					<th>Nachname</th>\n\
  					<th>Vorname</th>\n\
  					<th>Dollar</th>\n\
  					<th>Kreditstatus</script></th>\n\
  					<th>K&auml;ufe</script></th>\n\
  					<th>&nbsp;</th>\n\
  				</tr>\n");

	unsigned int i;
	struct pokerdb *db = loadpokerdb();
	time_t tm;
	time(&tm);
	if (db != NULL)		// empty
	{
		for (i = 0; i < db->usercount; i++) {
			websWrite(wp, "<tr>\n");
			char vlan_name[32];
			sprintf(vlan_name, "userid%d", i);
			char id[32];
			sprintf(id, "%05d", db->users[i].userid);
			websWrite(wp, "<td><input name=\"%s\" size=\"8\" value=\"%s\" /></td>\n", vlan_name, id);

			sprintf(vlan_name, "username%d", i);
			websWrite(wp, "<td><input name=\"%s\" size=\"8\" value=\"%s\" /></td>\n", vlan_name, (db->users[i].user != NULL && db->users[i].usersize) ? db->users[i].user : "");

			sprintf(vlan_name, "family%d", i);
			websWrite(wp, "<td><input name=\"%s\" size=\"8\" value=\"%s\" /></td>\n", vlan_name, (db->users[i].family != NULL && db->users[i].familysize) ? db->users[i].family : "");

			sprintf(vlan_name, "coins%d", i);
			websWrite(wp, "<td><input class=\"num\" name=\"%s\" size=\"5\" value=\"%d\" /></td>\n", vlan_name, db->users[i].coins);

			sprintf(vlan_name, "credit%d", i);
			websWrite(wp, "<td><input class=\"num\" name=\"%s\" size=\"5\" value=\"%d\" /></td>\n", vlan_name, db->users[i].credit);

			sprintf(vlan_name, "buys%d", i);
			websWrite(wp, "<td><input class=\"num\" name=\"%s\" size=\"5\" value=\"%d\" /></td>\n", vlan_name, db->users[i].buys);

			websWrite(wp,
				  "<td><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"user_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n</td>",
				  i);
			websWrite(wp, "</tr>\n");
		}
		freepokerdb(db);
	}
	websWrite(wp, "</table>\n<br />\n");
	websWrite(wp,
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"user_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
}

void ej_show_poker_edit(webs_t wp, int argc, char_t ** argv)
{

	unsigned int i;
	struct pokerdb *db = loadpokerdb();

	websWrite(wp, "<input name=\"poker_userid\" size=\"8\" value=\"%s\"/>\n", nvram_safe_get("poker_userid"));
	websWrite(wp,
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"Nutzer Laden\\\" onclick=\\\"load_user_submit(this.form)\\\" />\");\n//]]>\n</script><br>\n");

	if (nvram_get("poker_userid") == NULL)
		return;
	if (db != NULL)		// empty
	{
		for (i = 0; i < db->usercount; i++) {
			if (db->users[i].userid == atoi(nvram_safe_get("poker_userid"))) {
				websWrite(wp, "<div class=\"setting\">\n");
				websWrite(wp, "<div class=\"label\">Bankstatus</div>\n");
				websWrite(wp, "<input name=\"savings\" size=\"8\" value=\"%d\" disabled=\"true\"/> Chips<br>\n", db->users[i].coins);
				websWrite(wp, "<div class=\"label\">Nachname</div>\n");
				websWrite(wp, "<input name=\"name\" size=\"12\" value=\"%s\" disabled=\"true\"/><br>\n", db->users[i].family);
				websWrite(wp, "<div class=\"label\">Vorname</div>\n");
				websWrite(wp, "<input name=\"name2\" size=\"12\" value=\"%s\" disabled=\"true\"/><br>\n", db->users[i].user);
				if (!(db->users[i].credit)) {
					if (db->users[i].coins <= 0)
						websWrite(wp,
							  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"500 Chips Kredit\\\" onclick=\\\"poker_credit_submit(this.form)\\\" />\");\n//]]>\n</script><br>\n");
				}
				if (db->users[i].buys_today <= (P_LIMIT - 500)) {
					websWrite(wp,
						  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"500 Chips Kaufen\\\" onclick=\\\"poker_buy_submit(this.form,500)\\\" />\");\n//]]>\n</script>\n");
				}
				if (db->users[i].buys_today <= (P_LIMIT - 1000)) {
					websWrite(wp,
						  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"1000 Chips Kaufen\\\" onclick=\\\"poker_buy_submit(this.form,1000)\\\" />\");\n//]]>\n</script>\n");
				}
				if (db->users[i].buys_today <= (P_LIMIT - 1500)) {
					websWrite(wp,
						  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"1500 Chips Kaufen\\\" onclick=\\\"poker_buy_submit(this.form,1500)\\\" />\");\n//]]>\n</script>\n");

				}
				websWrite(wp, "<br>\n");

				if (db->users[i].coins > 0) {
					websWrite(wp, "<input name=\"checkout_savings\" size=\"8\" value=\"0\" />\n");
					websWrite(wp,
						  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"Abbuchen\\\" onclick=\\\"poker_checkout_submit(this.form)\\\" />\");\n//]]>\n</script><br>\n");
				}
				websWrite(wp, "<input name=\"give_savings\" size=\"8\" value=\"0\" />\n");
				websWrite(wp,
					  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"Einzahlen\\\" onclick=\\\"poker_back_submit(this.form)\\\" />\");\n//]]>\n</script><br>\n");
			}
		}
		freepokerdb(db);
	}

	websWrite(wp, "</table>\n<br />\n");
}
