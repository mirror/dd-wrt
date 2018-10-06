/*
 * raid.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#ifdef HAVE_RAID

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

static int checkfs(char *type)
{
	char fscheck[32];
	sprintf(fscheck, "/sbin/mkfs.%s", type);
	char fscheck2[32];
	sprintf(fscheck2, "/usr/bin/mkfs.%s", type);
	char fscheck3[32];
	sprintf(fscheck3, "/usr/sbin/mkfs.%s", type);

	FILE *p = fopen(fscheck, "rb");
	if (p) {
		fclose(p);
		return 1;
	}
	p = fopen(fscheck2, "rb");
	if (p) {
		fclose(p);
		return 1;
	}
	p = fopen(fscheck3, "rb");
	if (p) {
		fclose(p);
		return 1;
	}
	return 0;
}

void ej_show_raid(webs_t wp, int argc, char_t ** argv)
{
	websWrite(wp, "<h2><script type=\"text/javascript\">Capture(nas.raidmanager)</script></h2>");
	websWrite(wp, "<fieldset>\n");
	int i = 0;
	int xfs = checkfs("xfs");
	int ext2 = checkfs("ext2");
	int ext3 = checkfs("ext3");
	int ext4 = checkfs("ext4");
	int btrfs = checkfs("btrfs");
	while (1) {
		char *raid = nvram_nget("raid%d", i);
		char *raidname = nvram_nget("raidname%d", i);
		char *raidtype = nvram_nget("raidtype%d", i);
		char *raidlevel = nvram_nget("raidlevel%d", i);
		char *raidlz = nvram_nget("raidlz%d", i);
		char *raiddedup = nvram_nget("raiddedup%d", i);
		if (!strlen(raidtype))
			break;
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<table class=\"table center\" summary=\"Raid\">\n");

		if (!strcmp(raidtype, "md")) {
			websWrite(wp, "<tr>\n" "<th>Name</th>\n" "<th><script type=\"text/javascript\">Capture(ddns.typ)</script></th>\n" "<th>Level</th>\n" "<th><script type=\"text/javascript\">Capture(nas.fs)</script></th>\n" "<th>&nbsp;</th>\n" "</tr>\n");
		}
		if (!strcmp(raidtype, "btrfs")) {
			websWrite(wp, "<tr>\n" "<th>Name</th>\n" "<th><script type=\"text/javascript\">Capture(ddns.typ)</script></th>\n" "<th>Level</th>\n " "<th>&nbsp;</th>\n" "</tr>\n");
		}
		if (!strcmp(raidtype, "zfs")) {
			websWrite(wp, "<tr>\n" "<th>Name</th>\n" "<th><script type=\"text/javascript\">Capture(ddns.typ)</script></th>\n" "<th>Level</th>\n" "<th>Dedup</th>\n" "<th>LZ</th>\n" "<th>&nbsp;</th>\n" "</tr>\n");
		}

		websWrite(wp, "<tr>\n");
		websWrite(wp, "<td>\n");
		websWrite(wp, "<input name=\"raidname%d\" size=\"12\" value=\"%s\" />", i, raidname);
		websWrite(wp, "</td>\n");
		websWrite(wp, "<td>\n");
		websWrite(wp, "<select name=\"raidtype%d\">\n", i);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(wp, "document.write(\"<option value=\\\"md\\\" %s >Linux Raid</option>\");\n", !strcmp(raidtype, "md") ? "selected=\\\"selected\\\"" : "");
		if (btrfs)
			websWrite(wp, "document.write(\"<option value=\\\"btrfs\\\" %s >BTRFS</option>\");\n", !strcmp(raidtype, "btrfs") ? "selected=\\\"selected\\\"" : "");
#ifdef HAVE_ZFS
		websWrite(wp, "document.write(\"<option value=\\\"zfs\\\" %s >ZFS</option>\");\n", !strcmp(raidtype, "zfs") ? "selected=\\\"selected\\\"" : "");
#endif
		websWrite(wp, "//]]>\n</script></select>\n");
		websWrite(wp, "</td>\n");
		if (!strcmp(raidtype, "md")) {
			websWrite(wp, "<td>\n");
			websWrite(wp, "<select name=\"raidlevel%d\">\n", i);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
			websWrite(wp, "document.write(\"<option value=\\\"linear\\\" %s >Linear</option>\");\n", !strcmp(raidlevel, "linear") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"stripe\\\" %s >Stripe</option>\");\n", !strcmp(raidlevel, "stripe") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"mirror\\\" %s >\" + nas.mirror + \"</option>\");\n", !strcmp(raidlevel, "mirror") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"4\\\" %s >Raid4</option>\");\n", !strcmp(raidlevel, "4") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >Raid5</option>\");\n", !strcmp(raidlevel, "5") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"6\\\" %s >Raid6</option>\");\n", !strcmp(raidlevel, "6") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"10\\\" %s >Raid10</option>\");\n", !strcmp(raidlevel, "10") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "//]]>\n</script></select>\n");
			websWrite(wp, "</td>\n");
			websWrite(wp, "<td>\n");
			websWrite(wp, "<select name=\"raidfs%d\">\n", i);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
			if (ext2)
				websWrite(wp, "document.write(\"<option value=\\\"ext2\\\" %s >EXT2</option>\");\n", !strcmp(raidlevel, "ext2") ? "selected=\\\"selected\\\"" : "");
			if (ext3)
				websWrite(wp, "document.write(\"<option value=\\\"ext3\\\" %s >EXT3</option>\");\n", !strcmp(raidlevel, "ext3") ? "selected=\\\"selected\\\"" : "");
			if (ext4)
				websWrite(wp, "document.write(\"<option value=\\\"ext4\\\" %s >EXT4</option>\");\n", !strcmp(raidlevel, "ext4") ? "selected=\\\"selected\\\"" : "");
			if (xfs)
				websWrite(wp, "document.write(\"<option value=\\\"xfs\\\" %s >XFS</option>\");\n", !strcmp(raidlevel, "xfs") ? "selected=\\\"selected\\\"" : "");
			if (btrfs)
				websWrite(wp, "document.write(\"<option value=\\\"btrfs\\\" %s >BTRFS</option>\");\n", !strcmp(raidlevel, "btrfs") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "//]]>\n</script></select>\n");
			websWrite(wp, "</td>\n");
		}
		if (!strcmp(raidtype, "btrfs")) {
			websWrite(wp, "<td>\n");
			websWrite(wp, "<select name=\"raidlevel%d\">\n", i);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
			websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >Stripe</option>\");\n", !strcmp(raidlevel, "0") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + nas.mirror + \"</option>\");\n", !strcmp(raidlevel, "1") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >Raid5</option>\");\n", !strcmp(raidlevel, "5") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"6\\\" %s >Raid6</option>\");\n", !strcmp(raidlevel, "6") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"10\\\" %s >Raid10</option>\");\n", !strcmp(raidlevel, "10") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "//]]>\n</script></select>\n");
			websWrite(wp, "</td>\n");
		}
#ifdef HAVE_ZFS
		if (!strcmp(raidtype, "zfs")) {
			websWrite(wp, "<td>\n");
			websWrite(wp, "<select name=\"raidlevel%d\">\n", i);
			websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
			websWrite(wp, "document.write(\"<option value=\\\"0\\\" %s >Stripe</option>\");\n", !strcmp(raidlevel, "0") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"1\\\" %s >\" + nas.mirror + \"</option>\");\n", !strcmp(raidlevel, "1") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"5\\\" %s >Raid-Z1 (Raid 5)</option>\");\n", !strcmp(raidlevel, "5") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"6\\\" %s >Raid-Z2 (Raid 6)</option>\");\n", !strcmp(raidlevel, "6") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "document.write(\"<option value=\\\"z3\\\" %s >Raid-Z3</option>\");\n", !strcmp(raidlevel, "z3") ? "selected=\\\"selected\\\"" : "");
			websWrite(wp, "//]]>\n</script></select>\n");
			websWrite(wp, "</td>\n");
			websWrite(wp, "<td>\n");
			websWrite(wp, "<input type=\"checkbox\" name=\"raiddedup%d\" value=\"1\" %s/>", i, !strcmp(raiddedup, "1") ? "checked=\"checked\"" : "");
			websWrite(wp, "</td>\n");
			websWrite(wp, "<td>\n");
			websWrite(wp, "<input type=\"checkbox\" name=\"raidlz%d\" value=\"1\" %s/>", i, !strcmp(raidlz, "1") ? "checked=\"checked\"" : "");
			websWrite(wp, "</td>\n");
		}
#endif
		websWrite(wp, "<td>\n");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"raid_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  i);
		websWrite(wp, "</td>\n");
		websWrite(wp, "</tr>\n");
		websWrite(wp, "</table>\n");
		websWrite(wp, "<fieldset>\n");
		websWrite(wp, "<table class=\"table center\" summary=\"Raid Members\">\n");
		websWrite(wp, "<tr>\n" "<th><script type=\"text/javascript\">Capture(nas.raidmember)</script></th>\n" "<th>&nbsp;</th>\n" "</tr>\n");
		char var[128];
		char *next;
		int midx = 0;
		char *drives = getAllDrives();
		foreach(var, raid, next) {
			char drive[128];
			char *dnext;
			websWrite(wp, "<tr>\n");
			websWrite(wp, "<td>\n");
			websWrite(wp, "<select name=\"raid%dmember%d\">\n", i, midx);
			if (!strcmp(var, "none"))
				websWrite(wp, "<option value=\"none\">None</option>\n");
			if (drives) {
				foreach(drive, drives, dnext) {
					websWrite(wp, "<option value=\"%s\" %s >%s</option>\n", drive, !strcmp(drive, var) ? "selected=\"selected\"" : "", drive);
				}
			}

			websWrite(wp, "</td>\n");
			websWrite(wp, "<td>\n");
			websWrite(wp,
				  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.del + \"\\\" onclick=\\\"member_del_submit(this.form,%d, %d)\\\" />\");\n//]]>\n</script>\n",
				  i, midx);
			websWrite(wp, "</td>\n");
			websWrite(wp, "</tr>\n");
			midx++;
		}
		websWrite(wp, "</table>\n");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"member_add_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  i);
		websWrite(wp, "</fieldset>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" name=\\\"reboot_button\\\" type=\\\"button\\\" value=\\\"\" + nas.format + \"\\\" onclick=\\\"raid_format_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  i);
		i++;
	}
	websWrite(wp,
		  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"raid_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");

	websWrite(wp, "</fieldset>\n");
}

#endif
