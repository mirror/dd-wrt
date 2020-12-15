/*
 * sysctl.c
 *
 * Copyright (C) 2005 - 2020 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <unistd.h>
#include <stdarg.h>
#include <dirent.h>
#include <sys/stat.h>

#include <broadcom.h>

#include <nfs.h>
#include "fs_common.h"
void show_caption_pp(webs_t wp, const char *class, const char *caption, const char *pre, const char *post);

static char *getsysctl(char *path, char *name, char *nvname, char *fval)
{
	char *val = nvram_safe_get(nvname);
	if (*val)
		return val;
	char fname[128];
	sprintf(fname, "%s/%s", path, name);
	struct stat sb;
	stat(fname, &sb);
	if (!(sb.st_mode & S_IWUSR))
		return NULL;

	FILE *in = fopen(fname, "rb");
	if (!in)
		return NULL;
	fgets(fval, 127, in);
	int i;
	int len = strlen(fval);
	for (i = 0; i < len; i++) {
		if (fval[i] == '\t')
			fval[i] = 0x20;
		if (fval[i] == '\n')
			fval[i] = 0;
	}
	fclose(in);
	return fval;
}

static char *sysctl_blacklist[] = {	//
	"base_reachable_time",
	"nf_conntrack_max",
	"nf_conntrack_helper",
	"bridge-nf-call-arptables",
	"bridge-nf-call-ip6tables",
	"bridge-nf-call-iptables"
};

static void showdir(webs_t wp, char *path)
{
	DIR *directory;
	char buf[256];
	int cnt = 0;
	directory = opendir(path);
	struct dirent *entry;
	while ((entry = readdir(directory)) != NULL) {
		if (!strcmp(entry->d_name, "nf_log"))	// pointless
			continue;
		if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))
			continue;
		if (entry->d_type == DT_DIR) {
			char dir[1024];
			sprintf(dir, "%s/%s", path, entry->d_name);
			showdir(wp, dir);
			continue;
		}
	}
	closedir(directory);
	directory = opendir(path);
	while ((entry = readdir(directory)) != NULL) {
		if (entry->d_type == DT_REG) {
			char title[64] = { 0 };
			char fval[128];
			int a;
			for (a = 0; a < sizeof(sysctl_blacklist) / sizeof(char *); a++) {
				if (!strcmp(entry->d_name, sysctl_blacklist[a]))	// supress kernel warning
					goto next;
			}
			strcpy(title, &path[10]);
			int i;
			int len = strlen(title);
			for (i = 0; i < len; i++)
				if (title[i] == '/')
					title[i] = '.';
			char nvname[128];
			sprintf(nvname, "%s%s%s", title, strlen(title) ? "." : "", entry->d_name);
			char *value = getsysctl(path, entry->d_name, nvname, fval);
			if (value) {
				if (!cnt) {
					websWrite(wp, "<fieldset>\n");
					websWrite(wp, "<legend>%s</legend>\n", title);
				}
				websWrite(wp, "<div class=\"setting\">\n");
				websWrite(wp, "<div class=\"label\">%s</div>\n", entry->d_name);
				websWrite(wp, "<input maxlength=\"100\" size=\"40\" name=\"%s\" value=\"%s\" />\n", nvname, value);
				websWrite(wp, "</div>\n");
				cnt++;
			}
		      next:;
		}

	}
	closedir(directory);
	if (cnt)
		websWrite(wp, "</fieldset>\n");

}

void ej_sysctl(webs_t wp, int argc, char_t ** argv)
{
	showdir(wp, "/proc/sys");
}
