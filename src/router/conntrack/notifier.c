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
	if (nvram_match("wan_ipaddr",name)) // silently ignore the wan ip
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
				struct linkedlist *next =
				    malloc(sizeof(struct linkedlist));
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

void nextline(FILE * fp) // skip current line and goto next line
{
	while (1) {
		int c = getc(fp);
		if (c == 0xa)
			return;
		if (c == EOF)
			return;
	}
}

void getword(FILE * in, char *val) //read a word which is separated by spaces, so wait for the first non space character and end if the first occuring space
{
	int c = 0;
	while (1) {
		int v = getc(in);
		if (v == EOF)
			return;
		if (v == 0xa)
			break;
		if (v == 0x20 && c == 0)
			continue;
		if (v == 0x20)
			break;
		val[c++] = v;
	}
	val[c++] = 0;
}
/*
 * sends a email with the detailed connection statistic per port. this requires a modified sendmail command from busybox. the original one is buggy and does not work with alot of email servers like exim, so we modified
 * it to support direct message sending and authentication with commandline parameters
 */
void send_email(struct linkedlist *list, char *source, int value)
{
	char *email_line;
	char *server = nvram_safe_get("warn_server");
	char *from = nvram_safe_get("warn_from");
	char *fromfull = nvram_safe_get("warn_fromfull");
	char *to = nvram_safe_get("warn_to");
	char *domain = nvram_safe_get("warn_domain");

	char *user = nvram_safe_get("warn_user");
	char *pass = nvram_safe_get("warn_pass");
	static char subject[4096];
	sprintf(subject, "user %s reached connection limit\n", source);
	char *mess=NULL;
	static char line[1024];

	int c = sprintf(line, "ip %s has %d open connections\n", source, value);
	mess = realloc(mess,c+1);
	strcpy(mess,line);
	while (1) {
		if (!strcmp(list->name, source)) {
			c += sprintf(line,
				     "%d open connections on port %d\n",
				     list->value, list->port);
			mess = realloc(mess,c+1);
			strcat(mess,line);
		}
		list = list->next;
		if (list == NULL)
			break;
	}
	email_line=malloc(strlen(mess)+1024);
	
	if (strlen(user) > 0)
		sprintf(email_line,
			"sendmail -S %s -f %s -F \"%s\" -s \"%s\" -u \"%s\" -p \"%s\"  %s -m \"%s\" -d \"%s\"",
			server, from, fromfull, subject, user, pass, to,
			mess, domain);
	else
		sprintf(email_line,
			"sendmail -S %s -f %s -F \"%s\" -s \"%s\" %s -m \"%s\" -d \"%s\"",
			server, from, fromfull, subject, to, mess, domain);
	
	system(email_line);
	free(email_line);
	free(mess);

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
		unsigned char proto[64];
		unsigned char dummy[64];
		unsigned char dummy2[64];
		unsigned char p2[64];
		unsigned char p3[64];
		unsigned char state[64];
		unsigned char src[64];
		unsigned char dst[64];
		unsigned char sport[64];
		unsigned char dport[64];
		if (nf) { //nf_conntrack has 2 fields more
			getword(fp, dummy);
			getword(fp, dummy2);
			getword(fp, proto);
			getword(fp, p2);
			getword(fp, p3);
			getword(fp, state);
			getword(fp, src);
			getword(fp, dst);
			getword(fp, sport);
			getword(fp, dport);
		} else {
			getword(fp, proto);
			getword(fp, p2);
			getword(fp, p3);
			getword(fp, state);
			getword(fp, src);
			getword(fp, dst);
			getword(fp, sport);
			getword(fp, dport);
		}
		if (feof(fp))
			break;
		if (!strcmp(proto, "tcp")) { //tcp has one field more
			addEntry(&list, &src[4], &dport[6], 1); //add connection per port
			addEntry(&total, &src[4], "0", 1);      //add connection to total statistic
		} else {
			addEntry(&list, &state[4], &sport[6], 1);
			addEntry(&total, &state[4], "0", 1);
		}
		nextline(fp);
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
