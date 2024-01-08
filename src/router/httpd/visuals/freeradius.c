/*
 * freeradius.c
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
#ifdef HAVE_FREERADIUS

#define VISUALSOURCE 1

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

#include <wlutils.h>
#include <bcmparams.h>
#include <dirent.h>
#include <netdb.h>
#include <utils.h>
#include <wlutils.h>
#include <bcmnvram.h>

EJ_VISIBLE void ej_show_certificate_status(webs_t wp, int argc, char_t **argv)
{
	char buf[256];
	int percent = 0;
	if (f_exists("/jffs/etc/freeradius/certs/dh"))
		percent += 60;
	if (f_exists("/jffs/etc/freeradius/certs/server.csr"))
		percent += 5;
	if (f_exists("/jffs/etc/freeradius/certs/server.key"))
		percent += 5;
	if (f_exists("/jffs/etc/freeradius/certs/ca.pem"))
		percent += 5;
	if (f_exists("/jffs/etc/freeradius/certs/ca.key"))
		percent += 5;
	if (f_exists("/jffs/etc/freeradius/certs/server.crt"))
		percent += 5;
	if (f_exists("/jffs/etc/freeradius/certs/server.p12"))
		percent += 5;
	if (f_exists("/jffs/etc/freeradius/certs/server.pem"))
		percent += 5;
	if (f_exists("/jffs/etc/freeradius/certs/ca.der"))
		percent += 5;

	if (percent == 100) {
		websWrite(
			wp,
			"<script type=\"text/javascript\">Capture(freeradius.gencerdone)</script>\n");
	} else {
		websWrite(wp, live_translate(wp, "freeradius.gencertime"),
			  percent);
	}
}

#include <radiusdb.h>

/*struct radiususer {
	unsigned int fieldlen;
	unsigned int usersize;
	unsigned char *user;
	unsigned int passwordsize;
	unsigned char *passwd;
	unsigned int downstream;
	unsigned int upstream;
//more fields can be added in future
};

struct radiusdb {
	unsigned int usercount;
	struct radiususer *users;
};
*/
EJ_VISIBLE void ej_show_radius_users(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, "<table class=\"table\" summary=\"Radius Users\">\n");

	websWrite(
		wp,
		"<thead><tr>\n"
		"<th><script type=\"text/javascript\">Capture(freeradius.username)</script></th>\n"
		"<th><script type=\"text/javascript\">Capture(freeradius.password)</script></th>\n"
		"<th><script type=\"text/javascript\">Capture(freeradius.downstream)</script></th>\n"
		"<th><script type=\"text/javascript\">Capture(freeradius.upstream)</script></th>\n"
		"<th><script type=\"text/javascript\">Capture(freeradius.expiration)</script></th>\n"
		"<th><script type=\"text/javascript\">Capture(freeradius.certtbl)</script></th>\n"
		"<th class=\"center\"><script type=\"text/javascript\">Capture(share.enabled)</script></th>\n"
		"<th class=\"center\" width=\"10%%\"><script type=\"text/javascript\">Capture(share.actiontbl)</script></th>\n"
		"</tr></thead><tbody>\n");

	unsigned int i;
	struct radiusdb *db = loadradiusdb();
	time_t tm;
	time(&tm);
	if (db != NULL) // empty
	{
		for (i = 0; i < db->usercount; i++) {
			websWrite(wp, "<tr>\n");
			char vlan_name[32];
			sprintf(vlan_name, "username%d", i);
			websWrite(
				wp,
				"<td><input name=\"%s\" size=\"8\" value=\"%s\" /></td>\n",
				vlan_name,
				(db->users[i].user != NULL &&
				 db->users[i].usersize) ?
					db->users[i].user :
					"");

			sprintf(vlan_name, "password%d", i);
			websWrite(
				wp,
				"<td><input name=\"%s\" size=\"8\" value=\"%s\" /></td>\n",
				vlan_name,
				(db->users[i].passwd != NULL &&
				 db->users[i].passwordsize) ?
					db->users[i].passwd :
					"");

			sprintf(vlan_name, "downstream%d", i);
			websWrite(
				wp,
				"<td><input class=\"num\" name=\"%s\" size=\"5\" value=\"%d\" class=\"center\" /></td>\n",
				vlan_name, db->users[i].downstream);

			sprintf(vlan_name, "upstream%d", i);
			websWrite(
				wp,
				"<td><input class=\"num\" name=\"%s\" size=\"5\" value=\"%d\" class=\"center\" /></td>\n",
				vlan_name, db->users[i].upstream);

			sprintf(vlan_name, "expiration%d", i);
			long expiration = 0; //never
			if (db->users[i].expiration) {
				long curtime = ((tm / 60) / 60) / 24; //in days
				expiration = db->users[i].expiration - curtime;
			}
			websWrite(
				wp,
				"<td><input class=\"num\" name=\"%s\" size=\"3\" value=\"%d\" class=\"center\" /></td>\n",
				vlan_name, expiration);

			websWrite(
				wp,
				"<td><script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + freeradius.certdown + \"\\\" style=\\\"margin: auto; display: block;\\\" onclick=\\\"openWindow('FreeRadiusCert-%d.asp', 400, 430,'Certificate');\\\" />\");\n//]]>\n</script></td>\n",
				i);

			sprintf(vlan_name, "enabled%d", i);
			websWrite(
				wp,
				"<td class=\"center\"><input type=\"checkbox\" name=\"%s\" value=\"1\" %s/></td>\n",
				vlan_name,
				db->users[i].enabled ? "checked=\"checked\"" :
						       "");

			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"user_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n</td>",
				i);
			websWrite(wp, "</tr>\n");
		}
		freeradiusdb(db);
	}
	websWrite(wp, "<tr>\n");
	websWrite(wp, "<td colspan=\"7\">&nbsp;</td>\n");
	websWrite(wp, "<td class=\"center\">\n");
	websWrite(
		wp,
		"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"add\\\" type=\\\"button\\\" aria-label=\\\"\" + sbutton.add + \"\\\" onclick=\\\"user_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	websWrite(wp, "</td>\n");
	websWrite(wp, "</tr>\n");

	websWrite(wp, "</table>\n<br />\n");
}

EJ_VISIBLE void ej_show_radius_clients(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, "<table class=\"table\" summary=\"Radius Clients\">\n");

	websWrite(
		wp,
		"<tr>\n"
		"<th>IP / NET</th>\n"
		"<th><script type=\"text/javascript\">Capture(freeradius.sharedkey)</script></th>\n"
		"<th width=\"10%%\" class=\"center\"><script type=\"text/javascript\">Capture(share.actiontbl)</script></th>\n"
		"</tr>\n");
	unsigned int i;
	struct radiusclientdb *db = loadradiusclientdb();
	if (db != NULL) // empty
	{
		for (i = 0; i < db->usercount; i++) {
			websWrite(wp, "<tr>\n");
			char vlan_name[32];
			sprintf(vlan_name, "client%d", i);
			websWrite(
				wp,
				"<td><input name=\"%s\" size=\"20\" value=\"%s\" /></td>\n",
				vlan_name,
				(db->users[i].client != NULL &&
				 db->users[i].clientsize) ?
					db->users[i].client :
					"");

			sprintf(vlan_name, "shared%d", i);
			websWrite(
				wp,
				"<td><input name=\"%s\" size=\"20\" value=\"%s\" /></td>\n",
				vlan_name,
				(db->users[i].passwd != NULL &&
				 db->users[i].passwordsize) ?
					db->users[i].passwd :
					"");

			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"client_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script></td>\n",
				i);
			websWrite(wp, "</tr>\n");
		}
		freeradiusclientdb(db);
	}
	websWrite(wp, "<tr>\n");
	websWrite(wp, "<td colspan=\"2\">&nbsp;</td>\n");
	websWrite(wp, "<td class=\"center\">\n");
	websWrite(
		wp,
		"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"add\\\" type=\\\"button\\\" aria-label=\\\"\" + sbutton.add + \"\\\" onclick=\\\"client_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	websWrite(wp, "</td>\n");
	websWrite(wp, "</tr>\n");
	websWrite(wp, "</tbody></table>\n<br />\n");
}

#endif
