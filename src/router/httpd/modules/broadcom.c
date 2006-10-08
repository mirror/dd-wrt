
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
 * $Id: broadcom.c,v 1.9 2005/11/30 11:53:42 seg Exp $
 */

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else /* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <httpd.h>
#include <errno.h>
#endif /* WEBS */

#include <proto/ethernet.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <cyutils.h>
#include <support.h>
#include <cy_conf.h>
//#ifdef EZC_SUPPORT
#include <ezc.h>
//#endif
#include <broadcom.h>
#include <wlutils.h>


int gozila_action = 0;
int error_value = 0;
int browser_method;
int debug_value = 0;



//static char * rfctime(const time_t *timep);
//static char * reltime(unsigned int seconds);

//#if defined(linux)

#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#define service_restart() kill(1, SIGUSR1)
#define sys_restart() kill(1, SIGHUP)
#define sys_reboot() kill(1, SIGTERM)
#define sys_stats(url) eval("stats", (url))


//tofu
int tf_webWriteESCNV (webs_t wp, const char *nvname);

#ifdef HAVE_UPNP
static int tf_upnp (webs_t wp);
static void ej_tf_upnp (int eid, webs_t wp, int argc, char_t ** argv);
#endif

/* Example:
 * ISDIGIT("", 0); return true;
 * ISDIGIT("", 1); return false;
 * ISDIGIT("123", 1); return true;
 */
int
ISDIGIT (char *value, int flag)
{
  int i, tag = TRUE;


  if (!strcmp (value, ""))
    {
      if (flag)
	return 0;		// null
      else
	return 1;
    }

  for (i = 0; *(value + i); i++)
    {
      if (!isdigit (*(value + i)))
	{
	  tag = FALSE;
	  break;
	}
    }
  return tag;
}

/* Example:
 * ISASCII("", 0); return true;
 * ISASCII("", 1); return false;
 * ISASCII("abc123", 1); return true;
 */
int
ISASCII (char *value, int flag)
{
  int i, tag = TRUE;

#if COUNTRY == JAPAN
  return tag;			// don't check for japan version
#endif

  if (!strcmp (value, ""))
    {
      if (flag)
	return 0;		// null
      else
	return 1;
    }

  for (i = 0; *(value + i); i++)
    {
      if (!isascii (*(value + i)))
	{
	  tag = FALSE;
	  break;
	}
    }
  return tag;
}

/* Example:
 * legal_hwaddr("00:11:22:33:44:aB"); return true;
 * legal_hwaddr("00:11:22:33:44:5"); return false;
 * legal_hwaddr("00:11:22:33:44:HH"); return false;
 */
int
legal_hwaddr (char *value)
{
  unsigned int hwaddr[6];
  int tag = TRUE;
  int i, count;

  /* Check for bad, multicast, broadcast, or null address */
  for (i = 0, count = 0; *(value + i); i++)
    {
      if (*(value + i) == ':')
	{
	  if ((i + 1) % 3 != 0)
	    {
	      tag = FALSE;
	      break;
	    }
	  count++;
	}
      else if (isxdigit (*(value + i)))	/* one of 0 1 2 3 4 5 6 7 8 9 a b c d e f A B C D E F */
	continue;
      else
	{
	  tag = FALSE;
	  break;
	}
    }

  if (!tag || i != 17 || count != 5)	/* must have 17's characters and 5's ':' */
    tag = FALSE;
  else if (sscanf (value, "%x:%x:%x:%x:%x:%x",
		   &hwaddr[0], &hwaddr[1], &hwaddr[2],
		   &hwaddr[3], &hwaddr[4], &hwaddr[5]) != 6)
    {
      //(hwaddr[0] & 1) ||                // the bit 7 is 1
      //(hwaddr[0] & hwaddr[1] & hwaddr[2] & hwaddr[3] & hwaddr[4] & hwaddr[5]) == 0xff ){ // FF:FF:FF:FF:FF:FF
      //(hwaddr[0] | hwaddr[1] | hwaddr[2] | hwaddr[3] | hwaddr[4] | hwaddr[5]) == 0x00){ // 00:00:00:00:00:00
      tag = FALSE;
    }
  else
    tag = TRUE;


  return tag;
}

/* Example:
 * 255.255.255.0  (111111111111111111111100000000)  is a legal netmask
 * 255.255.0.255  (111111111111110000000011111111)  is an illegal netmask
 */
int
legal_netmask (char *value)
{
  struct in_addr ipaddr;
  int ip[4] = { 0, 0, 0, 0 };
  int i, j;
  int match0 = -1;
  int match1 = -1;
  int ret, tag;

  ret = sscanf (value, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);

  if (ret == 4 && inet_aton (value, &ipaddr))
    {
      for (i = 3; i >= 0; i--)
	{
	  for (j = 1; j <= 8; j++)
	    {
	      if ((ip[i] % 2) == 0)
		match0 = (3 - i) * 8 + j;
	      else if (((ip[i] % 2) == 1) && match1 == -1)
		match1 = (3 - i) * 8 + j;
	      ip[i] = ip[i] / 2;
	    }
	}
    }

  if (match0 >= match1)
    tag = FALSE;
  else
    tag = TRUE;


  return tag;
}


/* Example:
 * legal_ipaddr("192.168.1.1"); return true;
 * legal_ipaddr("192.168.1.1111"); return false;
 */
int
legal_ipaddr (char *value)
{
  struct in_addr ipaddr;
  int ip[4];
  int ret, tag;

  ret = sscanf (value, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);

  if (ret != 4 || !inet_aton (value, &ipaddr))
    tag = FALSE;
  else
    tag = TRUE;


  return tag;
}

/* Example:
 * legal_ip_netmask("192.168.1.1","255.255.255.0","192.168.1.100"); return true;
 * legal_ip_netmask("192.168.1.1","255.255.255.0","192.168.2.100"); return false;
 */
int
legal_ip_netmask (char *sip, char *smask, char *dip)
{
  struct in_addr ipaddr, netaddr, netmask;
  int tag;

  inet_aton (nvram_safe_get (sip), &netaddr);
  inet_aton (nvram_safe_get (smask), &netmask);
  inet_aton (dip, &ipaddr);

  netaddr.s_addr &= netmask.s_addr;

  if (netaddr.s_addr != (ipaddr.s_addr & netmask.s_addr))
    tag = FALSE;
  else
    tag = TRUE;


  return tag;
}


/* Example:
 * wan_dns = 1.2.3.4 10.20.30.40 15.25.35.45
 * get_dns_ip("wan_dns", 1, 2); produces "20"
 */
int
get_dns_ip (char *name, int which, int count)
{
  static char word[256];
  char *next;
  int ip;

  foreach (word, nvram_safe_get (name), next)
  {
    if (which-- == 0)
      {
	ip = get_single_ip (word, count);
	return ip;
      }
  }
  return 0;
}


/* Example:
 * wan_mac = 00:11:22:33:44:55
 * get_single_mac("wan_mac", 1); produces "11"
 */
int
get_single_mac (char *macaddr, int which)
{
  int mac[6] = { 0, 0, 0, 0, 0, 0 };
  int ret;

  ret =
    sscanf (macaddr, "%2X:%2X:%2X:%2X:%2X:%2X", &mac[0], &mac[1], &mac[2],
	    &mac[3], &mac[4], &mac[5]);
  return mac[which];
}


/* Example:
 * lan_ipaddr_0 = 192
 * lan_ipaddr_1 = 168
 * lan_ipaddr_2 = 1
 * lan_ipaddr_3 = 1
 * get_merge_ipaddr("lan_ipaddr", ipaddr); produces ipaddr="192.168.1.1"
 */
int
get_merge_ipaddr (webs_t wp, char *name, char *ipaddr)
{
  char ipname[30];
  int i;
  char buf[50] = { 0 };
  char *ip[4];
  char *tmp;
  //cprintf("ip addr\n");
  strcpy (ipaddr, "");
  //cprintf("safe get\n");
  char *ipa = nvram_safe_get (name);
  //cprintf("strcpy\n");
  if (ipa == NULL)
    strcpy (buf, "0.0.0.0");
  else
    strcpy (buf, ipa);
  //cprintf("strsep\n");
  char *b = (char *) &buf;
  ip[0] = strsep (&b, ".");
  ip[1] = strsep (&b, ".");
  ip[2] = strsep (&b, ".");
  ip[3] = b;


  for (i = 0; i < 4; i++)
    {
      //cprintf("merge %s_%d\n",name,i);
      snprintf (ipname, sizeof (ipname), "%s_%d", name, i);
      tmp = websGetVar (wp, ipname, ip[i]);
      if (tmp == NULL)
	return 0;
      strcat (ipaddr, tmp);
      if (i < 3)
	strcat (ipaddr, ".");
    }

  return 1;

}


/* Example:
 * wan_mac_0 = 00
 * wan_mac_1 = 11
 * wan_mac_2 = 22
 * wan_mac_3 = 33
 * wan_mac_4 = 44
 * wan_mac_5 = 55
 * get_merge_mac("wan_mac",mac); produces mac="00:11:22:33:44:55"
 */
int
get_merge_mac (webs_t wp, char *name, char *macaddr)
{
  char macname[30];
  char *mac;
  int i;

  strcpy (macaddr, "");

  for (i = 0; i < 6; i++)
    {
      snprintf (macname, sizeof (macname), "%s_%d", name, i);
      mac = websGetVar (wp, macname, "00");
      if (strlen (mac) == 1)
	strcat (macaddr, "0");
      strcat (macaddr, mac);
      if (i < 5)
	strcat (macaddr, ":");
    }

  return 1;

}

struct onload onloads[] = {
  //{ "Filters", filter_onload },
  {"WL_ActiveTable", wl_active_onload},
  {"MACClone", macclone_onload},
  {"FilterSummary", filtersummary_onload},
  {"Ping", ping_onload},
//  {"Traceroute", traceroute_onload},
};

void
ej_onload (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *type, *arg;
  int ret = 0;
  struct onload *v;

  if (ejArgs (argc, argv, "%s %s", &type, &arg) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  for (v = onloads; v < &onloads[STRUCT_LEN (onloads)]; v++)
    {
      if (!strcmp (v->name, type))
	{
	  ret = v->go (wp, arg);
	  return;
	}
    }

  return;
}

/* Meta tag command that will no allow page cached by browsers.
 * The will force the page to be refreshed when visited.
 */
void
ej_no_cache (int eid, webs_t wp, int argc, char_t ** argv)
{

  websWrite (wp, "<meta http-equiv=\"expires\" content=\"0\">\n");
  websWrite (wp,
	     "<meta http-equiv=\"cache-control\" content=\"no-cache\">\n");
  websWrite (wp, "<meta http-equiv=\"pragma\" content=\"no-cache\">\n");

  return;
}


/*
 * Example:
 * lan_ipaddr=192.168.1.1
 * <% prefix_ip_get("lan_ipaddr",1); %> produces "192.168.1."
 */
void
ej_prefix_ip_get (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name;
  int type;

  if (ejArgs (argc, argv, "%s %d", &name, &type) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  if (type == 1)
    websWrite (wp, "%d.%d.%d.", get_single_ip (nvram_safe_get (name), 0),
	       get_single_ip (nvram_safe_get (name), 1),
	       get_single_ip (nvram_safe_get (name), 2));
  if (type == 2)
    websWrite (wp, "%d.%d.", get_single_ip (nvram_safe_get (name), 0),
	       get_single_ip (nvram_safe_get (name), 1));

  return;
}

void
prefix_ip_get (char *name, char *buf, int type)
{
  if (type == 1)
    sprintf (buf, "%d.%d.%d.", get_single_ip (nvram_safe_get (name), 0),
	     get_single_ip (nvram_safe_get (name), 1),
	     get_single_ip (nvram_safe_get (name), 2));
  if (type == 2)
    sprintf (buf, "%d.%d.", get_single_ip (nvram_safe_get (name), 0),
	     get_single_ip (nvram_safe_get (name), 1));
}

/* Deal with side effects before committing */
int
sys_commit (void)
{

  if (nvram_match ("dhcpnvram", "1"))
    {				// update lease -- tofu
      eval ("killall", "-SIGUSR2", "dnsmasq");
      sleep (1);
    }

  //if (nvram_match("wan_proto", "pppoe") || nvram_match("wan_proto", "pptp") )
  //      nvram_set("wan_ifname", "ppp0");
  //else
  //      nvram_set("wan_ifname", nvram_get("pppoe_ifname"));
  return nvram_commit ();
}


char *
rfctime (const time_t * timep)
{
  static char s[201];
  struct tm tm;

  setenv ("TZ", nvram_safe_get ("time_zone"), 1);
  memcpy (&tm, localtime (timep), sizeof (struct tm));
  //strftime(s, 200, "%a, %d %b %Y %H:%M:%S %z", &tm);
  strftime (s, 200, "%a, %d %b %Y %H:%M:%S", &tm);	// spec for linksys
  return s;
}

static char *
reltime (unsigned int seconds)
{
  static char s[] = "XXXXX days, XX hours, XX minutes, XX seconds";
  char *c = s;

  if (seconds > 86400)		//60 * 60 * 24
    {
      c += sprintf (c, "%d days, ", seconds / 86400);	//60 * 60 * 24
      seconds %= 86400;		//60 * 60 * 24
    }
  if (seconds > 3600)		//60 * 60 
    {
      c += sprintf (c, "%d hours, ", seconds / 3600);	//60 * 60
      seconds %= 3600;		//60 * 60
    }
  if (seconds > 60)
    {
      c += sprintf (c, "%d minutes, ", seconds / 60);
      seconds %= 60;
    }
  c += sprintf (c, "%d seconds", seconds);

  return s;
}

/*
 * Example:
 * lan_ipaddr = 192.168.1.1
 * <% nvram_get("lan_ipaddr"); %> produces "192.168.1.1"
 */
static void
ej_nvram_get (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name;

  if (ejArgs (argc, argv, "%s", &name) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

#if COUNTRY == JAPAN
  websWrite (wp, "%s", nvram_safe_get (name));
#else
  /*for (c = nvram_safe_get (name); *c; c++)
     {
     if (isprint ((int) *c))    // &&
     //                  *c != '"' && *c != '&' && *c != '<' && *c != '>')
     ret += websWrite (wp, "%c", *c);
     else
     {
     if (*c == '"')
     ret += websWrite (wp, "&quot;");
     else if (*c == '&')
     ret += websWrite (wp, "&amp;");
     else if (*c == '<')
     ret += websWrite (wp, "&lt;");
     else if (*c == '>')
     ret += websWrite (wp, "&gt;");
     else if (*c == 13)
     continue;
     else
     ret += websWrite (wp, "&#%d", *c);
     }
     } */

  tf_webWriteESCNV (wp, name);	// test: buffered version of above

  return;
#endif

  return;
}

static void
ej_nvram_real_get (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name;

  if (ejArgs (argc, argv, "%s", &name) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  websWrite (wp, "%s", nvram_safe_get (name));

  return;
}

/*
 * Example:
 * lan_ipaddr = 192.168.1.1, gozila_action = 0
 * <% nvram_selget("lan_ipaddr"); %> produces "192.168.1.1"
 * lan_ipaddr = 192.168.1.1, gozila_action = 1, websGetVar(wp, "lan_proto", NULL) = 192.168.1.2;
 * <% nvram_selget("lan_ipaddr"); %> produces "192.168.1.2"
 */
static void
ej_nvram_selget (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name;

  if (ejArgs (argc, argv, "%s", &name) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
  if (gozila_action)
    {
      char *buf = websGetVar (wp, name, NULL);
      if (buf)
	{
	  websWrite (wp, "%s", buf);
	  return;
	}
    }

/*  for (c = nvram_safe_get (name); *c; c++)
    {
      if (isprint ((int) *c) &&
	  *c != '"' && *c != '&' && *c != '<' && *c != '>')
	ret += websWrite (wp, "%c", *c);
      else
	ret += websWrite (wp, "&#%d", *c);
    }*/
  tf_webWriteESCNV (wp, name);	// test: buffered version of above

  return;
}

/*
 * Example:
 * wan_mac = 00:11:22:33:44:55
 * <% nvram_mac_get("wan_mac"); %> produces "00-11-22-33-44-55"
 */
static void
ej_nvram_mac_get (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name, *c;
  char *mac;
  int i;

  if (ejArgs (argc, argv, "%s", &name) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
  c = nvram_safe_get (name);

  if (c)
    {
      mac = strdup (c);
      for (i = 0; *(mac + i); i++)
	{
	  if (*(mac + i) == ':')
	    *(mac + i) = '-';
	}
      websWrite (wp, "%s", mac);
      free (mac);		// leak, thx tofu
    }

  return;

}

/*
 * Example:
 * wan_proto = dhcp; gozilla = 0;
 * <% nvram_gozila_get("wan_proto"); %> produces "dhcp"
 *
 * wan_proto = dhcp; gozilla = 1; websGetVar(wp, "wan_proto", NULL) = static;
 * <% nvram_gozila_get("wan_proto"); %> produces "static"
 */
static void
ej_nvram_gozila_get (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name, *type;

  if (ejArgs (argc, argv, "%s", &name) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
  type = GOZILA_GET (name);

  websWrite (wp, "%s", type);
}

static void
ej_webs_get (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name, *value;

  if (ejArgs (argc, argv, "%s", &name) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  value = websGetVar (wp, name, NULL);

  if (value)
    websWrite (wp, "%s", value);

  return;
}

/*
 * Example:
 * lan_ipaddr = 192.168.1.1
 * <% get_single_ip("lan_ipaddr","1"); %> produces "168"
 */
static void
ej_get_single_ip (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name, *c;
  int which;

  if (ejArgs (argc, argv, "%s %d", &name, &which) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  c = nvram_safe_get (name);
  if (c)
    {
      if (!strcmp (c, PPP_PSEUDO_IP) || !strcmp (c, PPP_PSEUDO_GW))
	c = "0.0.0.0";
      else if (!strcmp (c, PPP_PSEUDO_NM))
	c = "255.255.255.0";

      websWrite (wp, "%d", get_single_ip (c, which));
    }
  else
    websWrite (wp, "0");

  return;
}

/*
 * Example:
 * wan_mac = 00:11:22:33:44:55
 * <% get_single_mac("wan_mac","1"); %> produces "11"
 */
static void
ej_get_single_mac (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name, *c;
  int which;
  int mac;

  if (ejArgs (argc, argv, "%s %d", &name, &which) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  c = nvram_safe_get (name);
  if (c)
    {
      mac = get_single_mac (c, which);
      websWrite (wp, "%02X", mac);
    }
  else
    websWrite (wp, "00");

  return;
}

/*
 * Example:
 * wan_proto = dhcp; gozilla = 0;
 * <% nvram_selmatch("wan_proto", "dhcp", "selected"); %> produces "selected"
 *
 * wan_proto = dhcp; gozilla = 1; websGetVar(wp, "wan_proto", NULL) = static;
 * <% nvram_selmatch("wan_proto", "static", "selected"); %> produces "selected"
 */

int
nvram_selmatch (webs_t wp, char *name, char *match)
{
  char *type = GOZILA_GET (name);
  if (!type)
    {
      if (nvram_match (name, match))
	{
	  return 1;
	}
    }
  else
    {
      if (!strcmp (type, match))
	{
	  return 1;
	}
    }
  return 0;
}

void
ej_nvram_selmatch (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name, *match, *output;

  if (ejArgs (argc, argv, "%s %s %s", &name, &match, &output) < 3)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  if (nvram_selmatch (wp, name, match))
    {
      websWrite (wp, output);
    }
  return;
}

void
ej_nvram_else_selmatch (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name, *match, *output1, *output2;
  char *type;

  if (ejArgs (argc, argv, "%s %s %s %s", &name, &match, &output1, &output2) <
      4)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  type = GOZILA_GET (name);

  if (!type)
    {
      if (nvram_match (name, match))
	{
	  websWrite (wp, output1);
	}
      else
	websWrite (wp, output2);
    }
  else
    {
      if (!strcmp (type, match))
	{
	  websWrite (wp, output1);
	}
      else
	websWrite (wp, output2);
    }

  return;
}

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_else_match("wan_proto", "dhcp", "0","1"); %> produces "0"
 * <% nvram_else_match("wan_proto", "static", "0","1"); %> produces "1"
 */
static void
ej_nvram_else_match (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name, *match, *output1, *output2;

  if (ejArgs (argc, argv, "%s %s %s %s", &name, &match, &output1, &output2) <
      4)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  if (nvram_match (name, match))
    websWrite (wp, output1);
  else
    websWrite (wp, output2);

  return;
}

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_match("wan_proto", "dhcp", "selected"); %> produces "selected"
 * <% nvram_match("wan_proto", "static", "selected"); %> does not produce
 */
static void
ej_nvram_match (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name, *match, *output;

  if (ejArgs (argc, argv, "%s %s %s", &name, &match, &output) < 3)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  if (nvram_match (name, match))
    websWrite (wp, output);

  return;
}

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_invmatch("wan_proto", "dhcp", "disabled"); %> does not produce
 * <% nvram_invmatch("wan_proto", "static", "disabled"); %> produces "disabled"
 */
static void
ej_nvram_invmatch (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name, *invmatch, *output;

  if (ejArgs (argc, argv, "%s %s %s", &name, &invmatch, &output) < 3)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  if (nvram_invmatch (name, invmatch))
    websWrite (wp, output);

  return;
}

/*
 * Example:
 * HEARTBEAT_SUPPORT = 1
 * <% support_match("HEARTBEAT_SUPPORT", "0", "selected"); %> does not produce
 * <% support_match("HEARTBEAT_SUPPORT", "1", "selected"); %> produces "selected"
 */
static void
ej_support_match (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name, *value, *output;

  if (ejArgs (argc, argv, "%s %s %s", &name, &value, &output) < 3)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

#ifdef HAVE_HTTPS
  if (do_ssl)
    {
      if (!strcmp (name, "HTTPS"))
	{
	  return;
	}
    }
#endif


  if (!strcmp (name, "WL_STA_SUPPORT") ||
      !strcmp (name, "BACKUP_RESTORE_SUPPORT") ||
      !strcmp (name, "SYSLOG_SUPPORT"))
    return;
#ifdef HAVE_MULTICAST
  if (!strcmp (name, "MULTICAST_SUPPORT") && !strcmp(value,"1"))
    websWrite(wp,output);
#endif
    
/*
   struct support_list *v;
   for (v = supports; v < &supports[SUPPORT_COUNT]; v++)
    {
      if (!strcmp (v->supp_name, name) && !strcmp (v->supp_value, value))
	{
	  websWrite (wp, output);
	  return;
	}
    }
*/
  return;
}


/*
 * Example:
 * HEARTBEAT_SUPPORT = 1
 * <% support_invmatch("HEARTBEAT_SUPPORT", "1", "<!--"); %> does not produce
 * HEARTBEAT_SUPPORT = 0
 * <% support_invmatch("HEARTBEAT_SUPPORT", "1", "-->"); %> produces "-->"
 */
static void
ej_support_invmatch (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name, *value, *output;

  if (ejArgs (argc, argv, "%s %s %s", &name, &value, &output) < 3)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#ifdef HAVE_HTTPS
  if (do_ssl)
    {
      if (!strcmp (name, "HTTPS"))
	{
	  return;
	}
    }
#endif
  if (!strcmp (name, "WL_STA_SUPPORT") ||
      !strcmp (name, "BACKUP_RESTORE_SUPPORT") ||
      !strcmp (name, "SYSLOG_SUPPORT"))
    {
      websWrite (wp, output);
      return;
    }
#ifdef HAVE_MULTICAST
  if (!strcmp (name, "MULTICAST_SUPPORT") && strcmp(value,"1"))
    websWrite(wp,output);
#endif
/*
  struct support_list *v;
  for (v = supports; v < &supports[SUPPORT_COUNT]; v++)
    {
      if (!strcmp (v->supp_name, name))
	{
	  if (strcmp (v->supp_value, value))
	    {
	      websWrite (wp, output);
	      return;
	    }
	  else
	    return;
	}
    }
*/
  return;
  
  //websWrite (wp, output);
}

/*
 * Example:
 * HEARTBEAT_SUPPORT = 1
 * <% support_elsematch("HEARTBEAT_SUPPORT", "1", "black", "red"); %> procude "black"
 */
static void
ej_support_elsematch (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name, *value, *output1, *output2;

  if (ejArgs (argc, argv, "%s %s %s %s", &name, &value, &output1, &output2) <
      3)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#ifdef HAVE_HTTPS
  if (do_ssl)
    {
      if (!strcmp (name, "HTTPS"))
	{
	  websWrite (wp, output1);
	  return;
	}
    }
#endif


  if (!strcmp (name, "WL_STA_SUPPORT") ||
      !strcmp (name, "BACKUP_RESTORE_SUPPORT") ||
      !strcmp (name, "SYSLOG_SUPPORT"))
    {
      websWrite (wp, output2);
      return;
    }
#ifdef HAVE_MULTICAST
  if (!strcmp (name, "MULTICAST_SUPPORT") && !strcmp(value,"1"))
    {
    websWrite(wp,output1);
    return;
    }
#endif

/*
  struct support_list *v;
  for (v = supports; v < &supports[SUPPORT_COUNT]; v++)
    {
      if (!strcmp (v->supp_name, name) && !strcmp (v->supp_value, value))
	{
	  websWrite (wp, output1);
	  return;
	}
    }
*/
  websWrite (wp, output2);
}

static void
ej_scroll (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *type;
  int y;

  if (ejArgs (argc, argv, "%s %d", &type, &y) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
  if (gozila_action)
    websWrite (wp, "%d", y);
  else
    websWrite (wp, "0");

  return;
}

/*
 * Example:
 * filter_mac=00:12:34:56:78:00 00:87:65:43:21:00
 * <% nvram_list("filter_mac", 1); %> produces "00:87:65:43:21:00"
 * <% nvram_list("filter_mac", 100); %> produces ""
 */
static void
ej_nvram_list (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name;
  int which;
  char word[256], *next;

  if (ejArgs (argc, argv, "%s %d", &name, &which) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  foreach (word, nvram_safe_get (name), next)
  {
    if (which-- == 0)
      websWrite (wp, word);
  }

  return;
}

/* Example:
 * wan_dns = 168.95.1.1 210.66.161.125 168.95.192.1
 * <% get_dns_ip("wan_dns", "1", "2"); %> produces "161"
 * <% get_dns_ip("wan_dns", "2", "3"); %> produces "1"
 */
void
ej_get_dns_ip (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name;
  int count, which;
  char word[256], *next;

  if (ejArgs (argc, argv, "%s %d %d", &name, &which, &count) < 3)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  foreach (word, nvram_safe_get (name), next)
  {
    if (which-- == 0)
      {
	websWrite (wp, "%d", get_single_ip (word, count));
	return;
      }
  }

  websWrite (wp, "0");		// not find
}


static unsigned long
inet_atoul (char *cp)
{
  struct in_addr in;

  (void) inet_aton (cp, &in);
  return in.s_addr;
}

int
valid_wep_key (webs_t wp, char *value, struct variable *v)
{
  int i;

  switch (strlen (value))
    {
    case 5:
    case 13:
      for (i = 0; *(value + i); i++)
	{
	  if (isascii (*(value + i)))
	    {
	      continue;
	    }
	  else
	    {
	      websDebugWrite (wp,
			      "Invalid <b>%s</b> %s: must be ascii code<br>",
			      v->longname, value);
	      return FALSE;
	    }
	}
      break;
    case 10:
    case 26:
      for (i = 0; *(value + i); i++)
	{
	  if (isxdigit (*(value + i)))
	    {			/* one of 0 1 2 3 4 5 6 7 8 9 a b c d e f A B C D E F */
	      continue;
	    }
	  else
	    {
	      websDebugWrite (wp,
			      "Invalid <b>%s</b> %s: must be hexadecimal digits<br>",
			      v->longname, value);
	      return FALSE;
	    }
	}
      break;

    default:
      websDebugWrite (wp,
		      "Invalid <b>%s</b>: must be 5 or 13 ASCII characters or 10 or 26 hexadecimal digits<br>",
		      v->longname);
      return FALSE;

    }

/*
	for(i=0 ; *(value+i) ; i++){
		if(isxdigit(*(value+i))){
			continue;
		}
		else{
			websDebugWrite(wp, "Invalid <b>%s</b> %s: must be hexadecimal digits<br>",
				  v->longname, value);
			return FALSE;
		}
	}

	if (i != length) {
		websDebugWrite(wp, "Invalid <b>%s</b> %s: must be %d characters<br>",
			  v->longname, value,length);
		return FALSE;
	}
*/
  return TRUE;
}

void
validate_statics (webs_t wp, char *value, struct variable *v)
{

  if (!sv_valid_statics (value))
    {
      websDebugWrite (wp,
		      "Invalid <b>%s</b> %s: not a legal statics entry<br>",
		      v->longname, value);
      return;
    }

  nvram_set (v->name, value);
}

int
valid_netmask (webs_t wp, char *value, struct variable *v)
{

  if (!legal_netmask (value))
    {
      websDebugWrite (wp, "Invalid <b>%s</b> %s: not a legal netmask<br>",
		      v->longname, value);
      return FALSE;
    }

  return TRUE;

}

static void
validate_netmask (webs_t wp, char *value, struct variable *v)
{
  if (valid_netmask (wp, value, v))
    nvram_set (v->name, value);
}

static void
validate_merge_netmask (webs_t wp, char *value, struct variable *v)
{
  char netmask[20], maskname[30];
  char *mask;
  int i;
  strcpy (netmask, "");
  for (i = 0; i < 4; i++)
    {
      snprintf (maskname, sizeof (maskname), "%s_%d", v->name, i);
      mask = websGetVar (wp, maskname, NULL);
      if (mask)
	{
	  strcat (netmask, mask);
	  if (i < 3)
	    strcat (netmask, ".");
	}
      else
	{
	  return;
	}
    }


  if (valid_netmask (wp, netmask, v))
    nvram_set (v->name, netmask);
}

//Added by Daniel(2004-07-29) for EZC
//char webs_buf[5000];
//int webs_buf_offset = 0;

static void
validate_list (webs_t wp, char *value, struct variable *v,
	       int (*valid) (webs_t, char *, struct variable *))
{
  int n, i;
  char name[100];
  char buf[1000] = "", *cur = buf;

  n = atoi (value);

  for (i = 0; i < n; i++)
    {
      snprintf (name, sizeof (name), "%s%d", v->name, i);
      if (!(value = websGetVar (wp, name, NULL)))
	return;
      if (!*value && v->nullok)
	continue;
      if (!valid (wp, value, v))
	continue;
      cur += snprintf (cur, buf + sizeof (buf) - cur, "%s%s",
		       cur == buf ? "" : " ", value);
    }
  nvram_set (v->name, buf);

}

int
valid_ipaddr (webs_t wp, char *value, struct variable *v)
{
  struct in_addr netaddr, netmask;

  if (!legal_ipaddr (value))
    {
      websDebugWrite (wp, "Invalid <b>%s</b> %s: not an IP address<br>",
		      v->longname, value);
      return FALSE;
    }

  if (v->argv)
    {
      if (!strcmp (v->argv[0], "lan"))
	{
	  if (*(value + strlen (value) - 2) == '.'
	      && *(value + strlen (value) - 1) == '0')
	    {
	      websDebugWrite (wp,
			      "Invalid <b>%s</b> %s: not an IP address<br>",
			      v->longname, value);
	      return FALSE;
	    }
	}

      else if (!legal_ip_netmask (v->argv[0], v->argv[1], value))
	{
	  (void) inet_aton (nvram_safe_get (v->argv[0]), &netaddr);
	  (void) inet_aton (nvram_safe_get (v->argv[1]), &netmask);
	  websDebugWrite (wp, "Invalid <b>%s</b> %s: not in the %s/",
			  v->longname, value, inet_ntoa (netaddr));
	  websDebugWrite (wp, "%s network<br>", inet_ntoa (netmask));
	  return FALSE;
	}
    }

  return TRUE;
}

static void
validate_ipaddr (webs_t wp, char *value, struct variable *v)
{
  if (valid_ipaddr (wp, value, v))
    nvram_set (v->name, value);
}

static void
validate_ipaddrs (webs_t wp, char *value, struct variable *v)
{
  validate_list (wp, value, v, valid_ipaddr);
}

int
valid_merge_ip_4 (webs_t wp, char *value, struct variable *v)
{
  char ipaddr[20];

  if (atoi (value) == 255)
    {
      websDebugWrite (wp, "Invalid <b>%s</b> %s: out of range 0 - 254 <br>",
		      v->longname, value);
      return FALSE;
    }

  sprintf (ipaddr, "%d.%d.%d.%s",
	   get_single_ip (nvram_safe_get ("lan_ipaddr"), 0),
	   get_single_ip (nvram_safe_get ("lan_ipaddr"), 1),
	   get_single_ip (nvram_safe_get ("lan_ipaddr"), 2), value);

  if (!valid_ipaddr (wp, ipaddr, v))
    {
      return FALSE;
    }


  return TRUE;
}

/*static void
validate_merge_ip_4 (webs_t wp, char *value, struct variable *v)
{
  if (!strcmp (value, ""))
    {
      nvram_set (v->name, "0");
      return;
    }

  if (valid_merge_ip_4 (wp, value, v))
    nvram_set (v->name, value);
}
*/
static void
validate_merge_ipaddrs (webs_t wp, char *value, struct variable *v)
{
  char ipaddr[20];

  get_merge_ipaddr (wp, v->name, ipaddr);

  if (valid_ipaddr (wp, ipaddr, v))
    nvram_set (v->name, ipaddr);
}

static void
validate_merge_mac (webs_t wp, char *value, struct variable *v)
{
  char macaddr[20];

  get_merge_mac (wp, v->name, macaddr);

  if (valid_hwaddr (wp, macaddr, v))
    nvram_set (v->name, macaddr);

}

static void
validate_dns (webs_t wp, char *value, struct variable *v)
{
  char buf[100] = "", *cur = buf;
  char ipaddr[20], ipname[30];
  char *ip;
  int i, j;

  for (j = 0; j < 3; j++)
    {
      strcpy (ipaddr, "");
      for (i = 0; i < 4; i++)
	{
	  snprintf (ipname, sizeof (ipname), "%s%d_%d", v->name, j, i);
	  ip = websGetVar (wp, ipname, NULL);
	  if (ip)
	    {
	      strcat (ipaddr, ip);
	      if (i < 3)
		strcat (ipaddr, ".");
	    }
	  else
	    return;
	}

      if (!strcmp (ipaddr, "0.0.0.0"))
	continue;
      if (!valid_ipaddr (wp, ipaddr, v))
	continue;
      cur += snprintf (cur, buf + sizeof (buf) - cur, "%s%s",
		       cur == buf ? "" : " ", ipaddr);
    }
  nvram_set (v->name, buf);

  dns_to_resolv ();
}

int
valid_choice (webs_t wp, char *value, struct variable *v)
{
  char **choice;

  for (choice = v->argv; *choice; choice++)
    {
      if (!strcmp (value, *choice))
	return TRUE;
    }

  websDebugWrite (wp, "Invalid <b>%s</b> %s: not one of ", v->longname,
		  value);
  for (choice = v->argv; *choice; choice++)
    websDebugWrite (wp, "%s%s", choice == v->argv ? "" : "/", *choice);
  websDebugWrite (wp, "<br>");
  return FALSE;
}

void
validate_choice (webs_t wp, char *value, struct variable *v)
{
  if (valid_choice (wp, value, v))
    nvram_set (v->name, value);
}

int
valid_range (webs_t wp, char *value, struct variable *v)
{
  int n, start, end;

  n = atoi (value);
  start = atoi (v->argv[0]);
  end = atoi (v->argv[1]);


  if (!ISDIGIT (value, 1) || n < start || n > end)
    {
      websDebugWrite (wp, "Invalid <b>%s</b> %s: out of range %d-%d<br>",
		      v->longname, value, start, end);
      return FALSE;
    }

  return TRUE;
}

static void
validate_range (webs_t wp, char *value, struct variable *v)
{
  char buf[20];
  int range;
  if (valid_range (wp, value, v))
    {
      range = atoi (value);
      snprintf (buf, sizeof (buf), "%d", range);
      nvram_set (v->name, buf);
    }
}

int
valid_name (webs_t wp, char *value, struct variable *v)
{
  int n, max;

  n = atoi (value);
  max = atoi (v->argv[0]);


  if (!ISASCII (value, 1))
    {
      websDebugWrite (wp,
		      "Invalid <b>%s</b> %s: NULL or have illegal characters<br>",
		      v->longname, value);
      return FALSE;
    }
  if (strlen (value) > max)
    {
      websDebugWrite (wp,
		      "Invalid <b>%s</b> %s: out of range 1-%d characters<br>",
		      v->longname, value, max);
      return FALSE;
    }

  return TRUE;
}

static void
validate_name (webs_t wp, char *value, struct variable *v)
{
  if (valid_name (wp, value, v))
    nvram_set (v->name, value);
}

int do_reboot = 0;
static void
validate_reboot (webs_t wp, char *value, struct variable *v)
{
  if (value && v)
    {
      nvram_set (v->name, value);
      do_reboot = 1;
    }
}

/* the html always show "d6nw5v1x2pc7st9m"
 * so we must filter it.
 */
static void
validate_password (webs_t wp, char *value, struct variable *v)
{
  if (strcmp (value, TMP_PASSWD) && valid_name (wp, value, v))
    {
      nvram_set (v->name, zencrypt(value));

      system ("/sbin/setpasswd");
    }
}

static void
validate_password2 (webs_t wp, char *value, struct variable *v)
{
  if (strcmp (value, TMP_PASSWD) && valid_name (wp, value, v))
    {
      nvram_set (v->name, value);

      system ("/sbin/setpasswd");
    }
}


int
valid_hwaddr (webs_t wp, char *value, struct variable *v)
{
  /* Make exception for "NOT IMPLELEMENTED" string */
  if (!strcmp (value, "NOT_IMPLEMENTED"))
    return (TRUE);

  /* Check for bad, multicast, broadcast, or null address */
  if (!legal_hwaddr (value))
    {
      websDebugWrite (wp, "Invalid <b>%s</b> %s: not a legal MAC address<br>",
		      v->longname, value);
      return FALSE;
    }

  return TRUE;
}

static void
validate_hwaddr (webs_t wp, char *value, struct variable *v)
{
  if (valid_hwaddr (wp, value, v))
    nvram_set (v->name, value);
}

static void
validate_hwaddrs (webs_t wp, char *value, struct variable *v)
{
  validate_list (wp, value, v, valid_hwaddr);
}

void
ej_get_http_prefix (int eid, webs_t wp, int argc, char_t ** argv)
{
  char http[10];
  char ipaddr[20];
  char port[10];

  char *http_enable = websGetVar (wp, "http_enable", NULL);
#ifdef HAVE_HTTPS
  char *https_enable = websGetVar (wp, "https_enable", NULL);

  if (do_ssl && http_enable == NULL && https_enable == NULL)
    {
      strcpy (http, "https");
    }
  else if (do_ssl && http_enable && https_enable)
    {
      if (atoi (https_enable) && atoi (http_enable))
	strcpy (http, "https");
      else if (atoi (https_enable) && !atoi (http_enable))
	strcpy (http, "https");
      else			// !atoi(https_enable) && atoi(http_enable)
	strcpy (http, "http");
    }
  else if (do_ssl && !http_enable && !https_enable)
    {
      strcpy (http, "http");
    }
  else if (!do_ssl && http_enable && https_enable)
    {
      if (atoi (https_enable) && atoi (http_enable))
	strcpy (http, "http");
      else if (atoi (https_enable) && !atoi (http_enable))
	strcpy (http, "https");
      else			// !atoi(https_enable) && atoi(http_enable)
	strcpy (http, "http");
    }
  else
#endif
    strcpy (http, "http");

  if (browser_method == USE_LAN)
    {				// Use LAN to browser
      if (nvram_match ("restore_defaults", "1")
	  || nvram_match ("sv_restore_defaults", "1"))
	{

	  if (getRouterBrand () != ROUTER_BUFFALO_WBR2G54S)	
	    strcpy (ipaddr, "192.168.1.1");	// Default IP
	  else
	    strcpy (ipaddr, "192.168.11.1");	// Default IP 

	  strcpy (http, "http");
	}
      else
	strcpy (ipaddr, nvram_safe_get ("lan_ipaddr"));
      strcpy (port, "");
    }
  else
    {

      if (nvram_match ("wan_proto", "pptp"))
	strcpy (ipaddr, nvram_safe_get ("pptp_get_ip"));
      else if (nvram_match ("wan_proto", "l2tp"))
	strcpy (ipaddr, nvram_safe_get ("l2tp_get_ip"));
      else
	strcpy (ipaddr, nvram_safe_get ("wan_ipaddr"));

      sprintf (port, ":%s", nvram_safe_get ("http_wanport"));
    }

  websWrite (wp, "%s://%s%s/", http, ipaddr, port);

  return;
}

void
ej_get_mtu (int eid, webs_t wp, int argc, char_t ** argv)
{
  struct mtu_lists *mtu_list;
  char *type;
  char *proto = GOZILA_GET ("wan_proto");

  if (ejArgs (argc, argv, "%s", &type) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  mtu_list = get_mtu (proto);

  if (!strcmp (type, "min"))
    websWrite (wp, "%s", mtu_list->min);
  else if (!strcmp (type, "max"))
    websWrite (wp, "%s", mtu_list->max);

  return;
}

/*
 * Variables are set in order (put dependent variables later). Set
 * nullok to TRUE to ignore zero-length values of the variable itself.
 * For more complicated validation that cannot be done in one pass or
 * depends on additional form components or can throw an error in a
 * unique painful way, write your own validation routine and assign it
 * to a hidden variable (e.g. filter_ip).
 */
/*
DD-WRT enhancement by seg
This functions parses all /etc/config/xxxxx.nvramconfig files and creates the
web var tab. so these vars arent defined anymore staticly
*/

#include <stdlib.h>
#include <malloc.h>
#include <dirent.h>
#include <stdlib.h>


int
endswith (char *str, char *cmp)
{
  int cmp_len, str_len, i;
  cmp_len = strlen (cmp);
  str_len = strlen (str);
  if (cmp_len > str_len)
    return (0);
  for (i = 0; i < cmp_len; i++)
    {
      if (str[(str_len - 1) - i] != cmp[(cmp_len - 1) - i])
	return (0);
    }
  return (1);
}

char *
toUP (char *a)
{
  int i;
  for (i = 0; i < strlen (a); i++)
    {
      if (a[i] > 'a' - 1 && a[i] < 'z' + 1)
	a[i] -= 'a' + 'A';
    }
  return a;
}

int
stricmp (char *a, char *b)
{
  if (strlen (a) != strlen (b))
    return -1;
  return strcmp (toUP (a), toUP (b));
}

void
StringStart (FILE * in)
{
  while (getc (in) != '"')
    {
      if (feof (in))
	return;
    }
}

char *
getFileString (FILE * in)
{
  char *buf;
  int i, b;
  buf = malloc (1024);
  StringStart (in);
  for (i = 0; i < 1024; i++)
    {
      b = getc (in);
      if (b == EOF)
	return NULL;
      if (b == '"')
	{
	  buf[i] = 0;
	  buf = realloc (buf, strlen (buf) + 1);
	  return buf;
	}
      buf[i] = b;
    }
  return buf;
}
static char *directories[] = {
  "/etc/config",
  "/jffs/config",
  "/mmc/config"
};

struct variable **variables;
void
Initnvramtab ()
{
  struct dirent *entry;
  DIR *directory;
  FILE *in;
  int varcount = 0, len, i;
  char *tmpstr;
  struct variable *tmp;
  variables = NULL;
  char buf[1024];
// format = VARNAME VARDESC VARVALID VARVALIDARGS FLAGS FLAGS
//open config directory directory =
  int idx;
  for (idx = 0; idx < 3; idx++)
    {
      directory = opendir (directories[idx]);
      if (directory == NULL)
	continue;
//list all files in this directory
      while ((entry = readdir (directory)) != NULL)
	{
	  if (endswith (entry->d_name, ".nvramconfig"))
	    {
	      sprintf (buf, "%s/%s", directories[idx], entry->d_name);
	      in = fopen (buf, "rb");
	      if (in == NULL)
		{
		  return;
		}
	      while (1)
		{
		  tmp = (struct variable *) malloc (sizeof (struct variable));
		  tmp->name = getFileString (in);
		  tmp->validate = NULL;
		  tmp->validate2 = NULL;
		  if (tmp->name == NULL)
		    break;
		  tmp->longname = getFileString (in);	//long string
		  tmpstr = getFileString (in);
		  tmp->argv = NULL;
		  if (!stricmp (tmpstr, "RANGE"))
		    {
		      tmp->validate = validate_range;
		      tmp->argv = (char **) malloc (sizeof (char **) * 3);
		      tmp->argv[0] = getFileString (in);
		      tmp->argv[1] = getFileString (in);
		      tmp->argv[2] = NULL;
		    }
		  if (!stricmp (tmpstr, "CHOICE"))
		    {
		      tmp->validate = validate_choice;
		      free (tmpstr);
		      tmpstr = getFileString (in);
		      len = atoi (tmpstr);
		      tmp->argv =
			(char **) malloc (sizeof (char **) * (len + 1));
		      for (i = 0; i < len; i++)
			{
			  tmp->argv[i] = getFileString (in);
			}
		      tmp->argv[i] = NULL;
		    }
#ifdef HAVE_SPUTNIK_APD
		  if (!stricmp (tmpstr, "MJIDTYPE"))
		    {
		      tmp->validate = validate_choice;
		      free (tmpstr);
		      tmpstr = getFileString (in);
		      len = atoi (tmpstr);
		      tmp->argv =
			(char **) malloc (sizeof (char **) * (len + 1));
		      for (i = 0; i < len; i++)
			{
			  tmp->argv[i] = getFileString (in);
			}
		      tmp->argv[i] = NULL;
		      nvram_set ("sputnik_rereg", "1");
		    }
#endif
		  if (!stricmp (tmpstr, "NOACK"))
		    {
		      tmp->validate = validate_noack;
		      len = 2;
		      tmp->argv =
			(char **) malloc (sizeof (char **) * (len + 1));
		      for (i = 0; i < len; i++)
			{
			  tmp->argv[i] = getFileString (in);
			}
		      tmp->argv[i] = NULL;
		    }
		  if (!stricmp (tmpstr, "NAME"))
		    {
		      tmp->validate = validate_name;
		      tmp->argv = (char **) malloc (sizeof (char **) * 2);
		      tmp->argv[0] = getFileString (in);
		      tmp->argv[1] = NULL;
		    }
		  if (!stricmp (tmpstr, "NULL"))
		    {
		      tmp->validate = NULL;
		      tmp->validate2 = NULL;
		    }
		  if (!stricmp (tmpstr, "WMEPARAM"))
		    {
		      tmp->validate = validate_wl_wme_params;
		    }
		  if (!stricmp (tmpstr, "PASSWORD"))
		    {
		      tmp->validate = validate_password;
		      tmp->argv = (char **) malloc (sizeof (char **) * 2);
		      tmp->argv[0] = getFileString (in);
		      tmp->argv[1] = NULL;
		    }
		  if (!stricmp (tmpstr, "PASSWORD2"))
		    {
		      tmp->validate = validate_password2;
		      tmp->argv = (char **) malloc (sizeof (char **) * 2);
		      tmp->argv[0] = getFileString (in);
		      tmp->argv[1] = NULL;
		    }
		  if (!stricmp (tmpstr, "LANIPADDR"))
		    {
		      tmp->validate = validate_lan_ipaddr;
		      tmp->argv = (char **) malloc (sizeof (char **) * 2);
		      tmp->argv[0] = getFileString (in);
		      tmp->argv[1] = NULL;
		    }
		  if (!stricmp (tmpstr, "WANIPADDR"))
		    {
		      tmp->validate = validate_wan_ipaddr;
		    }
		  if (!stricmp (tmpstr, "MERGEIPADDRS"))
		    {
		      tmp->validate = validate_merge_ipaddrs;
		    }
		  if (!stricmp (tmpstr, "DNS"))
		    {
		      tmp->validate = validate_dns;
		    }
		  if (!stricmp (tmpstr, "SAVEWDS"))
		    {
		      tmp->validate = NULL;
		      tmp->validate2 = save_wds;
		    }
		  if (!stricmp (tmpstr, "DHCP"))
		    {
		      tmp->validate = &dhcp_check;
		    }
		  if (!stricmp (tmpstr, "WPAPSK"))
		    {
		      tmp->validate = validate_wpa_psk;
		      tmp->argv = (char **) malloc (sizeof (char **) * 2);
		      tmp->argv[0] = getFileString (in);
		      tmp->argv[1] = NULL;
		    }
		  if (!stricmp (tmpstr, "STATICS"))
		    {
		      tmp->validate = validate_statics;
		    }
		  if (!stricmp (tmpstr, "REBOOT"))
		    {
		      tmp->validate = validate_reboot;
		    }
		  if (!stricmp (tmpstr, "IPADDR"))
		    {
		      tmp->validate = validate_ipaddr;
		    }
		  if (!stricmp (tmpstr, "STATICLEASES"))
		    {
		      tmp->validate = validate_staticleases;
		    }
#ifdef HAVE_CHILLILOCAL
		  if (!stricmp (tmpstr, "USERLIST"))
		    {
		      tmp->validate = validate_userlist;
		    }
#endif
#ifdef HAVE_RADLOCAL
		  if (!stricmp (tmpstr, "IRADIUSUSERLIST"))
		    {
		      tmp->validate = validate_iradius;
		    }
#endif
		  if (!stricmp (tmpstr, "IPADDRS"))
		    {
		      tmp->validate = validate_ipaddrs;
		    }
		  if (!stricmp (tmpstr, "NETMASK"))
		    {
		      tmp->validate = validate_netmask;
		    }
		  if (!stricmp (tmpstr, "MERGENETMASK"))
		    {
		      tmp->validate = validate_merge_netmask;
		    }
		  if (!stricmp (tmpstr, "WDS"))
		    {
		      tmp->validate = validate_wds;
		    }
		  if (!stricmp (tmpstr, "STATICROUTE"))
		    {
		      tmp->validate = validate_static_route;
		    }
		  if (!stricmp (tmpstr, "MERGEMAC"))
		    {
		      tmp->validate = validate_merge_mac;
		    }
		  if (!stricmp (tmpstr, "FILTERPOLICY"))
		    {
		      tmp->validate = validate_filter_policy;
		    }
		  if (!stricmp (tmpstr, "FILTERIPGRP"))
		    {
		      tmp->validate = validate_filter_ip_grp;
		    }
		  if (!stricmp (tmpstr, "FILTERPORT"))
		    {
		      tmp->validate = validate_filter_port;
		    }
		  if (!stricmp (tmpstr, "FILTERDPORTGRP"))
		    {
		      tmp->validate = validate_filter_dport_grp;
		    }
		  if (!stricmp (tmpstr, "BLOCKEDSERVICE"))
		    {
		      tmp->validate = validate_blocked_service;
		    }
		  if (!stricmp (tmpstr, "FILTERP2P"))
		    {
		      tmp->validate = validate_catchall;
		    }
		  if (!stricmp (tmpstr, "FILTERMACGRP"))
		    {
		      tmp->validate = validate_filter_mac_grp;
		    }
		  if (!stricmp (tmpstr, "FILTERWEB"))
		    {
		      tmp->validate = validate_filter_web;
		    }
		  if (!stricmp (tmpstr, "WLHWADDRS"))
		    {
		      tmp->validate = validate_wl_hwaddrs;
		    }
		  if (!stricmp (tmpstr, "FORWARDPROTO"))
		    {
		      tmp->validate = validate_forward_proto;
		    }
		  if (!stricmp (tmpstr, "FORWARDSPEC"))
		    {
		      tmp->validate = validate_forward_spec;
		    }
// changed by steve
		  /*if (!stricmp (tmpstr, "FORWARDUPNP"))
		     {
		     tmp->validate = validate_forward_upnp;
		     } */
// end changed by steve
		  if (!stricmp (tmpstr, "PORTTRIGGER"))
		    {
		      tmp->validate = validate_port_trigger;
		    }
		  if (!stricmp (tmpstr, "HWADDR"))
		    {
		      tmp->validate = validate_hwaddr;
		    }
		  if (!stricmp (tmpstr, "HWADDRS"))
		    {
		      tmp->validate = validate_hwaddrs;
		    }
		  if (!stricmp (tmpstr, "WLWEPKEY"))
		    {
		      tmp->validate = validate_wl_wep_key;
		    }
		  if (!stricmp (tmpstr, "MACMODE"))
		    {
		      tmp->validate = validate_macmode;
		    }

		  if (!stricmp (tmpstr, "WLAUTH"))
		    {
		      tmp->validate = validate_wl_auth;
		      tmp->argv = (char **) malloc (sizeof (char **) * 3);
		      tmp->argv[0] = getFileString (in);
		      tmp->argv[1] = getFileString (in);
		      tmp->argv[2] = NULL;
		    }
		  if (!stricmp (tmpstr, "WLWEP"))
		    {
		      tmp->validate = validate_wl_wep;
		      free (tmpstr);
		      tmpstr = getFileString (in);
		      len = atoi (tmpstr);
		      tmp->argv =
			(char **) malloc (sizeof (char **) * (len + 1));
		      for (i = 0; i < len; i++)
			{
			  tmp->argv[i] = getFileString (in);
			}
		      tmp->argv[i] = NULL;
		    }

		  if (!stricmp (tmpstr, "DYNAMICROUTE"))
		    {
		      tmp->validate = validate_dynamic_route;
		      free (tmpstr);
		      tmpstr = getFileString (in);
		      len = atoi (tmpstr);
		      tmp->argv =
			(char **) malloc (sizeof (char **) * (len + 1));
		      for (i = 0; i < len; i++)
			{
			  tmp->argv[i] = getFileString (in);
			}
		      tmp->argv[i] = NULL;
		    }
		  if (!stricmp (tmpstr, "WLGMODE"))
		    {
		      tmp->validate = validate_wl_gmode;
		      free (tmpstr);
		      tmpstr = getFileString (in);
		      len = atoi (tmpstr);
		      tmp->argv =
			(char **) malloc (sizeof (char **) * (len + 1));
		      for (i = 0; i < len; i++)
			{
			  tmp->argv[i] = getFileString (in);
			}
		      tmp->argv[i] = NULL;
		    }
		  if (!stricmp (tmpstr, "WLNETMODE"))
		    {
		      tmp->validate = validate_wl_net_mode;
		      free (tmpstr);
		      tmpstr = getFileString (in);
		      len = atoi (tmpstr);
		      tmp->argv =
			(char **) malloc (sizeof (char **) * (len + 1));
		      for (i = 0; i < len; i++)
			{
			  tmp->argv[i] = getFileString (in);
			}
		      tmp->argv[i] = NULL;
		    }
		  if (!stricmp (tmpstr, "AUTHMODE"))
		    {
		      tmp->validate = validate_auth_mode;
		      free (tmpstr);
		      tmpstr = getFileString (in);
		      len = atoi (tmpstr);
		      tmp->argv =
			(char **) malloc (sizeof (char **) * (len + 1));
		      for (i = 0; i < len; i++)
			{
			  tmp->argv[i] = getFileString (in);
			}
		      tmp->argv[i] = NULL;
		    }
#ifndef HAVE_MSSID
		  if (!stricmp (tmpstr, "SECURITYMODE"))
		    {
		      tmp->validate = validate_security_mode;
		      free (tmpstr);
		      tmpstr = getFileString (in);
		      len = atoi (tmpstr);
		      tmp->argv =
			(char **) malloc (sizeof (char **) * (len + 1));
		      for (i = 0; i < len; i++)
			{
			  tmp->argv[i] = getFileString (in);
			}
		      tmp->argv[i] = NULL;
		    }
#endif

		  free (tmpstr);
		  tmpstr = getFileString (in);
		  if (!stricmp (tmpstr, "TRUE"))
		    {
		      tmp->nullok = TRUE;
		    }
		  else
		    {
		      tmp->nullok = FALSE;
		    }
		  free (tmpstr);
		  tmpstr = getFileString (in);
		  tmp->ezc_flags = atoi (tmpstr);
		  free (tmpstr);
		  variables =
		    (struct variable **) realloc (variables,
						  sizeof (struct variable **)
						  * (varcount + 2));
		  variables[varcount++] = tmp;
		  variables[varcount] = NULL;
		}
	      fclose (in);
	    }
	}
      closedir (directory);
    }
}

/*
	{ "lan_ipaddr", "LAN IP Address", validate_lan_ipaddr, ARGV("lan"), FALSE },

	{ "router_name", "Routert Name", validate_name, ARGV("255"), TRUE, 0 },
	{ "wan_hostname","WAN Host Name", validate_name, ARGV("255"), TRUE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	{ "wan_domain", "WAN Domain Name", validate_name, ARGV("255"), TRUE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	{ "wan_ipaddr", "WAN IP Address", validate_wan_ipaddr, NULL, FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "wan_ipaddr", "WAN IP Address", validate_merge_ipaddrs, NULL, FALSE },
	//{ "wan_netmask", "WAN Subnet Mask", validate_merge_netmask, FALSE },
	//{ "wan_gateway", "WAN Gateway", validate_merge_ipaddrs, ARGV("wan_ipaddr","wan_netmask"), FALSE },
	{ "wan_proto", "WAN Protocol", validate_choice, ARGV("disabled", "dhcp", "static", "pppoe", "pptp", "l2tp", "heartbeat"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	{ "ntp_server", "NTP Server", NULL, NULL, TRUE, 0 },  // not use
	{ "ntp_mode", "NTP Mode", validate_choice, ARGV("manual","auto"), TRUE, 0 },
	{ "daylight_time", "Daylight", validate_choice, ARGV("0", "1"), TRUE, 0 },
	{ "time_zone", "Time Zone", validate_choice, ARGV("-12 1 0","-11 1 0","-10 1 0","-09 1 1","-08 1 1","-07 1 0","-07 2 1","-06 1 0","-06 2 1","-05 1 0","-05 2 1","-04 1 0","-04 2 1","-03.5 1 1","-03 1 0","-03 2 1","-02 1 0","-01 1 2","+00 1 0","+00 2 2","+01 1 0","+01 2 2","+02 1 0","+02 2 2","+03 1 0","+04 1 0","+05 1 0","+06 1 0","+07 1 0","+08 1 0","+08 2 0","+09 1 0","+10 1 0","+10 2 4","+11 1 0","+12 1 0","+12 2 4"), FALSE, 0 },
	//{ "pptp_server_ip", "WAN Gateway", validate_merge_ipaddrs, ARGV("wan_ipaddr","wan_netmask"), FALSE },
	{ "ppp_username", "Username", validate_name, ARGV("63"), FALSE, 0 },
	{ "ppp_passwd", "Password", validate_password, ARGV("63"), TRUE, 0 },
	{ "ppp_keepalive", "Keep Alive", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "ppp_demand", "Connect on Demand", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "ppp_idletime", "Max Idle Time", validate_range, ARGV("1", "9999"), FALSE, 0 },
	{ "ppp_redialperiod", "Redial Period", validate_range, ARGV("1", "9999"), FALSE, 0 },
	{ "ppp_service", "Service Name", validate_name, ARGV("63"), TRUE, 0 },	// 2003-03-19 by honor
	{ "ppp_static", "Enable /Disable Static IP", validate_choice, ARGV("0", "1"), TRUE, 0 },
	{ "ppp_static_ip", "Static IP", validate_merge_ipaddrs, NULL, FALSE, 0 },
	{ "wan_dns", "WAN DNS Server", validate_dns, NULL, FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	{ "lan_proto", "LAN Protocol", validate_choice, ARGV("dhcp", "static"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	{ "dhcp_check", "DHCP check", dhcp_check, NULL, FALSE, 0 },
	{ "dhcp_start", "DHCP Server LAN IP Address Range", validate_range, ARGV("0","255"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "dhcp_start", "DHCP Server LAN IP Address Range", validate_merge_ip_4, NULL, FALSE },
	{ "dhcp_num", "DHCP Users", validate_range, ARGV("1","253"), FALSE, 0 },
	{ "dhcp_lease", "DHCP Client Lease Time", validate_range, ARGV("0","99999"), FALSE, 0 },
	{ "wan_wins", "WAN WINS Server", validate_merge_ipaddrs, NULL, FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	{ "http_username", "Router Username", validate_name, ARGV("63"), TRUE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	{ "http_passwd", "Router Password", validate_password, ARGV("63"), TRUE, EZC_FLAGS_WRITE },
	{ "upnp_enable", "UPnP", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "web_wl_filter", "Wireless Access Web", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "http_enable", "HTTP Server", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "https_enable", "HTTPS Server", validate_choice, ARGV("0", "1"), FALSE, 0 },

	{ "samba_mount", "SambaFS Mount", validate_choice, ARGV("0", "1"), FALSE,0 },
	{ "samba_share", "SambaFS Share", NULL, NULL, FALSE,0 },
	{ "samba_user" , "SambaFS User" , NULL, NULL, FALSE,0 },
	{ "samba_password", "SambaFS Password", NULL,NULL, FALSE ,0},
	{ "samba_script", "SambaFS StartScript", NULL,NULL, FALSE,0 },
	{ "rflow_enable" , "RFLOW Enable" , validate_choice,ARGV("0","1"), FALSE,0 },
	{ "rflow_ip", "RFLOW IP", NULL,NULL, FALSE,0 },
	{ "rflow_port", "RFLOW PORT", NULL,NULL, FALSE,0 },
	{ "macupd_enable" , "MAC Update Enable" , validate_choice,ARGV("0","1"), FALSE,0 },
	{ "macupd_ip", "MAC Update IP", NULL,NULL, FALSE,0 },
	{ "macupd_port", "MAC update PORT", NULL,NULL, FALSE,0 },
	{ "macupd_interval", "MAC Update Interval", NULL,NULL, FALSE,0 },
	{ "status_auth","Status Site Authentication",NULL,NULL,FALSE,0},


	{ "rc_startup", "Startup Script", NULL, NULL, FALSE, 0 },
	{ "rc_firewall", "Firewall Script", NULL, NULL, FALSE, 0 },
	{ "lan_gateway", "LAN Gateway", validate_merge_ipaddrs, NULL, FALSE, 0 },
	{ "sv_localdns", "Local DNS", validate_merge_ipaddrs, NULL, FALSE, 0 },
	{ "lan_domain", "LAN Domain Name", validate_name, ARGV("255"), TRUE, 0 },
	{ "wl_mode", "Wireless Mode", validate_choice, ARGV("ap", "wet", "infra"), FALSE, 0 },
	{ "txpwr", "TX Power", validate_range, ARGV("0","251"),  FALSE, 0 },
	{ "txant", "TX Ant", validate_choice, ARGV("0","1","3"),  FALSE, 0 },
	{ "wl_antdiv", "RX Ant", validate_choice, ARGV("0","1","3"),  FALSE, 0 },
	{ "apwatchdog_enable", "AP Watchdog", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "apwatchdog_interval", "AP Watchdog", validate_range, ARGV("0", "86400"), FALSE, 0 },
	{ "boot_wait", "Boot Wait", validate_choice, ARGV("on", "off"), FALSE, 0 },
	{ "cron_enable", "Cron", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "dhcp_domain", "DHCP Domain", validate_choice, ARGV("wan", "lan"), FALSE, 0 },
	{ "dhcpd_statics", "DHCP", validate_statics, NULL, TRUE, 0 },
	{ "dhcpd_options", "DHCP", NULL, NULL, TRUE, 0 },
	{ "dnsmasq_enable", "DNS Masq", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "dnsmasq_options", "DNS Masq", NULL, NULL, TRUE, 0 },
	{ "httpd_enable", "Httpd", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "httpsd_enable", "Httpsd", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "loopback_enable", "Loopback", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "local_dns", "Local DNS", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "nas_enable", "NAS", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "ntp_enable", "NTP Client", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "pptpd_enable", "PPTPD", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "pptpd_lip", "PPTPD", NULL, NULL, FALSE, 0 },
	{ "pptpd_rip", "PPTPD", NULL, NULL, FALSE, 0 },
	{ "pptpd_auth", "PPTPD", NULL, NULL, FALSE, 0 },
	{ "pptp_encrypt", "PPTP", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "resetbutton_enable", "Resetbuttond", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "telnetd_enable", "Telnetd", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "sshd_enable", "SSHD", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "sshd_port", "SSHD", validate_range, ARGV("1", "65535"), FALSE, 0 },
	{ "sshd_passwd_auth", "SSHD", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "sshd_rsa_host_key", "SSHD", NULL, NULL, TRUE, 0 },
	{ "sshd_authorized_keys", "SSHD", NULL, NULL, TRUE , 0 },
	{ "syslogd_enable", "Syslog", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "syslogd_rem_ip", "Syslog", validate_ipaddr, NULL, TRUE, 0 },

	{ "wshaper_enable", "Bandwidth Mgmt", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "wshaper_dev", "Bandwidth Mgmt", validate_choice, ARGV("WAN", "LAN", "wLAN"), FALSE, 0 },
	{ "wshaper_downlink", "Bandwidth Mgmt", NULL, NULL, TRUE, 0 },
	{ "wshaper_uplink", "Bandwidth Mgmt", NULL, NULL, TRUE, 0 },
	{ "wshaper_nopriohostsrc", "Bandwidth Mgmt", NULL, NULL, TRUE, 0 },
	{ "wshaper_nopriohostdst", "Bandwidth Mgmt", NULL, NULL, TRUE, 0 },
	{ "wshaper_noprioportsrc", "Bandwidth Mgmt", NULL, NULL, TRUE, 0 },
	{ "wshaper_noprioportdst", "Bandwidth Mgmt", NULL, NULL, TRUE, 0 },
	{ "zebra_enable", "Zebra", validate_choice, ARGV("0", "1"), FALSE, 0 },


	// WDS vars
	{ "wl_wds1_enable","WDS separate bridge Enabled", save_wds, NULL, FALSE, 0 },
	{ "wl_wds1_hwaddr","WDS MAC", validate_wds, NULL, FALSE, 0 },
	{ "bird_ospf","Routing", NULL, NULL, TRUE, 0 },

	{ "snmpd_enable", "Snmpd", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "snmpd_syslocation", "Snmpd", NULL, NULL, TRUE, 0 },
	{ "snmpd_syscontact", "Snmpd", NULL, NULL, TRUE, 0 },
	{ "snmpd_sysname", "Snmpd", NULL, NULL, TRUE, 0 },
	{ "snmpd_rocommunity", "Snmpd", NULL, NULL, TRUE, 0 },
	{ "snmpd_rwcommunity", "Snmpd", NULL, NULL, TRUE, 0 },

	{ "wol_enable", "Wol", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "wol_interval", "Wol", validate_range, ARGV("1", "86400"), TRUE, 0 },
	{ "wol_hostname", "Wol", NULL, NULL, TRUE, 0 },
	{ "wol_macs", "Wol", NULL, NULL, TRUE, 0 },
	{ "wol_passwd", "Wol", NULL, NULL, TRUE, 0 },

	{ "chilli_enable", "Enable Chillispot", validate_choice, ARGV("0","1"), TRUE, 0 },
	{ "chilli_url", "Redirect URL", validate_name, ARGV("128"), TRUE, 0 },
	{ "chilli_radius1", "Primary Radius Server", validate_merge_ipaddrs, NULL, FALSE, 0 },
	{ "chilli_radius2", "Backup Radius Server", validate_merge_ipaddrs, NULL, FALSE, 0 },
	{ "chilli_pass", "Radius Password", validate_name, ARGV("128"), TRUE, 0 },
	{ "chilli_dns1", "Chillispot DNS1", validate_merge_ipaddrs, NULL, FALSE, 0 },

	{ "def_whwaddr", "User define wireless MAC Address", validate_merge_mac, NULL, TRUE, 0 },

	{ "log_dropped", "Access log D", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "log_rejected", "Access log R", validate_choice, ARGV("0", "1"), FALSE, 0 },
 	{ "log_accepted", "Access log A", validate_choice, ARGV("0", "1"), FALSE, 0 },

	{ "log_level", "Connection Logging", validate_range, ARGV("0", "3"), FALSE, 0 },
	{ "log_enable", "Access log", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "filter", "Firewall Protection", validate_choice, ARGV("on", "off"), FALSE, 0 },
	{ "filter_policy", "Filter", validate_filter_policy, NULL, FALSE, 0 },
	{ "filter_ip_value", "TCP/UDP IP Filter", validate_filter_ip_grp, NULL, FALSE, 0 },
	{ "filter_port", "TCP/UDP Port Filter", validate_filter_port, NULL, FALSE, 0 },
	{ "filter_dport_value", "TCP/UDP Port Filter", validate_filter_dport_grp, NULL, FALSE, 0 },
	{ "blocked_service", "TCP/UDP Port Filter", validate_blocked_service, NULL, FALSE, 0 },
	{ "filter_mac_value", "TCP/UDP MAC Filter", validate_filter_mac_grp, NULL, FALSE, 0 },
	{ "filter_web", "Website Filter", validate_filter_web, NULL, FALSE, 0 },
	{ "block_wan", "Block WAN Request", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "ident_pass", "IDENT passthrough", validate_choice, ARGV("0", "1"), TRUE, 0 },
	{ "block_loopback", "Filter Internet NAT redirection", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "block_proxy", "Block Proxy", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "block_java", "Block Java", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "block_activex", "Block ActiveX", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "block_cookie", "Block Cookie", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "multicast_pass", "Multicast Pass Through", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "ipsec_pass", "IPSec Pass Through", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "pptp_pass", "PPTP Pass Through", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "l2tp_pass", "L2TP Pass Through", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "remote_management", "Remote Management", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "remote_mgt_https", "Remote Management use https", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "http_wanport", "Router WAN Port", validate_range, ARGV("0", "65535"), TRUE, 0 },
	{ "remote_upgrade", "Remote Upgrade", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "mtu_enable", "MTU enable", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "wan_mtu", "WAN MTU", validate_range, ARGV("576","1500"), FALSE, 0 },
	{ "forward_port", "TCP/UDP Port Forward", validate_forward_proto, NULL, FALSE, 0 },
	{ "port_trigger", "TCP/UDP Port Trigger", validate_port_trigger, NULL, FALSE, 0 },
	{ "static_route", "Static Route", validate_static_route, NULL, FALSE, 0 },
	{ "wk_mode", "Working Mode", validate_dynamic_route, ARGV("gateway", "router", "ospf"), FALSE, 0 },
	//{ "dr_setting", "Dynamic Routing", validate_choice, ARGV("0", "1", "2", "3"), FALSE },
	//{ "dr_lan_tx", "Dynamic Routing LAN TX", validate_choice, ARGV("0","1 2"), FALSE },
	//{ "dr_lan_rx", "Dynamic Routing LAN RX", validate_choice, ARGV("0","1 2"), FALSE },
	//{ "dr_wan_tx", "Dynamic Routing WAN TX", validate_choice, ARGV("0","1 2"), FALSE },
	//{ "dr_wan_rx", "Dynamic Routing WAN RX", validate_choice, ARGV("0","1 2"), FALSE },
	{ "dmz_enable", "DMZ enable", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "dmz_ipaddr", "DMZ LAN IP Address", validate_range, ARGV("0","255"), FALSE, 0 },
	{ "mac_clone_enable", "User define WAN MAC Address", validate_choice, ARGV("0","1"), TRUE, 0 },
	{ "def_hwaddr", "User define WAN MAC Address", validate_merge_mac, NULL, TRUE, 0 },
	{ "upgrade_enable", "Tftp upgrade", validate_choice, ARGV("0", "1"), FALSE, 0 },
	{ "wl_enable", "Enable Wireless", validate_choice, ARGV("0","1"), TRUE, 0 },
	{ "wl_ssid", "Network Name (SSID)", validate_name, ARGV("32"), TRUE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
        { "wl_closed", "Network Type", validate_choice, ARGV("0", "1"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	{ "wl_country", "Country", validate_choice, ARGV("Worldwide", "Thailand", "Israel", "Jordan", "China", "Japan", "USA", "Europe", "USA Low", "Japan High", "All"), FALSE, 0 },
        { "wl_ap_isolate", "AP Isolate", validate_choice, ARGV("0", "1"), TRUE, 0 },
//        { "wl_mode", "AP Mode", validate_choice, ARGV("ap", "wet", "wds"), FALSE },
        { "wl_lazywds", "Bridge Restrict", validate_choice, ARGV("0", "1"), FALSE, 0 },
        { "wl_wds", "Remote Bridges", validate_hwaddrs, NULL, TRUE, 0 },
        { "wl_WEP_key", "Network Key Index", validate_wl_wep_key, NULL, FALSE, 0 },
        //{ "wl_passphrase", "Network Passphrase", validate_name, ARGV("20"), FALSE },
        //{ "wl_key", "Network Key Index", validate_range, ARGV("1","4"), FALSE },
        //{ "wl_key1", "Network Key 1", validate_wl_key, NULL, TRUE },
        //{ "wl_key2", "Network Key 2", validate_wl_key, NULL, TRUE },
        //{ "wl_key3", "Network Key 3", validate_wl_key, NULL, TRUE },
        //{ "wl_key4", "Network Key 4", validate_wl_key, NULL, TRUE },
        //{ "wl_wep_bit", "WEP Mode", validate_choice, ARGV("64", "128"), FALSE },
        { "wl_wep", "WEP Mode", validate_wl_wep, ARGV("off", "on", "restricted","tkip","aes","tkip+aes"), FALSE , EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	{ "wl_crypto", "Crypto Mode", validate_choice, ARGV("off","tkip","aes","tkip+aes"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
        { "wl_auth", "Authentication Mode", validate_wl_auth, ARGV("0", "1"), FALSE, 0 },
        { "wl_macmode1", "MAC Restrict Mode", validate_macmode, NULL , FALSE, 0 },
        //{ "wl_mac", "Allowed MAC Address", validate_hwaddrs, NULL, TRUE },
	{ "wl_radio", "Radio Enable", validate_choice, ARGV("0", "1"), FALSE, 0 }, //from 11.9
        { "wl_mac_list", "Filter MAC Address", validate_wl_hwaddrs, NULL, FALSE, 0 },
        //{ "wl_active_mac", "Active MAC Address", validate_wl_active_mac, NULL, FALSE },
        { "wl_channel", "802.11g Channel", validate_range, ARGV("0","14"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
        { "wl_rate", "802.11g Rate", validate_choice, ARGV("0", "1000000", "2000000", "5500000", "6000000", "9000000", "11000000", "12000000", "18000000", "24000000", "36000000", "48000000", "54000000"), FALSE, 0 },
        { "wl_rateset", "802.11g Supported Rates", validate_choice, ARGV("all", "default","12"), FALSE, 0 },
        { "wl_frag", "802.11g Fragmentation Threshold", validate_range, ARGV("256", "2346"), FALSE, 0 },
        { "wl_rts", "802.11g RTS Threshold", validate_range, ARGV("0", "2347"), FALSE, 0 },
        { "wl_dtim", "802.11g DTIM Period", validate_range, ARGV("1", "255"), FALSE, 0 },
        { "wl_bcn", "802.11g Beacon Interval", validate_range, ARGV("1", "65535"), FALSE, 0 },
        { "wl_gmode", "802.11g mode", validate_wl_gmode, ARGV("-1", "0", "1", "2", "4", "5"), FALSE, 0 },
        { "wl_net_mode", "802.11g mode", validate_wl_net_mode, ARGV("disabled", "mixed", "b-only", "g-only", "speedbooster"), FALSE, 0 },
	{ "wl_gmode_protection", "54g Protection", validate_choice, ARGV("off", "auto"), FALSE, 0 },
	{ "wl_frameburst", "Frame Bursting", validate_choice, ARGV("off", "on"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	{ "wl_plcphdr", "Preamble Type", validate_choice, ARGV("long", "short"), FALSE, 0 },
	{ "wl_phytype", "Radio Band", validate_choice, ARGV("a", "b", "g"), TRUE, 0 },
	{ "wl_wpa_psk", "WPA Pre-Shared Key", validate_wpa_psk, ARGV("64"), TRUE, EZC_FLAGS_WRITE },
	{ "wl_wpa_gtk_rekey", "WPA GTK Rekey Timer", validate_range, ARGV("0","99999"), TRUE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	{ "wl_radauth", "RADIUS Server ON", NULL, validate_choice, ARGV("0", "1"), FALSE ,0},
	{ "wl_radius_ipaddr", "RADIUS Server", validate_merge_ipaddrs, NULL, TRUE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	{ "wl_radius_port", "RADIUS Port", validate_range, ARGV("0", "65535"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	{ "wl_radius_key", "RADIUS Shared Secret", validate_name, ARGV("255"), TRUE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	{ "wl_auth_mode", "Auth Mode", validate_auth_mode, ARGV("disabled", "radius", "wpa", "psk"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	{ "security_mode", "Security Mode", validate_security_mode, ARGV("disabled", "radius", "wpa", "psk","wep"), FALSE, 0 },
	{ "wl_unit", "802.11 Instance", wl_unit, NULL, TRUE, 0 },
	{ "wl_ap_ssid", "SSID of associating AP", validate_name, ARGV("32"), TRUE, 0 },
	{ "wl_ap_ip", "Default IP of associating AP", validate_merge_ipaddrs, NULL, TRUE, 0 },

	//{ "ddns_enable", "DDNS", validate_choice, ARGV("0", "1"), FALSE },
	//{ "ddns_username", "DDNS username", validate_name, ARGV("63"), FALSE },
	//{ "ddns_passwd", "DDNS password", validate_password, ARGV("63"), FALSE },
	//{ "ddns_hostname", "DDNS hostname", validate_name, ARGV("255"), TRUE },
        //{ "ddns_server", "DDNS server", validate_choice,ARGV("ath.cx","dnsalias.com","dnsalias.net","dnsalias.org","dyndns.biz","dyndns.info","dyndns.org","dyndns.tv","gotdns.com","gotdns.org","homedns.org","homeftp.net","homeftp.org","homeip.net","homelinux.com","homelinux.net","homelinux.org","homeunix.com","homeunix.net","homeunix.org","kicks-ass.net","kicks-ass.org","merseine.nu","mine.nu","serveftp.net"), FALSE },
	{ "l2tp_server_ip", "L2TP Server", validate_merge_ipaddrs, NULL, FALSE, 0 },
	//{ "hb_server_ip", "Heart Beat Server", validate_merge_ipaddrs, NULL, FALSE },		//by tallest
	{ "hb_server_ip", "Heart Beat Server", validate_name, ARGV("63"), TRUE, 0 },
	{ "hb_server_ip", "Heart Beat Server", validate_merge_ipaddrs, NULL, FALSE, 0 },
	{ "os_server", "OS Server", NULL, NULL, TRUE, 0 },
	{ "stats_server", "Stats Server", NULL, NULL, TRUE, 0 },
	 EZC_SUPPORT
	{ "ezc_enable", "EZConfig", validate_choice, ARGV("0", "1"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE},
//#endif
	//{ "fw_disable", "Firewall", validate_choice, ARGV("0", "1"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "lan_stp", "Spanning Tree Protocol", validate_choice, ARGV("0", "1"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "lan_lease", "DHCP Server Lease Time", validate_range, ARGV("1", "604800"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "wan_desc", "Description", validate_name, ARGV("0", "255"), TRUE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "wan_hwaddr", "MAC Address", validate_hwaddr, NULL, TRUE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "wan_netmask", "Subnet Mask", validate_ipaddr, NULL, FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "wan_gateway", "Default Gateway", validate_ipaddr, NULL, TRUE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "wan_pppoe_username", "PPPoE Username", validate_name, ARGV("0", "255"), TRUE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "wan_pppoe_passwd", "PPPoE Password", validate_name, ARGV("0", "255"), TRUE, EZC_FLAGS_WRITE },
	//{ "wan_pppoe_service", "PPPoE Service Name", validate_name, ARGV("0", "255"), TRUE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "wan_pppoe_ac", "PPPoE Access Concentrator", validate_name, ARGV("0", "255"), TRUE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "wan_pppoe_keepalive", "PPPoE Keep Alive", validate_choice, ARGV("0", "1"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "wan_pppoe_demand", "PPPoE Connect on Demand", validate_choice, ARGV("0", "1"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "wan_pppoe_idletime", "PPPoE Max Idle Time", validate_range, ARGV("1", "3600"), TRUE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "wan_pppoe_mru", "PPPoE MRU", validate_range, ARGV("128", "16384"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "wan_pppoe_mtu", "PPPoE MTU", validate_range, ARGV("128", "16384"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "wl_country_code", "Country Code", validate_country, NULL, FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	{ "wl_afterburner", "AfterBurner Technology", validate_choice, ARGV("on", "off", "auto"), FALSE, EZC_FLAGS_READ | EZC_FLAGS_WRITE },
	//{ "wl_key", "Network Key Index", validate_range, ARGV("1", "4"), FALSE, EZC_FLAGS_WRITE },
	//{ "wl_key1", "Network Key 1", validate_wl_key, NULL, TRUE, EZC_FLAGS_WRITE},
	//{ "wl_key2", "Network Key 2", validate_wl_key, NULL, TRUE, EZC_FLAGS_WRITE},
	//{ "wl_key3", "Network Key 3", validate_wl_key, NULL, TRUE, EZC_FLAGS_WRITE},
	//{ "wl_key4", "Network Key 4", validate_wl_key, NULL, TRUE, EZC_FLAGS_WRITE},

};
*/

#ifdef HAVE_MACBIND
#include "../../../opt/mac.h"
#endif
//Added by Daniel(2004-07-29) for EZC
int
variables_arraysize (void)
{
  int varcount = 0;
  if (variables == NULL)
    return 0;
  while (variables[varcount] != NULL)
    {
      varcount++;
    }
//      return ARRAYSIZE(variables);
  return varcount;
}

static void
validate_cgi (webs_t wp)
{
  char *value;
  int i;
//int count=0;
//int argcount=0;
//struct variable *var;
//FILE *out;
/*
Initnvramtab();
while(variables[count]!=NULL)
{
fprintf(out,"%s %s %d %d ARGS:",variables[count]->name,variables[count]->longname,variables[count]->nullok,variables[count]->ezc_flags);
argcount=0;
var=variables[count];
if (var->argv!=NULL)
 {
 while (var->argv[argcount]!=NULL)
    {
    fprintf(out,"%s;",var->argv[argcount]);
    argcount++;
    }
    fprintf(out,"\n");
 }
count++;
}

fclose(out);
*/
//out=fopen("/tmp/dump.nv","wb");
//fprintf(out,"getting array_size\n");
//fflush(out);
#ifdef HAVE_MACBIND
  if (!nvram_match ("et0macaddr", MACBRAND))
    return;
#endif
  for (i = 0; i < variables_arraysize (); i++)
    {
      if (variables[i] == NULL)
	return;
//              fprintf(out,"getting websgetvar %s\n",variables[i]->name);
//              fflush(out);
      cprintf ("get %s\n", variables[i]->name);
      value = websGetVar (wp, variables[i]->name, NULL);
      if (!value)
	continue;
      cprintf ("validate %s\n", value);

//              fprintf(out,"name: %s value: %s\n",variables[i]->name,value);
//              fflush(out);
      if ((!*value && variables[i]->nullok)
	  || (!variables[i]->validate && !variables[i]->validate2))
	nvram_set (variables[i]->name, value);
      else
	{
	  if (variables[i]->validate)
	    {
	      cprintf (" use primary validator \n");
	      variables[i]->validate (wp, value, variables[i]);
	    }
	  else
	    {
	      cprintf (" use secondary validator \n");
	      variables[i]->validate2 (wp);
	    }
	}
      cprintf ("done()");



//      if ((!*value && variables[i]->nullok) || !variables[i]->validate)
//      nvram_set (variables[i]->name, value);
//      else
//      variables[i]->validate (wp, value, variables[i]);
//              fprintf(out,"okay\n");
//              fflush(out);
    }
  cprintf ("all vars validated\n");
//    fclose(out);
}


#ifdef HAVE_CCONTROL

static int execute (webs_t wp);
{
  char command[256];
  char *var = websGetVar (wp, "command", "");
  sprintf (command, "%s > /tmp/.result");
  system (command);
}

#endif

enum
{
  NOTHING,
  REBOOT,
  RESTART,
  SERVICE_RESTART,
  SYS_RESTART,
  REFRESH,
};

struct gozila_action gozila_actions[] = {
  /* bellow for setup page */
  {"index", "wan_proto", "", 1, REFRESH, wan_proto},
  {"index", "dhcpfwd", "", 1, REFRESH, dhcpfwd},
  {"index", "clone_mac", "", 1, REFRESH, clone_mac},
#ifdef HAVE_CCONTROL
  {"ccontrol", "execute", "", 1, REFRESH, execute},
#endif
  {"WanMAC", "clone_mac", "", 1, REFRESH, clone_mac},	// for cisco style
  {"DHCPTable", "delete", "", 2, REFRESH, delete_leases},
  {"Status", "release", "dhcp_release", 0, SYS_RESTART, dhcp_release},
  {"Status", "renew", "", 3, REFRESH, dhcp_renew},
  {"Status", "Connect", "start_pppoe", 1, RESTART, NULL},
  {"Status_Router", "release", "dhcp_release", 0, SYS_RESTART, dhcp_release},	// for cisco style
  {"Status_Router", "renew", "", 3, REFRESH, dhcp_renew},	// for cisco style
  {"Status", "Disconnect", "stop_pppoe", 2, SYS_RESTART, stop_ppp},
  {"Status", "Connect_pppoe", "start_pppoe", 1, RESTART, NULL},
  {"Status", "Disconnect_pppoe", "stop_pppoe", 2, SYS_RESTART, stop_ppp},
  {"Status", "Connect_pptp", "start_pptp", 1, RESTART, NULL},
  {"Status", "Disconnect_pptp", "stop_pptp", 2, SYS_RESTART, stop_ppp},
  {"Status", "Connect_heartbeat", "start_heartbeat", 1, RESTART, NULL},
  {"Status", "Disconnect_heartbeat", "stop_heartbeat", 2, SYS_RESTART,
   stop_ppp},
  {"Status_Router", "Disconnect", "stop_pppoe", 2, SYS_RESTART, stop_ppp},	// for cisco style
  {"Status_Router", "Connect_pppoe", "start_pppoe", 1, RESTART, NULL},	// for cisco style
  {"Status_Router", "Disconnect_pppoe", "stop_pppoe", 2, SYS_RESTART, stop_ppp},	// for cisco style
  {"Status_Router", "Connect_pptp", "start_pptp", 1, RESTART, NULL},	// for cisco style
  {"Status_Router", "Disconnect_pptp", "stop_pptp", 2, SYS_RESTART, stop_ppp},	// for cisco style
  {"Status_Router", "Connect_l2tp", "start_l2tp", 1, RESTART, NULL},	// for cisco style
  {"Status_Router", "Disconnect_l2tp", "stop_l2tp", 2, SYS_RESTART, stop_ppp},	// for cisco style{ "Status_Router",    "Connect_heartbeat",    "start_heartbeat",      1,      RESTART,                NULL},  // for cisco style
  {"Status_Router", "Disconnect_heartbeat", "stop_heartbeat", 2, SYS_RESTART, stop_ppp},	// for cisco style
  {"Filters", "save", "filters", 1, SYS_RESTART, save_policy},
  {"Filters", "delete", "filters", 1, SYS_RESTART, single_delete_policy},
  {"FilterSummary", "delete", "filters", 1, SYS_RESTART,
   summary_delete_policy},
  {"Routing", "del", "static_route_del", 1, SYS_RESTART, delete_static_route},
  {"RouteStatic", "del", "static_route_del", 1, SYS_RESTART,
   delete_static_route},
  {"WL_WEPTable", "key_64", "", 1, REFRESH, generate_key_64},
  {"WL_WEPTable", "key_128", "", 1, REFRESH, generate_key_128},
  {"WL_WPATable", "key_64", "", 1, REFRESH, generate_key_64},
  {"WL_WPATable", "key_128", "", 1, REFRESH, generate_key_128},
  {"WL_WPATable", "security", "", 1, REFRESH, set_security},
#ifdef HAVE_MSSID
  {"WL_WPATable", "save", "", 1, RESTART, security_save},
  {"WL_WPATable", "keysize", "", 1, REFRESH, security_save},
#endif
  {"WL_ActiveTable", "add_mac", "", 1, REFRESH, add_active_mac},
  /* Siafu addition */
  {"Ping", "wol", "", 1, REFRESH, ping_wol},
  /* Sveasoft addition */
  {"Wireless_WDS", "save", "", 0, REFRESH, save_wds},
  {"Ping", "startup", "", 1, SYS_RESTART, ping_startup},
  {"Ping", "firewall", "", 1, SYS_RESTART, ping_firewall},
  {"QoS", "add_svc", "", 1, REFRESH, qos_add_svc},
  {"QoS", "add_ip", "", 1, REFRESH, qos_add_ip},
  {"QoS", "add_mac", "", 1, REFRESH, qos_add_mac},
  {"QoS", "save", "filters", 1, SYS_RESTART, qos_save},
  /* end Sveasoft addition */
  {"Forward", "add_forward", "", 1, REFRESH, forward_add},
  {"Forward", "remove_forward", "", 1, REFRESH, forward_remove},
#ifdef HAVE_MSSID
  {"Wireless_Basic", "add_vifs", "", 1, REFRESH, add_vifs},
  {"Wireless_Basic", "remove_vifs", "", 1, REFRESH, remove_vifs},
#endif
  {"Wireless_Basic", "save", "", 1, RESTART, wireless_save},
  {"Services", "add_lease", "", 1, REFRESH, lease_add},
  {"Services", "remove_lease", "", 1, REFRESH, lease_remove},
#ifdef HAVE_CHILLILOCAL
  {"Hotspot", "add_user", "", 1, REFRESH, user_add},
  {"Hotspot", "remove_user", "", 1, REFRESH, user_remove},
#endif
#ifdef HAVE_RADLOCAL
  {"Hotspot", "add_iradius", "", 1, REFRESH, raduser_add},
#endif
  {"ForwardSpec", "add_forward_spec", "", 1, REFRESH, forwardspec_add},
  {"ForwardSpec", "remove_forward_spec", "", 1, REFRESH, forwardspec_remove},
  {"Triggering", "add_trigger", "", 1, REFRESH, trigger_add},
  {"Triggering", "remove_trigger", "", 1, REFRESH, trigger_remove},
  {"Port_Services", "save_services", "filters", 2, SYS_RESTART,
   save_services_port},
  {"QOSPort_Services", "save_qosservices", "filters", 2, SYS_RESTART,
   save_services_port},
  {"Ping", "start", "start_ping", 1, SERVICE_RESTART, diag_ping_start},
  {"Ping", "stop", "", 0, REFRESH, diag_ping_stop},
  {"Ping", "clear", "", 0, REFRESH, diag_ping_clear},
//  {"Traceroute", "start", "start_traceroute", 1, SYS_RESTART, diag_traceroute_start},
//  {"Traceroute", "stop", "", 0, REFRESH, diag_traceroute_stop},
//  {"Traceroute", "clear", "", 0, REFRESH, diag_traceroute_clear},
};

struct gozila_action *
handle_gozila_action (char *name, char *type)
{
  struct gozila_action *v;

  if (!name || !type)
    return NULL;

  for (v = gozila_actions; v < &gozila_actions[STRUCT_LEN (gozila_actions)];
       v++)
    {
      if (!strcmp (v->name, name) && !strcmp (v->type, type))
	{
	  return v;
	}
    }
  return NULL;
}

char my_next_page[30] = "";
int
gozila_cgi (webs_t wp, char_t * urlPrefix, char_t * webDir, int arg,
	    char_t * url, char_t * path, char_t * query)
{
  char *submit_button, *submit_type, *next_page;
  int action = REFRESH;
  int sleep_time;
  struct gozila_action *act;
  int ret;

  gozila_action = 1;
  my_next_page[0] = '\0';

  submit_button = websGetVar (wp, "submit_button", NULL);	/* every html must have the name */
  submit_type = websGetVar (wp, "submit_type", NULL);	/* add, del, renew, release ..... */

  nvram_set ("action_service", "");
  nvram_set ("action_service_arg1", "");

  cprintf ("submit_button=[%s] submit_type=[%s]\n", submit_button,
	   submit_type);
  act = handle_gozila_action (submit_button, submit_type);

  if (act)
    {
      cprintf ("name=[%s] type=[%s] service=[%s] sleep=[%d] action=[%d]\n",
	       act->name, act->type, act->service, act->sleep_time,
	       act->action);
      nvram_set ("action_service", act->service);
      sleep_time = act->sleep_time;
      action = act->action;
      if (act->go)
	ret = act->go (wp);
    }
  else
    {
      sleep_time = 0;
      action = REFRESH;
    }

  if (action == REFRESH)
    sleep (sleep_time);
  else if (action == SERVICE_RESTART)
    {
      sys_commit ();
      service_restart ();
      sleep (sleep_time);
    }
  else if (action == SYS_RESTART)
    {
      sys_commit ();
      sys_restart ();
    }
  else if (action == RESTART)
    {
      sys_commit ();
      sys_restart ();
    }
  if (my_next_page[0] != '\0')
    {
      sprintf (path, "%s", my_next_page);
    }
  else
    {
      next_page = websGetVar (wp, "next_page", NULL);
      if (next_page)
	sprintf (path, "%s", next_page);
      else
	sprintf (path, "%s.asp", submit_button);
    }
  cprintf ("refresh to %s\n", path);
  do_ej (path, wp);		//refresh
  websDone (wp, 200);

  gozila_action = 0;		//reset gozila_action
  generate_key = 0;
  clone_wan_mac = 0;

  return 1;
}

struct apply_action apply_actions[] = {
  /* bellow for setup page */
//#ifdef OEM == LINKSYS
  {"index", "index", 0, RESTART, NULL},
//#else
  {"OnePage", "", 0, RESTART, NULL},	// same as index
  {"Expose", "filters", 0, SYS_RESTART, NULL},	// same as DMZ
  {"VServer", "forward", 0, SERVICE_RESTART, NULL},	// same as Forward
#ifdef HAVE_UPNP
  {"UPnP", "forward_upnp", 0, SERVICE_RESTART, tf_upnp},	// upnp added
#endif
//#endif
  {"Security", "", 1, RESTART, NULL},
  {"System", "", 0, RESTART, NULL},
  {"DHCP", "dhcp", 0, SYS_RESTART, NULL},
  {"WL_WEPTable", "", 0, RESTART, NULL},
  {"WL_WPATable", "wireless", 0, SYS_RESTART, NULL},
  /* bellow for advanced page */
  {"DMZ", "filters", 0, SYS_RESTART, NULL},	// for cisco style
  {"Filters", "filters", 0, SYS_RESTART, NULL},
  {"FilterIPMAC", "filters", 0, SYS_RESTART, NULL},
  {"FilterIP", "filters", 0, SYS_RESTART, NULL},
  {"FilterMAC", "filters", 0, SYS_RESTART, NULL},
  {"FilterPort", "filters", 0, SYS_RESTART, NULL},
  {"VPN", "filters", 0, SYS_RESTART, NULL},	// for cisco style
  {"Firewall", "filters", 0, SYS_RESTART, NULL},	// for cisco style
  {"Forward", "forward", 0, SERVICE_RESTART, NULL},
  {"ForwardSpec", "forward", 0, SERVICE_RESTART, NULL},
  {"Routing", "", 0, RESTART, NULL},
  {"DDNS", "ddns", 4, SYS_RESTART, ddns_save_value},
  /* Sveasoft additions */
  {"Management", "management", 4, SYS_RESTART, NULL},
  {"Alive", "alive", 4, SYS_RESTART, NULL},
  {"Hotspot", "hotspot", 4, SYS_RESTART, NULL},
  {"Services", "services", 4, SYS_RESTART, NULL},
  {"Triggering", "filters", 0, SERVICE_RESTART, NULL},
  {"Wireless_WDS", "", 4, RESTART, NULL},
  {"QoS", "filters", 0, SYS_RESTART, NULL},
  {"Log", "logging", 0, SERVICE_RESTART, NULL},
  /* end Sveasoft additions */
  {"Wireless", "wireless", 0, SYS_RESTART, NULL},
  {"Wireless_Basic", "wireless", 0, RESTART, NULL},
  {"Wireless_Advanced", "wireless", 0, SYS_RESTART, NULL},
  {"Wireless_MAC", "wireless", 0, SYS_RESTART, NULL},
  {"WL_FilterTable", "macfilter", 0, SYS_RESTART, NULL},

  /* begin lonewolf additions */
  {"Vlan", "", 0, SYS_RESTART, port_vlan_table_save},
  /* end lonewolf additions */
};

struct apply_action *
handle_apply_action (char *name)
{
  struct apply_action *v;
  cprintf ("apply name = \n", name);
  if (!name)
    return NULL;

  for (v = apply_actions; v < &apply_actions[STRUCT_LEN (apply_actions)]; v++)
    {
      if (!strcmp (v->name, name))
	{
	  return v;
	}
    }
  return NULL;
}

int
getFileLen (FILE * in)
{
  int len;
  fseek (in, 0, SEEK_END);
  len = ftell (in);
  rewind (in);
  return len;
}

/*
              <TR align=middle>
                <TD width=76 height=30><FONT size=2><INPUT  maxlength=12 size=7 name=name12 onblur=valid_name(this,"Name") value='<% port_forward_table("name","12"); %>' class=num></FONT></TD>
                <TD width=70 height=30 valign=middle valign=middle><FONT face="Arial, Helvetica, sans-serif"><INPUT  maxlength=5 size=5 value='<% port_forward_table("from","12"); %>' name=from12 onblur=valid_range(this,1,65535,"Port") class=num><span >&nbsp;</span></FONT></TD>
                <TD width=58 height=30><INPUT  maxlength=5 size=5 value='<% port_forward_table("to","12"); %>' name=to12 onblur=valid_range(this,1,65535,"Port") class=num></TD>
                <TD align=middle width=78 height=30><FONT face=Arial color=blue>
			<SELECT size=1 name=pro12>
				<OPTION value=tcp <% port_forward_table("sel_tcp","12"); %>>TCP</OPTION>
				<OPTION value=udp <% port_forward_table("sel_udp","12"); %>>UDP</OPTION>
				<OPTION value=both <% port_forward_table("sel_both","12"); %>>Both</OPTION>
			</SELECT></FONT></TD>
                <TD width=96 height=30><FONT style="FONT-SIZE: 8pt" face=Arial><INPUT  maxlength=15 size=11 value='<% port_forward_table("ip","12"); %>' name=ip12  class=num></FONT></TD>
                <TD width=47 height=30><INPUT type=checkbox value=on name=enable12 <% port_forward_table("enable","12"); %>></TD></TR>

*/
#define FWSHOW2(a,b,c) sprintf(buffer,a,b,c); do_ej_buffer(buffer,wp);
#define FWSHOW1(a,b) sprintf(buffer,a,b); do_ej_buffer(buffer,wp);

static void
ej_show_forward (int eid, webs_t wp, int argc, char_t ** argv)
//ej_show_forward(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg,
//        char_t *url, char_t *path, char_t *query)
{
  int i;
  char buffer[1024], *count;
  int c = 0;
  count = nvram_safe_get ("forward_entries");
  if (count == NULL || strlen (count) == 0)
    {
      //return -1;      botho 07/03/06 add "- None -" if empty
      //websWrite (wp, "<tr></tr><tr></tr>\n");
      websWrite (wp, "<tr>\n");
      websWrite (wp,
		 "<td colspan=\"6\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n");
      websWrite (wp, "</tr>\n");
    }
  c = atoi (count);
  if (c <= 0)
    {
      //return -1;      botho 07/03/06 add "- None -" if empty
      //websWrite (wp, "<tr></tr><tr></tr>\n");
      websWrite (wp, "<tr>\n");
      websWrite (wp,
		 "<td colspan=\"6\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n");
      websWrite (wp, "</tr>\n");
    }
  for (i = 0; i < c; i++)
    {
      websWrite (wp, "<tr><td>\n");
      FWSHOW1
	("<input maxlength=\"12\" size=\"12\" name=\"name%d\" onblur=\"valid_name(this,'Name')\" value=\"",
	 i);
      port_forward_table (wp, "name", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      FWSHOW1
	("<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"from%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
	 i);
      port_forward_table (wp, "from", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      FWSHOW1
	("<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"to%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
	 i);
      port_forward_table (wp, "to", i);
      websWrite (wp, "\"/></td>\n");
      FWSHOW1 ("<td><select size=\"1\" name=\"pro%d\">\n", i);
      websWrite (wp, "<option value=\"tcp\" ");
      port_forward_table (wp, "sel_tcp", i);
      websWrite (wp, ">TCP</option>\n");
      websWrite (wp, "<option value=\"udp\" ");
      port_forward_table (wp, "sel_udp", i);
      websWrite (wp, ">UDP</option>\n");
      websWrite (wp,
		 "<script type=\"text/javascript\">document.write(\"<option value=\\\"both\\\" ");
      port_forward_table (wp, "sel_both", i);
      websWrite (wp, " >\" + share.both + \"</option>\");</script>\n");
      websWrite (wp, "</select></td>\n");
      websWrite (wp, "<td>\n");
      FWSHOW1
	("<input class=\"num\" maxlength=\"15\" size=\"15\" name=\"ip%d\" value=\"",
	 i);
      port_forward_table (wp, "ip", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      FWSHOW1 ("<input type=\"checkbox\" value=\"on\" name=\"enable%d\" ", i);
      port_forward_table (wp, "enable", i);
      websWrite (wp, " /></td>\n");
      websWrite (wp, "</tr>\n");
    }
  return;
}

static void
ej_show_forward_spec (int eid, webs_t wp, int argc, char_t ** argv)
//ej_show_forward(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg,
//        char_t *url, char_t *path, char_t *query)
{
  int i;
  char buffer[1024], *count;
  int c = 0;
  count = nvram_safe_get ("forwardspec_entries");
  if (count == NULL || strlen (count) == 0)
    {
      //return -1;      botho 07/03/06 add "- None -" if empty
      //websWrite (wp, "<tr></tr><tr></tr>\n");
      websWrite (wp, "<tr>\n");
      websWrite (wp,
		 "<td colspan=\"6\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n");
      websWrite (wp, "</tr>\n");
    }
  c = atoi (count);
  if (c <= 0)
    {
      //return -1;      botho 07/03/06 add "- None -" if empty
      //websWrite (wp, "<tr></tr><tr></tr>\n");
      websWrite (wp, "<tr>\n");
      websWrite (wp,
		 "<td colspan=\"6\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n");
      websWrite (wp, "</tr>\n");
    }
  for (i = 0; i < c; i++)
    {
      websWrite (wp, "<tr><td>\n");
      FWSHOW1
	("<input maxlength=\"12\" size=\"12\" name=\"name%d\" onblur=\"valid_name(this,'Name')\" value=\"",
	 i);
      port_forward_spec (wp, "name", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      FWSHOW1
	("<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"from%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
	 i);
      port_forward_spec (wp, "from", i);
      websWrite (wp, "\" /></td>\n");
      FWSHOW1 ("<td><select size=\"1\" name=\"pro%d\">\n", i);
      websWrite (wp, "<option value=\"tcp\" ");
      port_forward_spec (wp, "sel_tcp", i);
      websWrite (wp, ">TCP</option>\n");
      websWrite (wp, "<option value=\"udp\" ");
      port_forward_spec (wp, "sel_udp", i);
      websWrite (wp, ">UDP</option>\n");
      websWrite (wp,
		 "<script type=\"text/javascript\">document.write(\"<option value=\\\"both\\\" ");
      port_forward_spec (wp, "sel_both", i);
      websWrite (wp, " >\" + share.both + \"</option>\");</script>\n");
      websWrite (wp, "</select></td>\n");
      websWrite (wp, "<td>\n");
      FWSHOW1
	("<input class=\"num\" maxlength=\"15\" size=\"15\" name=\"ip%d\" value=\"",
	 i);
      port_forward_spec (wp, "ip", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      FWSHOW1
	("<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"to%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
	 i);
      port_forward_spec (wp, "to", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      FWSHOW1 ("<input type=\"checkbox\" value=\"on\" name=\"enable%d\" ", i);
      port_forward_spec (wp, "enable", i);
      websWrite (wp, " /></td>\n");
      websWrite (wp, "</tr>\n");
    }
  return;
}

static void
ej_show_triggering (int eid, webs_t wp, int argc, char_t ** argv)
{
  int i;
  char *count;
  char buffer[1024];
  int c = 0;
  count = nvram_safe_get ("trigger_entries");
  if (count == NULL || strlen (count) == 0)
    {
      //return -1;      botho 04/03/06 add "- None -" if empty
      //websWrite (wp, "<tr></tr><tr></tr>\n");
      websWrite (wp, "<tr>\n");
      websWrite (wp,
		 "<td colspan=\"6\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n");
      websWrite (wp, "</tr>\n");
    }
  c = atoi (count);
  if (c <= 0)
    {
      //return -1;      botho 07/03/06 add "- None -" if empty
      //websWrite (wp, "<tr></tr><tr></tr>\n");
      websWrite (wp, "<tr>\n");
      websWrite (wp,
		 "<td colspan=\"6\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n");
      websWrite (wp, "</tr>\n");
    }
  for (i = 0; i < c; i++)
    {
      websWrite (wp, "<tr><td>\n");
      FWSHOW2
	("<input maxlength=\"12\" size=\"12\" name=\"name%d\" onblur=\"valid_name(this,'Name')\" value='<%% port_trigger_table(\"name\",\"%d\"); %%>' />\n",
	 i, i);
      websWrite (wp, "</td><td>\n");
      FWSHOW2
	("<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"i_from%d\" onblur=\"valid_range(this,1,65535,'Port')\" value='<%% port_trigger_table(\"i_from\",\"%d\"); %%>' /> to\n",
	 i, i);
      websWrite (wp, "</td><td>\n");
      FWSHOW2
	("<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"i_to%d\" onblur=\"valid_range(this,1,65535,'Port')\" value='<%% port_trigger_table(\"i_to\",\"%d\"); %%>' />\n",
	 i, i);
      websWrite (wp, "</td><td>\n");
      FWSHOW2
	("<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"o_from%d\" onblur=\"valid_range(this,1,65535,'Port')\" value='<%% port_trigger_table(\"o_from\",\"%d\"); %%>' /> to\n",
	 i, i);
      websWrite (wp, "</td><td>\n");
      FWSHOW2
	("<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"o_to%d\" onblur=\"valid_range(this,1,65535,'Port')\" value='<%% port_trigger_table(\"o_to\",\"%d\"); %%>' />\n",
	 i, i);
      websWrite (wp, "</td><td>\n");
      FWSHOW2
	("<input type=\"checkbox\" name=\"enable%d\" value=\"on\" <%% port_trigger_table(\"enable\",\"%d\"); %%> />\n",
	 i, i);
      websWrite (wp, "</td></tr>\n");
    }
  return;
}


//SEG DD-WRT addition
static void
ej_show_styles (int eid, webs_t wp, int argc, char_t ** argv)
{
//<option value="blue" <% nvram_selected("router_style", "blue"); %>>Blue</option>
  DIR *directory;
  char buf[256];
  directory = opendir ("/www/style");
  struct dirent *entry;
  while ((entry = readdir (directory)) != NULL)
    {
      sprintf (buf, "style/%s/style.css", entry->d_name);
      if (getWebsFile(buf)==NULL)
        {
	sprintf (buf, "/www/style/%s/style.css", entry->d_name);
        FILE *test = fopen (buf, "rb");
        if (test == NULL)
	    continue;
        fclose (test);
	}
      websWrite (wp, "<option value=\"%s\" %s>%s</option>\n", entry->d_name,
		 nvram_match ("router_style",
			      entry->d_name) ? "selected=\"selected\"" : "",
		 entry->d_name);
    }
  closedir (directory);
  return;
}

static void
ej_show_languages (int eid, webs_t wp, int argc, char_t ** argv)
{
  DIR *directory;
  char buf[256];
  directory = opendir ("/www/lang_pack");
  if (directory == NULL)
    return;
  struct dirent *entry;
  while ((entry = readdir (directory)) != NULL)
    {
      sprintf (buf, "/www/lang_pack/%s", entry->d_name);
      FILE *test = fopen (buf, "rb");
      if (test == NULL)
	continue;
      fclose (test);
      if (strlen (entry->d_name) < 4)
	continue;
      strcpy (buf, entry->d_name);
      buf[strlen (buf) - 3] = 0;	//strip .js
      websWrite (wp,
		 "<script type=\"text/javascript\">document.write(\"<option value=\\\"%s\\\" %s >\" + management.lang_%s + \"</option>\");</script>\n",
		 buf, nvram_match ("language",
				   buf) ? "selected=\\\"selected\\\"" : "",
		 buf);
    }
  closedir (directory);
  return;
}

static void
ej_show_modules (int eid, webs_t wp, int argc, char_t ** argv)
{
  char buf[256];
  struct dirent *entry;
  DIR *directory;
//display modules
  int idx;
  for (idx = 0; idx < 3; idx++)
    {
      directory = opendir (directories[idx]);
      if (directory == NULL)
	continue;
//list all files in this directory
      while ((entry = readdir (directory)) != NULL)
	{
	  if (argc > 0)
	    {
	      if (endswith (entry->d_name, argv[0]))
		{
		  sprintf (buf, "%s/%s", directories[idx], entry->d_name);
		  do_ej(buf, wp);
		}
	    }
	  else
	    {
	      if (endswith (entry->d_name, ".webconfig"))
		{
		  sprintf (buf, "%s/%s", directories[idx], entry->d_name);
		  do_ej(buf, wp);
		}
	    }
	}
      closedir (directory);
    }
  return;
}


static void
do_shell_script (char *url, webs_t stream)
{
  char buf[256];
  sprintf (buf, "%s >/tmp/shellout.asp", url);
  system (buf);
  do_ej("/tmp/shellout.asp", stream);
}


static int
apply_cgi (webs_t wp, char_t * urlPrefix, char_t * webDir, int arg,
	   char_t * url, char_t * path, char_t * query)
{
  int action = NOTHING;
  char *value;
  char *submit_button, *next_page;

  int sleep_time = 0;
  int need_commit = 1;
  cprintf ("need reboot\n");
  int need_reboot = atoi (websGetVar (wp, "need_reboot", "0"));
  int ret_code;
  cprintf ("apply");
  error_value = 0;
  ret_code = -1;

	/********************/

  cprintf ("get change action\n");
  value = websGetVar (wp, "change_action", "");
  cprintf ("action = %s\n", value);

  if (value && !strcmp (value, "gozila_cgi"))
    {
      cprintf ("start gozila_cgi");
      gozila_cgi (wp, urlPrefix, webDir, arg, url, path, query);
      return 1;
    }
  cprintf ("get submit button");
	/********************/
  submit_button = websGetVar (wp, "submit_button", "");

  if (!query)
    goto footer;

  if (legal_ip_netmask
      ("lan_ipaddr", "lan_netmask",
       nvram_safe_get ("http_client_ip")) == TRUE)
    browser_method = USE_LAN;
  else
    browser_method = USE_WAN;

  need_commit = atoi (websGetVar (wp, "commit", "1"));
  cprintf ("get action\n");
  value = websGetVar (wp, "action", "");
  cprintf ("action = %s\n", value);
  /* Apply values */
  if (!strcmp (value, "Apply"))
    {
      struct apply_action *act;
      cprintf ("validate cgi");
      validate_cgi (wp);
      cprintf ("handle apply action\n");
      act = handle_apply_action (submit_button);
      cprintf ("done\n");
      //If web page configuration is changed, the EZC configuration function should be disabled.(2004-07-29)
      nvram_set ("is_default", "0");
      nvram_set ("is_modified", "1");

      if (act)
	{
	  cprintf
	    ("submit_button=[%s] service=[%s] sleep_time=[%d] action=[%d]\n",
	     act->name, act->service, act->sleep_time, act->action);
	  if ((act->action == SYS_RESTART)
	      || (act->action == SERVICE_RESTART))
	    nvram_set ("action_service", act->service);
	  else
	    nvram_set ("action_service", "");
	  sleep_time = act->sleep_time;
	  action = act->action;

	  if (act->go)
	    ret_code = act->go (wp);
	}
      else
	{
	  nvram_set ("action_service", "");
	  sleep_time = 1;
	  action = RESTART;
	}

      if (need_commit)
	{
	  //If web page configuration is changed, the EoU function should be disabled.(2004-05-06)
//        nvram_set ("eou_configured", "1");
//        eval ("wl", "custom_ie", "0");
	  diag_led (DIAG, STOP_LED);
	  //If web page configuration is changed, the EZC configuration function should be disabled.(2004-07-29)
	  //nvram_set("is_default", "0");
	  //nvram_set("is_modified", "1");
	  sys_commit ();
	}
    }
  /* Restore defaults */
  else if (!strncmp (value, "Restore", 7))
    {
      ACTION ("ACT_SW_RESTORE");
      nvram_set ("sv_restore_defaults", "1");
      eval ("killall", "-9", "udhcpc");
      sys_commit ();
      eval ("erase", "nvram");
      action = REBOOT;
    }

  /* Reboot */
  else if (!strncmp (value, "Reboot", 7))
    {
      action = REBOOT;
      do_ej ("Reboot.asp", wp);
      websDone (wp, 200);
      sleep (5);
      sys_reboot ();
      return 1;
    }
  /* Invalid action */
  else
    websDebugWrite (wp, "Invalid action %s<br>", value);


footer:
  if (do_reboot)
    action = REBOOT;

  /* The will let PC to re-get a new IP Address automatically */
  if (lan_ip_changed || need_reboot)
    action = REBOOT;

  if (action != REBOOT)
    {
      if (!error_value)
	{
	  if (my_next_page[0] != '\0')
	    {
	      sprintf (path, "%s", my_next_page);
	    }
	  else
	    {
	      next_page = websGetVar (wp, "next_page", NULL);
	      if (next_page)
		sprintf (path, "%s", next_page);
	      else
		sprintf (path, "%s.asp", submit_button);
	    }
	  cprintf ("refresh to %s\n", path);
	  do_ej (path, wp);	//refresh
	  websDone (wp, 200);

/* if (websGetVar (wp, "small_screen", NULL))		// this was replaced by the "saved" button value and all controls are now grey out
 * 	do_ej ("Success_s.asp", wp);
 * else
 * 	do_ej ("Success.asp", wp);
 */
	}
      else
	{
	  if (websGetVar (wp, "small_screen", NULL))
	    {
	      do_ej ("Fail_s.asp", wp);
	    }
	  else
	    {
	      do_ej ("Fail.asp", wp);
	    }
	  websDone (wp, 200);
	}
    }
  else
    {
      do_ej ("Reboot.asp", wp);
      websDone (wp, 200);
      sleep (5);
      sys_reboot ();
      return 1;
    }

  nvram_set ("upnp_wan_proto", "");
  sleep (sleep_time);

  if ((action == RESTART) || (action == SYS_RESTART))
    sys_restart ();
  else if (action == SERVICE_RESTART)
    service_restart ();

  return 1;

}

#ifdef WEBS

void
initHandlers (void)
{
  websAspDefine ("nvram_get", ej_nvram_get);
  websAspDefine ("nvram_real_get", ej_nvram_real_get);
  websAspDefine ("nvram_match", ej_nvram_match);
  websAspDefine ("nvram_invmatch", ej_nvram_invmatch);
  websAspDefine ("nvram_list", ej_nvram_list);
  websAspDefine ("filter_ip", ej_filter_ip);
  websAspDefine ("filter_port", ej_filter_port);
  websAspDefine ("forward_port", ej_forward_port);
  websAspDefine ("forward_spec", ej_forward_spec);
// changed by steve
//websAspDefine ("forward_upnp", ej_forward_upnp);      // upnp added
// end changed by steve

  websAspDefine ("static_route", ej_static_route);
  websAspDefine ("localtime", ej_localtime);
  websAspDefine ("dumplog", ej_dumplog;
#ifdef HAVE_SPUTNIK_APD
		 websAspDefine ("sputnik_apd_status", ej_sputnik_apd_status);
#endif
		 websAspDefine ("dumpleases", ej_dumpleases);
		 websAspDefine ("ppplink", ej_ppplink);
		 websUrlHandlerDefine ("/Management.asp", NULL, 0, apply_cgi,
				       0);
		 websUrlHandlerDefine ("/internal.cgi", NULL, 0, internal_cgi,
				       0);
		 //DD-WRT addition start
		 websUrlHandlerDefine ("/modules.cgi", NULL, 0, show_modules,
				       0);
		 //DD-WRT addition end
		 websSetPassword (nvram_safe_get ("http_passwd"));
		 websSetRealm ("DD-WRT Router OS Core");
		 }

#else /* !WEBS */
#ifdef HAVE_SKYTRON
int
do_auth (char *userid, char *passwd, char *realm)
{
  strncpy (userid, nvram_safe_get ("skyhttp_username"), AUTH_MAX);
  strncpy (passwd, nvram_safe_get ("skyhttp_passwd"), AUTH_MAX);
  //strncpy(realm, MODEL_NAME, AUTH_MAX);
  strncpy (realm, nvram_safe_get ("router_name"), AUTH_MAX);
  return 0;
}


int
do_auth2 (char *userid, char *passwd, char *realm)
{
  strncpy (userid, nvram_safe_get ("http_username"), AUTH_MAX);
  strncpy (passwd, nvram_safe_get ("http_passwd"), AUTH_MAX);
  //strncpy(realm, MODEL_NAME, AUTH_MAX);
  strncpy (realm, nvram_safe_get ("router_name"), AUTH_MAX);
  return 0;
}
#else
#ifdef HAVE_NEWMEDIA
int
do_auth2 (char *userid, char *passwd, char *realm)
{
  strncpy (userid, nvram_safe_get ("newhttp_username"), AUTH_MAX);
  strncpy (passwd, nvram_safe_get ("newhttp_passwd"), AUTH_MAX);
  //strncpy(realm, MODEL_NAME, AUTH_MAX);
  strncpy (realm, nvram_safe_get ("router_name"), AUTH_MAX);
  return 0;
}
#endif



int
do_auth (char *userid, char *passwd, char *realm)
{
  strncpy (userid, nvram_safe_get ("http_username"), AUTH_MAX);
  strncpy (passwd, nvram_safe_get ("http_passwd"), AUTH_MAX);
  //strncpy(realm, MODEL_NAME, AUTH_MAX);
  strncpy (realm, nvram_safe_get ("router_name"), AUTH_MAX);
  return 0;
}

int
do_cauth (char *userid, char *passwd, char *realm)
{
  if (nvram_match ("info_passwd", "0"))
    return -1;
  return do_auth (userid, passwd, realm);
}
#endif

#ifdef HAVE_DDLAN
int
do_auth2 (char *userid, char *passwd, char *realm)
{
  strncpy (userid, nvram_safe_get ("http2_username"), AUTH_MAX);
  strncpy (passwd, nvram_safe_get ("http2_passwd"), AUTH_MAX);
  //strncpy(realm, MODEL_NAME, AUTH_MAX);
  strncpy (realm, nvram_safe_get ("router_name"), AUTH_MAX);
  return 0;
}
#endif

//#ifdef EZC_SUPPORT
char ezc_version[128];
//#endif

char post_buf[10000] = { 0 };
extern int post;

static void			// support GET and POST 2003-08-22
do_apply_post (char *url, webs_t stream, int len, char *boundary)
{
  char buf[1024];
  int count;

  if (post == 1)
    {
      if (len > sizeof (post_buf) - 1)
	{
	  cprintf ("The POST data exceed length limit!\n");
	  return;
	}
      /* Get query */
      if (!(count = wfread (post_buf, 1, len, stream)))
	return;
      post_buf[count] = '\0';;
      len -= strlen (post_buf);

      /* Slurp anything remaining in the request */
      while (--len > 0)
#ifdef HAVE_HTTPS
	if (do_ssl)
#ifdef HAVE_OPENSSL
	  BIO_gets ((BIO *) stream, buf, 1);
#elif defined(HAVE_MATRIXSSL)
	  matrixssl_gets (stream, buf, 1);
#else
	  ;
#endif
	else
#endif
	  (void) fgetc (stream);
      init_cgi (post_buf);
    }
}

static void
do_style (char *url, webs_t stream)
{
  char *style = nvram_get ("router_style");
  if (style == NULL || strlen (style) == 0)
    do_file ("kromo.css", stream);
  else
    do_file (style, stream);
}


#ifdef HAVE_CHILLI
static void
do_fon_cgi (char *url, webs_t wp)
{

//  nvram_set ("router_style", "");
  nvram_set ("wl_ssid", "FON_HotSpot");
  nvram_set ("wl_ap_isolate", "1");	/* AP isolate mode */
  nvram_set ("dnsmasq_enable", "0");
  nvram_set ("dhcp_dnsmasq", "0");
  nvram_set ("chilli_enable", "1");
  nvram_set ("chilli_url", "https://login.fon.com/cp/index.php");
  nvram_set ("chilli_radius", "radius01.fon.com");
  nvram_set ("chilli_backup", "radius02.fon.com");
  nvram_set ("chilli_pass", "garrafon");
  nvram_set ("chilli_dns1", "0.0.0.0");
  nvram_set ("chilli_interface", "wlan");
  nvram_set ("chilli_radiusnasid", "");
  nvram_set ("chilli_uamsecret", "garrafon");
  nvram_set ("chilli_uamanydns", "1");
  nvram_set ("chilli_uamallowed",
	     "www.fon.com,www.paypal.com,www.paypalobjects.com,www.skype.com,www.dd-wrt.com,www.dd-wrt.org,www.dd-wrt.com.de,213.134.45.0/24");
  nvram_set ("chilli_macauth", "0");
  nvram_set ("chilli_additional", "");
  nvram_set ("fon_enable", "1");
  nvram_commit ();
  do_ej ("Reboot.asp", wp);
  websDone (wp, 200);
  sleep (4);
  sys_reboot ();
  init_cgi (NULL);

}

#endif

static void
do_apply_cgi (char *url, webs_t stream)
{
  char *path, *query;

  if (post == 1)
    {
      query = post_buf;
      path = url;
    }
  else
    {
      query = url;
      path = strsep (&query, "?") ? : url;
      init_cgi (query);
    }

  if (!query)
    return;

  apply_cgi (stream, NULL, NULL, 0, url, path, query);
  init_cgi (NULL);
}


void
ej_get_http_method (int eid, webs_t wp, int argc, char_t ** argv)
{
  websWrite (wp, "%s", "post");
}

static char *
getLanguageName ()
{
  char *lang = nvram_get ("language");
  cprintf ("get language %s\n", lang);
  char *l = malloc (60);
  if (lang == NULL)
    {
      cprintf ("return default\n");
      sprintf (l, "lang_pack/english.js");
      return l;
    }
  sprintf (l, "lang_pack/%s.js", lang);
  cprintf ("return %s\n", l);
  return l;
}

static void
do_language (char *path, webs_t stream)	//jimmy, https, 8/4/2003
{
  char *lang = getLanguageName ();
  do_file (lang, stream);
  free (lang);
  return;
}

char *
live_translate (char *tran)
{

  FILE *fp;
  char temp[256], temp1[256], *temp2;
  char *lang = getLanguageName ();
  char buf[64];
  sprintf (buf, "/www/%s", lang);
  free (lang);

  	strcpy (temp1, tran);
	strcat (temp1, "=\"");
  
	fp = fopen (buf, "r");
	

	while (fgets(temp, 256, fp) != NULL)
	{
		if ((strstr(temp, temp1)) != NULL)
		{
			temp2 = strtok(temp,"\"");
			temp2 = strtok(NULL,"\"");

			fclose (fp);
			return temp2;
		}
	}

fclose (fp);
return "Error";
  
}

/* obsolete, use do_pagehead
void
ej_charset (int eid, webs_t wp, int argc, char_t ** argv)
{

  char *lang = getLanguageName ();
  char buf[64];
  sprintf (buf, "/www/%s", lang);
  free (lang);
// lang_charset.set
  char *sstring = "lang_charset.set=\"";
  char s[128];
  FILE *in = fopen (buf, "rb");
  while (!feof (in))
    {
      fscanf (in, "%s", s);
      cprintf ("lang scan %s\n", s);
      char *cmp = strstr (s, sstring);
      cprintf ("strstr %s\n", cmp);
      if (cmp)
	{
	  fclose (in);
	  cmp += strlen (sstring);
	  cprintf ("source %s\n", cmp);
	  char *t2 = strstr (cmp, "\"");
	  if (t2 == NULL)
	    {
	      cprintf (" length was null\n");
	      return;		//error (typo?)
	    }
	  int len = t2 - cmp;
	  cprintf ("length = %d\n", len);
	  if (len < 0)
	    return;		//error (unknown)
	  char dest[128];
	  strncpy (dest, cmp, len);
	  dest[len] = 0;
	  cprintf ("destination %s\n", dest);
	  websWrite (wp,
		     "<meta http-equiv=\"Content-Type\" content=\"application/xhtml+xml; charset=%s\" />",
		     dest);
	  return;
	}
    }
  fclose (in);
}
*/

void
ej_do_menu (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *mainmenu, *submenu;

  if (ejArgs (argc, argv, "%s %s", &mainmenu, &submenu) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
    
int sipgate = nvram_match ("sipgate", "1");
#ifdef HAVE_SPUTNIK_APD
int sputnik = nvram_match ("apd_enable", "1");
#else
int sputnik = 0;
#endif
#ifdef HAVE_NEWMEDIA
int openvpn = nvram_match ("openvpn_enable", "1");
#else
int openvpn = 0;
#endif
int auth = nvram_match ("status_auth", "1");


char menu[8][11][32] = {{"index.asp","DDNS.asp","WanMAC.asp","Routing.asp","Vlan.asp","","","","","",""},
						{"Wireless_Basic.asp","Wireless_radauth.asp","WL_WPATable.asp","Wireless_MAC.asp","Wireless_Advanced.asp","Wireless_WDS.asp","","","","",""},
						{"Sipath.asp","cgi-bin-mf-phonebook.html","cgi-bin-mf-status.html","","","","","","","",""},
						{"Firewall.asp","VPN.asp","","","","","","","","",""},
						{"Filters.asp","","","","","","","","","",""},
						{"Forward.asp","ForwardSpec.asp","Triggering.asp","UPnP.asp","DMZ.asp","QoS.asp","","","","",""},
						{"Management.asp","Hotspot.asp","Services.asp","Alive.asp","Log.asp","Diagnostics.asp","Wol.asp","Factory_Defaults.asp","Upgrade.asp","config.asp",""},
						{"Status_Router.asp","Status_Lan.asp","Status_Wireless.asp","Status_SputnikAPD.asp","Status_OpenVPN.asp","Info.htm","","","","",""}};

/* real name is bmenu.menuname[i][j] */
char menuname[8][11][32] = {{"setup","setupbasic","setupddns","setupmacclone","setuprouting","setupvlan","","","","",""},
							{"wireless","wirelessBasic","wirelessRadius","wirelessSecurity","wirelessMac","wirelessAdvanced","wirelessWds","","","",""},
							{"sipath","sipathoverview","sipathphone","sipathstatus","","","","","","",""},
							{"security","firwall","vpn","","","","","","","",""},
							{"accrestriction","webaccess","","","","","","","","",""},
							{"applications","applicationsprforwarding","applicationspforwarding","applicationsptriggering","applicationsUpnp","applicationsDMZ","applicationsQoS","","","",""},
							{"admin","adminManagement","adminHotspot","adminServices","adminAlive","adminLog","adminDiag","adminWol","adminFactory","adminUpgrade","adminBackup"},
							{"statu","statuRouter","statuLAN","statuWLAN","statuSputnik","statuVPN","statuSysInfo","","","",""}};

int i,j;

	websWrite (wp, "<div id=\"menu\">\n");
	websWrite (wp, " <div id=\"menuMain\">\n");
	websWrite (wp, "  <ul id=\"menuMainList\">\n");

	  for (i=0; i<8; i++)
		{
		if ((!sipgate) && (!strcmp(menu[i][0], "Sipath.asp")))  //jump over Sipath
			i++;
		if (!strcmp (menu[i][0], mainmenu))
			{
			websWrite (wp, "   <li class=\"current\"><span><script type=\"text/javascript\">Capture(bmenu.%s)</script></span>\n", menuname[i][0]);
			websWrite (wp, "    <div id=\"menuSub\">\n");
			websWrite (wp, "     <ul id=\"menuSubList\">\n");
			
			for (j=0; j<11; j++)
				{
#ifdef HAVE_MADWIFI
				if (!strcmp(menu[i][j], "Wireless_radauth.asp"))
					j++;
				if (!strcmp(menu[i][j], "Wireless_Advanced.asp"))
					j++;
				if (!strcmp(menu[i][j], "Wireless_WDS.asp"))
					j++;
#endif
				if ((!sputnik) && !strcmp(menu[i][j], "Status_SputnikAPD.asp"))  //jump over Sputnik
					j++;
				if ((!openvpn) && !strcmp(menu[i][j], "Status_OpenVPN.asp"))  //jump over OpenVPN
					j++;
				if ((!auth) && !strcmp(menu[i][j], "Info.htm"))  //jump over Sys-Info
					j++;
					
				if (!strcmp(menu[i][j], submenu) && (strlen(menu[i][j]) != 0))
					{
					websWrite (wp, "      <li><span><script type=\"text/javascript\">Capture(bmenu.%s)</script></span></li>\n", menuname[i][j+1]);
					}
#ifdef HAVE_HTTPS  //until https will allow upgrade and backup
				else if ((strlen(menu[i][j]) != 0) && (do_ssl) && ((!strcmp(menu[i][j], "Upgrade.asp") || (!strcmp(menu[i][j], "config.asp")))))
					{
					websWrite (wp, "      <script type=\"text/javascript\">\n");
					websWrite (wp, "      document.write(\"<li><a style=\\\"cursor:pointer\\\" title=\\\"\" + errmsg.err46 + \"\\\" onclick=\\\"alert(errmsg.err45)\\\" ><em>\" + bmenu.%s + \"</em></a></li>\");\n", menuname[i][j+1]);
					websWrite (wp, "      </script>\n");
					}
#endif
				else if (strlen(menu[i][j]) != 0)
					{
				websWrite (wp, "      <li><a href=\"%s\"><script type=\"text/javascript\">Capture(bmenu.%s)</script></a></li>\n", menu[i][j], menuname[i][j+1]);
					}
				}
			websWrite (wp, "     </ul>\n");
			websWrite (wp, "    </div>\n");
			websWrite (wp, "    </li>\n");
			}
		else
			{
			websWrite (wp, "   <li><a href=\"%s\"><script type=\"text/javascript\">Capture(bmenu.%s)</script></a></li>\n", menu[i][0], menuname[i][0]);
			}
		}
		websWrite (wp, "  </ul>\n");
		websWrite (wp, " </div>\n");
		websWrite (wp, "</div>\n");

  return;
}

void
ej_do_pagehead (int eid, webs_t wp, int argc, char_t ** argv)	//Eko
{
		char *style = nvram_get ("router_style");
	
			websWrite (wp,
				"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n");
			websWrite (wp, "<html>\n");
			websWrite (wp, "\t<head>\n");
			websWrite (wp,
				"\t\t<meta http-equiv=\"Content-Type\" content=\"application/xhtml+xml; charset=%s\" />\n",
				live_translate("lang_charset.set"));
			if (nvram_invmatch ("dist_type", "micro"))
			{
				websWrite (wp,
					"\t\t<link rel=\"icon\" href=\"images/favicon.ico\" type=\"image/x-icon\" />\n");
				websWrite (wp,
					"\t\t<link rel=\"shortcut icon\" href=\"images/favicon.ico\" type=\"image/x-icon\" />\n");
			}
			
			websWrite (wp,
				"\t\t<script type=\"text/javascript\" src=\"common.js\"></script>\n");
			websWrite (wp,
				"\t\t<script type=\"text/javascript\" src=\"lang_pack/english.js\"></script>\n");
			websWrite (wp,
				"\t\t<script type=\"text/javascript\" src=\"lang_pack/language.js\"></script>\n");
			websWrite (wp,
				"\t\t<link type=\"text/css\" rel=\"stylesheet\" href=\"style/%s/style.css\" />\n",
				style);
			websWrite (wp,
				"\t\t<!--[if IE]><link type=\"text/css\" rel=\"stylesheet\" href=\"style/%s/style_ie.css\" /><![endif]-->",
				style);
		
			
}

void
ej_do_hpagehead (int eid, webs_t wp, int argc, char_t ** argv)	//Eko
{

			websWrite (wp,
				"<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n");
			websWrite (wp, "<html>\n");
			websWrite (wp, "\t<head>\n");
			websWrite (wp,
				"\t\t<meta http-equiv=\"Content-Type\" content=\"application/xhtml+xml; charset=%s\" />\n",
				live_translate("lang_charset.set"));
			websWrite (wp,
				"\t\t<script type=\"text/javascript\" src=\"../common.js\"></script>\n");
			websWrite (wp,
				"\t\t<script type=\"text/javascript\" src=\"../lang_pack/english.js\"></script>\n");
			websWrite (wp,
				"\t\t<script type=\"text/javascript\" src=\"../lang_pack/language.js\"></script>\n");
			websWrite (wp,
				"\t\t<link type=\"text/css\" rel=\"stylesheet\" href=\"help.css\">");

}

void
ej_show_timeoptions (int eid, webs_t wp, int argc, char_t ** argv)	//Eko
{

char timediffs[37][8] = {"-12","-11","-10","-09.5","-09","-08","-07","-06","-05","-04","-03.5","-03","-02","-01","+00",
								"+01","+02","+03","+03.5","+04","+05","+05.5","+05.75","+06","+06.5","+07","+08","+09","+09.5","+10","+10.5","+11","+11.5","+12","+12.75","+13","+14"};

char timezones[37][8] = {"-12:00","-11:00","-10:00","-09:30","-09:00","-08:00","-07:00","-06:00","-05:00","-04:00","-03:30","-03:00","-02:00","-01:00","",
								"+01:00","+02:00","+03:00","+03:30","+04:00","+05:00","+05:30","+05:45","+06:00","+06:30","+07:00","+08:00","+09:00","+09:30","+10:00","+10:30","+11:00","+11:30","+12:00","+12:45","+13:00","+14:00"};

char timeoption[5][2] = {"1","2","3","4","5"};

int i,j;
char str[11];

	for (i=0; i<37; i++)
	{
	  	for (j=0; j<5; j++)
	  	{
			strcpy (str, timediffs[i]);
			strcat (str, " 1 ");
			strcat (str, timeoption[j]);

			websWrite (wp,
			"<script type=\"text/javascript\">document.write(\"<option value=\\\"%s\\\" %s>UTC%s / \" + idx.summt_opt%s + \"</option>\");</script>\n",
			str, nvram_match ("time_zone", str) ? "selected=\\\"selected\\\"" : "", timezones[i], timeoption[j]);
		 }
	}
		
}


void
ej_do_statusinfo (int eid, webs_t wp, int argc, char_t ** argv)	//Eko
{

//				<div id="statusInfo">
//					<div class="info"><% tran("share.firmware"); %>: <script type="text/javascript">//<![CDATA[
//					document.write("<a title=\"" + share.about + "\" href=\"javascript:openAboutWindow()\"><% get_firmware_version(); %></a>");
//					//]]></script></div>
//					<div class="info"><% tran("share.time"); %>: <% get_uptime(); %></div>
//					<div class="info">WAN <% nvram_match("wl_mode","wet","disabled <!--"); %><% nvram_match("wan_proto","disabled","disabled <!--"); %>IP: <% nvram_status_get("wan_ipaddr"); %><% nvram_match("wan_proto","disabled","-->"); %><% nvram_match("wl_mode","wet","-->"); %></div>
//				</div>

  char *wan_ipaddr;
  int wan_link = check_wan_link (0);
  
    if (nvram_match ("wan_proto", "pptp"))
		{
		wan_ipaddr = wan_link ? nvram_safe_get ("pptp_get_ip") : nvram_safe_get ("wan_ipaddr");
    	}
	else if (!strcmp (nvram_safe_get ("wan_proto"), "pppoe"))
    	{
		wan_ipaddr = wan_link ? nvram_safe_get ("wan_ipaddr") : "0.0.0.0";
      	}
	else if (nvram_match ("wan_proto", "l2tp"))
		{
		wan_ipaddr = wan_link ? nvram_safe_get ("l2tp_get_ip") : nvram_safe_get ("wan_ipaddr");
		}
	else
		{
		wan_ipaddr = nvram_safe_get ("wan_ipaddr");
		}

	
		websWrite (wp, "<div id=\"statusInfo\">\n");
		websWrite (wp, "<div class=\"info\"><script type=\"text/javascript\">Capture(share.firmware)</script>: ");
		websWrite (wp, "<script type=\"text/javascript\">document.write(\"<a title=\\\"\" + share.about + \"\\\" href=\\\"javascript:openAboutWindow()\\\">");
		ej_get_firmware_version(0,wp,argc,argv);
		websWrite (wp, "</a>\");</script></div>\n"); 
		websWrite (wp, "<div class=\"info\"><script type=\"text/javascript\">Capture(share.time)</script>: ");
		ej_get_uptime(0,wp,argc,argv);
		websWrite (wp, "</div>\n");
		websWrite (wp, "<div class=\"info\">WAN");
			if (nvram_match ("wl_mode", "wet") || nvram_match("wan_proto", "disabled"))
				{
				websWrite (wp, ": <script type=\"text/javascript\">Capture(share.disabled)</script></div>\n");
				}
			else 
				{
				websWrite (wp, " IP: %s</div>\n", wan_ipaddr);
				}
		websWrite (wp, "</div>\n");	
		
}

static char no_cache[] =
  "Cache-Control: no-cache\r\n" "Pragma: no-cache\r\n" "Expires: 0";

struct mime_handler mime_handlers[] = {
  //{ "ezconfig.asp", "text/html", ezc_version, do_apply_ezconfig_post, do_ezconfig_asp, do_auth },
#ifdef HAVE_SKYTRON
  {"setupindex*", "text/html", no_cache, NULL, do_ej, do_auth2},
#endif
#ifdef HAVE_DDLAN
  {"Upgrade*", "text/html", no_cache, NULL, do_ej, do_auth2},
  {"Management*", "text/html", no_cache, NULL, do_ej, do_auth2},
  {"Services*", "text/html", no_cache, NULL, do_ej, do_auth2},
  {"Hotspot*", "text/html", no_cache, NULL, do_ej, do_auth2},
  {"Wireless*", "text/html", no_cache, NULL, do_ej, do_auth2},
  {"WL_*", "text/html", no_cache, NULL, do_ej, do_auth2},
  {"WPA*", "text/html", no_cache, NULL, do_ej, do_auth2},
  {"Log*", "text/html", no_cache, NULL, do_ej, do_auth2},
  {"Alive*", "text/html", no_cache, NULL, do_ej, do_auth2},
  {"Diagnostics*", "text/html", no_cache, NULL, do_ej, do_auth2},
  {"Wol*", "text/html", no_cache, NULL, do_ej, do_auth2},
  {"Factory_Defaults*", "text/html", no_cache, NULL, do_ej, do_auth2},
  {"config*", "text/html", no_cache, NULL, do_ej, do_auth2},
#endif

#ifdef HAVE_NEWMEDIA
  {"Services.asp", "text/html", no_cache, NULL, do_ej, do_auth2},
  {"Ping.asp", "text/html", no_cache, NULL, do_ej, do_auth2},
  {"Diagnostics.asp", "text/html", no_cache, NULL, do_ej, do_auth2},
#endif
  {"**.sh", "text/html", no_cache, NULL, do_shell_script, do_auth},
  {"**.asp", "text/html", no_cache, NULL, do_ej, do_auth},
  {"**.JPG", "image/jpeg", no_cache, NULL, do_file, NULL},
  {"style.css", "text/css", NULL, NULL, do_style, NULL},
  {"common.js", "text/javascript", NULL, NULL, do_file, NULL},
  {"lang_pack/language.js", "text/javascript", NULL, NULL, do_language, NULL},

  {"SysInfo.htm*", "text/plain", no_cache, NULL, do_ej, do_auth},
#ifdef HAVE_SKYTRON
  {"Info.htm*", "text/html", no_cache, NULL, do_ej, do_auth2},
  {"Info.live.htm", "text/html", no_cache, NULL, do_ej, do_auth},
  {"**.htm", "text/html", no_cache, NULL, do_ej, do_auth2},
  {"**.html", "text/html", no_cache, NULL, do_ej, do_auth2},
#else
  {"Info.htm*", "text/html", no_cache, NULL, do_ej, do_cauth},
  {"Info.live.htm", "text/html", no_cache, NULL, do_ej, do_cauth},
  {"**.htm", "text/html", no_cache, NULL, do_ej, NULL},
  {"**.html", "text/html", no_cache, NULL, do_ej, NULL},

#endif
  {"**.css", "text/css", NULL, NULL, do_file, NULL},
  {"**.gif", "image/gif", NULL, NULL, do_file, NULL},
  {"**.png", "image/png", NULL, NULL, do_file, NULL},
  {"**.jpg", "image/jpeg", NULL, NULL, do_file, NULL},
  {"**.ico", "image/x-icon", NULL, NULL, do_file, NULL},
  {"**.js", "text/javascript", NULL, NULL, do_file, NULL},
#ifdef HAVE_SKYTRON
  {"applyuser.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi,
   do_auth2},
#elif HAVE_NEWMEDIA
  {"applyuser.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi,
   do_auth2},
#elif HAVE_DDLAN
  {"applyuser.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi,
   NULL},
#else
  {"applyuser.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi,
   do_auth},
#endif
#ifdef HAVE_CHILLI
  {"fon.cgi*", "text/html", no_cache, NULL, do_fon_cgi, do_auth},
#endif
#ifdef HAVE_DDLAN
  {"apply.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi, NULL},
  {"upgrade.cgi*", "text/html", no_cache, do_upgrade_post, do_upgrade_cgi,
   NULL},
#else
  {"apply.cgi*", "text/html", no_cache, do_apply_post, do_apply_cgi, do_auth},
  {"upgrade.cgi*", "text/html", no_cache, do_upgrade_post, do_upgrade_cgi,
   do_auth},
#endif
//  {"Gozila.cgi*", "text/html", no_cache, NULL, do_setup_wizard, do_auth},     // for setup wizard
/*	{ "**.cfg", "application/octet-stream", no_cache, NULL, do_backup, do_auth }, */
#ifdef HAVE_DDLAN
  {"restore.cgi**", "text/html", no_cache, do_upgrade_post, do_upgrade_cgi,
   NULL},
#else
  {"restore.cgi**", "text/html", no_cache, do_upgrade_post, do_upgrade_cgi,
   do_auth},
#endif
  {"test.bin**", "application/octet-stream", no_cache, NULL, do_file,
   do_auth},

#ifdef HAVE_DDLAN
  {"nvrambak.bin*", "application/octet-stream", no_cache, NULL, nv_file_out,
   do_auth2},
  {"nvrambak**.bin*", "application/octet-stream", no_cache, NULL, nv_file_out,
   do_auth2},
  {"nvram.cgi*", "text/html", no_cache, nv_file_in, sr_config_cgi, NULL},
#else
  {"nvrambak.bin*", "application/octet-stream", no_cache, NULL, nv_file_out,
   do_auth},
  {"nvrambak**.bin*", "application/octet-stream", no_cache, NULL, nv_file_out,
   do_auth},
  {"nvram.cgi*", "text/html", no_cache, nv_file_in, sr_config_cgi, do_auth},
#endif

//for ddm
  {NULL, NULL, NULL, NULL, NULL, NULL}
};


/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_selected("wan_proto", "dhcp",); %> produces: selected="selected"
 * <% nvram_selected("wan_proto", "dhcp", "js"); %> produces: selected=\"selected\"
 * <% nvram_selected("wan_proto", "static"); %> does not produce
 */
static void
ej_nvram_selected (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name, *match, *javascript;
  int args;
  args = ejArgs (argc, argv, "%s %s %s", &name, &match, &javascript);
  if (args < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  if (nvram_match (name, match))
    {
      if (args == 3 && javascript != NULL && !strcmp (javascript, "js"))
	websWrite (wp, "selected=\\\"selected\\\"");
      else
	websWrite (wp, "selected=\"selected\"");
    }
  return;
}





static void
ej_getrebootflags (int eid, webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_RB500
  websWrite (wp, "1");
#elif HAVE_MAGICBOX
  websWrite (wp, "2");
#elif HAVE_GATEWORX
  websWrite (wp, "1");
#else
  websWrite (wp, "0");
#endif
}

static void
ej_tran (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name;
  int args;
  args = ejArgs (argc, argv, "%s", &name);
  if (args != 1)
    return;
  websWrite (wp, "<script type=\"text/javascript\">Capture(%s)</script>",
	     name);
  return;
}

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_checked("wan_proto", "dhcp"); %> produces: checked="checked"
 * <% nvram_checked("wan_proto", "dhcp", "js"); %> produces: checked=\"checked\"
 * <% nvram_checked("wan_proto", "static"); %> does not produce
 */
static void
ej_nvram_checked (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *name, *match, *javascript;
  int args;
  cprintf ("args\n");
  args = ejArgs (argc, argv, "%s %s %s", &name, &match, &javascript);
  if (args < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
  cprintf ("match()\n");
  if (nvram_match (name, match))
    {
      cprintf ("javascript check\n");
      if (args == 3 && javascript != NULL && !strcmp (javascript, "js"))
	{
	  cprintf ("write js\n");
	  websWrite (wp, "checked=\\\"checked\\\"");

	}
      else
	{
	  cprintf ("write non js\n");
	  websWrite (wp, "checked=\"checked\"");
	}
    }

  return;
}
#ifdef HAVE_CPUTEMP
static void
ej_get_cputemp(int eid, webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_GATEWORX
#define TEMP_MUL 100
FILE *fp=fopen("/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0028/temp_input","rb");
#else
#define TEMP_MUL 1000
FILE *fp=fopen("/sys/devices/platform/i2c-0/0-0048/temp1_input","rb");
#endif

if (fp==NULL)
    {
    websWrite(wp,"N/A"); //no i2c lm75 found
    return;
    }
int temp;
fscanf(fp,"%d",&temp);
fclose(fp);
int high=temp/TEMP_MUL;
int low=(temp-(high*TEMP_MUL))/(TEMP_MUL/10);
websWrite(wp,"%d.%d °C",high,low); //no i2c lm75 found
}


static void
ej_show_cpu_temperature (int eid, webs_t wp, int argc, char_t ** argv)
{
websWrite(wp,"<div class=\"setting\">\n");
websWrite(wp,"<div class=\"label\">CPU Temperature</div>\n");
websWrite(wp,"<span id=\"cpu_temp\"></span>&nbsp;\n");
websWrite(wp,"</div>\n");
}
#endif


#ifdef HAVE_VOLT
static void
ej_get_voltage(int eid, webs_t wp, int argc, char_t ** argv)
{
FILE *fp=fopen("/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0028/volt","rb");

if (fp==NULL)
    {
    websWrite(wp,"N/A"); //no i2c lm75 found
    return;
    }
int temp;
fscanf(fp,"%d",&temp);
fclose(fp);
//temp*=564;
int high=temp/1000;
int low=(temp-(high*1000))/100;
websWrite(wp,"%d.%d Volt",high,low); //no i2c lm75 found
}


static void
ej_show_voltage (int eid, webs_t wp, int argc, char_t ** argv)
{
websWrite(wp,"<div class=\"setting\">\n");
websWrite(wp,"<div class=\"label\">Board Voltage</div>\n");
websWrite(wp,"<span id=\"voltage\"></span>&nbsp;\n");
websWrite(wp,"</div>\n");
}
#endif


#ifdef HAVE_MSSID

static void showencstatus(webs_t wp,char *prefix)
{
char akm[64];
char ssid[64];
sprintf(akm,"%s_akm",prefix);
sprintf(ssid,"%s_ssid",prefix);
websWrite(wp,"<div class=\"setting\">\n");
websWrite(wp,"<div class=\"label\"><script type=\"text/javascript\">Capture(share.encrypt)</script>&nbsp;-&nbsp;<script type=\"text/javascript\">Capture(share.intrface)</script>&nbsp;%s</div>\n",prefix);
websWrite(wp,"<script type=\"text/javascript\">");
if (nvram_match(akm,"disabled"))
    {
    websWrite(wp,"Capture(share.disabled)");
    websWrite(wp,"</script>");
    return;
    }
else
    {	
    websWrite(wp,"Capture(share.enabled)");
    websWrite(wp,"</script>,&nbsp;");
    }
if (nvram_match(akm,"psk"))
    websWrite(wp,"WPA Pre-shared Key");
if (nvram_match(akm,"wpa"))
    websWrite(wp,"WPA RADIUS");
if (nvram_match(akm,"psk2"))
    websWrite(wp,"WPA2 Pre-shared Key");
if (nvram_match(akm,"wpa2"))
    websWrite(wp,"WPA2 RADIUS");
if (nvram_match(akm,"psk psk2"))
    websWrite(wp,"WPA2 Pre-shared Key Mixed");
if (nvram_match(akm,"wpa wpa2"))
    websWrite(wp,"WPA RADIUS Mixed");
if (nvram_match(akm,"radius"))
    websWrite(wp,"RADIUS");
if (nvram_match(akm,"wep"))
    websWrite(wp,"WEP");
websWrite(wp,"\n</div>\n");

}

static void
ej_get_txpower (int eid, webs_t wp, int argc, char_t ** argv)
{
#ifndef HAVE_MADWIFI
websWrite(wp,"%s mW",nvram_safe_get("txpwr"));
#else
websWrite(wp,"%s mW",nvram_safe_get("ath0_txpwr"));
#endif
}


static void
ej_getencryptionstatus (int eid, webs_t wp, int argc, char_t ** argv)
{
char mode[64];
char vifs[64];
  char *next;
  char var[80];
#ifndef HAVE_MADWIFI
sprintf(mode,"%s","wl0");
sprintf(vifs,"%s_vifs","wl0");
#else
sprintf(mode,"%s","ath0");
sprintf(vifs,"%s_vifs","wl0");
#endif
showencstatus(wp,mode);
  foreach (var, nvram_safe_get(vifs), next)
  {
  showencstatus(wp,var);
  }
}
static void
ej_getwirelessstatus (int eid, webs_t wp, int argc, char_t ** argv)
{
char mode[64];
#ifndef HAVE_MADWIFI
sprintf(mode,"%s_mode","wl0");
#else
sprintf(mode,"%s_mode","ath0");
#endif
websWrite(wp,"<script type=\"text/javascript\">");
if (nvram_match(mode,"wet") || nvram_match(mode,"sta") || nvram_match(mode,"infra") || nvram_match(mode,"apsta"))
    websWrite(wp,"Capture(info.ap)");
else
    websWrite(wp,"Capture(status_wireless.legend3)");
websWrite(wp,"</script>");
}
#endif

static void
ej_getwirelessmode (int eid, webs_t wp, int argc, char_t ** argv)
{
char mode[64];
#ifndef HAVE_MADWIFI
 #ifdef HAVE_MSSID
sprintf(mode,"%s_mode","wl0");
 #else
sprintf(mode,"%s_mode","wl");
 #endif
#else
sprintf(mode,"%s_mode","ath0");
#endif
websWrite(wp,"<script type=\"text/javascript\">");
if (nvram_match(mode,"wet"))
    websWrite(wp,"Capture(wl_basic.clientBridge)");
if (nvram_match(mode,"ap"))
    websWrite(wp,"Capture(wl_basic.ap)");
if (nvram_match(mode,"sta"))
    websWrite(wp,"Capture(wl_basic.client)");
if (nvram_match(mode,"infra"))
    websWrite(wp,"Capture(wl_basic.adhoc)");
if (nvram_match(mode,"apsta"))
    websWrite(wp,"Capture(wl_basic.repeater)");
if (nvram_match(mode,"wdssta"))
    websWrite(wp,"Capture(wl_basic.wdssta)");
if (nvram_match(mode,"wdsap"))
    websWrite(wp,"Capture(wl_basic.wdsap)");
websWrite(wp,"</script>&nbsp;\n");
}

static void
ej_getwirelessnetmode (int eid, webs_t wp, int argc, char_t ** argv)
{
char mode[64];
#ifndef HAVE_MADWIFI
 #ifdef HAVE_MSSID
sprintf(mode,"%s_net_mode","wl0");
 #else
sprintf(mode,"%s_net_mode","wl");
 #endif
#else
sprintf(mode,"%s_net_mode","ath0");
#endif
websWrite(wp,"<script type=\"text/javascript\">");
if (nvram_match(mode,"disabled"))
    websWrite(wp,"Capture(share.disabled)");
if (nvram_match(mode,"mixed"))
    websWrite(wp,"Capture(wl_basic.mixed)");
if (nvram_match(mode,"g-only"))
    websWrite(wp,"Capture(wl_basic.g)");
if (nvram_match(mode,"b-only"))
    websWrite(wp,"Capture(wl_basic.b)");
if (nvram_match(mode,"n-only"))
    websWrite(wp,"Capture(wl_basic.n)");
if (nvram_match(mode,"a-only"))
    websWrite(wp,"Capture(wl_basic.a)");
websWrite(wp,"</script>&nbsp;\n");
}


#ifdef HAVE_NEWMEDIA


static void
ej_show_openvpn_status (int eid, webs_t wp, int argc, char_t ** argv)
{
websWrite(wp,"<fieldset>\n<legend><script type=\"text/javascript\">Capture(share.state)</script></legend>\n");

system("/etc/openvpnstate.sh > /tmp/.temp");
FILE *in = fopen("/tmp/.temp","r");
while(!feof(in))
    {
    int b = getc(in);
    if (b!=EOF)
	wfputc(b,wp);
    }
fclose(in);
websWrite(wp,"</fieldset>");
websWrite(wp,"<fieldset>\n<legend><script type=\"text/javascript\">Capture(share.statu)</script></legend>\n");
system("/etc/openvpnstatus.sh > /tmp/.temp");
in = fopen("/tmp/.temp","r");
while(!feof(in))
    {
    int b = getc(in);
    if (b!=EOF)
	wfputc(b,wp);
    }
fclose(in);
websWrite(wp,"</fieldset>");
websWrite(wp,"<fieldset>\n<legend><script type=\"text/javascript\">Capture(log.legend)</script></legend>\n");
system("/etc/openvpnlog.sh > /tmp/.temp");
in = fopen("/tmp/.temp","r");
while(!feof(in))
    {
    int b = getc(in);
    if (b!=EOF)
	wfputc(b,wp);
    }
fclose(in);
websWrite(wp,"</fieldset>");

}
/* done in do_menu
static void
ej_show_openvpn (int eid, webs_t wp, int argc, char_t ** argv)
{
  if (nvram_match ("openvpn_enable", "1"))
    {
      websWrite (wp,
		 "<li><a href=\"Status_OpenVPN.asp\">VPN</a></li>\n");
    }
  return;
} */
#endif
static void
ej_get_radio_state (int eid, webs_t wp, int argc, char_t ** argv)
{
int radiooff = -1;

#ifdef HAVE_MADWIFI
		//????;  no idea how to check this
#else
wl_ioctl(get_wdev(), WLC_GET_RADIO, &radiooff, sizeof(int));

#endif

switch (radiooff)
	{	
	case 0:
		//websWrite (wp, "On&nbsp;&nbsp;<img style=\"border-width: 0em;\" src=\"images/radio_on.gif\" width=\"35\" height=\"10\"> ");
		//websWrite (wp, "On");
		websWrite (wp, "%s", live_translate ("wl_basic.radio_on"));
		break;
	case 1: // software disabled
	case 2: // hardware disabled
	case 3: // both are disabled
		//websWrite (wp, "Off&nbsp;&nbsp;<img style=\"border-width: 0em;\" src=\"images/radio_off.gif\" width=\"35\" height=\"10\"> ");
		//websWrite (wp, "Off");
		websWrite (wp, "%s", live_translate ("wl_basic.radio_off"));
		break;
	case -1:
		//websWrite (wp, "Unknown");
		websWrite (wp, "%s", live_translate ("share.unknown"));
		break;
	}
}


// web-writes html escaped (&#xxx;) nvram entries   / test buffered
int
tf_webWriteESCNV (webs_t wp, const char *nvname)
{
  char buf[512];
  int n;
  int r;
  const char *c;

  n = 0;
  r = 0;
  for (c = nvram_safe_get (nvname); *c; c++)
    {
      if ((isprint (*c)) && (*c != '"') && (*c != '&') && (*c != '<')
	  && (*c != '>') && (*c != '\''))
	{
	  buf[n++] = *c;
	}
      else
	{
	  sprintf (buf + n, "&#%d;", *c);
	  n += strlen (buf + n);
	}
      if (n > (sizeof (buf) - 10))
	{			// ! extra space for &...
	  buf[n] = 0;
	  n = 0;
	  r += wfputs (buf, wp);
	}
    }
  if (n > 0)
    {
      buf[n] = 0;
      r += wfputs (buf, wp);
    }
  wfflush (wp);
  return r;
}


struct ej_handler ej_handlers[] = {
  /* for all */
  {"nvram_get", ej_nvram_get},
/*	{ "nvram_get_len", ej_nvram_get_len }, */
  {"nvram_selget", ej_nvram_selget},
  {"nvram_match", ej_nvram_match},
  {"nvram_invmatch", ej_nvram_invmatch},
  {"nvram_selmatch", ej_nvram_selmatch},
  {"nvram_else_selmatch", ej_nvram_else_selmatch},
  {"nvram_else_match", ej_nvram_else_match},
  {"tran", ej_tran},
  {"nvram_list", ej_nvram_list},
  {"nvram_mac_get", ej_nvram_mac_get},
  {"nvram_gozila_get", ej_nvram_gozila_get},
  {"nvram_status_get", ej_nvram_status_get},
  {"nvram_real_get", ej_nvram_real_get},
  {"webs_get", ej_webs_get},
  {"support_match", ej_support_match},
  {"support_invmatch", ej_support_invmatch},
  {"support_elsematch", ej_support_elsematch},
  {"get_firmware_version", ej_get_firmware_version},
  {"get_firmware_title", ej_get_firmware_title},
  {"get_firmware_svnrev", ej_get_firmware_svnrev},
  {"get_model_name", ej_get_model_name},
  {"showad", ej_showad},
  {"get_single_ip", ej_get_single_ip},
  {"get_single_mac", ej_get_single_mac},
  {"prefix_ip_get", ej_prefix_ip_get},
  {"no_cache", ej_no_cache},
  {"scroll", ej_scroll},
  {"get_dns_ip", ej_get_dns_ip},
  {"onload", ej_onload},
  {"get_web_page_name", ej_get_web_page_name},
  {"show_logo", ej_show_logo},
  {"get_clone_mac", ej_get_clone_mac},
  /* for index */
  {"show_index_setting", ej_show_index_setting},
  {"compile_date", ej_compile_date},
  {"compile_time", ej_compile_time},
  {"get_wl_max_channel", ej_get_wl_max_channel},
  {"get_wl_domain", ej_get_wl_domain},
  /* for status */
  {"show_status", ej_show_status},
  {"show_status_setting", ej_show_status_setting},
  {"localtime", ej_localtime},
  {"dhcp_remaining_time", ej_dhcp_remaining_time},
  {"show_wan_domain", ej_show_wan_domain},
  {"show_wl_mac", ej_show_wl_mac},
  /* for dhcp */
  {"dumpleases", ej_dumpleases},
  /* for ddm */
  /* for log */
  {"dumplog", ej_dumplog},
#ifdef HAVE_SPUTNIK_APD
  {"sputnik_apd_status", ej_sputnik_apd_status},
//  {"show_sputnik", ej_show_sputnik},
#endif
#ifdef HAVE_NEWMEDIA
//  {"show_openvpn", ej_show_openvpn},
  {"show_openvpn_status", ej_show_openvpn_status},
#endif
  /* for filter */
  {"filter_init", ej_filter_init},
  {"filter_summary_show", ej_filter_summary_show},
  {"filter_ip_get", ej_filter_ip_get},
  {"filter_port_get", ej_filter_port_get},
  {"filter_dport_get", ej_filter_dport_get},
  {"filter_mac_get", ej_filter_mac_get},
  {"filter_policy_select", ej_filter_policy_select},
  {"filter_policy_get", ej_filter_policy_get},
  {"filter_tod_get", ej_filter_tod_get},
  {"filter_web_get", ej_filter_web_get},
  {"filter_port_services_get", ej_filter_port_services_get},
  /* for forward */
//  {"port_forward_table", ej_port_forward_table},
//  {"port_forward_spec", ej_port_forward_spec},
//  changed by steve
//  {"forward_upnp", ej_forward_upnp},
//   end changed by steve
  {"port_trigger_table", ej_port_trigger_table},
  /* for route */
  {"static_route_table", ej_static_route_table},
  {"static_route_setting", ej_static_route_setting},
  {"dump_route_table", ej_dump_route_table},
  /* for ddns */
  {"show_ddns_status", ej_show_ddns_status},
//  {"show_ddns_ip", ej_show_ddns_ip},
  /* for wireless */
  {"wireless_active_table", ej_wireless_active_table},
  {"wireless_filter_table", ej_wireless_filter_table},
  {"show_wl_wep_setting", ej_show_wl_wep_setting},
  {"get_wep_value", ej_get_wep_value},
  {"get_wl_active_mac", ej_get_wl_active_mac},
  {"get_wl_value", ej_get_wl_value},

//  {"show_wpa_setting", ej_show_wpa_setting},
  /* for test */
  {"wl_packet_get", ej_wl_packet_get},
  {"wl_ioctl", ej_wl_ioctl},
  {"dump_ping_log", ej_dump_ping_log},
//  {"dump_traceroute_log", ej_dump_traceroute_log},
  {"show_sysinfo", ej_show_sysinfo},
  {"show_miscinfo", ej_show_miscinfo},
  {"get_http_method", ej_get_http_method},
/*        { "get_backup_name", ej_get_backup_name }, */
/*	{ "per_port_option", ej_per_port_option}, */
  {"get_http_prefix", ej_get_http_prefix},
  {"dump_site_survey", ej_dump_site_survey},
  {"show_meminfo", ej_show_meminfo},
  {"get_mtu", ej_get_mtu},
  {"get_url", ej_get_url},
/* Sveasoft additions */
  {"get_wdsp2p", ej_get_wdsp2p},
  {"active_wireless", ej_active_wireless},
#ifdef HAVE_SKYTRON
  {"active_wireless2", ej_active_wireless2},
#endif
  {"active_wds", ej_active_wds},
  {"get_wds_mac", ej_get_wds_mac},
  {"get_wds_ip", ej_get_wds_ip},
  {"get_wds_netmask", ej_get_wds_netmask},
  {"get_wds_gw", ej_get_wds_gw},
  {"get_br1_ip", ej_get_br1_ip},
  {"get_br1_netmask", ej_get_br1_netmask},
  {"get_curchannel", ej_get_curchannel},
  {"get_currate", ej_get_currate},
  {"get_uptime", ej_get_uptime},
  {"get_services_options", ej_get_services_options},
  {"get_clone_wmac", ej_get_clone_wmac},
  {"show_modules", ej_show_modules},
  {"show_styles", ej_show_styles},
  {"show_languages", ej_show_languages},
  {"show_forward", ej_show_forward},
  {"show_forward_spec", ej_show_forward_spec},
  {"show_triggering", ej_show_triggering},
  {"nvram_selected", ej_nvram_selected},
  {"nvram_checked", ej_nvram_checked},
  {"get_qossvcs", ej_get_qossvcs2},
  {"get_qosips", ej_get_qosips2},
  {"get_qosmacs", ej_get_qosmacs2},
//      { "if_config_table",ej_if_config_table},
  {"wme_match_op", ej_wme_match_op},
//      { "show_advanced_qos", ej_show_advanced_qos },
#ifdef FBNFW
  {"list_fbn", ej_list_fbn},
#endif
  {"show_control", ej_show_control},
  {"show_paypal", ej_show_paypal},
  {"show_default_level", ej_show_default_level},
  {"show_staticleases", ej_show_staticleases},
  {"show_security", ej_show_security},
  {"show_dhcpd_settings", ej_show_dhcpd_settings},
  {"show_wds_subnet", ej_show_wds_subnet},
  {"show_infopage", ej_show_infopage},
  {"show_connectiontype", ej_show_connectiontype},
  {"show_routing", ej_show_routing},
#ifdef HAVE_MSSID
  {"show_wireless", ej_show_wireless},
#endif
#ifdef HAVE_RADLOCAL
  {"show_iradius_check", ej_show_iradius_check},
  {"show_iradius", ej_show_iradius},
#endif

#ifdef HAVE_CHILLILOCAL
  {"show_userlist", ej_show_userlist},
#endif
  {"show_cpuinfo", ej_show_cpuinfo},
  {"get_clkfreq", ej_get_clkfreq},
  {"dumpmeminfo", ej_dumpmeminfo},

/* Added by Botho 03.April.06 */
  {"dumpip_conntrack", ej_dumpip_conntrack},
/* Added by Botho 21.April.06 */
  {"js_include", ej_js_include},
  {"css_include", ej_css_include},
/* Added by Botho 10.May.06 */
  {"show_wan_to_switch", ej_show_wan_to_switch},

/* lonewolf additions */
  {"port_vlan_table", ej_port_vlan_table},
/* end lonewolf additions */
#ifdef HAVE_UPNP
  {"tf_upnp", ej_tf_upnp},
#endif
//  {"charset", ej_charset},
  {"do_menu", ej_do_menu},  //Eko
  {"do_pagehead", ej_do_pagehead},	//Eko
  {"do_hpagehead", ej_do_hpagehead},	//Eko
  {"show_timeoptions", ej_show_timeoptions}, //Eko
  {"do_statusinfo", ej_do_statusinfo},	//Eko
  {"show_clocks", ej_show_clocks},
  {"getrebootflags", ej_getrebootflags},
  {"getwirelessmode", ej_getwirelessmode},
  {"getwirelessnetmode", ej_getwirelessnetmode},
  {"get_radio_state", ej_get_radio_state},
#ifdef HAVE_MSSID
  {"getwirelessstatus", ej_getwirelessstatus},
  {"getencryptionstatus", ej_getencryptionstatus},
  {"get_txpower",ej_get_txpower},
#endif
#ifdef HAVE_CPUTEMP
  {"get_cputemp", ej_get_cputemp},
  {"show_cpu_temperature", ej_show_cpu_temperature},
#endif
#ifdef HAVE_VOLT
  {"get_voltage", ej_get_voltage},
  {"show_voltage", ej_show_voltage},
#endif

  {NULL, NULL}
};
#endif /* !WEBS */

#ifdef HAVE_UPNP
// changed by steve
// writes javascript-string safe text
static int
tf_webWriteJS (webs_t wp, const char *s)
{
  char buf[512];
  int n;
  int r;

  n = 0;
  r = 0;
  for (; *s; s++)
    {
      if ((*s != '"') && (*s != '\\') && (*s != '\'') && (isprint (*s)))
	{
	  buf[n++] = *s;
	}
      else
	{
	  sprintf (buf + n, "\\x%02x", *s);
	  n += 4;
	}
      if (n > (sizeof (buf) - 10))
	{			// ! extra space for \xHH
	  buf[n] = 0;
	  n = 0;
	  r += wfputs (buf, wp);
	}
    }
  if (n > 0)
    {
      buf[n] = 0;
      r += wfputs (buf, wp);
    }
  wfflush (wp);
  return r;
}

// handle UPnP.asp requests / added 10
static int
tf_upnp (webs_t wp)
{
  char *v;
  char s[64];

  if (((v = websGetVar (wp, "remove", NULL)) != NULL) && (*v))
    {
      if (strcmp (v, "all") == 0)
	{
	  nvram_set ("upnp_clear", "1");
	}
      else
	{
	  sprintf (s, "forward_port%s", v);
	  nvram_unset (s);
	}
    }
  // firewall + upnp service is restarted after this
  return 0;
}

//      <% tf_upnp(); %>
//      returns all "forward_port#" nvram entries containing upnp port forwardings
static void
ej_tf_upnp (int eid, webs_t wp, int argc, char_t ** argv)
{
  int i;
  char s[32];

  if (nvram_match ("upnp_enable", "1"))
    {
      for (i = 0; i < 50; i++)
	{
	  websWrite (wp, (i > 0) ? ",'" : "'");
	  sprintf (s, "forward_port%d", i);
	  tf_webWriteJS (wp, nvram_safe_get (s));
	  websWrite (wp, "'");
	}
    }

  return;

}

// end changed by steve
#endif
