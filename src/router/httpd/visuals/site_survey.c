/*
 * site_survey.c
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
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <signal.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <dd_defs.h>
#include <code_pattern.h>
#include <broadcom.h>
#include <proto/802.11.h>

#include <wlutils.h>

int getrate(int rate, int bw, int ac, int ax)
{
	int result = rate * 10;
	if (bw == 4)
		bw = 5;
	if (bw == 8)
		bw = 10;
	if (bw == 16)
		bw = 20;
	switch (rate) {
	case 150:
		if (ax) {
			if (bw == 2)
				result = 1434 / 8;
			if (bw == 5)
				result = 1434 / 4;
			if (bw == 10)
				result = 1434 / 2;
			if (bw == 20)
				result = 1434;
			if (bw == 40)
				result = 2868;
			if (bw == 80)
				result = 6005;
			if (bw == 160)
				result = 12010;
		} else if (ac) {
			if (bw == 2)
				result = 867 / 8;
			if (bw == 5)
				result = 867 / 4;
			if (bw == 10)
				result = 867 / 2;
			if (bw == 20)
				result = 867;
			if (bw == 40)
				result = 2000;
			if (bw == 80)
				result = 4333;
			if (bw == 160)
				result = 8667;
		} else {
			if (bw == 2)
				result = 722 / 8;
			if (bw == 5)
				result = 722 / 4;
			if (bw == 10)
				result = 722 / 2;
			if (bw == 20)
				result = 722;
			if (bw == 40)
				result = 1500;
			if (bw == 80)
				result = 3250;
			if (bw == 160)
				result = 6500;
		}
		break;
	case 300:
		if (ax) {
			if (bw == 2)
				result = 2868 / 8;
			if (bw == 5)
				result = 2868 / 4;
			if (bw == 10)
				result = 2868 / 2;
			if (bw == 20)
				result = 2868;
			if (bw == 40)
				result = 5735;
			if (bw == 80)
				result = 12010;
			if (bw == 160)
				result = 24020;
		} else if (ac) {
			if (bw == 2)
				result = 1733 / 8;
			if (bw == 5)
				result = 1733 / 4;
			if (bw == 10)
				result = 1733 / 2;
			if (bw == 20)
				result = 1733;
			if (bw == 40)
				result = 4000;
			if (bw == 80)
				result = 8667;
			if (bw == 160)
				result = 17333;
		} else {
			if (bw == 2)
				result = 1444 / 8;
			if (bw == 5)
				result = 1444 / 4;
			if (bw == 10)
				result = 1444 / 2;
			if (bw == 20)
				result = 1444;
			if (bw == 40)
				result = 3000;
			if (bw == 80)
				result = 6500;
			if (bw == 160)
				result = 13000;
		}
		break;
	case 450:
		if (ax) {
			if (bw == 2)
				result = 4301 / 8;
			if (bw == 5)
				result = 4301 / 4;
			if (bw == 10)
				result = 4301 / 2;
			if (bw == 20)
				result = 4301;
			if (bw == 40)
				result = 8125;
			if (bw == 80)
				result = 18015;
			if (bw == 160)
				result = 36029; // this rate is not specified
		} else if (ac) {
			if (bw == 2)
				result = 2889 / 8;
			if (bw == 5)
				result = 2889 / 4;
			if (bw == 10)
				result = 2889 / 2;
			if (bw == 20)
				result = 2889;
			if (bw == 40)
				result = 6000;
			if (bw == 80)
				result = 13000;
			if (bw == 160)
				result = 23400; // this rate is not specified
		} else {
			if (bw == 2)
				result = 2167 / 8;
			if (bw == 5)
				result = 2167 / 4;
			if (bw == 10)
				result = 2167 / 2;
			if (bw == 20)
				result = 2167;
			if (bw == 40)
				result = 4500;
			if (bw == 80)
				result = 9750;
			if (bw == 160)
				result = 19500; // this rate is not specified
		}
		break;
	case 600:
		if (ax) {
			if (bw == 2)
				result = 5735 / 8;
			if (bw == 5)
				result = 5735 / 4;
			if (bw == 10)
				result = 5735 / 2;
			if (bw == 20)
				result = 5735;
			if (bw == 40)
				result = 11471;
			if (bw == 80)
				result = 24020;
			if (bw == 160)
				result = 48039;
		} else if (ac) {
			if (bw == 2)
				result = 3467 / 8;
			if (bw == 5)
				result = 3467 / 4;
			if (bw == 10)
				result = 3467 / 2;
			if (bw == 20)
				result = 3467;
			if (bw == 40)
				result = 8000;
			if (bw == 80)
				result = 17333;
			if (bw == 160)
				result = 34667;
		} else {
			if (bw == 2)
				result = 2889 / 8;
			if (bw == 5)
				result = 2889 / 4;
			if (bw == 10)
				result = 2889 / 2;
			if (bw == 20)
				result = 2889;
			if (bw == 40)
				result = 6000;
			if (bw == 80)
				result = 13000;
			if (bw == 160)
				result = 16000;
		}
		break;
	}
	return result;
}

static char *speedstr(int speed, char *buf, size_t len)
{
	if (speed % 10)
		snprintf(buf, len, "%d.%d", speed / 10, speed % 10);
	else
		snprintf(buf, len, "%d", speed / 10);
	return buf;
}

static char *dtim_period(int dtim, char *mem)
{
	if (dtim)
		snprintf(mem, 32, "%d", dtim);
	else
		snprintf(mem, 32, "%s", "None");
	return mem;
}

EJ_VISIBLE void ej_dump_site_survey(webs_t wp, int argc, char_t **argv)
{
	int i;
	char *rates = NULL;
	char *name;
	char speedbuf[32];
	struct site_survey_list *site_survey_lists;
	name = argv[0];
	

	site_survey_lists = open_site_survey(name);
	if (site_survey_lists)
	    return;
	for (i = 0; i < SITE_SURVEY_NUM; i++) {
		char rates[64];

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

		if (site_survey_lists[i].channel & 0x1000) {
			int cbw = site_survey_lists[i].channel & 0x300;
			//0x000 = 20/40 mhz
			//0x100 = 80 mhz
			//0x200 = 8080 or 160 mhz
			int speed = site_survey_lists[i].rate_count;
			int s = 20;
			int narrow = atoi(nvram_nget("%s_channelbw", nvram_safe_get("wifi_display")));
			if (narrow == 5 || narrow == 10 || narrow == 2)
				s = narrow;
			//fprintf(stderr, "%d %d %d\n", s, speed, site_survey_lists[i].extcap);
			int hasac = 0;
			int hasax = 0;
			if (site_survey_lists[i].extcap & CAP_VHT)
				hasac = 1;
			if (site_survey_lists[i].extcap & CAP_AX)
				hasax = 1;
			switch (cbw) {
			case 0x0:
				if (site_survey_lists[i].extcap & CAP_SECCHANNEL)
					speed = getrate(speed, s * 2, hasac, hasax);
				else
					speed = getrate(speed, s, hasac, hasax);
				break;
			case 0x100:
				speed = getrate(speed, s * 4, hasac, hasax);
				break;
			case 0x200:
			case 0x300:
				speed = getrate(speed, s * 8, hasac, hasax);
				break;
			default:
				speed = speed * 10;
			}

			if ((site_survey_lists[i].channel & 0xff) < 15) {
				if (hasax && hasac)
					sprintf(rates, "%s(b/g/n/ac/ax)", speedstr(speed, speedbuf, sizeof(speedbuf)));
				else if (hasax)
					sprintf(rates, "%s(b/g/n/ax)", speedstr(speed, speedbuf, sizeof(speedbuf)));
				else if (hasac)
					sprintf(rates, "%s(b/g/n/ac)", speedstr(speed, speedbuf, sizeof(speedbuf)));
				else
					sprintf(rates, "%s(b/g/n)", speedstr(speed, speedbuf, sizeof(speedbuf)));
			} else {
				if (hasax)
					sprintf(rates, "%s(a/n/ac/ax)", speedstr(speed, speedbuf, sizeof(speedbuf)));
				else if (hasac)
					sprintf(rates, "%s(a/n/ac)", speedstr(speed, speedbuf, sizeof(speedbuf)));
				else
					sprintf(rates, "%s(a/n)", speedstr(speed, speedbuf, sizeof(speedbuf)));
			}

		} else if (site_survey_lists[i].channel & 0x2000) {
			int speed = 0;
			int rc = site_survey_lists[i].rate_count;
			switch (rc) {
			case 4:
			case 11:
				rc = 4;
				speed = 110;
				break;
			case 12:
			case 54:
				rc = 12;
				speed = 540;
				break;
			case 13:
				speed = 1080;
				break;
			default:
				speed = rc * 10;
			}

			if ((site_survey_lists[i].channel & 0xff) < 15) {
				sprintf(rates, "%s%s", speedstr(speed, speedbuf, sizeof(speedbuf)),
					rc == 4 ? "(b)" :
					rc < 14 ? "(b/g)" :
						  "(b/g/n)");
			} else {
				sprintf(rates, "%s%s", speedstr(speed, speedbuf, sizeof(speedbuf)), rc < 14 ? "(a)" : "(a/n)");
			}

		} else {
			int speed = 0;
			int rc = site_survey_lists[i].rate_count;
			switch (rc) {
			case 4:
			case 11:
				rc = 4;
				speed = 110;
				break;
			case 12:
			case 54:
				rc = 12;
				speed = 540;
				break;
			case 13:
				speed = 1080;
				break;
			default:
				speed = getrate(rc, 20, 0, 0);
			}

			if ((site_survey_lists[i].channel & 0xff) < 15) {
				sprintf(rates, "%s%s", speedstr(speed, speedbuf, sizeof(speedbuf)),
					rc == 4 ? "(b)" :
					rc < 14 ? "(b/g)" :
						  "(b/g/n)");
			} else {
				sprintf(rates, "%s%s", speedstr(speed, speedbuf, sizeof(speedbuf)), rc < 14 ? "(a)" : "(a/n)");
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
		strlcpy(open,
			(site_survey_lists[i].capability & DOT11_CAP_PRIVACY) ? live_translate(wp, "share.no") :
										live_translate(wp, "share.yes"),
			sizeof(open));

		char *netmode;
		int netmodecap = site_survey_lists[i].capability;

		netmodecap &= (DOT11_CAP_ESS | DOT11_CAP_IBSS);
		if (site_survey_lists[i].extcap & CAP_MESH)
			netmode = "Mesh";
		else if (netmodecap == DOT11_CAP_ESS)
			netmode = "AP";
		else if (netmodecap == DOT11_CAP_IBSS)
			netmode = "AdHoc";
		else
			netmode = live_translate(wp, "share.unknown");
		if (site_survey_lists[i].extcap & CAP_DWDS)
			netmode = "AP DWDS";
		if (site_survey_lists[i].extcap & CAP_MTIKWDS) {
			//since dd-wrt does broadcast this flag for wds in the same way, but without the dirty mikrotik implementation we may use it for detection
			netmode = "AP WDS";
		}

		char net[32];
		strcpy(net, netmode);
		websWrite(wp, "%c\"", i ? ',' : ' ');
		tf_webWriteJS(wp, tssid);
		unsigned long long quality;
		char dtim[32];
		if (site_survey_lists[i].active)
			quality = 100 - (site_survey_lists[i].busy * 100 / site_survey_lists[i].active);
		else
			quality = 100;
		char numsta[32];
		sprintf(numsta, "%d", site_survey_lists[i].numsta);
		websWrite(
			wp,
			"\",\"%s\",\"%s\",\"%d\",\"%d\",\"%s\",\"%s\",\"%d\",\"%d\",\"%llu\",\"%d\",\"%s\",\"%s\",\"%s\",\"%s\"\n",
			net, site_survey_lists[i].BSSID, site_survey_lists[i].channel & 0xff, site_survey_lists[i].frequency,
			site_survey_lists[i].numsta == -1 ? "N/A" : numsta, site_survey_lists[i].radioname,
			site_survey_lists[i].RSSI, site_survey_lists[i].phy_noise, quality, site_survey_lists[i].beacon_period,
			open, site_survey_lists[i].ENCINFO, dtim_period(site_survey_lists[i].dtim_period, dtim), rates);
	}
	debug_free(site_survey_lists);

	return;
}

#ifdef HAVE_WIVIZ

#if !defined(HAVE_MICRO) && !defined(__UCLIBC__)
#include <pthread.h>
static char *lastlock;
static char *lastunlock;
#define wiz_lock() pthread_mutex_lock(&wp->p->wiz_mutex_contr)
#define wiz_unlock() pthread_mutex_unlock(&wp->p->wiz_mutex_contr)
#else
#define wiz_lock()
#define wiz_unlock()
#endif

EJ_VISIBLE void ej_dump_wiviz_data(webs_t wp, int argc,
				   char_t **argv) // Eko, for
// testing
// only
{
	FILE *f;
	char buf[256];
	wiz_lock();
	killall("autokill_wiviz", SIGKILL);
	eval("autokill_wiviz");

	if (pidof("wiviz") > 0)
		killall("wiviz", SIGUSR1);
	else {
		char *hopseq = nvram_safe_get("hopseq");
		FILE *fp = fopen("/tmp/wiviz2-cfg", "wb");
		if (nvram_matchi("hopseq", 0))
			fprintf(fp, "channelsel=hop&");
		else if (strstr(hopseq, ","))
			fprintf(fp, "channelsel=hop&");
		else
			fprintf(fp, "channelsel=%s&", hopseq);
		fprintf(fp, "hopdwell=%s&hopseq=%s\n", nvram_safe_get("hopdwell"), hopseq);
		fclose(fp);
		eval("wiviz");
	}
	int cnt = 0;
	FILE *w = NULL;
	if ((f = fopen("/tmp/wiviz2-dump", "r")) != NULL) {
		while (nvram_invmatch("wiviz2_dump_done",
				      "1")) { // wait until writing is done
			struct timespec tim, tim2;
			tim.tv_sec = 0;
			tim.tv_nsec = 10000000L;
			nanosleep(&tim, &tim2);
			if (cnt++ > 100) {
				fclose(f);
				/* in case there is a problem, read backup */
				f = fopen("/tmp/wiviz2-old", "r");
				if (!f)
					goto err;
				goto read_old;
			}
		}
		w = fopen("/tmp/wiviz2-old", "wb");
		if (!w) {
			wiz_unlock();
			return;
		}
read_old:;
		while (!feof(f)) {
			char *str = fgets(buf, sizeof(buf), f);
			if (str) {
				websWrite(wp, "%s", buf);
				if (w)
					fprintf(w, "%s", buf);
			}
		}
		if (w)
			fclose(w);
		fclose(f);
	} else // dummy data - to prevent first time js
	// error
	{
err:;
		websWrite(
			wp,
			"top.hosts = new Array();\nvar hnum = 0;\nvar h;\nvar wiviz_cfg = new Object();\n wiviz_cfg.channel = 6\ntop.wiviz_callback(top.hosts, wiviz_cfg);\nfunction wiviz_callback(one, two) {\nalert(\'This asp is intended to run inside Wi-Viz.  You will now be redirected there.\');\nlocation.replace('Wiviz_Survey.asp');\n}\n");
	}
	nvram_unset("wiviz2_dump_done");
	wiz_unlock();
}

#endif
