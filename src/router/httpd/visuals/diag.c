/*
 * diag.c
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
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <broadcom.h>

void ping_onload(webs_t wp, char *arg)
{
	int pid;
	char *type = websGetVar(wp, "submit_type", "");

	pid = find_pid_by_ps("ping");

	if (pid > 0 && strncmp(type, "stop", 4)) { // pinging
		websWrite(wp, arg);
	}
}

EJ_VISIBLE void ej_dump_ping_log(webs_t wp, int argc, char_t **argv)
{
	int count = 0;
	FILE *fp;
	char line[254];
	char newline[300];
	int i;

	/*
	 * wait as long file size increases, but max. 10 s)
	 */
	int c, count1 = 0, count2 = -1, timeout = 0;
	while ((count1 > count2) && (timeout < 10)) {
		count2 = count1;
		count1 = 0;

		if ((fp = fopen(PING_TMP, "r")) != NULL) {
			c = fgetc(fp);
			while (c != EOF) {
				count1++;
				c = fgetc(fp);
			}
			fclose(fp);
			timeout++;
			struct timespec tim, tim2;
			tim.tv_sec = 0;
			tim.tv_nsec = 1000000000L;
			nanosleep(&tim, &tim2);
		}
	}
	/*
	 * end waiting 
	 */

	if ((fp = fopen(PING_TMP, "r")) != NULL) { // show result
		while (fgets(line, sizeof(line), fp) != NULL) {
			if (!strcmp(line, ""))
				continue;
			int nc = 0;
			int len = strlen(line);
			for (i = 0; i < (len - 1) && (nc < (sizeof(newline) - 1)); i++) {
				if (line[i] == '"' && nc < (sizeof(newline) - 7)) {
					memcpy(&newline[nc], "&quot;", 6);
					nc += 6;
				} else
					newline[nc++] = line[i];
			}
			newline[nc++] = 0;
			websWrite(wp, "%c\"%s\"\n", count ? ',' : ' ', newline);
			count++;
		}
		fclose(fp);
	}

	unlink(PING_TMP);

	return;
}
