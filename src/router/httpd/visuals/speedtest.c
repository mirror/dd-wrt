/*
 * speedtest.c
 *
 * Copyright (C) 2005 - 2023 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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
#ifdef HAVE_SPEEDTEST_CLI
#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <broadcom.h>

EJ_VISIBLE void ej_speed_up(webs_t wp, int argc, char_t **argv)
{
	FILE *in = fopen("/tmp/speedtest_upload_result", "rb");
	if (!in) {
		websWrite(wp, "0");
		return;
	}
	char str[32];
	fscanf(in, "%s", &str[0]);
	fclose(in);
	websWrite(wp, "%s", str);
}

EJ_VISIBLE void ej_speed_down(webs_t wp, int argc, char_t **argv)
{
	FILE *in = fopen("/tmp/speedtest_download_result", "rb");
	if (!in) {
		websWrite(wp, "0");
		return;
	}
	char str[32];
	fscanf(in, "%s", &str[0]);
	fclose(in);
	websWrite(wp, "%s", str);
}

EJ_VISIBLE void ej_speed_name(webs_t wp, int argc, char_t **argv)
{
	FILE *in = fopen("/tmp/speedtest_name", "rb");
	if (!in) {
		websWrite(wp, "&nbsp;");
		return;
	}
	char name[128];
	memset(name, 0, sizeof(name));
	fread(name, sizeof(name), 1, in);
	websWrite(wp, "%s", name);
	fclose(in);
}

EJ_VISIBLE void ej_speed_country(webs_t wp, int argc, char_t **argv)
{
	FILE *in = fopen("/tmp/speedtest_country", "rb");
	if (!in) {
		websWrite(wp, "&nbsp;");
		return;
	}
	char name[128];
	memset(name, 0, sizeof(name));
	fread(name, sizeof(name), 1, in);
	websWrite(wp, "%s", name);
	fclose(in);
}

EJ_VISIBLE void ej_speed_sponsor(webs_t wp, int argc, char_t **argv)
{
	FILE *in = fopen("/tmp/speedtest_sponsor", "rb");
	if (!in) {
		websWrite(wp, "&nbsp;");
		return;
	}
	char name[128];
	memset(name, 0, sizeof(name));
	fread(name, sizeof(name), 1, in);
	websWrite(wp, "%s", name);
	fclose(in);
}

#endif
