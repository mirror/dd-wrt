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

	if (pid > 0 && strncmp(type, "stop", 4)) {	// pinging
		websWrite(wp, arg);
	}

}

void ej_dump_ping_log(webs_t wp, int argc, char_t ** argv)
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

	while ((count1 > count2) && (timeout < 5)) {
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
			sleep(2);
		}
	}
	/*
	 * end waiting 
	 */

	if ((fp = fopen(PING_TMP, "r")) != NULL) {	// show result
		while (fgets(line, sizeof(line), fp) != NULL) {
			line[strlen(line) - 1] = '\0';
			if (!strcmp(line, ""))
				continue;
			int nc = 0;

			for (i = 0; i < strlen(line) + 1; i++) {
				if (line[i] == '"') {
					newline[nc++] = '&';
					newline[nc++] = 'q';
					newline[nc++] = 'u';
					newline[nc++] = 'o';
					newline[nc++] = 't';
					newline[nc++] = ';';
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
