/* 
 * roaming_daemon.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>
#include <shutils.h>
#include <bcmnvram.h>
#include <utils.h>
#include <wlutils.h>

static int open_site_survey(void);

struct site_survey_list *site_survey_lists;

static int open_site_survey(void)
{
	FILE *fp;
	site_survey_lists =
		calloc(sizeof(struct site_survey_list) * SITE_SURVEY_NUM, 1);

	if ((fp = fopen(SITE_SURVEY_DB, "r"))) {
		fread(&site_survey_lists[0],
		      sizeof(struct site_survey_list) * SITE_SURVEY_NUM, 1, fp);
		fclose(fp);
		return 1;
	}
	return 0;
}

#include "regexp/regexp.c"

static int roaming_daemon(void)
{
	nvram_seti("roaming_enable", 0);
	nvram_set("roaming_ssid", "");
	char *ifname = getSTA();

	if (!ifname)
		ifname = getWET();
	if (!ifname) {
		fprintf(stderr, "no station interface found!\n");
		return -1;
	}
#ifdef HAVE_MADWIFI
	char *ssid = nvram_nget("%s_ssid", ifname);
#else
	char *ssid = nvram_nget("wl%d_ssid", get_wl_instance(ifname));
#endif
	int i;
	int found = 0;
	int nlen = strlen(ssid);

	for (i = 0; i < nlen; i++) {
		if (ssid[i] == '*')
			found = 1;
	}
	if (!found) {
		fprintf(stderr, "no dynamic ssid found\n");
		return -1;
	}
	char regexpression[64];
	int c = 0;

	for (i = 0; i < nlen; i++) {
		if (ssid[i] == '*') {
			regexpression[c++] = '.';
			regexpression[c++] = '*';
		} else if (ssid[i] == '.') {
			regexpression[c++] = '\\';
			regexpression[c++] = '.';
		} else
			regexpression[c++] = ssid[i];
	}
	regexpression[c++] = 0;
	regexp *comp = NULL;

	comp = regcomp(regexpression, &c);
	if (comp == NULL) {
		fprintf(stderr, "error while compiling regular expression\n");
		return -1;
	}
	while (1) {
		eval("site_survey"); // call site survey to get scan results
		open_site_survey();
		char *bestssid = NULL;
		int bestrssi = -255;

		for (i = 0; i < SITE_SURVEY_NUM; i++) {
			if (site_survey_lists[i].BSSID[0] == 0 ||
			    site_survey_lists[i].channel == 0)
				break;
			if (site_survey_lists[i].SSID[0] ==
			    0) // empty ssid's or
				// hidden ssid's are
				// not supported
				continue;
			if (regexec(comp, &site_survey_lists[i].SSID[0])) {
				if (site_survey_lists[i].RSSI > bestrssi) {
					bestrssi = site_survey_lists[i].RSSI;
					bestssid =
						&site_survey_lists[i].SSID[0];
				}
			}
		}

		fprintf(stderr, "best result %s with rssi %d for roaming\n",
			bestssid, bestrssi);
		if (bestssid && !nvram_match("roaming_ssid", bestssid)) {
			fprintf(stderr,
				"selecting %s with rssi %d for roaming\n",
				bestssid, bestrssi);
			nvram_set("roaming_ssid", bestssid);
			nvram_seti("roaming_enable", 1);
#ifdef HAVE_MADWIFI
			eval("iwconfig", ifname, "essid", bestssid);
#else
			eval("wl", "-i", ifname, "join", bestssid);
#endif
			eval("killall", "wpa_supplicant");
			eval("supplicant", ifname, bestssid);
		}
		sleep(60);
	}
}

static int roaming_daemon_main(int argc, char *argv[])
{
	switch (fork()) {
	case -1:
		perror("fork failed");
		exit(1);
		break;
	case 0:
		roaming_daemon();
		exit(0);
		break;
	default:
		_exit(0);
	}
	return 0;
}
