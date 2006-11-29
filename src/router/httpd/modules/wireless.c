
/*
 * Broadcom Home Gateway Reference Design
 * Web Page Configuration Support Routines
 *
 * Copyright 2001-2003, Broadcom Corporation
 * All Rights Reserved.
 * 
 * THIS SOFTWARE IS OFFERED "AS IS", AND BROADCOM GRANTS NO WARRANTIES OF ANY
 * KIND, EXPRESS OR IMPLIED, BY STATUTE, COMMUNICATION OR OTHERWISE. BROADCOM
 * SPECIFICALLY DISCLAIMS ANY IMPLIED WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A SPECIFIC PURPOSE OR NONINFRINGEMENT CONCERNING THIS SOFTWARE.
 * $Id: wireless.c,v 1.6 2005/11/30 11:53:42 seg Exp $
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <error.h>
#include <signal.h>

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <broadcom.h>
#include <wlioctl.h>
#include <wlutils.h>

#define ASSOCLIST_TMP	"/tmp/.wl_assoclist"
#define OLD_NAME_IP	"/tmp/.old_name_ip"
#define ASSOCLIST_CMD	"wl assoclist"

#define LEASES_NAME_IP	"/tmp/.leases_name_ip"

#undef ABURN_WSEC_CHECK

#ifndef GMODE_AFTERBURNER
#	define GMODE_AFTERBURNER 7
#endif

/*
  WEP Format:
  wl_wep_buf=111:371A82447F:FBEA2AB7D4:1C9D814E6C:B4695172B4:2  (only for UI read)
  wl_wep_gen=111:371A82447F:FBEA2AB7D4:1C9D814E6C:B4695172B4:2  (only for UI read)
  wl_key=2
  wl_key1=371A82447F
  wl_key2=FBEA2AB7D4
  wl_key3=1C9D814E6C
  wl_key4=B4695172B4
  wl_passphrase=111 (only for UI)
  wl_wep_bit=64 (only for UI)

 */

extern unsigned char key128[4][13];
extern unsigned char key64[4][5];

struct wl_client_mac
{
  unsigned char hostname[32];
  char ipaddr[20];
  char hwaddr[20];
  int status;			// 0:offline 1:online
  int check;
} wl_client_macs[MAX_LEASES];

struct lease_table
{
  unsigned char hostname[32];
  char ipaddr[20];
  char hwaddr[20];
} *dhcp_lease_table;

int generate_key;
extern void gen_key (char *genstr, int weptype);
int nv_count;
extern struct variable variables[];
																																																																																													    /* channel info structure *///from 11.9
typedef struct
{
  uint chan;			/* channel number */
  uint freq;			/* in Mhz */
} chan_info_t;

static chan_info_t chan_info[] = {
  /* A channels */
  /* 11a usa low */
  {36, 5180},
  {40, 5200},
  {44, 5220},
  {48, 5240},
  {52, 5260},
  {56, 5280},
  {60, 5300},
  {64, 5320},

  /* 11a Europe */
  {100, 5500},
  {104, 5520},
  {108, 5540},
  {112, 5560},
  {116, 5580},
  {120, 5600},
  {124, 5620},
  {128, 5640},
  {132, 5660},
  {136, 5680},
  {140, 5700},

  /* 11a usa high */
  {149, 5745},
  {153, 5765},
  {157, 5785},
  {161, 5805},

  /* 11a japan */
  {184, 4920},
  {188, 4940},
  {192, 4960},
  {196, 4980},
  {200, 5000},
  {204, 5020},
  {208, 5040},
  {212, 5060},
  {216, 5080}
};

#ifndef HAVE_MSSID
void
validate_security_mode (webs_t wp, char *value, struct variable *v)
{
  char *security_mode_last = websGetVar (wp, "security_mode_last", NULL);
  char *wl_wep_last = websGetVar (wp, "wl_wep_last", NULL);
  int from_index_page = 0;
  char *wl_wep = NULL;

  //If you don't press "Edit Security Setting" to set some value, and direct select to enable "Wireless Security".
  //It'll returned, due to security_mode_buf is space.
  if (!strcmp (value, "enabled"))
    {
      if (nvram_match ("security_mode_last", ""))	// from index.asp and first time
	return;
      else
	{
	  if (!security_mode_last)
	    {			// from index.asp
	      from_index_page = 1;
	      value = nvram_safe_get ("security_mode_last");
	      wl_wep = nvram_safe_get ("wl_wep_last");
	    }
	  else
	    {			// from WL_WPATable.asp page
	      value = websGetVar (wp, "security_mode_last", NULL);
	      wl_wep = nvram_safe_get ("wl_wep_last");
	    }
	}
    }

  if (!valid_choice (wp, value, v))
    return;

  if (!strcmp (value, "disabled"))
    {
      nvram_set ("security_mode", "disabled");
      nvram_set ("wl_akm", "");
      nvram_set ("wl_auth_mode", "none");
      nvram_set ("wl_wep", "disabled");
    }
  else if (!strcmp (value, "psk"))
    {
      nvram_set ("wl_akm", value);
      nvram_set ("wl_auth_mode", "none");
      nvram_set ("wl_wep", "disabled");
    }
  else if (!strcmp (value, "wpa"))
    {
      nvram_set ("wl_akm", value);
      nvram_set ("wl_auth_mode", "none");
      nvram_set ("wl_wep", "disabled");
    }
  else if (!strcmp (value, "radius"))
    {
      nvram_set ("security_mode", "radius");
      nvram_set ("wl_akm", "");
      nvram_set ("wl_auth_mode", "radius");
      nvram_set ("wl_wep", "enabled");	// the nas need this value, the "restricted" is no longer need. (20040624 by honor)
    }
  else if (!strcmp (value, "wep"))
    {
      nvram_set ("wl_akm", "");
      nvram_set ("wl_auth_mode", "none");
      nvram_set ("wl_wep", "enabled");	// the nas need this value, the "restricted" is no longer need. (20040624 by honor)
    }
  else if (!strcmp (value, "psk2"))
    {				// WPA2 Only Mode
      nvram_set ("wl_akm", value);
      nvram_set ("wl_auth_mode", "none");
      nvram_set ("wl_wep", "disabled");
    }
  else if (!strcmp (value, "wpa2"))
    {				// WPA2 Only Mode
      nvram_set ("wl_akm", value);
      nvram_set ("wl_auth_mode", "none");
      nvram_set ("wl_wep", "disabled");
    }
  else if (!strcmp (value, "psk psk2"))
    {				// WPA2 Mixed Mode
      nvram_set ("wl_akm", value);
      nvram_set ("wl_auth_mode", "none");
      nvram_set ("wl_wep", "disabled");
    }
  else if (!strcmp (value, "wpa wpa2"))
    {				// WPA2 Mixed Mode
      nvram_set ("wl_akm", value);
      nvram_set ("wl_auth_mode", "none");
      nvram_set ("wl_wep", "disabled");
    }

  if (security_mode_last)
    nvram_set ("security_mode_last", security_mode_last);

  if (wl_wep_last)
    nvram_set ("wl_wep_last", wl_wep_last);

  nvram_set (v->name, value);
}
#endif
void
validate_wl_key (webs_t wp, char *value, struct variable *v)
{
  char *c;

  switch (strlen (value))
    {
    case 5:
    case 13:
      break;
    case 10:
    case 26:
      for (c = value; *c; c++)
	{
	  if (!isxdigit (*c))
	    {
	      websDebugWrite (wp,
			      "Invalid <b>%s</b>: character %c is not a hexadecimal digit<br>",
			      v->longname, *c);
	      return;
	    }
	}
      break;
    default:
      websDebugWrite (wp,
		      "Invalid <b>%s</b>: must be 5 or 13 ASCII characters or 10 or 26 hexadecimal digits<br>",
		      v->longname);
      return;
    }

  nvram_set (v->name, value);
}

void
validate_wl_wep (webs_t wp, char *value, struct variable *v)
{
  if (!valid_choice (wp, value, v))
    return;
#ifdef ABURN_WSEC_CHECK
  if (strcmp (value, "off")
      && atoi (nvram_safe_get ("wl_gmode")) == GMODE_AFTERBURNER)
    {
      websDebugWrite (wp,
		      "<br>Invalid <b>%s</b>: must be set to <b>Off</b> when 54g Mode is AfterBurner.",
		      v->longname);
      return;
    }
#endif
  nvram_set (v->name, value);
}

void
validate_auth_mode (webs_t wp, char *value, struct variable *v)
{
  if (!valid_choice (wp, value, v))
    return;
  nvram_set (v->name, value);
}

void
validate_wpa_psk (webs_t wp, char *value, struct variable *v)
{
  int len = strlen (value);
  char *c;

  if (len == 64)
    {
      for (c = value; *c; c++)
	{
	  if (!isxdigit ((int) *c))
	    {
	      websDebugWrite (wp,
			      "Invalid <b>%s</b>: character %c is not a hexadecimal digit<br>",
			      v->longname, *c);
	      return;
	    }
	}
    }
  else if (len < 8 || len > 63)
    {
      websDebugWrite (wp,
		      "Invalid <b>%s</b>: must be between 8 and 63 ASCII characters or 64 hexadecimal digits<br>",
		      v->longname);
      return;
    }

  nvram_set (v->name, value);
}

/* Hook to write wl_* default set through to wl%d_* variable set */
void
wl_unit (webs_t wp, char *value, struct variable *v)
{
  char tmp[100], prefix[] = "wlXXXXXXXXXX_";
  struct nvram_tuple cur[100], *last = cur, *t;

  /* Do not write through if no interfaces are present */
  if (atoi (value) < 0)
    return;

  /* Set prefix */
  snprintf (prefix, sizeof (prefix), "wl%d_", atoi (value));

  /* Write through to selected variable set */
  for (; v >= variables && !strncmp (v->name, "wl_", 3); v--)
    {
      /* Do not interleave get and set (expensive on Linux) */
      a_assert (last < &cur[ARRAYSIZE (cur)]);
      last->name = v->name;
      last->value = nvram_safe_get (v->name);
      last++;
    }

  for (t = cur; t < last; t++)
    nvram_set (strcat_r (prefix, &t->name[3], tmp), t->value);
}


#ifdef HAVE_MSSID

void
validate_wl_wep_key (webs_t wp, char *value, struct variable *v)
{
  char buf[200] = "";
  struct variable wl_wep_variables[] = {
  {longname: "Passphrase", argv:ARGV ("16")},
  {longname: "WEP Key", argv:ARGV ("5", "10")},
    //for 64 bit
  {longname: "WEP Key", argv:ARGV ("13", "26")},
    //for 128 bit
  {longname: "Default TX Key", argv:ARGV ("1", "4")},
  }, *which;

  char *wep_bit = "", *wep_passphrase = "", *wep_key1 = "", *wep_key2 =
    "", *wep_key3 = "", *wep_key4 = "", *wep_tx = "";
  char new_wep_passphrase[50] = "", new_wep_key1[30] = "", new_wep_key2[30] =
    "", new_wep_key3[30] = "", new_wep_key4[30] = "";
  int index;

  which = &wl_wep_variables[0];

  wep_bit = websGetVar (wp, "wl_wep_bit", NULL);	//64 or 128
  if (!wep_bit)
    return;
  if (strcmp (wep_bit, "64") && strcmp (wep_bit, "128"))
    return;

  wep_passphrase = websGetVar (wp, "wl_passphrase", "");
  //if(!wep_passphrase)   return ;

  //strip_space(wep_passphrase);
  if (strcmp (wep_passphrase, ""))
    {
      if (!valid_name (wp, wep_passphrase, &which[0]))
	{
	  error_value = 1;
	}
      else
	{
	  httpd_filter_name (wep_passphrase, new_wep_passphrase,
			     sizeof (new_wep_passphrase), SET);
	}
    }

  wep_key1 = websGetVar (wp, "wl_key1", "");
  wep_key2 = websGetVar (wp, "wl_key2", "");
  wep_key3 = websGetVar (wp, "wl_key3", "");
  wep_key4 = websGetVar (wp, "wl_key4", "");
  wep_tx = websGetVar (wp, "wl_key", NULL);

  if (!wep_tx)
    {
      error_value = 1;
      return;
    }

  index = (atoi (wep_bit) == 64) ? 1 : 2;

  if (strcmp (wep_key1, ""))
    {
      if (!valid_wep_key (wp, wep_key1, &which[index]))
	{
	  error_value = 1;
	}
      else
	{
	  httpd_filter_name (wep_key1, new_wep_key1, sizeof (new_wep_key1),
			     SET);
	}

    }
  if (strcmp (wep_key2, ""))
    {
      if (!valid_wep_key (wp, wep_key2, &which[index]))
	{
	  error_value = 1;
	}
      else
	{
	  httpd_filter_name (wep_key2, new_wep_key2, sizeof (new_wep_key2),
			     SET);
	}
    }
  if (strcmp (wep_key3, ""))
    {
      if (!valid_wep_key (wp, wep_key3, &which[index]))
	{
	  error_value = 1;
	}
      else
	{
	  httpd_filter_name (wep_key3, new_wep_key3, sizeof (new_wep_key3),
			     SET);
	}
    }
  if (strcmp (wep_key4, ""))
    {
      if (!valid_wep_key (wp, wep_key4, &which[index]))
	{
	  error_value = 1;
	}
      else
	{
	  httpd_filter_name (wep_key4, new_wep_key4, sizeof (new_wep_key4),
			     SET);
	}
    }


  if (!error_value)
    {
      snprintf (buf, sizeof (buf), "%s:%s:%s:%s:%s:%s", new_wep_passphrase,
		new_wep_key1, new_wep_key2, new_wep_key3, new_wep_key4,
		wep_tx);
      nvram_set ("wl_wep_bit", wep_bit);
      nvram_set ("wl_wep_buf", buf);

      nvram_set ("wl_passphrase", wep_passphrase);
      nvram_set ("wl_key", wep_tx);
      nvram_set ("wl_key1", wep_key1);
      nvram_set ("wl_key2", wep_key2);
      nvram_set ("wl_key3", wep_key3);
      nvram_set ("wl_key4", wep_key4);

      if (!strcmp (wep_key1, "") && !strcmp (wep_key2, "") && !strcmp (wep_key3, "") && !strcmp (wep_key4, ""))	// Allow null wep
	nvram_set ("wl_wep", "off");
      else
	nvram_set ("wl_wep", "restricted");
    }

}

#else

void
validate_wl_wep_key (webs_t wp, char *value, struct variable *v)
{
  char buf[200] = "";
  struct variable wl_wep_variables[] = {
  {longname: "Passphrase", argv:ARGV ("16")},
  {longname: "WEP Key", argv:ARGV ("5", "10")},
    //for 64 bit
  {longname: "WEP Key", argv:ARGV ("13", "26")},
    //for 128 bit
  {longname: "Default TX Key", argv:ARGV ("1", "4")},
  }, *which;

  char *wep_bit = "", *wep_passphrase = "", *wep_key1 = "", *wep_key2 =
    "", *wep_key3 = "", *wep_key4 = "", *wep_tx = "";
  char new_wep_passphrase[50] = "", new_wep_key1[30] = "", new_wep_key2[30] =
    "", new_wep_key3[30] = "", new_wep_key4[30] = "";
  int index;

  which = &wl_wep_variables[0];

  wep_bit = websGetVar (wp, "wl_wep_bit", NULL);	//64 or 128
  if (!wep_bit)
    return;
  if (strcmp (wep_bit, "64") && strcmp (wep_bit, "128"))
    return;

  wep_passphrase = websGetVar (wp, "wl_passphrase", "");
  //if(!wep_passphrase)   return ;

  //strip_space(wep_passphrase);
  if (strcmp (wep_passphrase, ""))
    {
      if (!valid_name (wp, wep_passphrase, &which[0]))
	{
	  error_value = 1;
	}
      else
	{
	  httpd_filter_name (wep_passphrase, new_wep_passphrase,
			     sizeof (new_wep_passphrase), SET);
	}
    }

  wep_key1 = websGetVar (wp, "wl_key1", "");
  wep_key2 = websGetVar (wp, "wl_key2", "");
  wep_key3 = websGetVar (wp, "wl_key3", "");
  wep_key4 = websGetVar (wp, "wl_key4", "");
  wep_tx = websGetVar (wp, "wl_key", NULL);

  if (!wep_tx)
    {
      error_value = 1;
      return;
    }

  index = (atoi (wep_bit) == 64) ? 1 : 2;

  if (strcmp (wep_key1, ""))
    {
      if (!valid_wep_key (wp, wep_key1, &which[index]))
	{
	  error_value = 1;
	}
      else
	{
	  httpd_filter_name (wep_key1, new_wep_key1, sizeof (new_wep_key1),
			     SET);
	}

    }
  if (strcmp (wep_key2, ""))
    {
      if (!valid_wep_key (wp, wep_key2, &which[index]))
	{
	  error_value = 1;
	}
      else
	{
	  httpd_filter_name (wep_key2, new_wep_key2, sizeof (new_wep_key2),
			     SET);
	}
    }
  if (strcmp (wep_key3, ""))
    {
      if (!valid_wep_key (wp, wep_key3, &which[index]))
	{
	  error_value = 1;
	}
      else
	{
	  httpd_filter_name (wep_key3, new_wep_key3, sizeof (new_wep_key3),
			     SET);
	}
    }
  if (strcmp (wep_key4, ""))
    {
      if (!valid_wep_key (wp, wep_key4, &which[index]))
	{
	  error_value = 1;
	}
      else
	{
	  httpd_filter_name (wep_key4, new_wep_key4, sizeof (new_wep_key4),
			     SET);
	}
    }


  if (!error_value)
    {
      snprintf (buf, sizeof (buf), "%s:%s:%s:%s:%s:%s", new_wep_passphrase,
		new_wep_key1, new_wep_key2, new_wep_key3, new_wep_key4,
		wep_tx);
      nvram_set ("wl_wep_bit", wep_bit);
      nvram_set ("wl_wep_buf", buf);

      nvram_set ("wl_passphrase", wep_passphrase);
      nvram_set ("wl_key", wep_tx);
      nvram_set ("wl_key1", wep_key1);
      nvram_set ("wl_key2", wep_key2);
      nvram_set ("wl_key3", wep_key3);
      nvram_set ("wl_key4", wep_key4);

      if (!strcmp (wep_key1, "") && !strcmp (wep_key2, "") && !strcmp (wep_key3, "") && !strcmp (wep_key4, ""))	// Allow null wep
	nvram_set ("wl_wep", "off");
      else
	nvram_set ("wl_wep", "restricted");
    }

}
#endif
void
validate_wl_auth (webs_t wp, char *value, struct variable *v)
{
  if (!valid_choice (wp, value, v))
    return;
  /*  // not to check , spec for linksys
     if (atoi(value) == 1) {
     char wl_key[] = "wl_keyXXX";

     snprintf(wl_key, sizeof(wl_key), "wl_key%s", nvram_safe_get("wl_key"));
     if (!strlen(nvram_safe_get(wl_key))) {
     websDebugWrite(wp, "Invalid <b>%s</b>: must first specify a valid <b>Network Key</b><br>", v->longname);
     return;
     }
     }
   */
  nvram_set (v->name, value);
}

void
validate_d11_channel (webs_t wp, char *value, struct variable *v)
{
  char *country = nvram_safe_get ("wl_country");
  int channel = atoi (value), min = 0, max = 0, i;

  if ((!strcmp (v->name, "d11b_channel"))
      || (!strcmp (v->name, "d11g_channel")))
    {
      if (!strcmp (country, "Japan") || !strcmp (country, "Thailand"))
	{
	  min = 1;
	  max = 14;
	}
      else if (!strcmp (country, "Jordan"))
	{
	  min = 10;
	  max = 13;
	}
      else if (!strcmp (country, "Israel"))
	{
	  min = 5;
	  max = 7;
	}
      else
	{
	  min = 1;
	  max = 13;
	}

      if (channel < min || channel > max)
	{
	  websDebugWrite (wp,
			  "Invalid <b>%s</b>: valid %s channels are %d-%d<br>",
			  v->longname, country, min, max);
	  return;
	}
    }
  else if (!strcmp (v->name, "d11a_channel"))
    {
      for (i = 0; i < ARRAYSIZE (chan_info); i++)
	{
	  if (chan_info[i].chan == channel)
	    break;
	}

      if (i >= ARRAYSIZE (chan_info))
	{
	  websDebugWrite (wp, "Invalid <b>%s</b>: valid %s channels are ",
			  v->longname, country);
	  for (i = 0; i < ARRAYSIZE (chan_info); i++)
	    {
	      channel = chan_info[i].chan;
	      websWrite (wp, "%d%s", channel, channel == 216 ? "<br>" : "/");
	    }
	  return;
	}
    }


  nvram_set (v->name, value);
}

#ifdef SUPPORT_11b
void
validate_d11b_rate (webs_t wp, char *value, struct variable *v)
{
  char *country = nvram_safe_get ("wl_country");
  int channel = atoi (nvram_safe_get ("d11b_channel"));

  if (!strcmp (country, "Japan") && channel == 14)
    {
      if (atoi (value) > 2000000)
	{
	  websDebugWrite (wp,
			  "Invalid <b>%s</b>: valid rates in Japan on channel 14 are Auto, 1 Mbps, and 2 Mbps<br>",
			  v->longname);
	  return;
	}
    }

  if (!valid_choice (wp, value, v))
    return;

  nvram_set (v->name, value);
}

void
validate_d11b_rateset (webs_t wp, char *value, struct variable *v)
{
  char *country = nvram_safe_get ("wl_country");
  int channel = atoi (nvram_safe_get ("d11b_channel"));

  if (!strcmp (country, "Japan") && channel == 14)
    {
      if (!strcmp (value, "all"))
	{
	  websDebugWrite (wp,
			  "Invalid <b>%s</b>: valid rate set in Japan on channel 14 is Default<br>",
			  v->longname);
	  return;
	}
    }

  if (!valid_choice (wp, value, v))
    return;

  nvram_set (v->name, value);
}
#endif

char *
wl_filter_mac_get (char *ifname,char *type, int which)
{
  static char word[50];
  char *wordlist, *next;
  int temp;

  if (!strcmp (nvram_safe_get ("wl_active_add_mac"), "1"))
    {
      //cprintf("%s(): wl_active_add_mac = 1\n",__FUNCTION__);
      char var[32];
      sprintf(var,"%s_active_mac",ifname);
      wordlist = nvram_safe_get (var);
    }
  else
    {
      //cprintf("%s(): wl_active_add_mac = 0\n",__FUNCTION__);
      char var[32];
      sprintf(var,"%s_maclist",ifname);
      wordlist = nvram_safe_get (var);
    }

  if (!wordlist)
    return "";

  temp = which;

  foreach (word, wordlist, next)
  {
    if (which-- == 0)
      {

	return word;
      }
  }
  return "";

}

/* Example:
 * 00:11:22:33:44:55=1 00:12:34:56:78:90=0 (ie 00:11:22:33:44:55 if filterd, and 00:12:34:56:78:90 is not)
 * wl_maclist = "00:11:22:33:44:55"
 */
void
validate_wl_hwaddrs (webs_t wp, char *value, struct variable *v)
{
  int i;

  char buf[19 * WL_FILTER_MAC_NUM * WL_FILTER_MAC_PAGE] = "", *cur = buf;
  char *wordlist;
  unsigned char m[6];
  char *ifname = websGetVar (wp, "ifname", NULL);	//64 or 128
  if (ifname==NULL)
    return;
  char mlist[32];
  sprintf(mlist,"%s_maclist",ifname);
  wordlist = nvram_safe_get (mlist);
  if (!wordlist)
    return;

  for (i = 0; i < WL_FILTER_MAC_NUM * WL_FILTER_MAC_PAGE; i++)
    {
      char filter_mac[] = "ath10.99_macXXX";
      char *mac = NULL;
      char mac1[20];

      snprintf (filter_mac, sizeof (filter_mac), "%s%s%d",ifname, "_mac", i);

      mac = websGetVar (wp, filter_mac, NULL);


      if (!mac || !strcmp (mac, "0") || !strcmp (mac, ""))
	{
	  continue;
	}
      //strip_space(mac);

      if (strlen (mac) == 12)
	{
	  sscanf (mac, "%02X%02X%02X%02X%02X%02X", (uint *) & m[0],
		  (uint *) & m[1], (uint *) & m[2], (uint *) & m[3],
		  (uint *) & m[4], (uint *) & m[5]);
	  sprintf (mac1, "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2],
		   m[3], m[4], m[5]);
	}
      else if (strlen (mac) == 17)
	{
	  sscanf (mac, "%02X:%02X:%02X:%02X:%02X:%02X", (uint *) & m[0],
		  (uint *) & m[1], (uint *) & m[2], (uint *) & m[3],
		  (uint *) & m[4], (uint *) & m[5]);
	  sprintf (mac1, "%02X:%02X:%02X:%02X:%02X:%02X", m[0], m[1], m[2],
		   m[3], m[4], m[5]);
	}
      else
	{
	  mac1[0] = 0;
	}

      if (!valid_hwaddr (wp, mac1, v))
	{
	  error_value = 1;
	  continue;
	}
      cur += snprintf (cur, buf + sizeof (buf) - cur, "%s%s",
		       cur == buf ? "" : " ", mac1);


    }


  if (!error_value)
    {
      nvram_set (v->name, buf);
      nvram_set (mlist, buf);
      nvram_set ("wl_active_mac", "");
    }
}

void
ej_wireless_filter_table (int eid, webs_t wp, int argc, char_t ** argv)
{
  int i;
  char *type;
  char *ifname;
  int item;
#if LANGUAGE == JAPANESE
  int box_len = 20;
#else
  int box_len = 17;
#endif

  char *mac_mess = "MAC";

  if (ejArgs (argc, argv, "%s %s", &type,&ifname) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  if (!strcmp (type, "input"))
    {
      websWrite (wp, "<div class=\"col2l\">\n");
      websWrite (wp, "<fieldset><legend>Table 1</legend>\n");
      for (i = 0; i < WL_FILTER_MAC_NUM / 2; i++)
	{

	  item = 0 * WL_FILTER_MAC_NUM + i + 1;

	  websWrite (wp,
		     "<div>%s %03d : <input maxlength=\"17\" onblur=\"valid_macs_all(this)\" size=%d name=\"%s_mac%d\" value=\"%s\"/>&nbsp;&nbsp;&nbsp;",
		     mac_mess, item, box_len,ifname, item - 1,
		     wl_filter_mac_get (ifname,"mac", item - 1));

	  websWrite (wp,
		     "%s %03d : <input maxlength=\"17\" onblur=\"valid_macs_all(this)\" size=%d name=\"%s_mac%d\" value=\"%s\"/></div>\n",
		     mac_mess, item + (WL_FILTER_MAC_NUM / 2), box_len,ifname,
		     item + (WL_FILTER_MAC_NUM / 2) - 1,
		     wl_filter_mac_get (ifname,"mac",
					item + (WL_FILTER_MAC_NUM / 2) - 1));

	}

      websWrite (wp, "</fieldset></div>\n");
      websWrite (wp, "<div class=\"col2r\">\n");
      websWrite (wp, "<fieldset><legend>Table 2</legend>\n");

      for (i = 0; i < WL_FILTER_MAC_NUM / 2; i++)
	{

	  item = 1 * WL_FILTER_MAC_NUM + i + 1;

	  websWrite (wp,
		     "<div/>%s %03d : <input maxlength=\"17\" onblur=\"valid_macs_all(this)\" size=%d name=\"%s_mac%d\" value=\"%s\"/>&nbsp;&nbsp;&nbsp;",
		     mac_mess, item, box_len, ifname,item - 1,
		     wl_filter_mac_get (ifname,"mac", item - 1));

	  websWrite (wp,
		     "%s %03d : <input maxlength=\"17\" onblur=\"valid_macs_all(this)\" size=%d name=\"%s_mac%d\" value=\"%s\"/></div>\n",
		     mac_mess, item + (WL_FILTER_MAC_NUM / 2), box_len,ifname,
		     item + (WL_FILTER_MAC_NUM / 2) - 1,
		     wl_filter_mac_get (ifname,"mac",
					item + (WL_FILTER_MAC_NUM / 2) - 1));

	}

      websWrite (wp, "</fieldset>\n");
      websWrite (wp, "</div><br clear=\"all\" /><br />\n");

    }



  //cprintf("%s():set wl_active_add_mac = 0\n",__FUNCTION__);
  nvram_set ("wl_active_add_mac", "0");
  return;
}

int
add_active_mac (webs_t wp)
{
  char buf[1000] = "", *cur = buf;
  int i, count = 0;

  nvram_set ("wl_active_add_mac", "1");

  for (i = 0; i < MAX_LEASES; i++)
    {
      char active_mac[] = "onXXX";
      char *index = NULL;

      snprintf (active_mac, sizeof (active_mac), "%s%d", "on", i);
      index = websGetVar (wp, active_mac, NULL);
      if (!index)
	continue;

      count++;

      cur += snprintf (cur, buf + sizeof (buf) - cur, "%s%s",
		       cur == buf ? "" : " ",
		       wl_client_macs[atoi (index)].hwaddr);
    }
  for (i = 0; i < MAX_LEASES; i++)
    {
      char active_mac[] = "offXXX";
      char *index;

      snprintf (active_mac, sizeof (active_mac), "%s%d", "off", i);
      index = websGetVar (wp, active_mac, NULL);
      if (!index)
	continue;

      count++;
      cur += snprintf (cur, buf + sizeof (buf) - cur, "%s%s",
		       cur == buf ? "" : " ",
		       wl_client_macs[atoi (index)].hwaddr);
    }
  nvram_set ("wl_active_mac", buf);
  return 0;
}


int
dhcp_lease_table_init (void)
{
  FILE *fp, *fp_w;
  int count = 0;
  if (nvram_match ("dhcp_dnsmasq", "1"))
    {
      unsigned long expires;
      char mac[32];
      char ip[32];
      char hostname[256];
      char buf[512];
      char *p;
      killall("dnsmasq",SIGUSR2);
      sleep (1);

      if ((fp_w = fopen (LEASES_NAME_IP, "w")) != NULL)
	{
	  // Parse leases file
	  if ((fp = fopen ("/tmp/dnsmasq.leases", "r")) != NULL)
	    {
	      while (fgets (buf, sizeof (buf), fp))
		{
		  if (sscanf
		      (buf, "%lu %17s %15s %255s", &expires, mac, ip,
		       hostname) != 4)
		    continue;
		  p = mac;
		  while ((*p = toupper (*p)) != 0)
		    ++p;
		  fprintf (fp_w, "%s %s %s\n", mac, ip, hostname);
		  ++count;
		}
	      fclose (fp);
	    }
	  fclose (fp_w);
	}
    }
  else
    {
      struct lease_t lease;
      struct in_addr addr;
      char mac[20] = "";
      killall("udhcpd",SIGUSR1);
      fp_w = fopen (LEASES_NAME_IP, "w");

      // Parse leases file 
      if ((fp = fopen ("/tmp/udhcpd.leases", "r")))
	{
	  while (fread (&lease, sizeof (lease), 1, fp))
	    {
	      snprintf (mac, sizeof (mac), "%02X:%02X:%02X:%02X:%02X:%02X",
			lease.chaddr[0], lease.chaddr[1], lease.chaddr[2],
			lease.chaddr[3], lease.chaddr[4], lease.chaddr[5]);
	      if (!strcmp ("00:00:00:00:00:00", mac))
		continue;
	      addr.s_addr = lease.yiaddr;
	      fprintf (fp_w, "%s %s %s\n", mac, inet_ntoa (addr),
		       lease.hostname);
	      count++;
	    }
	  fclose (fp);
	}
      fclose (fp_w);
    }

  return count;
}

void
save_hostname_ip (void)
{
  FILE *fp, *fp_w;
  char line[80];
  char leases[3][50];
  int i = 0, j = 0, count;
  int match = 0;
  struct wl_client
  {
    unsigned char hostname[32];
    char ipaddr[20];
    char hwaddr[20];
  } wl_clients[MAX_LEASES];

  for (i = 0; i < MAX_LEASES; i++)
    {				// init value
      strcpy (wl_clients[i].hostname, "");
      strcpy (wl_clients[i].ipaddr, "");
      strcpy (wl_clients[i].hwaddr, "");
    }
  i = 0;
  if ((fp = fopen (OLD_NAME_IP, "r")))
    {
      while (fgets (line, sizeof (line), fp) != NULL)
	{
	  // 00:11:22:33:44:55 192.168.1.100 honor
	  strcpy (leases[0], "");
	  strcpy (leases[1], "");
	  strcpy (leases[2], "");
	  if (sscanf (line, "%s %s %s", leases[0], leases[1], leases[2]) != 3)
	    continue;
	  snprintf (wl_clients[i].hwaddr, sizeof (wl_clients[i].hwaddr), "%s",
		    leases[0]);
	  snprintf (wl_clients[i].ipaddr, sizeof (wl_clients[i].ipaddr), "%s",
		    leases[1]);
	  snprintf (wl_clients[i].hostname, sizeof (wl_clients[i].hostname),
		    "%s", leases[2]);
	  i++;
	}
      fclose (fp);

    }
  count = i;

  for (i = 0; i < nv_count; i++)
    {				// init value
      if (wl_client_macs[i].status == 1
	  && strcmp (wl_client_macs[i].ipaddr, ""))
	{			// online && have ip address
	  for (j = 0; j < MAX_LEASES; j++)
	    {
	      match = 0;
	      if (!strcmp (wl_clients[j].hwaddr, wl_client_macs[i].hwaddr))
		{
		  snprintf (wl_clients[j].ipaddr,
			    sizeof (wl_clients[j].ipaddr), "%s",
			    wl_client_macs[i].ipaddr);
		  snprintf (wl_clients[j].hostname,
			    sizeof (wl_clients[j].hostname), "%s",
			    wl_client_macs[i].hostname);
		  match = 1;
		  break;
		}

	    }
	  if (match == 0)
	    {
	      snprintf (wl_clients[count].hwaddr,
			sizeof (wl_clients[i].hwaddr), "%s",
			wl_client_macs[i].hwaddr);
	      snprintf (wl_clients[count].ipaddr,
			sizeof (wl_clients[i].ipaddr), "%s",
			wl_client_macs[i].ipaddr);
	      snprintf (wl_clients[count].hostname,
			sizeof (wl_clients[i].hostname), "%s",
			wl_client_macs[i].hostname);
	      count++;
	    }
	}
    }

  if ((fp_w = fopen (OLD_NAME_IP, "w")))
    {
      for (i = 0; i < MAX_LEASES; i++)
	{
	  if (strcmp (wl_clients[i].hwaddr, ""))
	    fprintf (fp_w, "%s %s %s\n", wl_clients[i].hwaddr,
		     wl_clients[i].ipaddr, wl_clients[i].hostname);
	}
      fclose (fp_w);
    }

}

void
get_hostname_ip (char *type, char *filename)
{
  FILE *fp;
  char line[80];
  char leases[3][50];
  int i;

  if ((fp = fopen (filename, "r")))
    {				// find out hostname and ip
      while (fgets (line, sizeof (line), fp) != NULL)
	{
	  strcpy (leases[0], "");
	  strcpy (leases[1], "");
	  strcpy (leases[2], "");
	  // 00:11:22:33:44:55 192.168.1.100 honor
	  if (sscanf (line, "%s %s %s", leases[0], leases[1], leases[2]) != 3)
	    continue;
	  for (i = 0; i < MAX_LEASES; i++)
	    {
	      if (!strcmp (leases[0], wl_client_macs[i].hwaddr))
		{
		  snprintf (wl_client_macs[i].ipaddr,
			    sizeof (wl_client_macs[i].ipaddr), "%s",
			    leases[1]);
		  snprintf (wl_client_macs[i].hostname,
			    sizeof (wl_client_macs[i].hostname), "%s",
			    leases[2]);
		  break;
		}
	    }
	}
    }
  fclose (fp);
}

void
ej_wireless_active_table (int eid, webs_t wp, int argc, char_t ** argv)
{
  int i, flag = 0;
  char *type;
  char word[256], *next;
  FILE *fp;
  char list[2][20];
  char line[80];
  int dhcp_table_count;
  char cmd[80];

  if (ejArgs (argc, argv, "%s", &type) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  if (!strcmp (type, "online"))
    {
      for (i = 0; i < MAX_LEASES; i++)
	{			// init value
	  strcpy (wl_client_macs[i].hostname, "");
	  strcpy (wl_client_macs[i].ipaddr, "");
	  strcpy (wl_client_macs[i].hwaddr, "");
	  wl_client_macs[i].status = -1;
	  wl_client_macs[i].check = 0;
	}

      nv_count = 0;		// init mac list
      foreach (word, nvram_safe_get ("wl_mac_list"), next)
      {
	snprintf (wl_client_macs[nv_count].hwaddr,
		  sizeof (wl_client_macs[nv_count].hwaddr), "%s", word);
	wl_client_macs[nv_count].status = 0;	// offline (default)
	wl_client_macs[nv_count].check = 1;	// checked
	nv_count++;
      }
      snprintf (cmd, sizeof (cmd), "%s > %s", ASSOCLIST_CMD, ASSOCLIST_TMP);
      system2 (cmd);		// get active wireless mac


      if ((fp = fopen (ASSOCLIST_TMP, "r")))
	{
	  while (fgets (line, sizeof (line), fp) != NULL)
	    {
	      int match = 0;
	      strcpy (list[0], "");
	      strcpy (list[1], "");
	      if (sscanf (line, "%s %s", list[0], list[1]) != 2)	// assoclist 00:11:22:33:44:55
		continue;
	      if (strcmp (list[0], "assoclist"))
		continue;
	      for (i = 0; i < nv_count; i++)
		{
		  if (!strcmp (wl_client_macs[i].hwaddr, list[1]))
		    {
		      wl_client_macs[i].status = 1;	// online
		      wl_client_macs[i].check = 1;	// checked
		      match = 1;

		      break;
		    }
		}
	      if (match == 0)
		{
		  snprintf (wl_client_macs[nv_count].hwaddr,
			    sizeof (wl_client_macs[nv_count].hwaddr), "%s",
			    list[1]);
		  wl_client_macs[nv_count].status = 1;	// online
		  wl_client_macs[nv_count].check = 0;	// no checked
		  nv_count++;
		}
	    }
	  fclose (fp);
	}
      if (!strcmp (type, "online"))
	{
	  dhcp_table_count = dhcp_lease_table_init ();	// init dhcp lease table and get count
	  get_hostname_ip ("online", LEASES_NAME_IP);
	}
      save_hostname_ip ();
    }

  if (!strcmp (type, "offline"))
    {
      get_hostname_ip ("offline", OLD_NAME_IP);
    }

  if (!strcmp (type, "online"))
    {
      for (i = 0; i < nv_count; i++)
	{
	  if (wl_client_macs[i].status != 1)
	    continue;
	  websWrite (wp, "\
 <tr align=\"middle\"> \n\
    <td height=\"20\" width=\"167\">%s</td> \n\
    <td height=\"20\" width=\"140\">%s</td> \n\
    <td height=\"20\" width=\"156\">%s</td> \n\
    <td height=\"20\" width=\"141\"><input type=\"checkbox\" name=\"on%d\" value=\"%d\" %s></td> \n\
 </tr>\n", wl_client_macs[i].hostname, wl_client_macs[i].ipaddr, wl_client_macs[i].hwaddr, flag++, i, wl_client_macs[i].check ? "checked=\"checked\"" : "");
	}
    }
  else if (!strcmp (type, "offline"))
    {
      for (i = 0; i < nv_count; i++)
	{
	  if (wl_client_macs[i].status != 0)
	    continue;
	  websWrite (wp, "\
 <tr align=\"middle\"> \n\
    <td height=\"20\" width=\"167\">%s</td> \n\
    <td height=\"20\" width=\"140\">%s</td> \n\
    <td height=\"20\" width=\"156\">%s</td> \n\
    <td height=\"20\" width=\"141\"><input type=\"checkbox\" name=\"off%d\" value=\"%d\" %s></td> \n\
 </tr>\n", wl_client_macs[i].hostname, wl_client_macs[i].ipaddr, wl_client_macs[i].hwaddr, flag++, i, wl_client_macs[i].check ? "checked=\"checked\"" : "");

	}
    }
  //if(dhcp_lease_table)  free(dhcp_lease_table);
  return;
}

char *
get_wep_value (char *type, char *_bit, char *prefix)
{
  static char word[200];
  char *next, *wordlist;
  char wl_wep[] = "wlX.XX_wep_XXXXXX";
  char *wl_passphrase, *wl_key1, *wl_key2, *wl_key3, *wl_key4, *wl_key_tx;

  if (generate_key)
    {
      snprintf (wl_wep, sizeof (wl_wep), "%s_wep_gen", prefix);
    }
  else
    {
      snprintf (wl_wep, sizeof (wl_wep), "%s_wep_buf", prefix);
    }

  cprintf ("get %s from %s with bit %s and prefix %s\n", type, wl_wep, _bit,
	   prefix);

  wordlist = nvram_safe_get (wl_wep);
  cprintf ("wordlist = %s\n", wordlist);
  //if(strcmp(wordlist,"") && !strcmp(_bit,"64")){
  foreach (word, wordlist, next)
  {
    wl_key1 = word;
    wl_passphrase = strsep (&wl_key1, ":");
    if (!wl_passphrase || !wl_key1)
      continue;
    wl_key2 = wl_key1;
    wl_key1 = strsep (&wl_key2, ":");
    if (!wl_key1 || !wl_key2)
      continue;
    wl_key3 = wl_key2;
    wl_key2 = strsep (&wl_key3, ":");
    if (!wl_key2 || !wl_key3)
      continue;
    wl_key4 = wl_key3;
    wl_key3 = strsep (&wl_key4, ":");
    if (!wl_key3 || !wl_key4)
      continue;
    wl_key_tx = wl_key4;
    wl_key4 = strsep (&wl_key_tx, ":");
    if (!wl_key4 || !wl_key_tx)
      continue;

    cprintf ("key1 = %s\n", wl_key1);
    cprintf ("key2 = %s\n", wl_key2);
    cprintf ("key3 = %s\n", wl_key3);
    cprintf ("key4 = %s\n", wl_key4);
    cprintf ("pass = %s\n", wl_passphrase);


    if (!strcmp (type, "passphrase"))
      {
	return wl_passphrase;
      }
    else if (!strcmp (type, "key1"))
      {
	return wl_key1;
      }
    else if (!strcmp (type, "key2"))
      {
	return wl_key2;
      }
    else if (!strcmp (type, "key3"))
      {
	return wl_key3;
      }
    else if (!strcmp (type, "key4"))
      {
	return wl_key4;
      }
    else if (!strcmp (type, "tx"))
      {
	return wl_key_tx;
      }
  }
  return "";
}

void
ej_get_wep_value (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *type, *bit;
  char *value = "", new_value[50] = "";

  if (ejArgs (argc, argv, "%s", &type) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
  cprintf ("get wep value %s\n", type);
#ifdef HAVE_MADWIFI
  bit = GOZILA_GET ("ath0_wep_bit");

  value = get_wep_value (type, bit, "ath0");
#else
  bit = GOZILA_GET ("wl_wep_bit");
  cprintf ("bit = %s\n", bit);
  value = get_wep_value (type, bit, "wl");
#endif
  cprintf ("value = %s\n", value);
  httpd_filter_name (value, new_value, sizeof (new_value), GET);
  cprintf ("newvalue = %s\n", new_value);

  websWrite (wp, "%s", new_value);
}

void
ej_show_wl_wep_setting (int eid, webs_t wp, int argc, char_t ** argv)
{

/*
	char *type;

	type = gozila_action ? websGetVar(wp, "wl_wep_bit", NULL) : nvram_safe_get("wl_wep_bit");

ret += websWrite(wp," \
              <TR> \n\
                <TH align=right width=150 bgColor=#6666cc height=25>&nbsp;Passphrase:&nbsp;&nbsp;</TH> \n\
                <TD align=left width=435 height=25>&nbsp;\n\
                	<INPUT maxLength=16 name=wl_passphrase size=20 value='%s'>&nbsp; \n\
			<INPUT type=hidden value=Null name=generateButton> \n\
			<INPUT type=button value='Generate' onclick=generateKey(this.form) name=wepGenerate></TD></TR>\n",get_wep_value("passphrase",type));

ret += websWrite(wp," \
              <TR> \n\
                <TH vAlign=center align=right width=150 bgColor=#6666cc height=25>Key 1:&nbsp;&nbsp;</TH> \n\
                <TD vAlign=bottom width=435 bgColor=#ffffff height=25>&nbsp;\n\
			<INPUT size=35 name=wl_key1 value='%s'></TD></TR>\n",get_wep_value("key1",type));

ret += websWrite(wp," \
              <TR> \n\
                <TH vAlign=center align=right width=150 bgColor=#6666cc height=25>Key 2:&nbsp;&nbsp;</TH> \n\
                <TD vAlign=bottom width=435 bgColor=#ffffff height=25>&nbsp;\n\
			<INPUT size=35 name=wl_key2 value='%s'></TD></TR>\n",get_wep_value("key2",type));
ret += websWrite(wp," \
              <TR> \n\
                <TH vAlign=center align=right width=150 bgColor=#6666cc height=25>Key 3:&nbsp;&nbsp;</TH> \n\
                <TD vAlign=bottom width=435 bgColor=#ffffff height=25>&nbsp;\n\
			<INPUT size=35 name=wl_key3 value='%s'></TD></TR>\n",get_wep_value("key3",type));
ret += websWrite(wp," \
              <TR> \n\
                <TH vAlign=center align=right width=150 bgColor=#6666cc height=25>Key 4:&nbsp;&nbsp;</TH> \n\
                <TD vAlign=bottom width=435 bgColor=#ffffff height=25>&nbsp;\n\
			<INPUT size=35 name=wl_key4 value='%s'></TD></TR>\n",get_wep_value("key4",type));
*/
  return;
}

void
validate_macmode (webs_t wp, char *value, struct variable *v)
{
  char *wl_macmode1, *wl_macmode;

  wl_macmode = websGetVar (wp, "wl_macmode", NULL);
  wl_macmode1 = websGetVar (wp, "wl_macmode1", NULL);

  if (!wl_macmode1)
    return;

  if (!strcmp (wl_macmode1, "disabled"))
    {
      nvram_set ("wl_macmode1", "disabled");
      nvram_set ("wl_macmode", "disabled");
    }
  else if (!strcmp (wl_macmode1, "other"))
    {
      if (!wl_macmode)
	nvram_set ("wl_macmode", "deny");
      else
	nvram_set ("wl_macmode", wl_macmode);
      nvram_set ("wl_macmode1", "other");
    }
}

int
generate_wep_key (webs_t wp, int key, char *prefix)
{
  int i;
  char buf[256];
  char *passphrase, *bit, *tx;
  char var[80];
  sprintf (var, "%s_wep_bit", prefix);
  bit = websGetVar (wp, var, NULL);
  sprintf (var, "%s_passphrase", prefix);
  passphrase = websGetVar (wp, var, NULL);
  sprintf (var, "%s_key", prefix);
  tx = websGetVar (wp, var, NULL);
  cprintf ("bits = %s\n", bit);
  if (!bit || !passphrase || !tx)
    return 0;

  gen_key (passphrase, atoi (bit));

  if (atoi (bit) == 64)
    {
      char key1[27] = "";
      char key2[27] = "";
      char key3[27] = "";
      char key4[27] = "";
      for (i = 0; i < 5; i++)
	sprintf (key1 + (i << 1), "%02X", key64[0][i]);
      for (i = 0; i < 5; i++)
	sprintf (key2 + (i << 1), "%02X", key64[1][i]);
      for (i = 0; i < 5; i++)
	sprintf (key3 + (i << 1), "%02X", key64[2][i]);
      for (i = 0; i < 5; i++)
	sprintf (key4 + (i << 1), "%02X", key64[3][i]);

      snprintf (buf, sizeof (buf), "%s:%s:%s:%s:%s:%s", passphrase, key1,
		key2, key3, key4, tx);
      //nvram_set("wl_wep_gen_64",buf);
      cprintf ("buf = %s\n", buf);
      sprintf (var, "%s_wep_gen", prefix);

      nvram_set (var, buf);
    }
  else if (atoi (bit) == 128)
    {
      char key1[27] = "";
      char key2[27] = "";
      char key3[27] = "";
      char key4[27] = "";

      for (i = 0; i < 13; i++)
	sprintf (key1 + (i << 1), "%02X", key128[0][i]);
      key1[26] = 0;

      for (i = 0; i < 13; i++)
	sprintf (key2 + (i << 1), "%02X", key128[1][i]);
      key2[26] = 0;

      for (i = 0; i < 13; i++)
	sprintf (key3 + (i << 1), "%02X", key128[2][i]);
      key3[26] = 0;

      for (i = 0; i < 13; i++)
	sprintf (key4 + (i << 1), "%02X", key128[3][i]);
      key4[26] = 0;
      //cprintf("passphrase[%s]\n", passphrase);
      //filter_name(passphrase, new_passphrase, sizeof(new_passphrase), SET);
      //cprintf("new_passphrase[%s]\n", new_passphrase);
      cprintf ("key1 = %s\n", key1);
      cprintf ("key2 = %s\n", key2);
      cprintf ("key3 = %s\n", key3);
      cprintf ("key4 = %s\n", key4);

      snprintf (buf, sizeof (buf), "%s:%s:%s:%s:%s:%s", passphrase, key1,
		key2, key3, key4, tx);
      cprintf ("buf = %s\n", buf);
      //nvram_set("wl_wep_gen_128",buf);
      sprintf (var, "%s_wep_gen", prefix);
      nvram_set (var, buf);
    }

  return 1;
}

int
generate_key_64 (webs_t wp)
{
  int ret;
  cprintf ("gen wep key 64");
  generate_key = 1;
#ifdef HAVE_MADWIFI
  ret =
    generate_wep_key (wp, 64, websGetVar (wp, "security_varname", "ath0"));
#else
  ret = generate_wep_key (wp, 64, websGetVar (wp, "security_varname", "wl"));
#endif
  return ret;
}

int
generate_key_128 (webs_t wp)
{
  int ret;
  cprintf ("gen wep key 128");
  generate_key = 1;
#ifdef HAVE_MADWIFI
  ret =
    generate_wep_key (wp, 128, websGetVar (wp, "security_varname", "ath0"));
#else
  ret = generate_wep_key (wp, 128, websGetVar (wp, "security_varname", "wl"));
#endif
  return ret;
}

int
wl_active_onload (webs_t wp, char *arg)
{
  int ret = 0;

  if (!strcmp (nvram_safe_get ("wl_active_add_mac"), "1"))
    {
      websWrite (wp, arg);
    }

  return ret;

}

// only for nonbrand
void
ej_get_wl_active_mac (int eid, webs_t wp, int argc, char_t ** argv)
{
  char cmd[80], line[80];
  char list[2][20];
  FILE *fp;
  int count = 0;

  snprintf (cmd, sizeof (cmd), "%s > %s", ASSOCLIST_CMD, ASSOCLIST_TMP);
  system2 (cmd);			// get active wireless mac

  if ((fp = fopen (ASSOCLIST_TMP, "r")))
    {
      while (fgets (line, sizeof (line), fp) != NULL)
	{
	  strcpy (list[0], "");
	  strcpy (list[1], "");
	  if (sscanf (line, "%s %s", list[0], list[1]) != 2)	// assoclist 00:11:22:33:44:55
	    continue;
	  if (strcmp (list[0], "assoclist"))
	    continue;
	  websWrite (wp, "%c'%s'", count ? ',' : ' ', list[1]);
	  count++;
	}
    }

  return;
}

void
ej_get_wl_value (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *type;

  if (ejArgs (argc, argv, "%s", &type) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
  if (!strcmp (type, "default_dtim"))
    {
      websWrite (wp, "1");	// This is a best value for 11b test
    }
  else if (!strcmp (type, "wl_afterburner_override"))
    {
      FILE *fp;
      char line[254];
      if ((fp = popen ("wl afterburner_override", "r")))
	{
	  fgets (line, sizeof (line), fp);
	  websWrite (wp, "%s", chomp (line));
	  pclose (fp);
	}
    }
  return;

}

#ifdef HAVE_MSSID

static void
save_prefix (webs_t wp, char *prefix)
{
  char n[80];
  char radius[80];
  char p2[80];
  strcpy (p2, prefix);
  if (strcmp (prefix, "wl0"))
    rep (p2, '.', 'X');


#ifdef HAVE_MADWIFI
/*_8021xtype
_8021xuser
_8021xpasswd
_8021xca
_8021xpem
_8021xprv
*/
sprintf (n, "%s_8021xtype", prefix);
copytonv (wp, n);
sprintf (n, "%s_tls8021xuser", prefix);
copytonv (wp, n);
sprintf (n, "%s_tls8021xpasswd", prefix);
copytonv (wp, n);
sprintf (n, "%s_tls8021xca", prefix);
copytonv (wp, n);
sprintf (n, "%s_tls8021xpem", prefix);
copytonv (wp, n);
sprintf (n, "%s_tls8021xprv", prefix);
copytonv (wp, n);

sprintf (n, "%s_peap8021xuser", prefix);
copytonv (wp, n);
sprintf (n, "%s_peap8021xpasswd", prefix);
copytonv (wp, n);
sprintf (n, "%s_peap8021xca", prefix);
copytonv (wp, n);


#endif

  sprintf (n, "%s_crypto", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_wpa_psk", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_wpa_gtk_rekey", prefix);
  copytonv (wp, n);

  sprintf (n, "%s_radius_ipaddr", prefix);
  //copytonv (wp,n);
  if (get_merge_ipaddr (wp, n, radius))
    nvram_set (n, radius);
  sprintf (n, "%s_radius_port", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_radius_key", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_key1", prefix);
  char *key1 = websGetVar(wp,n,"");
  
  copytonv (wp, n);
  sprintf (n, "%s_key2", prefix);
  char *key2 = websGetVar(wp,n,"");
  copytonv (wp, n);
  sprintf (n, "%s_key3", prefix);
  char *key3 = websGetVar(wp,n,"");
  copytonv (wp, n);
  sprintf (n, "%s_key4", prefix);
  char *key4 = websGetVar(wp,n,"");
  copytonv (wp, n);
  sprintf (n, "%s_passphrase", prefix);
  char *pass = websGetVar(wp,n,"");
  copytonv (wp, n);
  sprintf (n, "%s_key", prefix);
  char *tx = websGetVar(wp,n,"");
  copytonv (wp, n);
  sprintf (n, "%s_wep_bit", prefix);
  copytonv (wp, n);
  char buf[128];
  snprintf (buf, sizeof (buf), "%s:%s:%s:%s:%s:%s", pass,
		key1, key2, key3, key4,
		tx);
  sprintf (n, "%s_wep_buf", prefix);
  nvram_set (n, buf);



//  sprintf (n, "%s_wep_gen", prefix);
//  char *wep = nvram_safe_get (n);
//  sprintf (n, "%s_wep_buf", prefix);
//  nvram_set (n, wep);
//  sprintf (n, "%s_wep_gen", prefix);
  //nvram_unset(n);
  sprintf (n, "%s_security_mode", p2);
  char n2[80];
  sprintf (n2, "%s_akm", prefix);
  char *v = websGetVar (wp, n, NULL);
  if (v)
    {
      char auth[32];
      char wep[32];
      sprintf (auth, "%s_auth_mode", prefix);
      sprintf (wep, "%s_wep", prefix);
      if (!strcmp (v, "wep"))
	{
	  nvram_set (auth, "none");
	  nvram_set (wep, "enabled");
	}
      else if (!strcmp (v, "radius"))
	{
	  nvram_set (auth, "radius");
	  nvram_set (wep, "enabled");
	}
      else
	{
	  nvram_set (auth, "none");
	  nvram_set (wep, "disabled");
	}
      nvram_set (n2, v);
    }

  copytonv (wp, n);

}

static int
security_save_prefix (webs_t wp, char *prefix)
{

  save_prefix (wp, prefix);
  char *next;
  char var[80];
  char v[60];
  sprintf (v, "%s_vifs", prefix);
  char *vifs = nvram_safe_get (v);
  if (vifs == NULL)
    return 0;
  foreach (var, vifs, next)
  {
    save_prefix (wp, var);
  }
  //nvram_commit ();
  return 0;
}

int
security_save (webs_t wp)
{
#ifdef HAVE_MADWIFI
  int dc = getdevicecount ();
  int i;
  for (i = 0; i < dc; i++)
    {
      char b[16];
      sprintf (b, "ath%d", i);
      security_save_prefix (wp, b);
    }
#else
  security_save_prefix (wp, "wl0");
#endif
}

#endif



#ifdef HAVE_MSSID

void
ej_show_wpa_setting (int eid, webs_t wp, int argc, char_t ** argv,
		     char *prefix)
{
  char *type, *security_mode;
  char var[80];
  sprintf (var, "%s_security_mode", prefix);
  cprintf ("show wpa setting\n");
  if (ejArgs (argc, argv, "%s", &type) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
  rep (var, '.', 'X');
  security_mode = GOZILA_GET (var);
  rep (var, 'X', '.');
  cprintf ("security mode %s = %s\n", security_mode, var);
  if (!strcmp (security_mode, "psk")
      || !strcmp (security_mode, "psk2")
      || !strcmp (security_mode, "psk psk2"))
    show_preshared (wp, prefix);
  //do_ej ("WPA_Preshared.asp", wp);
#if UI_STYLE != CISCO
  else if (!strcmp (security_mode, "disabled"))
    show_preshared (wp, prefix);
//    do_ej ("WPA_Preshared.asp", wp);
#endif
  else if (!strcmp (security_mode, "radius"))
    {
      //do_ej ("Radius.asp", wp);
      show_radius (wp, prefix);
      //show_wep (wp, prefix);
      //do_ej ("WEP.asp", wp);
    }
  else if (!strcmp (security_mode, "wpa")
	   || !strcmp (security_mode, "wpa2")
	   || !strcmp (security_mode, "wpa wpa2"))
    show_wparadius (wp, prefix);
  //do_ej ("WPA_Radius.asp", wp);

  else if (!strcmp (security_mode, "wep"))
    show_wep (wp, prefix);
#ifdef HAVE_MADWIFI
  else if (!strcmp (security_mode, "8021X"))
    show_80211X (wp, prefix);
#endif

  //do_ej ("WEP.asp", wp);

  return;
}
#else
void
ej_show_wpa_setting (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *type, *security_mode;

  if (ejArgs (argc, argv, "%s", &type) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  security_mode = GOZILA_GET ("security_mode");

  if (!strcmp (security_mode, "psk")
      || !strcmp (security_mode, "psk2")
      || !strcmp (security_mode, "psk psk2"))
    do_ej ("WPA_Preshared.asp", wp);
#if UI_STYLE != CISCO
  else if (!strcmp (security_mode, "disabled"))
    do_ej ("WPA_Preshared.asp", wp);
#endif
  else if (!strcmp (security_mode, "radius"))
    {
      do_ej ("Radius.asp", wp);
      do_ej ("WEP.asp", wp);
    }
  else if (!strcmp (security_mode, "wpa")
	   || !strcmp (security_mode, "wpa2")
	   || !strcmp (security_mode, "wpa wpa2"))
    do_ej ("WPA_Radius.asp", wp);
  else if (!strcmp (security_mode, "wep"))
    do_ej ("WEP.asp", wp);

  return;
}

#endif


void
ej_wl_ioctl (int eid, webs_t wp, int argc, char_t ** argv)
{
  int unit, val;
  char tmp[100], prefix[] = "wlXXXXXXXXXX_";
  char *op, *type, *var;
  char *name;

  if (ejArgs (argc, argv, "%s %s %s", &op, &type, &var) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  if ((unit = atoi (nvram_safe_get ("wl_unit"))) < 0)
    return;

  snprintf (prefix, sizeof (prefix), "wl%d_", unit);
  name = nvram_safe_get (strcat_r (prefix, "ifname", tmp));

  if (strcmp (op, "get") == 0)
    {
      if (strcmp (type, "int") == 0)
	{
#ifdef HAVE_MSSID
	  websWrite (wp, "%u",
		     wl_iovar_getint (name, var, &val) == 0 ? val : 0);
#else
	  websWrite (wp, "%u", wl_get_int (name, var, &val) == 0 ? val : 0);
#endif
	  return;
	}
    }
  return;
}

void
validate_wl_gmode (webs_t wp, char *value, struct variable *v)
{
  if (!valid_choice (wp, value, v))
    return;
  if (atoi (value) == GMODE_AFTERBURNER)
    {
      nvram_set ("wl_lazywds", "0");
      nvram_set ("wl_wds", "");
      nvram_set ("wl_mode", "ap");
      /*
         if(nvram_invmatch("security_mode", "disabled") && nvram_invmatch("security_mode", "wep")){
         nvram_set("security_mode", "disabled");
         nvram_set("security_mode_last", nvram_safe_get("security_mode"));
         nvram_set("security_mode", "disabled");
         }
       */
    }
  nvram_set (v->name, value);

  return;
  /* force certain wireless variables to fixed values */
  if (atoi (value) == GMODE_AFTERBURNER)
    {
      if (nvram_invmatch ("wl_auth_mode", "disabled") ||
#ifdef ABURN_WSEC_CHECK
	  nvram_invmatch ("wl_wep", "off") ||
#endif
	  nvram_invmatch ("wl_mode", "ap") ||
	  nvram_invmatch ("wl_lazywds", "0") || nvram_invmatch ("wl_wds", ""))
	{
	  /* notify the user */
#ifdef ABURN_WSEC_CHECK
	  websWrite (wp, "Invalid <b>%s</b>: AfterBurner mode requires:"
		     "<br><b>Network Authentication</b> set to <b>Disabled</b>"
		     "<br><b>Data Encryption</b> set to <b>Off</b>"
		     "<br><b>Mode</b> set to <b>Access Point</b>"
//                              "<br><b>Bridge Restrict</b> set to <b>Enabled</b>"
		     "<br><b>WDS devices</b> disabled." "<br>", v->longname);
#else
	  websWrite (wp, "Invalid <b>%s</b>: AfterBurner mode requires:"
		     "<br><b>Network Authentication</b> set to <b>Disabled</b>"
		     "<br><b>Mode</b> set to <b>Access Point</b>"
//                              "<br><b>Bridge Restrict</b> set to <b>Enabled</b>"
		     "<br><b>WDS devices</b> disabled." "<br>", v->longname);
#endif
	  return;
	}
    }
}

/* UI Mode		GMODE			Afterburner Override	Basic Rate Set	FrameBurst	CTS Protection
 * Mixed		6 - AfterBurner		-1			Default		ON		-1(auto)
 * 54g-Only		6 - AfterBurner		-1			ALL		ON		0(off)
 * 11b-Only		0 - 54g Legacy B	NA			Default		ON		-1(auto)
 */

/* Sveasoft note: settings for b-only, mixed, and g-mode set back to original defaults before "afterburner" mods.
   Afterburner bizarre settings maintained for "speedbooster" mode */

void
convert_wl_gmode (char *value)
{
/*if (nvram_match("wl_mode","ap"))
{
	if(!strcmp(value, "disabled")){
		nvram_set("wl_net_mode", value);
		nvram_set("wl_gmode", "-1");
	}
	else if(!strcmp(value, "mixed")){
		nvram_set("wl_net_mode", value);
		nvram_set("wl_gmode", "6");
		nvram_set("wl_afterburner", "auto");	// From 3.61.13.0
//		nvram_set("wl_afterburner_override", "-1");
		nvram_set("wl_rateset", "default");
		nvram_set("wl_frameburst", "on");
	//	nvram_set("wl_gmode_protection", "off");
	}
	else if(!strcmp(value, "g-only")){
		nvram_set("wl_net_mode", value);
		// In order to backward compatiable old firmware, we reserve original value "6", and we will exec "wl gmode 1" later
		nvram_set("wl_gmode", "6");
		nvram_set("wl_afterburner", "auto");
		nvram_set("wl_rateset", "all");
		nvram_set("wl_frameburst", "on");
		//nvram_set("wl_gmode_protection", "off");
	}
	else if(!strcmp(value, "speedbooster")){
		nvram_set("wl_net_mode", value);
		nvram_set("wl_gmode", "6");
		nvram_set("wl_afterburner_override", "1");
		nvram_set("wl_rateset", "all");
		nvram_set("wl_frameburst", "on");
	//	nvram_set("wl_gmode_protection", "off");
	}

	else if(!strcmp(value, "b-only")){
		nvram_set("wl_net_mode", value);
		nvram_set("wl_gmode", "0");
		nvram_set("wl_afterburner", "off");
		nvram_set("wl_rateset", "default");
		nvram_set("wl_frameburst", "on");

	}
}else*/
  {
//  fprintf(stderr,"gmode = %s\n",value);
#ifndef HAVE_MSSID
    if (nvram_match ("wl_net_mode", value))
      {
	return;
      }
#endif
    if (!strcmp (value, "disabled"))
      {
	nvram_set ("wl_net_mode", value);
	nvram_set ("wl_gmode", "-1");
#ifdef HAVE_MSSID
	nvram_set ("wl_nmode", "-1");
#endif
	nvram_set("wl_nreqd", "0");
      }
    else if (!strcmp (value, "mixed"))
      {
	nvram_set ("wl_net_mode", value);
#ifdef HAVE_MSSID
	nvram_set ("wl_gmode", "1");
#else
	nvram_set ("wl_gmode", "6");
#endif
	nvram_set ("wl_afterburner", "auto");	// From 3.61.13.0
	nvram_set ("wl_rateset", "default");
#ifdef HAVE_MSSID
	nvram_set ("wl_nmode", "-1");
#endif
	nvram_set ("wl_frameburst", "on");
	nvram_set ("wl_phytype", "g");
	nvram_set("wl_nreqd", "0");

      }
#ifdef HAVE_MSSID
    else if (!strcmp (value, "bg-mixed"))
      {
	nvram_set ("wl_net_mode", value);
	nvram_set ("wl_gmode", "1");
	nvram_set ("wl_afterburner", "auto");	// From 3.61.13.0
	nvram_set ("wl_rateset", "default");
#ifdef HAVE_MSSID
	nvram_set ("wl_nmode", "0");
#endif
	nvram_set ("wl_frameburst", "on");
	nvram_set ("wl_phytype", "g");
	nvram_set("wl_nreqd", "0");

      }
#endif
    else if (!strcmp (value, "g-only"))
      {
	nvram_set ("wl_net_mode", value);
#ifdef HAVE_MSSID
	nvram_set ("wl_nmode", "0");
#endif
	nvram_set ("wl_gmode", "2");
	nvram_set ("wl_phytype", "g");
	nvram_set("wl_nreqd", "0");

      }
    else if (!strcmp (value, "b-only"))
      {
	nvram_set ("wl_net_mode", value);
	nvram_set ("wl_gmode", "0");
#ifdef HAVE_MSSID
	nvram_set ("wl_nmode", "0");
#endif
	nvram_set ("wl_afterburner", "off");	// From 3.61.13.0
	nvram_set ("wl_rateset", "default");
	nvram_set ("wl_frameburst", "on");
	nvram_set ("wl_phytype", "g");
	nvram_set("wl_nreqd", "0");

      }
#ifdef HAVE_MSSID
    else if (!strcmp (value, "n-only"))
      {
      
        nvram_set("wl_net_mode", value);
	nvram_set("wl_gmode", "1");
	nvram_set("wl_nmode", "2");
	nvram_set("wl_nreqd", "1");
	nvram_set("wl_afterburner", "off");	// From 3.61.13.0
	nvram_set ("wl_phytype", "n");
      }
#endif
    else if (!strcmp (value, "a-only"))
      {
	nvram_set ("wl_net_mode", value);
	nvram_set ("wl_phytype", "a");
	nvram_set("wl_nreqd", "0");
      }



  }
}

void
validate_wl_net_mode (webs_t wp, char *value, struct variable *v)
{

  if (!valid_choice (wp, value, v))
    return;

  convert_wl_gmode (value);

  nvram_set (v->name, value);
}

void
ej_wme_match_op (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name, *match, *output;
  char word[256], *next;

  if (ejArgs (argc, argv, "%s %s %s", &name, &match, &output) < 3)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  foreach (word, nvram_safe_get (name), next)
  {
    if (!strcmp (word, match))
      {
	websWrite (wp, output);
	return;
      }
  }

  return;
}

void
validate_noack (webs_t wp, char *value, struct variable *v)
{
  char *wme;

  /* return if wme is not enabled */
  if (!(wme = websGetVar (wp, "wl_wme", NULL)))
    return;
  else if (strcmp (wme, "on"))
    return;

  validate_choice (wp, value, v);
}

void
validate_wl_wme_params (webs_t wp, char *value, struct variable *v)
{
  int n, i;
  int cwmin = 0, cwmax = 0;
  char *wme, *afterburner;
  char name[100];
  char buf[1000] = "", *cur = buf;
  struct
  {
    char *name;
    int range;
    char *arg1;
    char *arg2;
  } field_attrib[] =
  {
    {
    "WME AC CWmin", 1, "0", "32767"},
    {
    "WME AC CWmax", 1, "0", "32767"},
    {
    "WME AC AIFSN", 1, "1", "15"},
    {
    "WME AC TXOP(b)", 1, "0", "65504"},
    {
    "WME AC TXOP(a/g)", 1, "0", "65504"},
    {
    "WME AC Admin Forced", 0, "on", "off"}
  };

  /* return if wme is not enabled */
  if (!(wme = websGetVar (wp, "wl_wme", NULL)))
    return;
  else if (strcmp (wme, "on"))
    return;

  /* return if afterburner enabled */
  if ((afterburner = websGetVar (wp, "wl_afterburner", NULL))
      && (!strcmp (afterburner, "auto")))
    return;

  n = atoi (value) + 1;

  for (i = 0; i < n; i++)
    {
      snprintf (name, sizeof (name), "%s%d", v->name, i);
      if (!(value = websGetVar (wp, name, NULL)))
	return;
      if (!*value && v->nullok)
	continue;

      if (i == 0)
	cwmin = atoi (value);
      else if (i == 1)
	{
	  cwmax = atoi (value);
	  if (cwmax < cwmin)
	    {
	      websDebugWrite (wp,
			      "Invalid <b>%s</b> %d: greater than <b>%s</b> %d<br>",
			      field_attrib[0].name, cwmin,
			      field_attrib[i].name, cwmax);
	      return;
	    }
	}
      if (field_attrib[i].range)
	{
	  if (atoi (value) < atoi (field_attrib[i].arg1)
	      || atoi (value) > atoi (field_attrib[i].arg2))
	    {
	      websDebugWrite (wp,
			      "Invalid <b>%s</b> %d: should be in range %s to %s<br>",
			      field_attrib[i].name, atoi (value),
			      field_attrib[i].arg1, field_attrib[i].arg2);
	      return;
	    }
	}
      else
	{
	  if (strcmp (value, field_attrib[i].arg1)
	      && strcmp (value, field_attrib[i].arg2))
	    {
	      websDebugWrite (wp,
			      "Invalid <b>%s</b> %s: should be %s or %s<br>",
			      field_attrib[i].name, value,
			      field_attrib[i].arg1, field_attrib[i].arg2);
	    }
	}

      cur += snprintf (cur, buf + sizeof (buf) - cur, "%s%s",
		       cur == buf ? "" : " ", value);
    }

  nvram_set (v->name, buf);
}
