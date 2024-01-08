/*
 * raid.c
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
#ifdef HAVE_ZFS
	if (!strcmp(type, "zfs"))
		return 1;
#endif
	const char *fscheck[] = { "/sbin", "/usr/bin", "/usr/sbin" };

	int i;
	for (i = 0; i < sizeof(fscheck) / sizeof(const char *); i++) {
		char fsc[32];
		sprintf(fsc, "%s/mkfs.%s", fscheck[i], type);
		if (f_exists(fsc))
			return 1;
	}
	return 0;
}

EJ_VISIBLE void ej_support_fs(webs_t wp, int argc, char_t **argv)
{
	if (argc < 1) {
		websWrite(wp, "0");
		return;
	}
	websWrite(wp, "%d", checkfs(argv[0]));
}

#ifdef HAVE_RAID

static char *getfsname(char *drive)
{
	int fd, filekind;
	struct stat sb;
	SOURCE *s;
	SECTION section;
	char *fs;
#if defined(HAVE_X86) || defined(HAVE_NEWPORT) || defined(HAVE_RB600) || \
	defined(HAVE_EROUTER) && !defined(HAVE_WDR4900)
	char *root = getdisc();
	if (strlen(root) == 3) { // sdX
		char tmp[64];
		sprintf(tmp, "/dev/%s", root);
		if (!strncmp(drive, tmp, 8))
			goto err;
	}

	if (strlen(root) == 7) { // mmcblkX
		char tmp[64];
		sprintf(tmp, "/dev/%s", root);
		if (!strncmp(drive, tmp, 12))
			goto err;
	}
#endif
	if (stat(drive, &sb) < 0) {
		goto err;
	}
	filekind = analyze_stat(&sb, drive);
	if (filekind < 0)
		goto err;

	fd = open(drive, O_RDONLY);
	if (fd < 0) {
		goto err;
	}
	/* (try to) guard against TTY character devices */
	if (filekind == 2) {
		if (isatty(fd)) {
			goto err;
		}
	}

	/* create a source */
	s = init_file_source(fd, filekind);
	section.source = s;
	section.pos = 0;
	section.size = s->size_known ? s->size : 0;
	section.flags = 0;
	set_discmessage_off();
	char *retvalue = "Unknown";
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
		retvalue =
			NULL; // ignore squashfs / cramfs drives since its likelly just the boot device
		goto ret;
	}
	if (detect_dos_partmap(&section, -1)) {
		retvalue = "Partition";
		goto ret;
	}
	if (detect_gpt_partmap(&section, -1)) {
		retvalue = "Partition";
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
	if (detect_zfs(&section,
		       -1)) { // shall we skip that to to prevent damages?
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
	if (detect_apfs_volume(&section, -1)) {
		retvalue = "APFS";
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
		goto ret;
	}
	if (detect_blank(&section, -1)) {
		retvalue = "Empty";
	}

ret:;
#if defined(HAVE_X86) || defined(HAVE_NEWPORT) || defined(HAVE_RB600) || \
	defined(HAVE_EROUTER) && !defined(HAVE_WDR4900)
	free(root);
#endif
	set_discmessage_on();

	/* finish it up */
	close_source(s);
	return retvalue;
err:;
#if defined(HAVE_X86) || defined(HAVE_NEWPORT) || defined(HAVE_RB600) || \
	defined(HAVE_EROUTER) && !defined(HAVE_WDR4900)
	free(root);
#endif
	return NULL;
}

static int ismember(char *name)
{
	int i = 0;
	while (1) {
		char *raid = nvram_nget("raid%d", i);
		char *raidtype = nvram_nget("raidtype%d", i);
		if (!*(raidtype))
			return 0;
		char var[128];
		char *next;
		foreach(var, raid, next)
		{
			if (!strcmp(name, var))
				return 1;
		}
		i++;
	}
	return 0;
}

EJ_VISIBLE void ej_show_raid(webs_t wp, int argc, char_t **argv)
{
	websWrite(
		wp,
		"<h2><script type=\"text/javascript\">Capture(nas.raidmanager)</script></h2>");
	websWrite(
		wp,
		"<fieldset>\n<legend><script type=\"text/javascript\">Capture(nas.drivemanager)</script></legend>\n");
	int i = 0;
	int xfs = checkfs("xfs");
	int ext2 = checkfs("ext2");
	int ext3 = checkfs("ext3");
	int ext4 = checkfs("ext4");
	int apfs = checkfs("apfs");
	int btrfs = checkfs("btrfs");
	int exfat = checkfs("exfat");
	int ntfs = checkfs("ntfs");
	int fat32 = checkfs("fat");
	int zfs = checkfs("zfs");
	char *drives = getAllDrives();
	char drive[128];
	char *dnext;
	while (1) {
		char *raid = nvram_nget("raid%d", i);
		char *raidname = nvram_nget("raidname%d", i);
		char *raidtype = nvram_nget("raidtype%d", i);
		char *raidlevel = nvram_nget("raidlevel%d", i);
		char *raidlz = nvram_nget("raidlz%d", i);
		char *raidlzlevel = nvram_nget("raidlzlevel%d", i);
		char *raidfs = nvram_nget("raidfs%d", i);
		char *raiddedup = nvram_nget("raiddedup%d", i);
		if (!*(raidtype))
			break;
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp,
			  "<table class=\"table raid\" summary=\"Raid\">\n");
		if (!strcmp(raidtype, "md")) {
			websWrite(
				wp,
				"<thead><tr>\n"
				"<th><script type=\"text/javascript\">Capture(nas.raidnametbl)</script></th>\n"
				"<th><script type=\"text/javascript\">Capture(ddns.typ)</script></th>\n"
				"<th><script type=\"text/javascript\">Capture(nas.raidleveltbl)</script></th>\n"
				"<th><script type=\"text/javascript\">Capture(nas.fs)</script></th>\n"
				"<th class=\"center\" width=\"10%%\"><script type=\"text/javascript\">Capture(share.actiontbl)</script></th>\n"
				"</tr></thead>\n");
		}
		if (!strcmp(raidtype, "btrfs")) {
			if (!strcmp(raidlz, "gzip") || !strcmp(raidlz, "zstd"))
				websWrite(
					wp,
					"<thead><tr>\n"
					"<th><script type=\"text/javascript\">Capture(nas.raidnametbl)</script></th>\n"
					"<th><script type=\"text/javascript\">Capture(ddns.typ)</script></th>\n"
					"<th><script type=\"text/javascript\">Capture(nas.raidleveltbl)</script></th>\n"
					"<th><script type=\"text/javascript\">Capture(nas.compression)</script></th>\n"
					"<th><script type=\"text/javascript\">Capture(nas.raidleveltbl)</script></th>\n"
					"<th class=\"center\"><script type=\"text/javascript\">Capture(share.actiontbl)</script></th>\n"
					"</tr></thead>\n");
			else
				websWrite(
					wp,
					"<thead><tr>\n"
					"<th><script type=\"text/javascript\">Capture(nas.raidnametbl)</script></th>\n"
					"<th><script type=\"text/javascript\">Capture(ddns.typ)</script></th>\n"
					"<th><script type=\"text/javascript\">Capture(nas.raidleveltbl)</script></th>\n"
					"<th><script type=\"text/javascript\">Capture(nas.compression)</script></th>\n"
					"<th class=\"center\" colspan=\"2\" width=\"10%%\"><script type=\"text/javascript\">Capture(share.actiontbl)</script></th>\n"
					"</tr></thead>\n");
		}
		if (!strcmp(raidtype, "zfs")) {
			if (!strcmp(raidlz, "gzip") || !strcmp(raidlz, "zstd"))
				websWrite(
					wp,
					"<thead><tr>\n"
					"<th><script type=\"text/javascript\">Capture(nas.raidnametbl)</script></th>\n"
					"<th><script type=\"text/javascript\">Capture(ddns.typ)</script></th>\n"
					"<th><script type=\"text/javascript\">Capture(nas.raidleveltbl)</script></th>\n"
					"<th><script type=\"text/javascript\">Capture(nas.raiddeduptbl)</script></th>\n"
					"<th><script type=\"text/javascript\">Capture(nas.compression)</script></th>\n"
					"<th><script type=\"text/javascript\">Capture(nas.raidleveltbl)</script></th>\n"
					"<th class=\"center\" colspan=\"2\" width=\"10%%\"><script type=\"text/javascript\">Capture(share.actiontbl)</script></th>\n"
					"</tr></thead>\n");
			else
				websWrite(
					wp,
					"<thead><tr>\n"
					"<th><script type=\"text/javascript\">Capture(nas.raidnametbl)</script></th>\n"
					"<th><script type=\"text/javascript\">Capture(ddns.typ)</script></th>\n"
					"<th><script type=\"text/javascript\">Capture(nas.raidleveltbl)</script></th>\n"
					"<th><script type=\"text/javascript\">Capture(nas.raiddeduptbl)</script></th>\n"
					"<th><script type=\"text/javascript\">Capture(nas.compression)</script></th>\n"
					"<th class=\"center\" colspan=\"2\" width=\"10%%\"><script type=\"text/javascript\">Capture(share.actiontbl)</script></th>\n"
					"</tr></thead>\n");
		}

		websWrite(wp, "<tbody><tr>\n"
			      "<td>\n");
		websWrite(
			wp,
			"<input name=\"raidname%d\" size=\"20\" value=\"%s\" />",
			i, raidname);
		websWrite(wp, "</td>\n"
			      "<td>\n");
		websWrite(
			wp,
			"<select name=\"raidtype%d\" onchange=\"raid_save_submit(this.form)\">\n",
			i);
		websWrite(wp,
			  "<script type=\"text/javascript\">\n//<![CDATA[\n");
		websWrite(
			wp,
			"document.write(\"<option value=\\\"md\\\" %s >Linux Raid</option>\");\n",
			!strcmp(raidtype, "md") ? "selected=\\\"selected\\\"" :
						  "");
		if (btrfs)
			websWrite(
				wp,
				"document.write(\"<option value=\\\"btrfs\\\" %s >BTRFS</option>\");\n",
				!strcmp(raidtype, "btrfs") ?
					"selected=\\\"selected\\\"" :
					"");
#ifdef HAVE_ZFS
		websWrite(
			wp,
			"document.write(\"<option value=\\\"zfs\\\" %s >ZFS</option>\");\n",
			!strcmp(raidtype, "zfs") ? "selected=\\\"selected\\\"" :
						   "");
#endif
		websWrite(wp, "//]]>\n</script></select>\n"
			      "</td>\n");
		if (!strcmp(raidtype, "md")) {
			websWrite(wp, "<td>\n");
			websWrite(wp, "<select name=\"raidlevel%d\">\n", i);
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"linear\\\" %s >Linear</option>\");\n",
				!strcmp(raidlevel, "linear") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"0\\\" %s >Stripe</option>\");\n",
				!strcmp(raidlevel, "0") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"1\\\" %s >\" + nas.mirror + \"</option>\");\n",
				!strcmp(raidlevel, "1") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"4\\\" %s >Raid4</option>\");\n",
				!strcmp(raidlevel, "4") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"5\\\" %s >Raid5</option>\");\n",
				!strcmp(raidlevel, "5") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"6\\\" %s >Raid6</option>\");\n",
				!strcmp(raidlevel, "6") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"10\\\" %s >Raid10</option>\");\n",
				!strcmp(raidlevel, "10") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(wp, "//]]>\n</script></select>\n");
			websWrite(wp, "</td>\n"
				      "<td>\n");
			websWrite(wp, "<select name=\"raidfs%d\">\n", i);
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n");
			if (ext2)
				websWrite(
					wp,
					"document.write(\"<option value=\\\"ext2\\\" %s >EXT2</option>\");\n",
					!strcmp(raidfs, "ext2") ?
						"selected=\\\"selected\\\"" :
						"");
			if (ext3)
				websWrite(
					wp,
					"document.write(\"<option value=\\\"ext3\\\" %s >EXT3</option>\");\n",
					!strcmp(raidfs, "ext3") ?
						"selected=\\\"selected\\\"" :
						"");
			if (ext4)
				websWrite(
					wp,
					"document.write(\"<option value=\\\"ext4\\\" %s >EXT4</option>\");\n",
					!strcmp(raidfs, "ext4") ?
						"selected=\\\"selected\\\"" :
						"");
			if (exfat)
				websWrite(
					wp,
					"document.write(\"<option value=\\\"exfat\\\" %s >EXFAT</option>\");\n",
					!strcmp(raidfs, "exfat") ?
						"selected=\\\"selected\\\"" :
						"");
			if (xfs)
				websWrite(
					wp,
					"document.write(\"<option value=\\\"xfs\\\" %s >XFS</option>\");\n",
					!strcmp(raidfs, "xfs") ?
						"selected=\\\"selected\\\"" :
						"");
			if (btrfs)
				websWrite(
					wp,
					"document.write(\"<option value=\\\"btrfs\\\" %s >BTRFS</option>\");\n",
					!strcmp(raidfs, "btrfs") ?
						"selected=\\\"selected\\\"" :
						"");
			if (ntfs)
				websWrite(
					wp,
					"document.write(\"<option value=\\\"ntfs\\\" %s >NTFS</option>\");\n",
					!strcmp(raidfs, "ntfs") ?
						"selected=\\\"selected\\\"" :
						"");
			if (zfs)
				websWrite(
					wp,
					"document.write(\"<option value=\\\"zfs\\\" %s >ZFS</option>\");\n",
					!strcmp(raidfs, "zfs") ?
						"selected=\\\"selected\\\"" :
						"");
			if (apfs)
				websWrite(
					wp,
					"document.write(\"<option value=\\\"apfs\\\" %s >APFS</option>\");\n",
					!strcmp(raidfs, "apfs") ?
						"selected=\\\"selected\\\"" :
						"");
			websWrite(wp, "//]]>\n</script></select>\n"
				      "</td>\n");
		}
		if (!strcmp(raidtype, "btrfs")) {
			websWrite(wp, "<td>\n");
			websWrite(wp, "<select name=\"raidlevel%d\">\n", i);
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"0\\\" %s >Stripe</option>\");\n",
				!strcmp(raidlevel, "0") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"1\\\" %s >\" + nas.mirror + \"</option>\");\n",
				!strcmp(raidlevel, "1") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"5\\\" %s >Raid5</option>\");\n",
				!strcmp(raidlevel, "5") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"6\\\" %s >Raid6</option>\");\n",
				!strcmp(raidlevel, "6") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"10\\\" %s >Raid10</option>\");\n",
				!strcmp(raidlevel, "10") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(wp, "//]]>\n</script></select>\n");
			websWrite(wp, "</td>\n"
				      "<td>\n");
			websWrite(
				wp,
				"<select name=\"raidlz%d\" onchange=\"raid_save_submit(this.form)\">\n",
				i);
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"0\\\" %s >\" + share.off + \"</option>\");\n",
				!strcmp(raidlz, "0") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"lzo\\\" %s >lzo</option>\");\n",
				!strcmp(raidlz, "lzo") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"gzip\\\" %s >gzip</option>\");\n",
				!strcmp(raidlz, "gzip") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"zstd\\\" %s >zstd</option>\");\n",
				!strcmp(raidlz, "zstd") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(wp, "//]]>\n</script></select>\n"
				      "</td>\n");
			if (!strcmp(raidlz, "gzip") ||
			    !strcmp(raidlz, "zstd")) {
				websWrite(wp, "<td>\n");
				websWrite(wp,
					  "<select name=\"raidlzlevel%d\">\n",
					  i);
				websWrite(
					wp,
					"<script type=\"text/javascript\">\n//<![CDATA[\n");
				int level;
				int maxlevel = 19;
				if (!strcmp(raidlz, "gzip"))
					maxlevel = 10;

				for (level = 0; level < maxlevel; level++) {
					char num[16];
					sprintf(num, "%d", level);
					if (!level)
						websWrite(
							wp,
							"document.write(\"<option value=\\\"0\\\" %s >\" + share.deflt + \"</option>\");\n",
							!strcmp(raidlzlevel,
								"0") ?
								"selected=\\\"selected\\\"" :
								"");
					else
						websWrite(
							wp,
							"document.write(\"<option value=\\\"%d\\\" %s >%d</option>\");\n",
							level,
							!strcmp(raidlzlevel,
								num) ?
								"selected=\\\"selected\\\"" :
								"",
							level);
				}
				websWrite(wp, "//]]>\n</script></select>\n"
					      "</td>\n");
			}
		}
#ifdef HAVE_ZFS
		if (!strcmp(raidtype, "zfs")) {
			websWrite(wp, "<td>\n");
			websWrite(wp, "<select name=\"raidlevel%d\">\n", i);
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"0\\\" %s >Stripe</option>\");\n",
				!strcmp(raidlevel, "0") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"1\\\" %s >\" + nas.mirror + \"</option>\");\n",
				!strcmp(raidlevel, "1") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"5\\\" %s >Raid-Z1</option>\");\n",
				!strcmp(raidlevel, "5") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"6\\\" %s >Raid-Z2</option>\");\n",
				!strcmp(raidlevel, "6") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"z3\\\" %s >Raid-Z3</option>\");\n",
				!strcmp(raidlevel, "z3") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(wp, "//]]>\n</script></select>\n");
			websWrite(wp, "</td>\n"
				      "<td class=\"center\">\n");
			websWrite(
				wp,
				"<input type=\"checkbox\" name=\"raiddedup%d\" value=\"1\" %s/>",
				i,
				!strcmp(raiddedup, "1") ?
					"checked=\"checked\"" :
					"");
			websWrite(wp, "</td>\n"
				      "<td>\n");
			websWrite(
				wp,
				"<select name=\"raidlz%d\" onchange=\"raid_save_submit(this.form)\">\n",
				i);
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"0\\\" %s >\" + share.off + \"</option>\");\n",
				!strcmp(raidlz, "0") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"gzip\\\" %s >gzip</option>\");\n",
				!strcmp(raidlz, "gzip") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"lz4\\\" %s >lz4</option>\");\n",
				!strcmp(raidlz, "lz4") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"lzjb\\\" %s >lzjb</option>\");\n",
				!strcmp(raidlz, "lzjb") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"zle\\\" %s >zle</option>\");\n",
				!strcmp(raidlz, "zle") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"zstd\\\" %s >zstd</option>\");\n",
				!strcmp(raidlz, "zstd") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(wp, "//]]>\n</script></select>\n"
				      "</td>\n");

			if (!strcmp(raidlz, "gzip") ||
			    !strcmp(raidlz, "zstd")) {
				websWrite(wp, "<td>\n");
				websWrite(wp,
					  "<select name=\"raidlzlevel%d\">\n",
					  i);
				websWrite(
					wp,
					"<script type=\"text/javascript\">\n//<![CDATA[\n");
				int level;
				int maxlevel = 19;
				if (!strcmp(raidlz, "gzip"))
					maxlevel = 10;

				for (level = 0; level < maxlevel; level++) {
					char num[16];
					sprintf(num, "%d", level);
					if (!level)
						websWrite(
							wp,
							"document.write(\"<option value=\\\"0\\\" %s >\" + share.deflt + \"</option>\");\n",
							!strcmp(raidlzlevel,
								"0") ?
								"selected=\\\"selected\\\"" :
								"");
					else
						websWrite(
							wp,
							"document.write(\"<option value=\\\"%d\\\" %s >%d</option>\");\n",
							level,
							!strcmp(raidlzlevel,
								num) ?
								"selected=\\\"selected\\\"" :
								"",
							level);
				}
				websWrite(wp, "//]]>\n</script></select>\n"
					      "</td>\n");
			}
			websWrite(
				wp,
				"<td class=\"center\" style=\"margin-top: 2px; display: block\">\n");
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.scrub + \"\\\" onclick=\\\"zfs_scrub_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
				i);
			websWrite(wp, "</td>\n");
		}
#endif
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"raid_del_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			i);
		websWrite(
			wp,
			"</td>\n"
			"</tr></tbody>\n"
			"</table>\n"
			"<fieldset>\n<legend><script type=\"text/javascript\">Capture(bmenu.adminman)</script></legend>\n"
			"<table class=\"table\" summary=\"Raid Members\">\n"
			"<thead><tr>\n"
			"<th><script type=\"text/javascript\">Capture(nas.raidmember)</script></th>\n"
			"<th class=\"center\" width=\"10%%\" ><script type=\"text/javascript\">Capture(share.actiontbl)</script></th>\n"
			"</tr></thead><tbody>\n");
		char var[128];
		char *next;
		int midx = 0;
#ifdef HAVE_X86
		char check[64];
		char *d = getdisc();
		sprintf(check, "/dev/%s", d);
		free(d);
#endif
		foreach(var, raid, next)
		{
			websWrite(wp, "<tr>\n"
				      "<td>\n");
			websWrite(wp, "<select name=\"raid%dmember%d\">\n", i,
				  midx);
			if (!strcmp(var, "none"))
				websWrite(
					wp,
					"<option value=\"none\"><script type=\"text/javascript\">Capture(share.none)</script></option>\n");
			if (drives) {
				foreach(drive, drives, dnext)
				{
#ifdef HAVE_X86
					if (!strcmp(drive, check))
						continue;
#endif
					websWrite(
						wp,
						"<option value=\"%s\" %s >%s</option>\n",
						drive,
						!strcmp(drive, var) ?
							"selected=\"selected\"" :
							"",
						drive);
				}
			}

			websWrite(wp, "</td>\n");
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<td class=\\\"center\\\" title=\\\"\" + sbutton.del + \"\\\"><input class=\\\"remove\\\" aria-label=\\\"\" + sbutton.del + \"\\\" type=\\\"button\\\" onclick=\\\"member_del_submit(this.form,%d, %d)\\\" />\");\n//]]>\n</script>\n",
				i, midx);
			websWrite(wp, "</td>\n"
				      "</tr>\n");
			midx++;
		}

		websWrite(wp, "<tr>\n"
			      "<td></td>\n"
			      "<td class=\"center\">\n");
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"add\\\" type=\\\"button\\\" aria-label=\\\"\" + sbutton.add + \"\\\" onclick=\\\"member_add_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			i);
		websWrite(wp, "</td>\n"
			      "</tr></tbody>\n"
			      "</table>\n"
			      "</fieldset>\n"
			      "</div>\n");
		websWrite(
			wp,
			"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button red_btn\\\" name=\\\"format_raid\\\" type=\\\"button\\\" value=\\\"\" + nas.format + \"\\\" onclick=\\\"raid_format_submit(this.form,%d)\\\" />\");\n//]]>\n</script>\n",
			i);
		i++;
	}
	websWrite(
		wp,
		"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"raid_add_submit(this.form)\\\" />\");\n//]]>\n</script>\n");
	websWrite(
		wp,
		"</fieldset><br />\n"
		"<h2><script type=\"text/javascript\">Capture(nas.drivemanager)</script></h2>"
		"<fieldset>\n<legend><script type=\"text/javascript\">Capture(bmenu.adminman)</script></legend>\n"
		"<table id=\"drives\" class=\"table\" summary=\"Drive List\">\n<thead>\n"
		"<tr>\n"
		"<th><script type=\"text/javascript\">Capture(nas.drive)</script></th>\n"
		"<th><script type=\"text/javascript\">Capture(idx.label)</script></th>\n"
		"<th><script type=\"text/javascript\">Capture(nas.fs)</script></th>\n"
		"<th class=\"center\" width=\"10%%\"><script type=\"text/javascript\">Capture(share.actiontbl)</script></th>\n"
		"</tr>\n</thead><tbody>");
	int idx = 0;
	if (drives) {
		foreach(drive, drives, dnext)
		{
			int canformat = 0;
			char *fs = getfsname(drive);
			if (!fs)
				continue;
			if (ismember(drive))
				continue;
			websWrite(
				wp,
				"<input type=\"hidden\" name=\"drivename%d\" value=\"%s\"\n",
				idx, &drive[5]);
			websWrite(wp, "<tr>\n"
				      "<td>\n");
			websWrite(
				wp,
				"<input name=\"fs%d\" size=\"14\" value=\"%s\" disabled=\"disabled\"/>",
				idx, drive);
			websWrite(wp, "</td>\n"
				      "<td>\n");
			websWrite(
				wp,
				"<input name=\"label%d\" size=\"12\" value=\"%s\"/>",
				idx, nvram_nget("%s_label", &drive[5]));
			websWrite(wp, "</td>\n"
				      "<td>\n");
			websWrite(
				wp,
				"<select id=\"format%d\" name=\"format%d\" onchange=\"drive_fs_changed(this.form,%d, this.form.format%d.selectedIndex)\">\n",
				idx, idx, idx, idx);
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"unk\\\" >\" + share.unknown + \"</option>\");\n");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"unk\\\" >\" + share.empty + \"</option>\");\n",
				!strcmp(fs, "Empty") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"unk\\\" %s>Partition</option>\");\n",
				!strcmp(fs, "Partition") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"ext2\\\" %s >EXT2</option>\");\n",
				!strcmp(fs, "EXT2") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"ext3\\\" %s >EXT3</option>\");\n",
				!strcmp(fs, "EXT3") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"ext4\\\" %s >EXT4</option>\");\n",
				!strcmp(fs, "EXT4") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"exfat\\\" %s >EXFAT</option>\");\n",
				!strcmp(fs, "EXFAT") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"xfs\\\" %s >XFS</option>\");\n",
				!strcmp(fs, "XFS") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"btrfs\\\" %s >BTRFS</option>\");\n",
				!strcmp(fs, "BTRFS") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"ntfs\\\" %s >NTFS</option>\");\n",
				!strcmp(fs, "NTFS") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"fat32\\\" %s >FAT32</option>\");\n",
				!strcmp(fs, "FAT32") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"zfs\\\" %s >ZFS</option>\");\n",
				!strcmp(fs, "ZFS") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(
				wp,
				"document.write(\"<option value=\\\"apfs\\\" %s >APFS</option>\");\n",
				!strcmp(fs, "APFS") ?
					"selected=\\\"selected\\\"" :
					"");
			websWrite(wp, "//]]>\n</script></select>\n");
			websWrite(wp, "</td>\n"
				      "<td class=\"center\">\n");
			int dis = 1;
			if (!strcmp(fs, "XFS") && xfs)
				dis = 0;
			else if (!strcmp(fs, "EXT2") && ext2)
				dis = 0;
			else if (!strcmp(fs, "EXT3") && ext3)
				dis = 0;
			else if (!strcmp(fs, "EXT4") && ext4)
				dis = 0;
			else if (!strcmp(fs, "APFS") && apfs)
				dis = 0;
			else if (!strcmp(fs, "BTRFS") && btrfs)
				dis = 0;
			else if (!strcmp(fs, "EXFAT") && exfat)
				dis = 0;
			else if (!strcmp(fs, "NTFS") && ntfs)
				dis = 0;
			else if (!strcmp(fs, "FAT32") && fat32)
				dis = 0;
			else if (!strcmp(fs, "ZFS") && zfs)
				dis = 0;
			websWrite(
				wp,
				"<script type=\"text/javascript\">\n//<![CDATA[\n document.write(\"<input class=\\\"button margin-0 red_btn\\\" id=\\\"drive_format%d\\\" name=\\\"format_drive\\\" type=\\\"button\\\" value=\\\"\" + nas.format + \"\\\" onclick=drive_format_submit(this.form,%d,\\\"%s\\\") %s />\");\n//]]>\n</script>\n",
				idx, idx, drive,
				!dis ? "" : "disabled=\\\"true\\\"");
			websWrite(wp, "</td>\n</tr>\n");
			idx++;
		}
	}
	websWrite(wp, "</tbody></table>\n");

	if (drives) {
		foreach(drive, drives, dnext)
		{
			char *fs = getfsname(drive);
			if (!fs)
				continue;
			websWrite(
				wp,
				"<input type=\"hidden\" name=\"drivename%d\" value=\"%s\">\n",
				idx, &drive[5]);
		}
	}
	websWrite(
		wp,
		"<input type=\"hidden\" name=\"drivecount\" id=\"drivecount\" value=\"%d\">\n",
		idx);
	websWrite(wp, "</fieldset><br />\n");
}

#endif
