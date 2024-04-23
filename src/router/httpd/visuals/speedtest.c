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

static void print_file(webs_t wp, char *filename, int string)
{
	FILE *in = fopen(filename, "rb");
	if (!in) {
		if (string)
			websWrite(wp, "&nbsp;");
		else
			websWrite(wp, "0");
		return;
	}
	char name[128];
	memset(name, 0, sizeof(name));
	fread(name, sizeof(name), 1, in);
	fclose(in);
	websWrite(wp, "%s", name);
}
EJ_VISIBLE void ej_speed_up(webs_t wp, int argc, char_t **argv)
{
	print_file(wp, "/tmp/speedtest_upload_result", 0);
}

EJ_VISIBLE void ej_speed_down(webs_t wp, int argc, char_t **argv)
{
	print_file(wp, "/tmp/speedtest_download_result", 0);
}

EJ_VISIBLE void ej_speed_name(webs_t wp, int argc, char_t **argv)
{
	print_file(wp, "/tmp/speedtest_name", 1);
}

EJ_VISIBLE void ej_speed_country(webs_t wp, int argc, char_t **argv)
{
	print_file(wp, "/tmp/speedtest_country", 1);
}

EJ_VISIBLE void ej_speed_sponsor(webs_t wp, int argc, char_t **argv)
{
	print_file(wp, "/tmp/speedtest_sponsor", 1);
}

EJ_VISIBLE void ej_speed_latency(webs_t wp, int argc, char_t **argv)
{
	print_file(wp, "/tmp/speedtest_latency", 0);
}

#endif
