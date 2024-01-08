/*
 * sysctl.c
 *
 * Copyright (C) 2005 - 2020 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

struct sysctl_priv {
	webs_t wp;
	int cnt;
};

static void showsysctl(char *path, char *nvname, char *name, char *sysval, void *priv)
{
	struct sysctl_priv *p = (struct sysctl_priv *)priv;
	webs_t wp = p->wp;
	if (!path) {
		if (p->cnt) {
			websWrite(wp, "</fieldset>\n<br />\n");
		}
		p->cnt = 0;
		return;
	}
	char fval[128];
	char *val = nvram_safe_get(nvname);
	if (*val) {
		sysval = val;
	}

	if (!p->cnt) {
		char title[64];
		strcpy(title, nvname);
		char *p = strrchr(title, '.');
		if (p)
			*p = 0;
		websWrite(wp,
			  "<fieldset>\n"
			  "<legend>%s</legend>\n",
			  title);
	}
	websWrite(wp,
		  "<div class=\"setting\">\n" //
		  "<div class=\"label\" style=\"width: 22.6em\">%s</div>\n",
		  name);
	websWrite(wp, "<input maxlength=\"100\" size=\"40\" name=\"%s\" value=\"%s\" />\n", nvname, sysval);
	websWrite(wp, "</div>\n");
	p->cnt++;
	return;
}

EJ_VISIBLE void ej_sysctl(webs_t wp, int argc, char_t **argv)
{
	struct sysctl_priv p;
	p.wp = wp;
	p.cnt = 0;
	sysctl_apply(&p, &showsysctl);
}
