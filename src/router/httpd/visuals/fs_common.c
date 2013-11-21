#if defined(HAVE_MINIDLNA) || defined(HAVE_NAS_SERVER)
#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>
#include <math.h>

#include <broadcom.h>
#include <cymac.h>
#include "fs_common.h"

static struct fsentry *parsefsentry(char line[256])
{

	struct fsentry *entry = calloc(1, sizeof(struct fsentry));
	char *token, *perm;
	int tokcount = 0;

	line[strlen(line) - 1] = '\0';	// replace new line with null
	token = strtok(line, " ");
	while (token != NULL) {
		// check for values
		if (tokcount == 0) {
			strcpy(entry->fs, token);
		} else if (tokcount == 2) {
			if (!strncmp(token, "/tmp/proftpd", 12)) {
				free(entry);
				return NULL;	//skip it
			}
			if (!strncmp(token, "/tmp/mnt/", 9)) {
				char newpath[64];
				strcpy(newpath, token);
				char *slash = strrchr(newpath, '/') + 1;
				sprintf(entry->mp, "/mnt/%s", slash);	// convert symbolic link to absolute path
			} else {
				strcpy(entry->mp, token);
			}
		} else if (tokcount == 4) {
			strcpy(entry->fstype, token);
		} else if (tokcount == 5) {
			perm = strtok(token, ",");
			strncpy(entry->perms, perm + 1, strlen(perm) - 1);
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
	struct fsentry *list, *tmplist, *current;
	int count = 0;

	if ((fp = popen("mount", "r"))) {
		//current = list;
		while (fgets(line, sizeof(line), fp)) {
			//fprintf(stderr, "[MOUNTS] %s\n", line);
			tmplist = parsefsentry(line);
			if (!tmplist)
				continue;
			if (count == 0) {
				list = tmplist;
				current = list;
			} else {
				current->next = tmplist;
				current = current->next;
			}
			count++;
		}
		pclose(fp);
	}
	struct fsentry *entry = calloc(1, sizeof(struct fsentry));
	strcpy(entry->fs, "/mnt");
	strcpy(entry->fstype, "dummy");
	strcpy(entry->perms, "rw");
	strcpy(entry->mp, "/mnt");
	current->next = entry;
	current = current->next;
	return list;
}
#endif
