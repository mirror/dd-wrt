/*
 * lib.c
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
#include <ctype.h>
#include <dirent.h>
#include <stdlib.h>
#include <net/if_arp.h>
#include <stdarg.h>

#include <broadcom.h>
#include <dd_defs.h>
#include <code_pattern.h>
#include <cy_conf.h>
#include <revision.h>

EJ_VISIBLE void ej_compile_date(webs_t wp, int argc, char_t **argv)
{
	char year[8], mon[4], day[4];
	char string[20];

	sscanf(__DATE__, "%s %s %s", mon, day, year);
	snprintf(string, sizeof(string), "%s. %s, %s", mon, day, year);

	websWrite(wp, "%s", string);
}

EJ_VISIBLE void ej_compile_time(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, "%s", __TIME__);
}

EJ_VISIBLE void ej_get_backup_name(webs_t wp, int argc, char_t **argv)
{
	char *name = nvram_safe_get("router_name");
	char *printname;
	asprintf(&printname, "nvrambak_r%s%s%s_%s.bin", SVN_REVISION,
		 *name ? "_" : "", *name ? name : "",
		 nvram_safe_get("DD_BOARD"));
	if (!printname)
		return;
	int len = strlen(printname);
	char *target = malloc(len * 3 + 1);
	if (!target) {
		debug_free(printname);
		return;
	}
	int i, t = 0;
	for (i = 0; i < len; i++) {
		if (printname[i] == ' ') {
			target[t++] = '%';
			target[t++] = '2';
			target[t++] = '0';
		} else
			target[t++] = printname[i];
	}
	target[t++] = 0;
	debug_free(printname);
	websWrite(wp, "%s", target);
	debug_free(target);
}

#ifndef HAVE_SPECIALEDITION

static void _ej_get_firmware_version(webs_t wp, int argc, char_t **argv,
				     int noreg)
{
#if defined(HAVE_ESPOD) || defined(HAVE_ONNET) || defined(HAVE_IMMERSIVE) || \
	defined(HAVE_HDWIFI) || defined(HAVE_IDEXX) || defined(HAVE_RAYTRONIK)
	char *p;
	char string[32], date[16];
	sprintf(string, CYBERTAN_VERSION);
	sprintf(date, "%s", BUILD_DATE);
#endif
#ifdef HAVE_BUFFALO
	websWrite(wp, "DD-WRT v3.0-r%s %s%s (" BUILD_DATE ")", SVN_REVISION,
		  nvram_safe_get("dist_type"), DIST_OPT);
#else

#ifdef HAVE_REGISTER
	if (!noreg && wp->isregistered && !wp->isregistered_real) {
		websWrite(wp, "Click here to ACTIVATE %d Hour Trial",
			  getTrialCount());
	} else
#endif
	{
#ifdef HAVE_WIKINGS
#ifdef HAVE_SUB3
#define V "ExcelMed"
#elif HAVE_SUB6
#define V "ExcelMin"
#else
#define V "Excellent"
#endif
		websWrite(wp, "Excel Networks (%s series) V 2.10", V);
#undef V
#elif HAVE_ESPOD
#ifdef HAVE_SUB3
#define V "A600"
#elif HAVE_SUB6
#define V "A1000"
#elif HAVE_SUB9
#define V "Hermes"
#elif HAVE_SUB12
#define V "Lite"
#else
#define V "MIMO"
#endif
		if (argc == 2) {
			websWrite(wp, "ESPOD v1.0611 (%s) / ESPOD %s Series",
				  date, V);
		} else {
			websWrite(
				wp,
				"ESPOD v1.0611 (%s)</a><div>\");document.write(\"<div class=\\\"info\\\">Device: ESPOD %s Series<a>",
				date, V);
		}
#undef V
#elif HAVE_CARLSONWIRELESS
		websWrite(wp, "Carlson Wireless v6.2 (%s)", SVN_REVISION);
#elif HAVE_IMMERSIVE
		if (argc == 2) {
			websWrite(wp, "Build date %s", date);
		} else {
			websWrite(wp, "SUPPORT %s (%s)", SVN_REVISION, date);
		}
#elif HAVE_HDWIFI
		if (argc == 2) {
			websWrite(wp, "Build date %s", date);
		} else {
			websWrite(wp, "HDWIFI r%s (%s)", SVN_REVISION, date);
		}
#elif HAVE_IPR
		websWrite(wp, "IPR-CP v1.0 (%s)", SVN_REVISION);
#elif HAVE_ONNET_BLANK
		websWrite(wp, "Enterprise AP (%s)", date);
#elif HAVE_ONNET
		if (nvram_match("DD_BOARD", "Atheros Hornet")) {
			websWrite(wp, "OTAi 9331 (%s)", date);
		} else if (nvram_match("DD_BOARD", "Compex WPE72")) {
			websWrite(wp, "OTAi 724 (%s)", date);
		} else if (nvram_match("DD_BOARD", "ACCTON AC622")) {
			if (iscpe()) {
				websWrite(wp, "OTAi 724S (%s)", date);
			} else {
				websWrite(wp, "OTAi 724AP (%s)", date);
			}
		} else if (nvram_match("DD_BOARD", "ACCTON AC722")) {
			if (iscpe()) {
				websWrite(wp, "OTAi 724S (%s)", date);
			} else {
				websWrite(wp, "OTAi 724AP (%s)", date);
			}
		} else if (nvram_match("DD_BOARD", "Compex WP546")) {
			websWrite(wp, "OTAi 724S (%s)", date);
		} else if (nvram_match("DD_BOARD", "Compex MMS344")) {
			websWrite(wp, "OTAi DBDC344 (%s)", date);
		} else if (nvram_match("DD_BOARD", "Yuncore XD3200")) {
			websWrite(wp, "OTAi 9563-AC (%s)", date);
		} else if (nvram_match("DD_BOARD", "Yuncore XD9531")) {
			websWrite(wp, "OTAi 9531 (%s)", date);
		} else if (nvram_match("DD_BOARD", "Yuncore SR3200")) {
			websWrite(wp, "OTAi 1200-AC (%s)", date);
		} else if (nvram_match("DD_BOARD", "Yuncore CPE890")) {
			websWrite(wp, "OTAi 5900-AC (%s)", date);
		} else if (nvram_match("DD_BOARD", "Alfa AP120C")) {
			websWrite(wp, "OTAi 600dbdc (%s)", date);
		} else if (nvram_match("DD_BOARD", "Yuncore CPE880")) {
			websWrite(wp, "OTAi-9334 (%s)", date);
		} else {
			websWrite(wp, "OTAi %s (%s)",
				  nvram_safe_get("DD_BOARD"), date);
		}
#elif HAVE_RAYTRONIK
		websWrite(wp, "RN-150M %s %s%s", MINOR_VERSION,
			  nvram_safe_get("dist_type"), DIST_OPT);
#elif HAVE_KORENRON
		websWrite(wp, "KORENRON %s %s%s", MINOR_VERSION,
			  nvram_safe_get("dist_type"), DIST_OPT);
#elif HAVE_TESTEM
		websWrite(wp, "TESTEM %s %s%s", MINOR_VERSION,
			  nvram_safe_get("dist_type"), DIST_OPT);
#elif HAVE_ANTAIRA
		websWrite(wp, "Antaira r%s (" BUILD_DATE ")", SVN_REVISION);
#elif HAVE_SANSFIL
		websWrite(wp, "SANSFIL %s %s%s", MINOR_VERSION,
			  nvram_safe_get("dist_type"), DIST_OPT);
#elif HAVE_HOBBIT
		websWrite(wp, "HQ-NDS %s %s%s", MINOR_VERSION,
			  nvram_safe_get("dist_type"), DIST_OPT);
#elif HAVE_ERC
		websWrite(wp, "RemoteEngineer FW 1.1 r%s (" BUILD_DATE ")",
			  SVN_REVISION);
#elif HAVE_IDEXX
#ifdef HAVE_IDEXX_WORLD
		websWrite(wp, "DD-WRT v3.0-r%s %s%s (" BUILD_DATE ") WW",
			  SVN_REVISION, nvram_safe_get("dist_type"), DIST_OPT);
#else
		websWrite(wp, "DD-WRT v3.0-r%s %s%s (" BUILD_DATE ") US",
			  SVN_REVISION, nvram_safe_get("dist_type"), DIST_OPT);
#endif
#elif HAVE_TMK
		websWrite(wp, "KMT-WAS 3.0 r%s (" BUILD_DATE ") std",
			  SVN_REVISION);
#elif HAVE_NDTRADE
		websWrite(wp, "ND TRADE v3.0-r%s %s%s (" BUILD_DATE ")",
			  SVN_REVISION, nvram_safe_get("dist_type"), DIST_OPT);
#else
		websWrite(wp, "DD-WRT v3.0-r%s %s%s (" BUILD_DATE ")",
			  SVN_REVISION, nvram_safe_get("dist_type"), DIST_OPT);
#endif
	}
#endif
}
#endif

EJ_VISIBLE void ej_get_firmware_version(webs_t wp, int argc, char_t **argv)
{
	_ej_get_firmware_version(wp, argc, argc, 0);
}

EJ_VISIBLE void ej_get_firmware_version_noreg(webs_t wp, int argc,
					      char_t **argv)
{
	_ej_get_firmware_version(wp, argc, argc, 1);
}
EJ_VISIBLE void ej_get_firmware_title(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, "Wireless-G Broadband Router");
}

EJ_VISIBLE void ej_get_firmware_svnrev(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, "%s", SVN_REVISION);
}

EJ_VISIBLE void ej_get_web_page_name(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, "%s.asp", websGetVar(wp, "submit_button", "index"));
}

EJ_VISIBLE void ej_get_model_name(webs_t wp, int argc, char_t **argv)
{
	// return websWrite(wp,"%s",MODEL_NAME);
	websWrite(wp, "%s", nvram_safe_get("router_name"));
}

EJ_VISIBLE void ej_show_logo(webs_t wp, int argc, char_t **argv)
{
	return;
}

int protocol_to_num(char *proto)
{
	if (!strcmp(proto, "icmp"))
		return 1;
	else if (!strcmp(proto, "tcp"))
		return 6;
	else if (!strcmp(proto, "udp"))
		return 17;
	else if (!strcmp(proto, "both"))
		return 23;
	else if (!strcmp(proto, "l7"))
		return 99;
	else if (!strcmp(proto, "p2p"))
		return 100;
#ifdef HAVE_OPENDPI
	else if (!strcmp(proto, "dpi"))
		return 101;
	else if (!strcmp(proto, "risk"))
		return 102;
#endif
	else
		return 0;
}
