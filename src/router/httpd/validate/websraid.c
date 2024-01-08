/*
 * websraid.c
 *
 * Copyright (C) 2018 Sebastian Gottschall <sebastian.gottschall@newmedia-net.de>
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
#define VALIDSOURCE 1

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else /* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <httpd.h>
#include <errno.h>
#endif /* WEBS */

#include <proto/ethernet.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <dd_defs.h>
#include <cy_conf.h>
// #ifdef EZC_SUPPORT
#include <ezc.h>
// #endif
#include <broadcom.h>
#include <wlutils.h>
#include <netdb.h>
#include <utils.h>
#include <stdarg.h>
#include <sha1.h>
#include <linux/fs.h>
#include <fcntl.h>

void add_raid(webs_t wp)
{
	int idx = 0;
	while (1) {
		char *type = nvram_nget("raidtype%d", idx);
		if (!*(type))
			break;
		idx++;
	}
	nvram_nset("md", "raidtype%d", idx);
}

void format_drive(webs_t wp)
{
	char *val = websGetVar(wp, "raid_del_value", NULL);
	if (!val)
		return;
	int idx = atoi(val);
	char s_format[32];
	sprintf(s_format, "format%d", idx);
	char s_label[32];
	sprintf(s_label, "label%d", idx);

	char *fs = websGetVar(wp, "do_format_drive", NULL);
	char *format = websGetVar(wp, s_format, NULL);
	char *label = websGetVar(wp, s_label, NULL);
	if (!fs || !format)
		return;
	if (!label)
		label = "";
	if (*(label)) {
		char labelstr[32];
		sprintf(labelstr, "%s_label", &fs[5]);
		nvram_set(labelstr, label);
		nvram_async_commit();
	}
	eval("umount", fs);
	char name[32];
	sprintf(name, "mkfs.%s", format);
	if (!strcmp(format, "xfs") || !strcmp(format, "btrfs")) {
		if (*(label))
			eval(name, "-f", "-L", label, fs);
		else
			eval(name, "-f", fs);
	} else if (!strcmp(format, "ntfs")) {
		if (*(label))
			eval(name, "-f", "-L", label, "-Q", "-F", fs);
		else
			eval(name, "-f", "-Q", "-F", fs);
	} else if (!strcmp(format, "exfat")) {
		if (*(label))
			eval(name, "-n", label, fs);
		else
			eval(name, fs);
	} else if (!strcmp(format, "fat32")) {
		if (*(label))
			eval("mkfs.fat", "-n", label, "-F", "32", fs);
		else
			eval("mkfs.fat", "-F", "32", fs);
	} else if (!strncmp(format, "ext", 3)) {
		if (*(label))
			eval(name, "-F", "-L", label, "-E", "lazy_itable_init=1", fs);
		else
			eval(name, "-F", "-E", "lazy_itable_init=1", fs);
	} else {
		if (*(label))
			eval(name, "-F", "-L", label, fs);
		else
			eval(name, "-F", fs);
	}
	int fd = open(fs, O_RDONLY | O_NONBLOCK);
	ioctl(fd, BLKRRPART, NULL);
	close(fd);
}

#ifdef HAVE_ZFS
void zfs_scrub(webs_t wp)
{
	char *val = websGetVar(wp, "zfs_scrub_value", NULL);
	if (!val)
		return;
	int idx = atoi(val);
	char *poolname = nvram_nget("raidname%d", idx);
	eval("zpool", "scrub", poolname);
}
#endif

void del_raid(webs_t wp)
{
	char *val = websGetVar(wp, "raid_del_value", NULL);
	if (!val)
		return;
	int idx = atoi(val);

	char *raid = nvram_nget("raid%d", idx);
	char *poolname = nvram_nget("raidname%d", idx);
	char *next;
	char drive[64];
	char dev[32];
	int drives = 0;
	foreach(drive, raid, next)
	{
		drives++;
		eval("umount", drive);
	}
	sprintf(dev, "/dev/md%d", idx);
	eval("umount", dev);
	eval("mdadm", "--stop", dev);
	eval("zpool", "destroy", poolname);

	int cnt = 0;
	while (1) {
		char *type = nvram_nget("raidtype%d", cnt);
		if (!*(type)) {
			break;
		}
		cnt++;
	}
	if (idx == 0) {
		nvram_nset(NULL, "raidtype%d", idx);
		nvram_nset(NULL, "raidname%d", idx);
		nvram_nset(NULL, "raidlevel%d", idx);
		nvram_nset(NULL, "raid%d", idx);
		nvram_nset(NULL, "raidfs%d", idx);
		nvram_nset(NULL, "raidlz%d", idx);
		nvram_nset(NULL, "raidlzlevel%d", idx);
		nvram_nset(NULL, "raiddedup%d", idx);
		return;
	}
	if (cnt == 0)
		return;
	int i;
	for (i = idx; i < cnt - 1; i++) {
		nvram_nset(nvram_nget("raidtype%d", i), "raidtype%d", i + 1);
		nvram_nset(nvram_nget("raidname%d", i), "raidname%d", i + 1);
		nvram_nset(nvram_nget("raidlevel%d", i), "raidlevel%d", i + 1);
		nvram_nset(nvram_nget("raid%d", i), "raid%d", i + 1);
		nvram_nset(nvram_nget("raidfs%d", i), "raidfs%d", i + 1);
		if (nvram_nmatch("zfs", "raidtype%d", i + 1)) {
			nvram_nset(nvram_nget("raidlz%d", i), "raidlz%d", i + 1);
			nvram_nset(nvram_nget("raiddedup%d", i), "raiddedup%d", i + 1);
			if (nvram_nmatch("gzip", "raidlzlevel%d", i) || nvram_nmatch("zstd", "raidlzlevel%d", i))
				nvram_nset(nvram_nget("raidlzlevel%d", i), "raidlzlevel%d", i + 1);
		}
	}
	nvram_nset(NULL, "raidtype%d", i);
	nvram_nset(NULL, "raidname%d", i);
	nvram_nset(NULL, "raidlevel%d", i);
	nvram_nset(NULL, "raid%d", i);
	nvram_nset(NULL, "raidfs%d", i);
	nvram_nset(NULL, "raidlz%d", i);
	nvram_nset(NULL, "raidlzlevel%d", i);
	nvram_nset(NULL, "raiddedup%d", i);
}

void add_raid_member(webs_t wp)
{
	char *val = websGetVar(wp, "raid_member_add_value", NULL);
	if (!val)
		return;
	int idx = atoi(val);
	char *raid = nvram_nget("raid%d", idx);
	char *newv = NULL;
	if (!*(raid))
		asprintf(&newv, "none");
	else
		asprintf(&newv, "%s none", raid);
	nvram_nset(newv, "raid%d", idx);
	debug_free(newv);
}

void del_raid_member(webs_t wp)
{
	char *val = websGetVar(wp, "raid_member_add_value", NULL);
	char *del = websGetVar(wp, "raid_member_del_value", NULL);
	if (!val)
		return;
	if (!del)
		return;
	int idx = atoi(val);
	int didx = atoi(del);
	char *raid = nvram_nget("raid%d", idx);
	char *next;
	char drive[128];
	char *a = NULL;
	int cnt = 0;
	foreach(drive, raid, next)
	{
		a = realloc(a, cnt ? strlen(a) + strlen(drive) + 2 : strlen(drive) + 1);
		if (cnt != didx) {
			if (!cnt)
				a[0] = 0;
			else
				strcat(a, " ");
			strcat(a, drive);
		} else {
			if (!cnt)
				a[0] = 0;
		}
		cnt++;
	}

	nvram_nset(a, "raid%d", idx);
	if (a)
		debug_free(a);
}

void raid_save(webs_t wp)
{
	int idx = 0;
	while (1) {
		char raidname[32];
		sprintf(raidname, "raidname%d", idx);
		char *rn = websGetVar(wp, raidname, NULL);
		if (!rn)
			break;
		nvram_nset(rn, "raidname%d", idx);

		char raidtype[32];
		sprintf(raidtype, "raidtype%d", idx);
		char *rt = websGetVar(wp, raidtype, NULL);
		if (!rt)
			break;
		nvram_nset(rt, "raidtype%d", idx);

		char raidlevel[32];
		sprintf(raidlevel, "raidlevel%d", idx);
		char *rl = websGetVar(wp, raidlevel, NULL);
		if (!rl)
			break;
		nvram_nset(rl, "raidlevel%d", idx);

		if (!strcmp(rt, "zfs")) {
			char raidlz[32];
			sprintf(raidlz, "raidlz%d", idx);
			char *rlz = websGetVar(wp, raidlz, NULL);
			nvram_nset(rlz, "raidlz%d", idx);

			char raidlzlevel[32];
			sprintf(raidlzlevel, "raidlzlevel%d", idx);
			char *rlzlevel = websGetVar(wp, raidlzlevel, NULL);
			if (rlzlevel)
				nvram_nset(rlzlevel, "raidlzlevel%d", idx);

			char raiddedup[32];
			sprintf(raiddedup, "raiddedup%d", idx);
			char *rdd = websGetVar(wp, raiddedup, "0");
			nvram_nset(rdd, "raiddedup%d", idx);
		}

		char raidfs[32];
		sprintf(raidfs, "raidfs%d", idx);
		char *rfs = websGetVar(wp, raidfs, NULL);
		if (rfs)
			nvram_nset(rfs, "raidfs%d", idx);

		int midx = 0;
		char *a = NULL;
		while (1) {
			char member[32];
			sprintf(member, "raid%dmember%d", idx, midx);
			char *mb = websGetVar(wp, member, NULL);
			if (!mb)
				break;
			a = realloc(a, a ? strlen(a) + strlen(mb) + 2 : strlen(mb) + 1);
			if (!midx)
				a[0] = 0;
			else
				strcat(a, " ");
			strcat(a, mb);
			midx++;
		}
		nvram_nset(a, "raid%d", idx);
		if (a)
			debug_free(a);
		idx++;
	}
	idx = 0;
	while (1) {
		char drivename[32];
		sprintf(drivename, "drivename%d", idx);
		char label[32];
		sprintf(label, "label%d", idx);
		char *dn = websGetVar(wp, drivename, NULL);
		if (!dn)
			break;
		idx++;
		char *l = websGetVar(wp, label, NULL);
		if (!l)
			continue;
		nvram_nset(l, "%s_label", dn);
	}
}

void format_raid(webs_t wp)
{
	char *val = websGetVar(wp, "raid_del_value", NULL);
	if (!val)
		return;
	int idx = atoi(val);
	nvram_nset("0", "raiddone%d", idx);
	raid_save(wp);
	eval("stopservice", "raid");
	eval("startservice", "raid");
}

#endif
