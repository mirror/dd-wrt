/*
 * fs_common.c
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
#if defined(HAVE_MINIDLNA) || defined(HAVE_NAS_SERVER)
#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <broadcom.h>

#include "fs_common.h"

static struct fsentry *parsefsentry(char line[256])
{
	struct fsentry *entry = calloc(1, sizeof(struct fsentry));
	char *token, *perm;
	int tokcount = 0;

	line[strlen(line) - 1] = '\0'; // replace new line with null
	token = strtok(line, " ");
	while (token != NULL) {
		// check for values
		if (tokcount == 0) {
			strcpy(entry->fs, token);
		} else if (tokcount == 2) {
			if (!strncmp(token, "/tmp/proftpd", 12)) {
				debug_free(entry);
				return NULL; //skip it
			}
			if (!strncmp(token, "/tmp/mnt/", 9)) {
				char newpath[64];
				strcpy(newpath, token);
				char *slash = strrchr(newpath, '/') + 1;
				sprintf(entry->mp, "/mnt/%s",
					slash); // convert symbolic link to absolute path
			} else {
				strcpy(entry->mp, token);
			}
		} else if (tokcount == 4) {
			strcpy(entry->fstype, token);
		} else if (tokcount == 5) {
			perm = strtok(token, ",");
			strlcpy(entry->perms, perm + 1, strlen(perm) - 1);
		}
		// next token
		token = strtok(NULL, " ");
		tokcount++;
	}
	return entry;
}

struct fsentry *getfsentries(void)
{
	char line[512];
	FILE *fp;
	struct fsentry *list = NULL, *tmplist, *current = NULL;
	int count = 0;

	if ((fp = popen("mount", "r"))) {
		//current = list;
		while (fgets(line, sizeof(line), fp)) {
			//fprintf(stderr, "[MOUNTS] %s\n", line);
			tmplist = parsefsentry(line);
			if (!tmplist)
				continue;
			if (count == 0) {
				list = current = tmplist;
			} else {
				current->next = tmplist;
				current = current->next;
			}
			count++;
		}
		pclose(fp);
	}
	if (!list)
		return NULL;
	struct fsentry *entry = calloc(1, sizeof(struct fsentry));
	strcpy(entry->fs, "/mnt");
	strcpy(entry->fstype, "dummy");
	strcpy(entry->perms, "rw");
	strcpy(entry->mp, "/mnt");
	current->next = entry;
	return list;
}
#endif
