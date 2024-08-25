/*
 * gps.c
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
#ifdef HAVE_GPSI

#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdarg.h>

#include <broadcom.h>

#ifdef HAVE_UNIWIP
EJ_VISIBLE void ej_gps_status(webs_t wp, int argc, char_t **argv)
{
	int antennastate = get_gpio(242);
	if (!antennastate)
		websWrite(wp, "<script type=\"text/javascript\">Capture(status_gpsi.ant_conn)</script>");
	else
		websWrite(wp, "<script type=\"text/javascript\">Capture(status_gpsi.ant_disc)</script>");
}
#else
EJ_VISIBLE void ej_gps_status(webs_t wp, int argc, char_t **argv)
{
	websWrite(wp, "%s", nvram_safe_get("gps_status_text"));
}
#endif

EJ_VISIBLE void ej_getlongitude(webs_t wp, int argc, char_t **argv)
{
	char *lon = nvram_safe_get("gps_lon");
	char lon_deg[4];
	char lon_min1[3];
	char lon_min2[32];
	char lon_min3[32];
	if (*(lon) == 0) {
		websWrite(wp, "<script type=\"text/javascript\">Capture(status_gpsi.na)</script>");
		return;
	}
	strncpy(lon_deg, lon, 3);
	strncpy(lon_min1, lon + 3, 2);
	strcpy(lon_min2, lon + 6);

	lon_deg[3] = 0;
	lon_min1[2] = 0;

	sprintf(lon_min3, "0.%s%s", lon_min1, lon_min2);
	float lon_dec = atof(lon_min3) / .6f;
	float lon_val = atof(lon_deg) + lon_dec;
	if (nvram_invmatch("gps_lon_e", "E"))
		lon_val *= -1;
	websWrite(wp, "%f", lon_val);
	return;
}

EJ_VISIBLE void ej_getlatidude(webs_t wp, int argc, char_t **argv)
{
	char *lat = nvram_safe_get("gps_lat");
	char lat_deg[4];
	char lat_min1[3];
	char lat_min2[32];
	char lat_min3[32];
	if (*(lat) == 0) {
		websWrite(wp, "<script type=\"text/javascript\">Capture(status_gpsi.na)</script>");
		return;
	}
	strncpy(lat_deg, lat, 2);
	strncpy(lat_min1, lat + 2, 2);
	strcpy(lat_min2, lat + 5);
	lat_deg[2] = 0;
	lat_min1[2] = 0;
	sprintf(lat_min3, "0.%s%s", lat_min1, lat_min2);
	float lat_dec = atof(lat_min3) / .6f;
	float lat_val = atof(lat_deg) + lat_dec;
	if (nvram_invmatch("gps_lat_e", "N"))
		lat_val *= -1;

	websWrite(wp, "%f", lat_val);
	return;
}

EJ_VISIBLE void ej_getgpslink(webs_t wp, int argc, char_t **argv)
{
	char *lon = nvram_safe_get("gps_lon");
	char lon_deg[4];
	char lon_min1[3];
	char lon_min2[32];
	char lon_min3[32];
	if (*(lon) == 0) {
		websWrite(wp, "<script type=\"text/javascript\">Capture(status_gpsi.na)</script>");
		return;
	}
	strncpy(lon_deg, lon, 3);
	strncpy(lon_min1, lon + 3, 2);
	strcpy(lon_min2, lon + 6);
	lon_deg[3] = 0;
	lon_min1[2] = 0;
	sprintf(lon_min3, "0.%s%s", lon_min1, lon_min2);
	float lon_dec = atof(lon_min3) / .6f;
	float lon_val = atof(lon_deg) + lon_dec;
	if (nvram_invmatch("gps_lon_e", "E"))
		lon_val *= -1;

	char *lat = nvram_safe_get("gps_lat");
	char lat_deg[4];
	char lat_min1[3];
	char lat_min2[32];
	char lat_min3[32];
	if (*(lat) == 0) {
		websWrite(wp, "<script type=\"text/javascript\">Capture(status_gpsi.na)</script>");
		return;
	}
	strncpy(lat_deg, lat, 2);
	strncpy(lat_min1, lat + 2, 2);
	strcpy(lat_min2, lat + 5);
	lat_deg[2] = 0;
	lat_min1[2] = 0;
	sprintf(lat_min3, "0.%s%s", lat_min1, lat_min2);
	float lat_dec = atof(lat_min3) / .6f;
	float lat_val = atof(lat_deg) + lat_dec;
	if (nvram_invmatch("gps_lat_e", "N"))
		lat_val *= -1;

	websWrite(wp, "<a href=\"https://maps.google.com/maps?q=%f,%f\" target=\"_blank\">Google Maps</a>", lat_val, lon_val);
}

#endif
