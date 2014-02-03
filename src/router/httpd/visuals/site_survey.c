/*
 * site_survey.c
 *
 * Copyright (C) 2005 - 2013 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <cyutils.h>
#include <code_pattern.h>
#include <broadcom.h>
#include <proto/802.11.h>

#include <wlutils.h>

static struct site_survey_list site_survey_lists[SITE_SURVEY_NUM];

static int open_site_survey(void)
{
	FILE *fp;

	bzero(site_survey_lists, sizeof(site_survey_lists));

	if ((fp = fopen(SITE_SURVEY_DB, "r"))) {
		fread(&site_survey_lists[0], sizeof(site_survey_lists), 1, fp);
		fclose(fp);
		return TRUE;
	}
	return FALSE;
}

#ifdef FBNFW

void ej_list_fbn(webs_t wp, int argc, char_t ** argv)
{
	int i;

	system2("site_survey");

	open_site_survey();
	for (i = 0; i < SITE_SURVEY_NUM; i++) {

		if (site_survey_lists[i].SSID[0] == 0 || site_survey_lists[i].BSSID[0] == 0 || (site_survey_lists[i].channel & 0xff) == 0)
			break;

		if (startswith(site_survey_lists[i].SSID, "www.fbn-dd.de")) {
			websWrite(wp, "<option value=\"");
			tf_webWriteJS(wp, site_survey_lists[i].SSID);
			websWrite(wp, "\">");
			tf_webWriteJS(wp, site_survey_lists[i].SSID);
			websWrite(wp, "</option>\n");
		}

	}
}

#endif
void ej_dump_site_survey(webs_t wp, int argc, char_t ** argv)
{
	int i;
	char buf[10] = { 0 };
	char *rates = NULL;
	char *name;

	name = websGetVar(wp, "hidden_scan", NULL);
	if (name == NULL || strlen(name) == 0)
		system2("site_survey");
	else {
		sysprintf("site_survey \"%s\"", name);
	}

	open_site_survey();

	for (i = 0; i < SITE_SURVEY_NUM; i++) {

		if (site_survey_lists[i].BSSID[0] == 0 || (site_survey_lists[i].channel & 0xff) == 0)
			break;

		// fix for " in SSID
		char *tssid = (site_survey_lists[i].SSID[0] == 0) ? "hidden" : &site_survey_lists[i].SSID[0];
		int pos = 0;
		int tpos;
		int ssidlen = strlen(tssid);

		while (pos < ssidlen) {
			if (tssid[pos] == '\"') {
				for (tpos = ssidlen; tpos > pos - 1; tpos--)
					tssid[tpos + 1] = tssid[tpos];

				tssid[pos] = '\\';
				pos++;
				ssidlen++;
			}
			pos++;
		}
		// end fix for " in SSID
		char strbuf[64];

		if (site_survey_lists[i].channel & 0x1000) {
			int cbw = site_survey_lists[i].channel & 0x100;
			//0x000 = 80 mhz
			//0x100 = 8080 mhz
			//0x200 = 160 mhz
			int speed = site_survey_lists[i].rate_count;

			switch (cbw) {
			case 0:
				if (speed == 150)
					speed = 433;
				else if (speed == 300)
					speed = 867;
				else if (speed == 450)
					speed = 1300;
			case 0x100:
			case 0x200:
				if (speed == 150)
					speed = 867;
				else if (speed == 300)
					speed = 1733;
				else if (speed == 450)
					speed = 2600;
			}
			rates = strbuf;

			if ((site_survey_lists[i].channel & 0xff) < 15) {
				sprintf(rates, "%d(b/g/n/ac)", speed);
			} else {
				sprintf(rates, "%d(a/n/ac)", speed);
			}

		} else {
			if ((site_survey_lists[i].channel & 0xff) < 15) {
				if (site_survey_lists[i].rate_count == 4)
					rates = "11(b)";
				else if (site_survey_lists[i].rate_count == 12)
					rates = "54(b/g)";
				else if (site_survey_lists[i].rate_count == 13)
					rates = "108(b/g)";
				else if (site_survey_lists[i].rate_count == 300)
					rates = "300(b/g/n)";
				else if (site_survey_lists[i].rate_count == 450)
					rates = "450(b/g/n)";
				else if (site_survey_lists[i].rate_count == 150)
					rates = "150(b/g/n)";
				else {
					rates = buf;
					snprintf(rates, 9, "%d", site_survey_lists[i].rate_count);
				}
			} else {
				if (site_survey_lists[i].rate_count == 4)
					rates = "11(b)";	//bogus, never shown. but if, its definitly b with weired channel setting
				else if (site_survey_lists[i].rate_count == 12)
					rates = "54(a)";
				else if (site_survey_lists[i].rate_count == 13)
					rates = "108(a)";
				else if (site_survey_lists[i].rate_count == 300)
					rates = "300(a/n)";
				else if (site_survey_lists[i].rate_count == 450)
					rates = "450(a/n)";
				else if (site_survey_lists[i].rate_count == 150)
					rates = "150(a/n)";
				else {
					rates = buf;
					snprintf(rates, 9, "%d", site_survey_lists[i].rate_count);
				}

			}
		}

		/*
		 * #define DOT11_CAP_ESS 0x0001 #define DOT11_CAP_IBSS 0x0002 #define 
		 * DOT11_CAP_POLLABLE 0x0004 #define DOT11_CAP_POLL_RQ 0x0008 #define 
		 * DOT11_CAP_PRIVACY 0x0010 #define DOT11_CAP_SHORT 0x0020 #define
		 * DOT11_CAP_PBCC 0x0040 #define DOT11_CAP_AGILITY 0x0080 #define
		 * DOT11_CAP_SPECTRUM 0x0100 #define DOT11_CAP_SHORTSLOT 0x0400
		 * #define DOT11_CAP_CCK_OFDM 0x2000 
		 */

		char open[32];
		strncpy(open, (site_survey_lists[i].capability & DOT11_CAP_PRIVACY) ? live_translate("share.no")
			: live_translate("share.yes"), 31);

		char *netmode;
		int netmodecap = site_survey_lists[i].capability;

		netmodecap &= (DOT11_CAP_ESS | DOT11_CAP_IBSS);
		if (netmodecap == DOT11_CAP_ESS)
			netmode = "AP";
		else if (netmodecap == DOT11_CAP_IBSS)
			netmode = "AdHoc";
		else
			netmode = live_translate("share.unknown");
		char net[32];
		strcpy(net, netmode);
		websWrite(wp, "%c\"", i ? ',' : ' ');
		tf_webWriteJS(wp, tssid);
		websWrite(wp,
			  "\",\"%s\",\"%s\",\"%d (%d MHz)\",\"%d\",\"%d\",\"%d\",\"%s\",\"%s\",\"%d\",\"%s\"\n",
			  net, site_survey_lists[i].BSSID,
			  site_survey_lists[i].channel & 0xff,
			  site_survey_lists[i].frequency,
			  site_survey_lists[i].RSSI, site_survey_lists[i].phy_noise, site_survey_lists[i].beacon_period, open, site_survey_lists[i].ENCINFO, site_survey_lists[i].dtim_period, rates);

	}

	return;
}

#ifdef HAVE_WIVIZ

void ej_dump_wiviz_data(webs_t wp, int argc, char_t ** argv)	// Eko, for
								// testing
								// only
{
	FILE *f;
	char buf[256];

	killall("autokill_wiviz", SIGTERM);
	eval("autokill_wiviz");
	eval("run_wiviz");

	if ((f = fopen("/tmp/wiviz2-dump", "r")) != NULL) {
		while (fgets(buf, sizeof(buf), f)) {
			websWrite(wp, "%s", buf);
		}
		fclose(f);
	} else			// dummy data - to prevent first time js
		// error
	{
		websWrite(wp,
			  "top.hosts = new Array();\nvar hnum = 0;\nvar h;\nvar wiviz_cfg = new Object();\n wiviz_cfg.channel = 6\ntop.wiviz_callback(top.hosts, wiviz_cfg);\nfunction wiviz_callback(one, two) {\nalert(\'This asp is intended to run inside Wi-Viz.  You will now be redirected there.\');\nlocation.replace('Wiviz_Survey.asp');\n}\n");
	}
}

#endif
