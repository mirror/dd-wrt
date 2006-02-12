
/*
 *********************************************************
 *   Copyright 2004, CyberTAN  Inc.  All Rights Reserved *
 *********************************************************

 This is UNPUBLISHED PROPRIETARY SOURCE CODE of CyberTAN Inc.
 the contents of this file may not be disclosed to third parties,
 copied or duplicated in any form without the prior written
 permission of CyberTAN Inc.

 This software should be used as a reference only, and it not
 intended for production use!


 THIS SOFTWARE IS OFFERED "AS IS", AND CYBERTAN GRANTS NO WARRANTIES OF ANY
 KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE.  CYBERTAN
 SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE
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
int open_site_survey (void);
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
  int ret;
  char *dev = name;
  unlink (SITE_SURVEY_DB);
  puts ("switch radio");

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
  printf ("set sta\n");
  wl_ioctl (dev, WLC_GET_AP, &oldap, sizeof (oldap));
  if (oldap > 0)
    wl_ioctl (dev, WLC_SET_AP, &ap, sizeof (ap));
  puts ("do scan");
  if (wl_ioctl (dev, WLC_SCAN, &params, 64) < 0)
    return -1;
  sleep (1);
  bzero (buf, sizeof (buf));
  scan_res->buflen = sizeof (buf);

  puts ("scan results");
  if (wl_ioctl (dev, WLC_SCAN_RESULTS, buf, WLC_IOCTL_MAXLEN) < 0)
    return -1;

  printf ("buflen=[%d] version=[%d] count=[%d]\n", scan_res->buflen,
	  scan_res->version, scan_res->count);

  if (scan_res->count == 0)
    {
      cprintf ("Can't find any wireless device\n");
      goto endss;
    }

  bss_info = &scan_res->bss_info[0];
  puts ("create db");
  for (i = 0; i < scan_res->count; i++)
    {
      strcpy (site_survey_lists[i].SSID, bss_info->SSID);
      strcpy (site_survey_lists[i].BSSID,
	      ether_etoa (bss_info->BSSID.octet, mac));
      site_survey_lists[i].channel = bss_info->channel;
      site_survey_lists[i].RSSI = bss_info->RSSI;
      site_survey_lists[i].phy_noise = bss_info->phy_noise;
      site_survey_lists[i].beacon_period = bss_info->beacon_period;
      site_survey_lists[i].capability = bss_info->capability;
      site_survey_lists[i].rate_count = bss_info->rateset.count;
      site_survey_lists[i].dtim_period = bss_info->dtim_period;

      bss_info = (wl_bss_info_t *) ((uint32) bss_info + bss_info->length);
    }
  puts ("write db");
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
  puts ("switch back");
  puts ("reset sta");
  if (oldap > 0)
    wl_ioctl (dev, WLC_SET_AP, &oldap, sizeof (oldap));

  /*if(!nvram_match("wl_mode", "sta") && !nvram_match("wl_mode", "wet") && !nvram_match("wl_mode", "bridge")) {
     eval("wl", "ap", "1");

     //   C_led(1);
     //   sys_restart();
     } */
  //sleep(3);
  C_led (0);
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
