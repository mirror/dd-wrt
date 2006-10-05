/*
 * site_survey.c
 *
 * Copyright (C) 2005 - 2006 Sebastian Gottschall <sebastian.gottschall@blueline-ag.de>
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
#include <string.h>
#include <unistd.h>
#include <signal.h>

#include <typedefs.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <wlioctl.h>
#include <wlutils.h>

#define sys_restart() kill(1, SIGHUP)
#define SITE_SURVEY_DB	"/tmp/site_survey"
#define SITE_SURVEY_NUM	50

int write_site_survey (void);
static int open_site_survey (void);
int write_site_survey (void);

struct site_survey_list
{
  uint8 SSID[32];
  unsigned char BSSID[18];
  uint8 channel;		/* Channel no. */
  int16 RSSI;			/* receive signal strength (in dBm) */
  int8 phy_noise;		/* noise (in dBm) */
  uint16 beacon_period;		/* units are Kusec */
  uint16 capability;		/* Capability information */
  uint rate_count;		/* # rates in this set */
  uint8 dtim_period;		/* DTIM period */
} site_survey_lists[SITE_SURVEY_NUM];

int
site_survey_main (int argc, char *argv[])
{
  char *name = nvram_safe_get ("wl0_ifname");
  unsigned char buf[10000];
  wl_scan_results_t *scan_res = (wl_scan_results_t *) buf;
  wl_bss_info_t *bss_info;
  unsigned char mac[20];
  int i;
  char *dev = name;
  unlink (SITE_SURVEY_DB);
  int ap = 0, oldap = 0;
  wl_scan_params_t params;

  memset (&params, 0, sizeof (params));

  /* use defaults (same parameters as wl scan) */

  memset (&params.bssid, 0xff, sizeof (params.bssid));
  if (argc > 1)
    {
      params.ssid.SSID_len = strlen (argv[1]);
      strcpy (params.ssid.SSID, argv[1]);
    }
  params.bss_type = DOT11_BSSTYPE_ANY;
  params.scan_type = -1;
  params.nprobes = -1;
  params.active_time = -1;
  params.passive_time = -1;
  params.home_time = -1;

  /* can only scan in STA mode */
  
  wl_ioctl (dev, WLC_GET_AP, &oldap, sizeof (oldap));
  if (oldap > 0)
    wl_ioctl (dev, WLC_SET_AP, &ap, sizeof (ap));
  if (wl_ioctl (dev, WLC_SCAN, &params, 64) < 0)
    {
    fprintf(stderr,"scan failed\n");
    return -1;
    }
  sleep (1);
  bzero (buf, sizeof (buf));
  scan_res->buflen = sizeof (buf);

  if (wl_ioctl (dev, WLC_SCAN_RESULTS, buf, WLC_IOCTL_MAXLEN) < 0)
  {
    fprintf(stderr,"scan results failed\n");
    return -1;
  }

  fprintf (stderr,"buflen=[%d] version=[%d] count=[%d]\n", scan_res->buflen,
	  scan_res->version, scan_res->count);

  if (scan_res->count == 0)
    {
      cprintf ("Can't find any wireless device\n");
      goto endss;
    }

  bss_info = &scan_res->bss_info[0];
  for (i = 0; i < scan_res->count; i++)
    {
      strcpy (site_survey_lists[i].SSID, bss_info->SSID);
      strcpy (site_survey_lists[i].BSSID,
	      ether_etoa (bss_info->BSSID.octet, mac));
#ifndef HAVE_RB500
#ifndef HAVE_MSSID
      site_survey_lists[i].channel = bss_info->channel;
#else
      site_survey_lists[i].channel = bss_info->chanspec & 0xff;
#endif
#endif
      site_survey_lists[i].RSSI = bss_info->RSSI;
      site_survey_lists[i].phy_noise = bss_info->phy_noise;
      site_survey_lists[i].beacon_period = bss_info->beacon_period;
      site_survey_lists[i].capability = bss_info->capability;
      site_survey_lists[i].rate_count = bss_info->rateset.count;
      site_survey_lists[i].dtim_period = bss_info->dtim_period;

      bss_info = (wl_bss_info_t *) ((uint32) bss_info + bss_info->length);
    }
  write_site_survey ();
  open_site_survey ();
  for (i = 0; i < SITE_SURVEY_NUM && site_survey_lists[i].SSID[0]; i++)
    {
      printf
	("[%2d] SSID[%20s] BSSID[%s] channel[%2d] rssi[%d] noise[%d] beacon[%d] cap[%x] dtim[%d] rate[%d]\n",
	 i, site_survey_lists[i].SSID, site_survey_lists[i].BSSID,
	 site_survey_lists[i].channel, site_survey_lists[i].RSSI,
	 site_survey_lists[i].phy_noise, site_survey_lists[i].beacon_period,
	 site_survey_lists[i].capability, site_survey_lists[i].dtim_period,
	 site_survey_lists[i].rate_count);
    }

endss:
  if (oldap > 0)
    wl_ioctl (dev, WLC_SET_AP, &oldap, sizeof (oldap));
  
  C_led (0);
#ifdef HAVE_MSSID
  eval("wl","up");
#endif
  return 0;
}

int
write_site_survey (void)
{
  FILE *fp;

  if ((fp = fopen (SITE_SURVEY_DB, "w")))
    {
      fwrite (&site_survey_lists[0], sizeof (site_survey_lists), 1, fp);
      fclose (fp);
      return FALSE;
    }
  return TRUE;
}

static int
open_site_survey (void)
{
  FILE *fp;

  bzero (site_survey_lists, sizeof (site_survey_lists));

  if ((fp = fopen (SITE_SURVEY_DB, "r")))
    {
      fread (&site_survey_lists[0], sizeof (site_survey_lists), 1, fp);
      fclose (fp);
      return TRUE;
    }
  return FALSE;
}
