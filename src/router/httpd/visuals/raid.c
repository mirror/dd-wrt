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
#include <disc.h>

/* simply check for availability of the required filesystem tools */
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

static char *getfsname(char *drive)
{
	int fd, filekind;
	struct stat sb;
	SOURCE *s;
	SECTION section;
	char *fs;

	if (stat(drive, &sb) < 0) {
		return NULL;
	}
	filekind = analyze_stat(&sb, drive);
	if (filekind < 0)
		return NULL;

	fd = open(drive, O_RDONLY);
	if (fd < 0) {
		return NULL;
	}
	/* (try to) guard against TTY character devices */
	if (filekind == 2) {
		if (isatty(fd)) {
			return NULL;
		}
	}

	/* create a source */
	s = init_file_source(fd, filekind);
	section.source = s;
	section.pos = 0;
	section.size = s->size_known ? s->size : 0;
	section.flags = 0;
	set_discmessage_off();
	char *retvalue = "Empty";
	/* first look if the drive contains partions, if yes, ignore them to ensure to keep them */
	if (s->size == 0) {
		retvalue = NULL;
		goto ret;
	}
	if (detect_linux_raid(&section, -1)) {
		retvalue = NULL;
		goto ret;
	}
	if (detect_solaris_vtoc(&section, -1)) {
		retvalue = NULL;
		goto ret;
	}
	if (detect_solaris_disklabel(&section, -1)) {
		retvalue = NULL;
		goto ret;
	}
	if (detect_bsd_disklabel(&section, -1)) {
		retvalue = NULL;
		goto ret;
	}
	if (detect_linux_lvm(&section, -1)) {
		retvalue = NULL;
		goto ret;
	}
	if (detect_linux_lvm2(&section, -1)) {
		retvalue = NULL;
		goto ret;
	}
	if (detect_linux_swap(&section, -1)) {
		retvalue = NULL;
		goto ret;
	}
	if (detect_linux_misc(&section, -1)) {
		retvalue = NULL;	// ignore squashfs / cramfs drives since its likelly just the boot device
		goto ret;
	}
	if (detect_dos_partmap(&section, -1)) {
		retvalue = NULL;
		goto ret;
	}
	if (detect_gpt_partmap(&section, -1)) {
		retvalue = NULL;
		goto ret;
	}
	if (detect_apple_partmap(&section, -1)) {
		retvalue = NULL;
		goto ret;
	}

	if (detect_ntfs(&section, -1)) {
		retvalue = "NTFS";
		goto ret;
	}
	int fslevel = detect_ext234(&section, -1);
	if (fslevel == 2) {
		retvalue = "EXT2";
		goto ret;
	}
	if (fslevel == 3) {
		retvalue = "EXT3";
		goto ret;
	}
	if (fslevel == 4) {
		retvalue = "EXT4";
		goto ret;
	}
	if (detect_btrfs(&section, -1)) {
		retvalue = "BTRFS";
		goto ret;
	}
	if (detect_zfs(&section, -1)) {	// shall we skip that to to prevent damages?
		retvalue = "ZFS";
		goto ret;
	}
	if (detect_exfat(&section, -1)) {
		retvalue = "EXFAT";
		goto ret;
	}
	if (detect_hpfs(&section, -1)) {
		retvalue = "HPFS";
		goto ret;
	}
	if (detect_xfs(&section, -1)) {
		retvalue = "XFS";
		goto ret;
	}
	fslevel = detect_fat(&section, -1);
	if (fslevel == 1) {
		retvalue = "FAT12";
		goto ret;
	}
	if (fslevel == 2) {
		retvalue = "FAT16";
		goto ret;
	}
	if (fslevel == 3) {
		retvalue = "FAT32";
		goto ret;
	}
	if (detect_apple_volume(&section, -1)) {
		retvalue = "HFS";
	}

      ret:;
	set_discmessage_on();

	/* finish it up */
	close_source(s);
	return retvalue;
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
	int exfat = checkfs("exfat");
	int ntfs = checkfs("ntfs");
	char *drives = getAllDrives();
	char drive[128];
	char *dnext;
	while (1) {
		char *raid = nvram_nget("raid%d", i);
		char *raidname = nvram_nget("raidname%d", i);
		char *raidtype = nvram_nget("raidtype%d", i);
		char *raidlevel = nvram_nget("raidlevel%d", i);
		char *raidlz = nvram_nget("raidlz%d", i);
		char *raidfs = nvram_nget("raidfs%d", i);
		char *raiddedup = nvram_nget("raiddedup%d", i);
		if (!strlen(raidtype))
			break;
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<table class=\"table center\" summary=\"Raid\">\n");

		if (!strcmp(raidtype, "md")) {
			websWrite(wp,
				  "<tr>\n" "<th>Name</th>\n" "<th><script type=\"text/javascript\">Capture(ddns.typ)</script></th>\n" "<th>Level</th>\n"
				  "<th><script type=\"text/javascript\">Capture(nas.fs)</script></th>\n" "<th>&nbsp;</th>\n" "</tr>\n");
		}
		if (!strcmp(raidtype, "btrfs")) {
			websWrite(wp, "<tr>\n" "<th>Name</th>\n" "<th><script type=\"text/javascript\">Capture(ddns.typ)</script></th>\n" "<th>Level</th>\n " "<th>&nbsp;</th>\n" "</tr>\n");
		}
		if (!strcmp(raidtype, "zfs")) {
			websWrite(wp,
				  "<tr>\n" "<th>Name</th>\n" "<th><script type=\"text/javascript\">Capture(ddns.typ)</script></th>\n" "<th>Level</th>\n" "<th>Dedup</th>\n" "<th>LZ</th>\n" "<th>&nbsp;</th>\n" "</tr>\n");
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
				websWrite(wp, "document.write(\"<option value=\\\"ext2\\\" %s >EXT2</option>\");\n", !strcmp(raidfs, "ext2") ? "selected=\\\"selected\\\"" : "");
			if (ext3)
				websWrite(wp, "document.write(\"<option value=\\\"ext3\\\" %s >EXT3</option>\");\n", !strcmp(raidfs, "ext3") ? "selected=\\\"selected\\\"" : "");
			if (ext4)
				websWrite(wp, "document.write(\"<option value=\\\"ext4\\\" %s >EXT4</option>\");\n", !strcmp(raidfs, "ext4") ? "selected=\\\"selected\\\"" : "");
			if (exfat)
				websWrite(wp, "document.write(\"<option value=\\\"exfat\\\" %s >EXFAT</option>\");\n", !strcmp(raidfs, "exfat") ? "selected=\\\"selected\\\"" : "");
			if (xfs)
				websWrite(wp, "document.write(\"<option value=\\\"xfs\\\" %s >XFS</option>\");\n", !strcmp(raidfs, "xfs") ? "selected=\\\"selected\\\"" : "");
			if (btrfs)
				websWrite(wp, "document.write(\"<option value=\\\"btrfs\\\" %s >BTRFS</option>\");\n", !strcmp(raidfs, "btrfs") ? "selected=\\\"selected\\\"" : "");
			if (ntfs)
				websWrite(wp, "document.write(\"<option value=\\\"ntfs\\\" %s >NTFS</option>\");\n", !strcmp(raidfs, "ntfs") ? "selected=\\\"selected\\\"" : "");
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
		foreach(var, raid, next) {
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

	websWrite(wp, "<h2><script type=\"text/javascript\">Capture(nas.drivemanager)</script></h2>");
	websWrite(wp, "<fieldset>\n");
	websWrite(wp, "<table class=\"table center\" summary=\"Drive List\">\n");
	websWrite(wp, "<tr>\n" "<th><script type=\"text/javascript\">Capture(nas.drive)</script></th>\n" "<th><script type=\"text/javascript\">Capture(nas.fs)</script></th>\n" "<th>&nbsp;</th>\n" "</tr>\n");
	int idx = 0;
	foreach(drive, drives, dnext) {
		int canformat = 0;
		char *fs = getfsname(drive);
		if (!fs)
			continue;
		websWrite(wp, "<tr>\n");
		websWrite(wp, "<td>\n");
		websWrite(wp, "<input name=\"fs%d\" size=\"32\" value=\"%s\" />", idx, drive);
		websWrite(wp, "</td>\n");
		websWrite(wp, "<td>\n");
		websWrite(wp, "<select name=\"format%d\">\n", idx);
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");

		websWrite(wp, "document.write(\"<option value=\\\"unk\\\" >Unknown</option>\");\n");
		websWrite(wp, "document.write(\"<option value=\\\"ext2\\\" %s >EXT2</option>\");\n", !strcmp(fs, "EXT2") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"ext3\\\" %s >EXT3</option>\");\n", !strcmp(fs, "EXT3") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"ext4\\\" %s >EXT4</option>\");\n", !strcmp(fs, "EXT4") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"exfat\\\" %s >EXFAT</option>\");\n", !strcmp(fs, "EXFAT") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"xfs\\\" %s >XFS</option>\");\n", !strcmp(fs, "XFS") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"btrfs\\\" %s >BTRFS</option>\");\n", !strcmp(fs, "BTRFS") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"ntfs\\\" %s >NTFS</option>\");\n", !strcmp(fs, "NTFS") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"fat32\\\" %s >FAT32</option>\");\n", !strcmp(fs, "FAT32") ? "selected=\\\"selected\\\"" : "");
		websWrite(wp, "document.write(\"<option value=\\\"zfs\\\" %s >ZFS</option>\");\n", !strcmp(fs, "ZFS") ? "selected=\\\"selected\\\"" : "");

		websWrite(wp, "//]]>\n</script></select>\n");
		websWrite(wp, "</td>\n");

		websWrite(wp, "<td>\n");
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" name=\\\"reboot_button\\\" type=\\\"button\\\" value=\\\"\" + nas.format + \"\\\" onclick=\\\"drive_format_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			  idx);
		websWrite(wp, "</td>\n");

		websWrite(wp, "</tr>\n");
		idx++;
	}

	websWrite(wp, "</table>\n");

	websWrite(wp, "</fieldset>\n");
}

#endif
