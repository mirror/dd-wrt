/*
 * notifier.c - connection limit email notifier (some sort of IDS or lets say the beginning of cyclic connection analysis)
 *
 * Copyright (C) 2009 Sebastian Gottschall <gottschall@dd-wrt.com>
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

#include <stdio.h>
#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <bcmnvram.h>
#include <unistd.h>

typedef struct linkedlist {
	char *name;
	int value;
	int port;
	struct linkedlist *prev;
	struct linkedlist *next;
};

void addEntry(struct linkedlist *list, char *name, char *port, int value)
{
	struct linkedlist *first = list;
	if (nvram_match("wan_ipaddr", name))	// silently ignore the wan ip
		return;
	int p = atoi(port);
	if (list == NULL)
		return;
	while (1) {
		if (!strcmp(first->name, name) && first->port == p) {
			first->value += value;
			return;
		} else {
			if (first->next == NULL) {
				struct linkedlist *next = malloc(sizeof(struct linkedlist));
				next->name = strdup(name);
				next->value = value;
				next->port = p;
				first->next = next;
				next->prev = first;
				next->next = NULL;
				return;
			}
			first = first->next;
		}
	}
}

void freeList(struct linkedlist *list)
{
	struct linkedlist *first = list->next;
	if (list == NULL)
		return;
	while (1) {
		if (first->next == NULL)
			return;
		struct linkedlist *next = first->next;
		free(first->name);
		free(first);
		first = next;
	}
}

/*
 * sends a email with the detailed connection statistic per port. this requires a modified sendmail command from busybox. the original one is buggy and does not work with alot of email servers like exim, so we modified
 * it to support direct message sending and authentication with commandline parameters
 */
void send_email(struct linkedlist *list, char *source, int value)
{
	char *server = nvram_safe_get("warn_server");
	char *from = nvram_safe_get("warn_from");
	char *fromfull = nvram_safe_get("warn_fromfull");
	char *to = nvram_safe_get("warn_to");
	char *domain = nvram_safe_get("warn_domain");

	char *user = nvram_safe_get("warn_user");
	char *pass = nvram_safe_get("warn_pass");

	FILE *fp;

	static char command[256];

	fp = fopen("/tmp/warn_mail", "w");

	if (fp != NULL) {
		fprintf(fp, "From: %s\n", fromfull);
		fprintf(fp, "Subject: DD-WRT: user %s reached connection limit\n\n", source);
		fprintf(fp, "ip %s has %d open connections\n", source, value);

		while (1) {
			if (!strcmp(list->name, source)) {
				fprintf(fp, "%d open connections on port %d\n", list->value, list->port);
			}
			list = list->next;
			if (list == NULL)
				break;
		}
		fprintf(fp, "\r\n\r\n");

		if (strlen(user) > 0) {
			sprintf(command, "sendmail -S %s -f %s -au %s -ap %s %s -d %s < /tmp/warn_mail", server, from, user, pass, to, domain);
		} else {
			sprintf(command, "sendmail -S %s -f %s  %s -d %s < /tmp/warn_mail", server, from, to, domain);
		}

		fclose(fp);

		system(command);

		unlink("/tmp/warn_mail");
	}

}

int main(int argc, char *argv[])
{
	if (!nvram_match("warn_enabled", "1"))
		return 0;
	int nf = 0;
	struct linkedlist list;
	struct linkedlist total;
	list.name = "entry";
	list.value = 0;
	list.next = NULL;
	list.prev = NULL;

	total.name = "entry";
	total.value = 0;
	total.next = NULL;
	total.prev = NULL;
	FILE *fp = fopen("/proc/net/ip_conntrack", "rb");
	if (fp == NULL) {
		fp = fopen("/proc/net/nf_conntrack", "rb");
		nf = 1;
	}
	while (1) {
		char proto[64];
		char state[64];
		char src[64];
		char dst[64];
		char sport[64];
		char dport[64];
		//default tcp
		fscanf(fp, "%*s %*s %s %*s %*s %s %s %s %s %s", proto, state, src, dst, sport, dport);
		if (!strcmp(proto, "udp") || !strcmp(proto, "unknown"))	// parse udp
			fscanf(fp, "%*s %*s %s %*s %*s %s %s %s %s", proto, src, dst, sport, dport);
		if (!strcmp(proto, "gre"))	// parse gre
			fscanf(fp, "%*s %*s %s %*s %*s %*s %*s %s %s %s %s", proto, src, dst, sport, dport);
		if (!strcmp(proto, "sctp"))	// parse sctp
			fscanf(fp, "%*s %*s %s %*s %*s %*s %s %s %s %s", proto, src, dst, sport, dport);
		if (feof(fp))
			break;
		if (!strcmp(proto, "tcp") && strcmp(state, "ESTABLISHED"))
			continue;

		addEntry(&list, &src[4], &dport[6], 1);	//add connection per port
		addEntry(&total, &src[4], "0", 1);	//add connection to total statistic
	}
	fclose(fp);
	struct linkedlist *entry = &total;
	int limit = atoi(nvram_default_get("warn_connlimit", "500"));
	while (1) {
		if (entry->value > limit) {
			send_email(&list, entry->name, entry->value);
		}
		entry = entry->next;
		if (entry == NULL)
			break;
	}
	freeList(&list);
	freeList(&total);
	return 0;
}
