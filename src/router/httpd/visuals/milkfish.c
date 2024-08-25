/*
 * milkfish.c
 *
 * Copyright (C) 2005 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/route.h>

#include <broadcom.h>

EJ_VISIBLE void ej_exec_milkfish_service(webs_t wp, int argc, char_t **argv)
{
	FILE *fp;
	char line[254];
	char *request = argv[0];

	if ((fp = popen(request, "r"))) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			websWrite(wp, line);
			websWrite(wp, "<br>");
		}
		pclose(fp);
	}

	return;
}

EJ_VISIBLE void ej_exec_milkfish_phonebook(webs_t wp, int argc, char_t **argv)
{
	FILE *fp;
	char line[254];
	char *request = argv[0];

	if ((fp = popen(request, "r"))) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			websWrite(wp, line);
		}
		pclose(fp);
	}

	return;
}

void show_subscriber_table(webs_t wp, char *type, int which)
{
	char word[256];
	char *next, *wordlist;
	char *user, *pass;
	char new_user[200], new_pass[200];

	wordlist = nvram_safe_get("milkfish_ddsubscribers");

	foreach(word, wordlist, next)
	{
		if (which-- == 0) {
			pass = word;
			user = strsep(&pass, ":");
			if (!user || !pass)
				continue;

			if (!strcmp(type, "user")) {
				httpd_filter_name(user, new_user, sizeof(new_user), GET);
				websWrite(wp, "%s", new_user);
			} else if (!strcmp(type, "pass")) {
				httpd_filter_name(pass, new_pass, sizeof(new_pass), GET);
				websWrite(wp, "%s", new_pass);
			}
			return;
		}
	}
}

EJ_VISIBLE void ej_exec_show_subscribers(webs_t wp, int argc, char_t **argv)
{
	int i;
	char *count;
	int c = 0;

	count = nvram_safe_get("milkfish_ddsubscribersnum");
	if (count == NULL || *(count) == 0 || (c = atoi(count)) <= 0) {
		show_caption_pp(wp, NULL, "share.none", "<tr>\n<td colspan=\"4\" class=\"center\" valign=\"middle\">- ",
				" -</td>\n</tr>\n");
	}
	for (i = 0; i < c; i++) {
		websWrite(
			wp,
			"<tr><td>\n<input maxlength=\"30\" size=\"30\" name=\"user%d\" onblur=\"valid_name(this,'Name')\" value=\"",
			i);
		show_subscriber_table(wp, "user", i);
		websWrite(
			wp,
			"\" /></td>\n<td>\n<input maxlength=\"30\" size=\"30\" name=\"pass%d\" onblur=\"valid_name(this,'Name')\" value=\"",
			i);
		show_subscriber_table(wp, "pass", i);
		websWrite(wp, "\" /></td>\n</tr>\n");
	}
	return;
}

void show_aliases_table(webs_t wp, char *type, int which)
{
	char word[256];
	char *next, *wordlist;
	char *user, *pass;
	char new_user[200], new_pass[200];

	wordlist = nvram_safe_get("milkfish_ddaliases");

	foreach(word, wordlist, next)
	{
		if (which-- == 0) {
			pass = word;
			user = strsep(&pass, ":");
			if (!user || !pass)
				continue;

			if (!strcmp(type, "user")) {
				httpd_filter_name(user, new_user, sizeof(new_user), GET);
				websWrite(wp, "%s", new_user);
			} else if (!strcmp(type, "pass")) {
				httpd_filter_name(pass, new_pass, sizeof(new_pass), GET);
				websWrite(wp, "%s", new_pass);
			}
			return;
		}
	}
}

EJ_VISIBLE void ej_exec_show_aliases(webs_t wp, int argc, char_t **argv)
{
	int i;
	char *count;
	int c = 0;

	count = nvram_safe_get("milkfish_ddaliasesnum");
	if (count == NULL || *(count) == 0 || (c = atoi(count)) <= 0) {
		show_caption_pp(wp, NULL, "share.none", "<tr>\n<td colspan=\"4\" class=\"center\" valign=\"middle\">- ",
				" -</td>\n</tr>\n");
	}
	for (i = 0; i < c; i++) {
		websWrite(
			wp,
			"<tr><td>\n<input maxlength=\"30\" size=\"30\" name=\"user%d\" onblur=\"valid_name(this,'Name')\" value=\"",
			i);
		show_aliases_table(wp, "user", i);
		websWrite(
			wp,
			"\" /></td>\n<td>\n<input maxlength=\"50\" size=\"50\" name=\"pass%d\" onblur=\"valid_name(this,'Name')\" value=\"",
			i);
		show_aliases_table(wp, "pass", i);
		websWrite(wp, "\" /></td>\n</tr>\n");
	}
	return;
}

void show_registrations_table(webs_t wp, char *type, int which)
{
	char word[256];
	char *next, *wordlist;
	char *user, *contact, *agent;
	char new_user[200], new_contact[200], new_agent[200];

	wordlist = nvram_safe_get("milkfish_ddactive");

	foreach(word, wordlist, next)
	{
		if (which-- == 0) {
			contact = word;
			user = strsep(&contact, ":");
			if (!user || !contact)
				continue;

			agent = contact;
			contact = strsep(&agent, ":");
			if (!contact || !agent)
				continue;

			if (!strcmp(type, "user")) {
				httpd_filter_name(user, new_user, sizeof(new_user), GET);
				websWrite(wp, "%s", new_user);
			} else if (!strcmp(type, "contact")) {
				httpd_filter_name(contact, new_contact, sizeof(new_contact), GET);
				websWrite(wp, "%s", new_contact);
			} else if (!strcmp(type, "agent")) {
				httpd_filter_name(agent, new_agent, sizeof(new_agent), GET);
				websWrite(wp, "%s", new_agent);
			}

			return;
		}
	}
}

EJ_VISIBLE void ej_exec_show_registrations(webs_t wp, int argc, char_t **argv)
{
	int i;
	char *count;
	int c = 0;

	count = nvram_safe_get("milkfish_ddactivenum");
	if (count == NULL || *(count) == 0 || (c = atoi(count)) <= 0) {
		show_caption_pp(wp, NULL, "share.none", "<tr>\n<td colspan=\"4\" class=\"center\" valign=\"middle\">- ",
				" -</td>\n</tr>\n");
	}
	for (i = 0; i < c; i++) {
		websWrite(
			wp,
			"<tr><td>\n<input maxlength=\"20\" size=\"20\" name=\"user%d\" onblur=\"valid_name(this,'Name')\" value=\"",
			i);
		show_registrations_table(wp, "user", i);
		websWrite(
			wp,
			"\" readonly=\"readonly\" /></td>\n<td>\n<input maxlength=\"50\" size=\"50\" name=\"contact%d\" onblur=\"valid_name(this,'Name')\" value=\"",
			i);
		show_registrations_table(wp, "contact", i);
		websWrite(
			wp,
			"\" readonly=\"readonly\" /></td>\n<td>\n<input maxlength=\"50\" size=\"50\" name=\"agent%d\" onblur=\"valid_name(this,'Name')\" value=\"",
			i);
		show_registrations_table(wp, "agent", i);
		websWrite(wp, "\" readonly=\"readonly\" /></td>\n</tr>\n");
	}
	return;
}
