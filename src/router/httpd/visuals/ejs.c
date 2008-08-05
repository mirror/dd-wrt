#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/statfs.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <broadcom.h>
#include <cymac.h>
#include <wlutils.h>
#include <bcmparams.h>
#include <dirent.h>
#include <netdb.h>
#include <utils.h>
#include <bcmnvram.h>




void (*do_ej_buffer) (char *buffer, webs_t stream) = NULL;
int (*httpd_filter_name) (char *old_name, char *new_name, size_t size,
			  int type) = NULL;
char *(*websGetVar) (webs_t wp, char *var, char *d) = NULL;
int (*websWrite) (webs_t wp, char *fmt, ...) = NULL;
struct wl_client_mac *wl_client_macs = NULL;
void (*do_ej) (char *path, webs_t stream, char *query) = NULL;	// jimmy, https, 8/4/2003
int (*ejArgs) (int argc, char_t ** argv, char_t * fmt, ...) = NULL;
FILE *(*getWebsFile) (char *path) = NULL;
int (*wfflush) (FILE * fp) = NULL;
int (*wfputc) (char c, FILE * fp) = NULL;
int (*wfputs) (char *buf, FILE * fp) = NULL;
char *(*live_translate) (char *tran) = NULL;
websRomPageIndexType *websRomPageIndex = NULL;
char *(*GOZILA_GET)(webs_t wp,char *name) = NULL;


#ifdef HAVE_HTTPS
int do_ssl;
#endif
int gozila_action;
int browser_method;

void
initWeb (struct Webenvironment *env)
{

  cprintf ("set websgetwar %p:%p->%p:%p\n", env, env->PwebsGetVar,
	   &websGetVar, websGetVar);
  websGetVar = env->PwebsGetVar;
  httpd_filter_name = env->Phttpd_filter_name;
  cprintf ("set wl_client_macs\n");
  wl_client_macs = env->Pwl_client_macs;
  cprintf ("set webswrite\n");
  websWrite = env->PwebsWrite;
  cprintf ("set do_ej_buffer\n");
  do_ej_buffer = env->Pdo_ej_buffer;
  cprintf ("set do_ej\n");
  do_ej = env->Pdo_ej;
#ifdef HAVE_HTTPS
  cprintf ("set do_ssl\n");
  do_ssl = env->Pdo_ssl;
#endif
  cprintf ("set ejargs\n");
  ejArgs = env->PejArgs;
  cprintf ("set getwebsfile\n");
  getWebsFile = env->PgetWebsFile;
  cprintf ("set wwflush\n");
  wfflush = env->Pwfflush;
  cprintf ("set wfputs\n");
  wfputc = env->Pwfputc;
  cprintf ("set wfputs\n");
  wfputs = env->Pwfputs;
  cprintf ("set websrompageindex\n");
  websRomPageIndex = env->PwebsRomPageIndex;
  cprintf ("set gozila_action\n");
  gozila_action = env->Pgozila_action;
  cprintf ("set browser_method\n");
  browser_method = env->Pbrowser_method;
  cprintf ("set live_translate\n");
  live_translate = env->Plive_translate;
  GOZILA_GET = env->PGOZILA_GET;
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
ej_onload (webs_t wp, int argc, char_t ** argv)
{
  char *type, *arg;
  struct onload *v;

#ifndef FASTWEB
  if (argc < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif
  type = argv[0];
  arg = argv[1];

  for (v = onloads; v < &onloads[STRUCT_LEN (onloads)]; v++)
    {
      if (!strcmp (v->name, type))
	{
	  v->go (wp, arg);
	  return;
	}
    }

  return;
}

/* Meta tag command that will no allow page cached by browsers.
 * The will force the page to be refreshed when visited.
 */
void
ej_no_cache (webs_t wp, int argc, char_t ** argv)
{
  websWrite (wp, "<meta http-equiv=\"expires\" content=\"0\">\n");
  websWrite (wp,
	     "<meta http-equiv=\"cache-control\" content=\"no-cache\">\n");
  websWrite (wp, "<meta http-equiv=\"pragma\" content=\"no-cache\">\n");

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

/*
 * Example:
 * lan_ipaddr=192.168.1.1
 * <% prefix_ip_get("lan_ipaddr",1); %> produces "192.168.1."
 */
void
ej_prefix_ip_get (webs_t wp, int argc, char_t ** argv)
{
  char *name;
  int type;

#ifndef FASTWEB
  if (argc < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif
  name = argv[0];
  type = atoi (argv[1]);

  if (type == 1)
    websWrite (wp, "%d.%d.%d.", get_single_ip (nvram_safe_get (name), 0),
	       get_single_ip (nvram_safe_get (name), 1),
	       get_single_ip (nvram_safe_get (name), 2));
  if (type == 2)
    websWrite (wp, "%d.%d.", get_single_ip (nvram_safe_get (name), 0),
	       get_single_ip (nvram_safe_get (name), 1));

  return;
}


/*
 * Example:
 * lan_ipaddr = 192.168.1.1
 * <% nvram_get("lan_ipaddr"); %> produces "192.168.1.1"
 */
void
ej_nvram_get (webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
  if (argc < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif

#if COUNTRY == JAPAN
  websWrite (wp, "%s", nvram_safe_get (argv[0]));
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

  tf_webWriteESCNV (wp, argv[0]);	// test: buffered version of above

  return;
#endif

  return;
}

void
ej_nvram_real_get (webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
  if (argc < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif
  websWrite (wp, "%s", nvram_safe_get (argv[0]));

  return;
}

/*
 * Example:
 * lan_ipaddr = 192.168.1.1, gozila_action = 0
 * <% nvram_selget("lan_ipaddr"); %> produces "192.168.1.1"
 * lan_ipaddr = 192.168.1.1, gozila_action = 1, websGetVar(wp, "lan_proto", NULL) = 192.168.1.2;
 * <% nvram_selget("lan_ipaddr"); %> produces "192.168.1.2"
 */
void
ej_nvram_selget (webs_t wp, int argc, char_t ** argv)
{
  char *name;

#ifndef FASTWEB
  if (argc < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif
  name = argv[0];
  if (gozila_action)
    {
      char *buf = websGetVar (wp, name, NULL);
      if (buf)
	{
	  websWrite (wp, "%s", buf);
	  return;
	}
    }
  tf_webWriteESCNV (wp, name);	// test: buffered version of above

  return;
}

/*
 * Example:
 * wan_mac = 00:11:22:33:44:55
 * <% nvram_mac_get("wan_mac"); %> produces "00-11-22-33-44-55"
 */
void
ej_nvram_mac_get (webs_t wp, int argc, char_t ** argv)
{
  char *c;
  char *mac;
  int i;

#ifndef FASTWEB
  if (argc < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif
  c = nvram_safe_get (argv[0]);

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
void
ej_nvram_gozila_get (webs_t wp, int argc, char_t ** argv)
{
  char *type;

#ifndef FASTWEB
  if (argc < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif
  type = GOZILA_GET (wp,argv[0]);

  websWrite (wp, "%s", type);
}

void
ej_webs_get (webs_t wp, int argc, char_t ** argv)
{
  char *value;

#ifndef FASTWEB
  if (argc < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif

  value = websGetVar (wp, argv[0], NULL);

  if (value)
    websWrite (wp, "%s", value);

  return;
}

/*
 * Example:
 * lan_ipaddr = 192.168.1.1
 * <% get_single_ip("lan_ipaddr","1"); %> produces "168"
 */
void
ej_get_single_ip (webs_t wp, int argc, char_t ** argv)
{
  char *c;

#ifndef FASTWEB
  if (argc < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif

  c = nvram_safe_get (argv[0]);
  if (c)
    {
      if (!strcmp (c, PPP_PSEUDO_IP) || !strcmp (c, PPP_PSEUDO_GW))
	c = "0.0.0.0";
      else if (!strcmp (c, PPP_PSEUDO_NM))
	c = "255.255.255.0";

      websWrite (wp, "%d", get_single_ip (c, atoi (argv[1])));
    }
  else
    websWrite (wp, "0");

  return;
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

/*
 * Example:
 * wan_mac = 00:11:22:33:44:55
 * <% get_single_mac("wan_mac","1"); %> produces "11"
 */
void
ej_get_single_mac (webs_t wp, int argc, char_t ** argv)
{
  char *c;
  int mac;

#ifndef FASTWEB
  if (argc < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif

  c = nvram_safe_get (argv[0]);
  if (c)
    {
      mac = get_single_mac (c, atoi (argv[1]));
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
  char *type = GOZILA_GET (wp,name);
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
ej_nvram_selmatch (webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
  if (argv < 3)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif
  if (nvram_selmatch (wp, argv[0], argv[1]))
    {
      websWrite (wp, argv[2]);
    }
  return;
}

void
ej_nvram_else_selmatch (webs_t wp, int argc, char_t ** argv)
{
  char *type;

#ifndef FASTWEB
  if (argc < 4)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif

  type = GOZILA_GET (wp,argv[0]);

  if (!type)
    {
      if (nvram_match (argv[0], argv[1]))
	{
	  websWrite (wp, argv[2]);
	}
      else
	websWrite (wp, argv[3]);
    }
  else
    {
      if (!strcmp (type, argv[1]))
	{
	  websWrite (wp, argv[2]);
	}
      else
	websWrite (wp, argv[3]);
    }

  return;
}

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_else_match("wan_proto", "dhcp", "0","1"); %> produces "0"
 * <% nvram_else_match("wan_proto", "static", "0","1"); %> produces "1"
 */
void
ej_nvram_else_match (webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
  if (argc < 4)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif

  if (nvram_match (argv[0], argv[1]))
    websWrite (wp, argv[2]);
  else
    websWrite (wp, argv[3]);

  return;
}



void
ej_startswith (webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
  if (argc < 3)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif
  if (startswith (nvram_safe_get (argv[0]), argv[1]))
    websWrite (wp, argv[2]);

  return;
}

void
ej_ifdef (webs_t wp, int argc, char_t ** argv)
{
  char *name, *output;

#ifndef FASTWEB
  if (argc < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif
  name = argv[0];
  output = argv[1];
#ifdef HAVE_MICRO
  if (!strcmp (name, "MICRO"))
    {
      websWrite (wp, output);
      return;
    }
#endif
#ifdef HAVE_EXTHELP
  if (!strcmp (name, "EXTHELP"))
    {
      websWrite (wp, output);
      return;
    }
#endif
  if (!strcmp (name, "MINI"))	//to include mini + mini-special
    {
      if (startswith (nvram_safe_get ("dist_type"), "mini"))
	{
	  websWrite (wp, output);
	  return;
	}
    }
  if (!strcmp (name, "VPN"))	//to include vpn + vpn-special
    {
      if (startswith (nvram_safe_get ("dist_type"), "vpn"))
	{
	  websWrite (wp, output);
	  return;
	}
    }
#ifdef HAVE_MULTICAST
  if (!strcmp (name, "MULTICAST"))
    {
      websWrite (wp, output);
      return;
    }
#endif
#ifdef HAVE_WIVIZ
  if (!strcmp (name, "WIVIZ"))
    {
      websWrite (wp, output);
      return;
    }
#endif
#ifdef HAVE_RSTATS
  if (!strcmp (name, "RSTATS"))
    {
      websWrite (wp, output);
      return;
    }
#endif
#ifdef HAVE_ACK
  if (!strcmp (name, "ACK"))
    {
      websWrite (wp, output);
      return;
    }
#endif
#ifdef HAVE_SSHD
  if (!strcmp (name, "SSHD"))
    {
      websWrite (wp, output);
      return;
    }
#endif

  return;
}

void
ej_ifndef (webs_t wp, int argc, char_t ** argv)
{
  char *name, *output;

#ifndef FASTWEB
  if (argc < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif
  name = argv[0];
  output = argv[1];

#ifdef HAVE_MICRO
  if (!strcmp (name, "MICRO"))
    return;
#endif
#ifdef HAVE_MULTICAST
  if (!strcmp (name, "MULTICAST"))
    return;
#endif
#ifdef HAVE_WIVIZ
  if (!strcmp (name, "WIVIZ"))
    return;
#endif
#ifdef HAVE_RSTATS
  if (!strcmp (name, "RSTATS"))
    return;
#endif
#ifdef HAVE_ACK
  if (!strcmp (name, "ACK"))
    return;
#endif
#ifdef HAVE_SAMBA
  if (!strcmp (name, "SAMBA"))
    return;
#endif
#ifdef HAVE_JFFS2
  if (!strcmp (name, "JFFS2"))
    return;
#endif
#ifdef HAVE_GPSI
  if (!strcmp (name, "GPSI"))
    return;
#endif
#ifdef HAVE_MMC
  if (!strcmp (name, "MMC"))
    return;
#endif
#ifdef HAVE_SPUTNIK_APD
  if (!strcmp (name, "SPUTNIK_APD"))
    return;
#endif
#ifdef HAVE_RFLOW
  if (!strcmp (name, "RFLOW"))
    return;
#endif
#ifdef HAVE_USB
  if (!strcmp (name, "USB"))
    return;
#endif
#ifdef HAVE_SSHD
  if (!strcmp (name, "SSHD"))
    return;
#endif
#ifdef HAVE_PPPOESERVER
  if (!strcmp (name, "PPPOESERVER"))
    return;
#endif
#ifdef HAVE_MILKFISH
  if (!strcmp (name, "MILKFISH"))
    return;
#endif
// HAVE_AFTERBURNER
  if (!strcmp (name, "AFTERBURNER"))
    {
#ifdef HAVE_MADWIFI
      return;
#else
      int afterburner = 0;
      char cap[WLC_IOCTL_SMLEN];
      char caps[WLC_IOCTL_SMLEN];
      char *name = nvram_safe_get ("wl0_ifname");
      char *next;

      if (wl_iovar_get (name, "cap", (void *) caps, WLC_IOCTL_SMLEN) == 0)
	{
	  foreach (cap, caps, next)
	  {
	    if (!strcmp (cap, "afterburner"))
	      afterburner = 1;
	  }

	  if (afterburner)
	    return;
	}
#endif
    }
// end HAVE_AFTERBURNER
// HAVE_HASWIFI
  if (!strcmp (name, "HASWIFI"))
    {
      if (haswifi ())
	return;
    }
// end HAVE_HASWIFI

  websWrite (wp, output);

  return;
}

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_match("wan_proto", "dhcp", "selected"); %> produces "selected"
 * <% nvram_match("wan_proto", "static", "selected"); %> does not produce
 */
void
ej_nvram_match (webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
  if (argc < 3)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif

  if (nvram_match (argv[0], argv[1]))
    websWrite (wp, argv[2]);

  return;
}


/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_invmatch("wan_proto", "dhcp", "disabled"); %> does not produce
 * <% nvram_invmatch("wan_proto", "static", "disabled"); %> produces "disabled"
 */
void
ej_nvram_invmatch (webs_t wp, int argc, char_t ** argv)
{
  char *name, *invmatch, *output;

#ifdef FASTWEB
  ejArgs (argc, argv, "%s %s %s", &name, &invmatch, &output);
#else
  if (ejArgs (argc, argv, "%s %s %s", &name, &invmatch, &output) < 3)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif

  if (!nvram_match (name, invmatch))
    websWrite (wp, output);

  return;
}

/*
static void
ej_scroll (webs_t wp, int argc, char_t ** argv)
{
  char *type;
  int y;

#ifdef FASTWEB
  ejArgs (argc, argv, "%s %d", &type, &y);
#else
  if (ejArgs (argc, argv, "%s %d", &type, &y) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif
  if (gozila_action)
    websWrite (wp, "%d", y);
  else
    websWrite (wp, "0");

  return;
}
*/
/*
 * Example:
 * filter_mac=00:12:34:56:78:00 00:87:65:43:21:00
 * <% nvram_list("filter_mac", 1); %> produces "00:87:65:43:21:00"
 * <% nvram_list("filter_mac", 100); %> produces ""
 */
void
ej_nvram_list (webs_t wp, int argc, char_t ** argv)
{
  int which;
  char word[256], *next;

#ifndef FASTWEB
  if (argc < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif
  which = atoi (argv[1]);
  char *list = nvram_safe_get (argv[0]);
  foreach (word, list, next)
  {
    if (which-- == 0)
      websWrite (wp, word);
  }

  return;
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
  char *list = nvram_safe_get (name);
  foreach (word, list, next)
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
 * wan_dns = 168.95.1.1 210.66.161.125 168.95.192.1
 * <% get_dns_ip("wan_dns", "1", "2"); %> produces "161"
 * <% get_dns_ip("wan_dns", "2", "3"); %> produces "1"
 */
void
ej_get_dns_ip (webs_t wp, int argc, char_t ** argv)
{
  int which;
  char word[256], *next;

#ifndef FASTWEB
  if (argc < 3)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif
  which = atoi (argv[1]);
  char *list = nvram_safe_get (argv[0]);
  foreach (word, list, next)
  {
    if (which-- == 0)
      {
	websWrite (wp, "%d", get_single_ip (word, atoi (argv[2])));
	return;
      }
  }

  websWrite (wp, "0");		// not find
}


void
ej_get_http_prefix (webs_t wp, int argc, char_t ** argv)
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
ej_get_mtu (webs_t wp, int argc, char_t ** argv)
{
  struct mtu_lists *mtu_list;
  char *type;
  char *proto = GOZILA_GET (wp,"wan_proto");
  type = argv[1];
#ifndef FASTWEB
  if (argc < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif

  mtu_list = get_mtu (proto);

  if (!strcmp (type, "min"))
    websWrite (wp, "%s", mtu_list->min);
  else if (!strcmp (type, "max"))
    websWrite (wp, "%s", mtu_list->max);

  return;
}

void
ej_show_forward (webs_t wp, int argc, char_t ** argv)
//ej_show_forward(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg,
//        char_t *url, char_t *path, char_t *query)
{
  int i;
  char *count;
  int c = 0;
  count = nvram_safe_get ("forward_entries");
  if (count == NULL || strlen (count) == 0 || (c = atoi (count)) <= 0)
    {
      //return -1;      botho 07/03/06 add "- None -" if empty
      websWrite (wp, "<tr>\n");
      websWrite (wp,
		 "<td colspan=\"6\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n");
      websWrite (wp, "</tr>\n");
    }
  for (i = 0; i < c; i++)
    {
      websWrite (wp, "<tr><td>\n");
      websWrite (wp,
		 "<input maxlength=\"12\" size=\"12\" name=\"name%d\" onblur=\"valid_name(this,'Name')\" value=\"",
		 i);
      port_forward_table (wp, "name", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      websWrite (wp,
		 "<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"from%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
		 i);
      port_forward_table (wp, "from", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      websWrite (wp,
		 "<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"to%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
		 i);
      port_forward_table (wp, "to", i);
      websWrite (wp, "\"/></td>\n");
      websWrite (wp, "<td><select size=\"1\" name=\"pro%d\">\n", i);
      websWrite (wp, "<option value=\"tcp\" ");
      port_forward_table (wp, "sel_tcp", i);
      websWrite (wp, ">TCP</option>\n");
      websWrite (wp, "<option value=\"udp\" ");
      port_forward_table (wp, "sel_udp", i);
      websWrite (wp, ">UDP</option>\n");
      websWrite (wp, "<script type=\"text/javascript\">\n//<![CDATA[\n\
		 		document.write(\"<option value=\\\"both\\\" ");
      port_forward_table (wp, "sel_both", i);
      websWrite (wp, " >\" + share.both + \"</option>\");\n\
      	\n//]]>\n</script>\n");
      websWrite (wp, "</select></td>\n");
      websWrite (wp, "<td>\n");
      websWrite (wp,
		 "<input class=\"num\" maxlength=\"15\" size=\"15\" name=\"ip%d\" value=\"",
		 i);
      port_forward_table (wp, "ip", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      websWrite (wp,
		 "<input type=\"checkbox\" value=\"on\" name=\"enable%d\" ",
		 i);
      port_forward_table (wp, "enable", i);
      websWrite (wp, " /></td>\n");
      websWrite (wp, "</tr>\n");
    }
  return;
}

void
ej_show_forward_spec (webs_t wp, int argc, char_t ** argv)
//ej_show_forward(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg,
//        char_t *url, char_t *path, char_t *query)
{
  int i;
  char *count;
  int c = 0;
  count = nvram_safe_get ("forwardspec_entries");
  if (count == NULL || strlen (count) == 0 || (c = atoi (count)) <= 0)
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
      websWrite (wp,
		 "<input maxlength=\"12\" size=\"12\" name=\"name%d\" onblur=\"valid_name(this,'Name')\" value=\"",
		 i);
      port_forward_spec (wp, "name", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      websWrite (wp,
		 "<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"from%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
		 i);
      port_forward_spec (wp, "from", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td><select size=\"1\" name=\"pro%d\">\n", i);
      websWrite (wp, "<option value=\"tcp\" ");
      port_forward_spec (wp, "sel_tcp", i);
      websWrite (wp, ">TCP</option>\n");
      websWrite (wp, "<option value=\"udp\" ");
      port_forward_spec (wp, "sel_udp", i);
      websWrite (wp, ">UDP</option>\n");
      websWrite (wp, "<script type=\"text/javascript\">\n//<![CDATA[\n\
		 		document.write(\"<option value=\\\"both\\\" ");
      port_forward_spec (wp, "sel_both", i);
      websWrite (wp, " >\" + share.both + \"</option>\");\n\
      		\n//]]>\n</script>\n");
      websWrite (wp, "</select></td>\n");
      websWrite (wp, "<td>\n");
      websWrite (wp,
		 "<input class=\"num\" maxlength=\"15\" size=\"15\" name=\"ip%d\" value=\"",
		 i);
      port_forward_spec (wp, "ip", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      websWrite (wp,
		 "<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"to%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
		 i);
      port_forward_spec (wp, "to", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td>\n");
      websWrite (wp,
		 "<input type=\"checkbox\" value=\"on\" name=\"enable%d\" ",
		 i);
      port_forward_spec (wp, "enable", i);
      websWrite (wp, " /></td>\n");
      websWrite (wp, "</tr>\n");
    }
  return;
}

void
ej_show_triggering (webs_t wp, int argc, char_t ** argv)
{
  int i;
  char *count;
  int c = 0;
  count = nvram_safe_get ("trigger_entries");
  if (count == NULL || strlen (count) == 0 || (c = atoi (count)) <= 0)
    {
      websWrite (wp, "<tr>\n");
      websWrite (wp,
		 "<td colspan=\"6\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n");
      websWrite (wp, "</tr>\n");
    }
  for (i = 0; i < c; i++)
    {
      websWrite (wp, "<tr>\n");
      websWrite (wp,
		 "<td><input maxlength=\"12\" size=\"12\" name=\"name%d\" onblur=\"valid_name(this,'Name')\" value=\"",
		 i);
      port_trigger_table (wp, "name", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp,
		 "<td><input class=\"num\" maxlength=\"5\" size=\"5\" name=\"i_from%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
		 i);
      port_trigger_table (wp, "i_from", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp,
		 "<td><input class=\"num\" maxlength=\"5\" size=\"5\" name=\"i_to%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
		 i);
      port_trigger_table (wp, "i_to", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp, "<td><select size=\"1\" name=\"pro%d\">\n", i);
      websWrite (wp, "<option value=\"tcp\" ");
      port_trigger_table (wp, "sel_tcp", i);
      websWrite (wp, ">TCP</option>\n");
      websWrite (wp, "<option value=\"udp\" ");
      port_trigger_table (wp, "sel_udp", i);
      websWrite (wp, ">UDP</option>\n");
      websWrite (wp, "<script type=\"text/javascript\">\n//<![CDATA[\n\
		 		document.write(\"<option value=\\\"both\\\" ");
      port_trigger_table (wp, "sel_both", i);
      websWrite (wp, " >\" + share.both + \"</option>\");\n\
      		\n//]]>\n</script>\n");
      websWrite (wp, "</select></td>\n");
      websWrite (wp,
		 "<td><input class=\"num\" maxlength=\"5\" size=\"5\" name=\"o_from%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
		 i);
      port_trigger_table (wp, "o_from", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp,
		 "<td><input class=\"num\" maxlength=\"5\" size=\"5\" name=\"o_to%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"",
		 i);
      port_trigger_table (wp, "o_to", i);
      websWrite (wp, "\" /></td>\n");
      websWrite (wp,
		 "<td><input type=\"checkbox\" value=\"on\" name=\"enable%d\" ",
		 i);
      port_trigger_table (wp, "enable", i);
      websWrite (wp, " /></td>\n");
      websWrite (wp, "</tr>\n");
    }
  return;
}

//SEG DD-WRT addition
void
ej_show_styles (webs_t wp, int argc, char_t ** argv)
{
//<option value="blue" <% nvram_selected("router_style", "blue"); %>>Blue</option>
  DIR *directory;
  char buf[256];
  directory = opendir ("/www/style");
  struct dirent *entry;
  while ((entry = readdir (directory)) != NULL)
    {
      sprintf (buf, "style/%s/style.css", entry->d_name);
      FILE *web = getWebsFile (buf);
      if (web == NULL)
	{
	  sprintf (buf, "/www/style/%s/style.css", entry->d_name);
	  FILE *test = fopen (buf, "rb");
	  if (test == NULL)
	    continue;
	  fclose (test);
	}
      fclose (web);

      websWrite (wp, "<option value=\"%s\" %s>%s</option>\n", entry->d_name,
		 nvram_match ("router_style",
			      entry->d_name) ? "selected=\"selected\"" : "",
		 entry->d_name);
    }
  closedir (directory);
  return;
}

#ifdef HAVE_LANGUAGE
//extern websRomPageIndexType websRomPageIndex[];
void
ej_show_languages (webs_t wp, int argc, char_t ** argv)
{
  char buf[256];
  websWrite (wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
  int i = 0;
  while (websRomPageIndex[i].path != NULL)
    {
      cprintf ("checking %s\n", websRomPageIndex[i].path);
      if (!strncmp
	  (websRomPageIndex[i].path, "lang_pack/", strlen ("lang_pack/")))
	{
	  cprintf ("found language\n");
	  if (strlen (websRomPageIndex[i].path) < 14)
	    continue;
	  strcpy (buf, websRomPageIndex[i].path);
	  char *mybuf = &buf[strlen ("lang_pack/")];
	  mybuf[strlen (mybuf) - 3] = 0;	//strip .js
	  websWrite (wp,
		     "document.write(\"<option value=\\\"%s\\\" %s >\" + management.lang_%s + \"</option>\");\n",
		     mybuf, nvram_match ("language",
					 mybuf) ? "selected=\\\"selected\\\""
		     : "", mybuf);
	}
      i++;
    }
  websWrite (wp, "//]]>\n</script>\n");
  return;
}
#endif

static char *directories[] = {
  "/etc/config",
  "/jffs/etc/config",
  "/mmc/etc/config"
};

void
ej_show_modules (webs_t wp, int argc, char_t ** argv)
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
		  do_ej (buf, wp, NULL);
		}
	    }
	  else
	    {
	      if (endswith (entry->d_name, ".webconfig"))
		{
		  sprintf (buf, "%s/%s", directories[idx], entry->d_name);
		  do_ej (buf, wp, NULL);
		}
	    }
	}
      closedir (directory);
    }
  return;
}

void
ej_get_totaltraff (webs_t wp, int argc, char_t ** argv)
{
  char *type;
  static char wanface[32];
  char line[256];
  unsigned long rcvd, sent, megcounti, megcounto;
  FILE *in;

#ifndef FASTWEB
  if (argc < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif
  type = argv[0];

  if (!nvram_match ("ttraff_enable", "1"))
    return;

  strncpy (wanface, get_wan_face (), sizeof (wanface));

  in = fopen ("/proc/net/dev", "rb");
  if (in == NULL)
    return;

  while (fgets (line, sizeof (line), in) != NULL)
    {
      int ifl = 0;
      if (!strchr (line, ':'))
	continue;
      while (line[ifl] != ':')
	ifl++;
      line[ifl] = 0;		/* interface */
      if (strstr (line, wanface))
	{
	  sscanf (line + ifl + 1,
		  "%lu %*ld %*ld %*ld %*ld %*ld %*ld %*ld %lu %*ld %*ld %*ld %*ld %*ld %*ld %*ld",
		  &rcvd, &sent);

	}
    }

  fclose (in);

  rcvd >>= 20;			//output in MBytes
  sent >>= 20;


  if ((in = fopen ("/tmp/.megc", "r")) != NULL)
    {
      fgets (line, sizeof (line), in);
      sscanf (line, "%lu:%lu", &megcounti, &megcounto);
      rcvd += megcounti;
      sent += megcounto;
      fclose (in);
    }

  if (!strcmp (type, "in"))
    {
      websWrite (wp, "%lu", rcvd);	//output in MBytes
    }
  else if (!strcmp (type, "out"))
    {
      websWrite (wp, "%lu", sent);
    }
  return;
}

void
show_bwif (webs_t wp, char *ifname, char *name)
{
  websWrite (wp, "<h2>%s - %s</h2>\n", live_translate ("status_band.h2"),
	     name);
  websWrite (wp, "<fieldset>\n");
  websWrite (wp,
	     "<iframe src=\"/graph_if.svg?%s\" width=\"555\" height=\"275\" frameborder=\"0\" type=\"image/svg+xml\">\n",
	     ifname);
  websWrite (wp, "</iframe>\n");
  websWrite (wp, "</fieldset>\n");
  websWrite (wp, "<br />\n");
}


void
ej_show_bandwidth (webs_t wp, int argc, char_t ** argv)
{
  show_bwif (wp, nvram_safe_get ("lan_ifname"), "LAN");
  if (!nvram_match ("wan_proto", "disabled"))
    {
      if (strlen (nvram_safe_get ("wan_iface")) > 0)
	{
	  show_bwif (wp, nvram_safe_get ("wan_iface"), "WAN");
	}
      else
	{
	  if (strlen (nvram_safe_get ("wan_ifname")) > 0)
	    {
	      show_bwif (wp, nvram_safe_get ("wan_ifname"), "WAN");
	    }
	}
    }
#ifdef HAVE_MADWIFI
  char var[80];
  char *next;
  int c = getdevicecount ();
  int i;
  for (i = 0; i < c; i++)
    {
      char dev[32];
      sprintf (dev, "ath%d", i);
      char name[32];
      sprintf (name, "%s (%s)", live_translate ("share.wireless"), dev);
      show_bwif (wp, dev, name);
      char *vifs = nvram_nget ("%s_vifs", dev);
      if (vifs == NULL)
	continue;
      foreach (var, vifs, next)
      {
	sprintf (name, "%s (%s)", live_translate ("share.wireless"), var);
	show_bwif (wp, var, name);
      }
      int s;
      for (s = 1; s <= 10; s++)
	{
	  char *wdsdev;
	  wdsdev = nvram_nget ("%s_wds%d_if", dev, s);
	  if (strlen (wdsdev) == 0)
	    continue;
	  if (nvram_nmatch ("0", "%s_wds%d_enable", dev, s))
	    continue;
	  sprintf (name, "%s (%s)", live_translate ("share.wireless"),
		   wdsdev);
	  show_bwif (wp, wdsdev, name);
	}

    }

#else
  char name[32];
  sprintf (name, "%s", live_translate ("share.wireless"));
  show_bwif (wp, get_wdev (), name);
#endif
#ifdef HAVE_WAVESAT
  char name[32];
  sprintf (name, "%s", live_translate ("wl_wimax.titl"));
  show_bwif (wp, "ofdm", name);
#endif
}

void
ej_get_http_method (webs_t wp, int argc, char_t ** argv)
{
  websWrite (wp, "%s", "post");
}

void
ej_do_menu (webs_t wp, int argc, char_t ** argv)
{
  char *mainmenu, *submenu;

#ifndef FASTWEB
  if (argc < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif
  mainmenu = argv[0];
  submenu = argv[1];

  int vlan_supp = check_vlan_support ();
#ifdef HAVE_SPUTNIK_APD
  int sputnik = nvram_match ("apd_enable", "1");
#else
  int sputnik = 0;
#endif
  int openvpn =
    nvram_match ("openvpn_enable", "1") | nvram_match ("openvpncl_enable",
						       "1");
  int auth = nvram_match ("status_auth", "1");
#ifdef HAVE_MADWIFI
#ifdef HAVE_NOWIFI
  int wifi = 0;
#else
  int wifi = haswifi ();
#endif
#endif
  int wimaxwifi = 0;
  char menu[8][11][32] =
    { {"index.asp", "DDNS.asp", "WanMAC.asp", "Routing.asp", "Vlan.asp",
       "Networking.asp", "", "", "", "", ""},
  {"Wireless_Basic.asp", "SuperChannel.asp", "WiMAX.asp",
   "Wireless_radauth.asp", "WL_WPATable.asp",
   "Wireless_MAC.asp", "Wireless_Advanced.asp", "Wireless_WDS.asp", "", "",
   ""},
  {"Services.asp", "PPPoE_Server.asp", "PPTP.asp", "Hotspot.asp",
   "Milkfish.asp", "eop-tunnel.asp", "AnchorFree.asp", "", "", "", ""},
  {"Firewall.asp", "VPN.asp", "", "", "", "", "", "", "", "", ""},
  {"Filters.asp", "", "", "", "", "", "", "", "", "", ""},
  {"ForwardSpec.asp", "Forward.asp", "Triggering.asp", "UPnP.asp", "DMZ.asp",
   "QoS.asp", "P2P.asp", "", "", "", ""},
  {"Management.asp", "Alive.asp", "Diagnostics.asp", "Wol.asp",
   "Factory_Defaults.asp", "Upgrade.asp", "config.asp", "", "", "", ""},
  {"Status_Router.asp", "Status_Internet.asp", "Status_Lan.asp",
   "Status_Wireless.asp",
   "Status_SputnikAPD.asp", "Status_OpenVPN.asp", "Status_Bandwidth.asp",
   "Info.htm", "", "", ""}
  };

/* real name is bmenu.menuname[i][j] */
  char menuname[8][12][32] =
    { {"setup", "setupbasic", "setupddns", "setupmacclone", "setuprouting",
       "setupvlan", "networking", "", "", "", "", ""},
  {"wireless", "wirelessBasic", "wirelessSuperchannel", "wimax",
   "wirelessRadius", "wirelessSecurity",
   "wirelessMac", "wirelessAdvanced", "wirelessWds", "", "", ""},
  {"services", "servicesServices", "servicesPppoesrv", "servicesPptp",
   "servicesHotspot", "servicesMilkfish", "setupeop", "servicesAnchorFree",
   "", "", "", ""},
  {"security", "firwall", "vpn", "", "", "", "", "", "", "", "", ""},
  {"accrestriction", "webaccess", "", "", "", "", "", "", "", "", "", ""},
  {"applications", "applicationspforwarding", "applicationsprforwarding",
   "applicationsptriggering", "applicationsUpnp", "applicationsDMZ",
   "applicationsQoS", "applicationsP2P", "", "", "", ""},
  {"admin", "adminManagement", "adminAlive",
   "adminDiag", "adminWol", "adminFactory", "adminUpgrade", "adminBackup",
   "", "", "", ""},
  {"statu", "statuRouter", "statuInet", "statuLAN", "statuWLAN",
   "statuSputnik",
   "statuVPN", "statuBand", "statuSysInfo", "", "", ""}
  };

#ifdef HAVE_MADWIFI
  //fill up WDS
  int ifcount = getifcount ("wifi");
  int a;
  for (a = 0; a < ifcount; a++)
    {
      sprintf (&menu[1][a + 7][0], "Wireless_WDS-ath%d.asp", a);
      if (ifcount == 1)
	sprintf (&menuname[1][a + 8][0], "wirelessWds");
      else
	sprintf (&menuname[1][a + 8][0], "wirelessWds%d", a);
    }
#else
  int ifcount = get_wl_instances ();
  int a;
  for (a = 0; a < ifcount; a++)
    {
      sprintf (&menu[1][a + 7][0], "Wireless_WDS-wl%d.asp", a);
      if (ifcount == 1)
	sprintf (&menuname[1][a + 8][0], "wirelessWds");
      else
	sprintf (&menuname[1][a + 8][0], "wirelessWdswl%d", a);
    }
#endif

  int i, j;

  websWrite (wp, "<div id=\"menu\">\n");
  websWrite (wp, " <div id=\"menuMain\">\n");
  websWrite (wp, "  <ul id=\"menuMainList\">\n");
#ifdef HAVE_WAVESAT
  wimaxwifi = 1;
#endif
  for (i = 0; i < 8; i++)
    {
#ifdef HAVE_MADWIFI
      if (!wifi && !wimaxwifi && !strcmp (menu[i][0], "Wireless_Basic.asp"))
	i++;
#endif
      if (!strcmp (menu[i][0], mainmenu))
	{
#ifdef HAVE_MADWIFI
	  if (!wifi && wimaxwifi
	      && !strcmp (menu[i][0], "Wireless_Basic.asp"))
	    websWrite (wp,
		       "   <li class=\"current\"><span><script type=\"text/javascript\">Capture(bmenu.wimax)</script></span>\n");
	  else
#endif
	    websWrite (wp,
		       "   <li class=\"current\"><span><script type=\"text/javascript\">Capture(bmenu.%s)</script></span>\n",
		       menuname[i][0]);
	  websWrite (wp, "    <div id=\"menuSub\">\n");
	  websWrite (wp, "     <ul id=\"menuSubList\">\n");

	  for (j = 0; j < 11; j++)
	    {

#ifdef HAVE_MADWIFI
	      if (!wifi && !strncmp (menu[i][j], "Wireless_Basic.asp", 8))
		j++;
#ifndef HAVE_SUPERCHANNEL
	      if (!strcmp (menu[i][j], "SuperChannel.asp"))	//jump over PPTP in micro build
		j++;
#else
	      if (!strcmp (menu[i][j], "SuperChannel.asp") && (issuperchannel () || !wifi))	//jump over PPTP in micro build
		j++;
#endif
#else
	      if (!strcmp (menu[i][j], "SuperChannel.asp"))	//jump over PPTP in micro build
		j++;
#endif
#ifndef HAVE_WAVESAT
	      if (!strcmp (menu[i][j], "WiMAX.asp"))	//jump over WiMAX
		j++;
#else
	      if (!wimaxwifi && !strcmp (menu[i][j], "WiMAX.asp"))	//jump over WiMAX
		j++;
#endif
#ifdef HAVE_MADWIFI
	      if (!wifi && !strcmp (menu[i][j], "WL_WPATable.asp"))	//jump over PPTP in micro build
		j++;
	      if (!strcmp (menu[i][j], "Wireless_radauth.asp"))
		j++;
	      if (!wifi && !strncmp (menu[i][j], "Wireless_MAC.asp", 8))
		j++;
	      if (!strcmp (menu[i][j], "Wireless_Advanced.asp"))
		j++;
	      if (!wifi && !strncmp (menu[i][j], "Wireless_WDS", 12))
		j++;
	      if (!wifi && !strcmp (menu[i][j], "Status_Wireless.asp"))
		j++;

#endif
	      if ((!vlan_supp) && !strcmp (menu[i][j], "Vlan.asp"))	//jump over VLANs if vlan not supported
		j++;

#ifndef HAVE_PPPOESERVER
	      if (!strcmp (menu[i][j], "PPPoE_Server.asp"))
		j++;
#endif
#ifdef HAVE_MICRO
	      if (!strcmp (menu[i][j], "PPTP.asp"))	//jump over PPTP in micro build
		j++;
#endif
#ifdef HAVE_GLAUCO
	      if (!strcmp (menu[i][j], "Factory_Defaults.asp"))
		j++;
	      if (!strcmp (menu[i][j], "Upgrade.asp"))
		j++;
#endif
#ifndef HAVE_MILKFISH
	      if (!strcmp (menu[i][j], "Milkfish.asp"))
		j++;
#endif
#ifndef HAVE_WOL
	      if (!strcmp (menu[i][j], "Wol.asp"))
		j++;
#endif
#ifndef HAVE_EOP_TUNNEL
	      if (!strcmp (menu[i][j], "eop-tunnel.asp"))
		j++;
#endif
#ifndef HAVE_VLANTAGGING
	      if (!strcmp (menu[i][j], "Networking.asp"))
		j++;
#endif
#ifndef HAVE_CTORRENT
	      if (!strcmp (menu[i][j], "P2P.asp"))
		j++;
#endif
	      if ((!sputnik) && !strcmp (menu[i][j], "Status_SputnikAPD.asp"))	//jump over Sputnik
		j++;
	      if ((!openvpn) && !strcmp (menu[i][j], "Status_OpenVPN.asp"))	//jump over OpenVPN
		j++;
	      if ((!auth) && !strcmp (menu[i][j], "Info.htm"))	//jump over Sys-Info
		j++;
#ifdef HAVE_MADWIFI
	      if (!strcmp (menu[i][j], submenu)
		  && (strlen (menu[i][j])
		      && !strcmp (menu[i][j], "Wireless_Basic.asp") && !wifi
		      && wimaxwifi))
		{
		  websWrite (wp,
			     "      <li><span><script type=\"text/javascript\">Capture(bmenu.wimax)</script></span></li>\n");
		}
#endif
	      else if (!strcmp (menu[i][j], submenu) && (strlen (menu[i][j])))
		{
		  websWrite (wp,
			     "      <li><span><script type=\"text/javascript\">Capture(bmenu.%s)</script></span></li>\n",
			     menuname[i][j + 1]);
		}
#ifdef HAVE_HTTPS		//until https will allow upgrade and backup
	      else if ((strlen (menu[i][j]) != 0) && (do_ssl)
		       &&
		       ((!strcmp (menu[i][j], "Upgrade.asp")
			 || (!strcmp (menu[i][j], "config.asp")))))
		{
		  websWrite (wp,
			     "      <script type=\"text/javascript\">\n//<![CDATA[\n");
		  websWrite (wp,
			     "      document.write(\"<li><a style=\\\"cursor:pointer\\\" title=\\\"\" + errmsg.err46 + \"\\\" onclick=\\\"alert(errmsg.err45)\\\" ><em>\" + bmenu.%s + \"</em></a></li>\");\n",
			     menuname[i][j + 1]);
		  websWrite (wp, "      \n//]]>\n</script>\n");
		}
#endif
#ifdef HAVE_MADWIFI
	      else if (strlen (menu[i][j])
		       && !strcmp (menu[i][j], "Wireless_Basic.asp") && !wifi
		       && wimaxwifi)
		{
		  websWrite (wp,
			     "      <li><a href=\"WiMAX.asp\"><script type=\"text/javascript\">Capture(bmenu.wimax)</script></a></li>\n");
		}
#endif
	      else if (strlen (menu[i][j]))
		{
		  websWrite (wp,
			     "      <li><a href=\"%s\"><script type=\"text/javascript\">Capture(bmenu.%s)</script></a></li>\n",
			     menu[i][j], menuname[i][j + 1]);
		}
	    }
	  websWrite (wp, "     </ul>\n");
	  websWrite (wp, "    </div>\n");
	  websWrite (wp, "    </li>\n");
	}
#ifdef HAVE_MADWIFI
      else if (!strcmp (menu[i][0], "Wireless_Basic.asp") && !wifi
	       && wimaxwifi)
	{
	  websWrite (wp,
		     "      <li><a href=\"WiMAX.asp\"><script type=\"text/javascript\">Capture(bmenu.wimax)</script></a></li>\n");
	}
#endif
      else
	{
	  websWrite (wp,
		     "   <li><a href=\"%s\"><script type=\"text/javascript\">Capture(bmenu.%s)</script></a></li>\n",
		     menu[i][0], menuname[i][0]);
	}
    }
  websWrite (wp, "  </ul>\n");
  websWrite (wp, " </div>\n");
  websWrite (wp, "</div>\n");

  return;
}

void
ej_do_pagehead (webs_t wp, int argc, char_t ** argv)	//Eko
{
  char *style = nvram_get ("router_style");

#ifndef FASTWEB
  if (argc < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif


  /*websWrite (wp,
     "<\?xml version=\"1.0\" encoding=\"%s\"\?>\n",
     live_translate("lang_charset.set"));
     IE Problem ... */
  websWrite (wp,
	     "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n");
  websWrite (wp, "<html>\n");
  websWrite (wp, "\t<head>\n");
  websWrite (wp,
	     "\t\t<meta http-equiv=\"Content-Type\" content=\"application/xhtml+xml; charset=%s\" />\n",
	     live_translate ("lang_charset.set"));
#ifndef HAVE_MICRO
  websWrite (wp,
	     "\t\t<link rel=\"icon\" href=\"images/favicon.ico\" type=\"image/x-icon\" />\n");
  websWrite (wp,
	     "\t\t<link rel=\"shortcut icon\" href=\"images/favicon.ico\" type=\"image/x-icon\" />\n");
#endif
  websWrite (wp,
	     "\t\t<script type=\"text/javascript\" src=\"common.js\"></script>\n");
  websWrite (wp,
	     "\t\t<script type=\"text/javascript\" src=\"lang_pack/english.js\"></script>\n");
#ifdef HAVE_LANGUAGE	     
  websWrite (wp,
	     "\t\t<script type=\"text/javascript\" src=\"lang_pack/language.js\"></script>\n");
#endif
  websWrite (wp,
	     "\t\t<link type=\"text/css\" rel=\"stylesheet\" href=\"style/%s/style.css\" />\n",
	     style);
  websWrite (wp,
	     "\t\t<!--[if IE]><link type=\"text/css\" rel=\"stylesheet\" href=\"style/%s/style_ie.css\" /><![endif]-->\n",
	     style);

#ifdef HAVE_PWC
  websWrite (wp,
	     "\t\t<script type=\"text/javascript\" src=\"js/prototype.js\"></script>\n");
  websWrite (wp,
	     "\t\t<script type=\"text/javascript\" src=\"js/effects.js\"></script>\n");
  websWrite (wp,
	     "\t\t<script type=\"text/javascript\" src=\"js/window.js\"></script>\n");
  websWrite (wp,
	     "\t\t<script type=\"text/javascript\" src=\"js/window_effects.js\"></script>\n");
  websWrite (wp,
	     "\t\t<link type=\"text/css\" rel=\"stylesheet\" href=\"style/pwc/default.css\" />\n");
  websWrite (wp,
	     "\t\t<link type=\"text/css\" rel=\"stylesheet\" href=\"style/pwc/ddwrt.css\" />\n");
#endif
  websWrite (wp, "\t\t<title>%s", nvram_get ("router_name"));
  if (strlen (argv[0]) != 0)
    {
      websWrite (wp, " - %s", live_translate (argv[0]));
    }
  websWrite (wp, "</title>\n");

}

void
ej_do_hpagehead (webs_t wp, int argc, char_t ** argv)	//Eko
{
  char *htitle;

#ifdef FASTWEB
  ejArgs (argc, argv, "%s", &htitle);
#else
  if (ejArgs (argc, argv, "%s", &htitle) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif
  websWrite (wp,
	     "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n");
  if (!strcmp (htitle, "doctype_only"))
    return;			//stop here, for About.htm
  websWrite (wp, "<html>\n");
  websWrite (wp, "\t<head>\n");
  websWrite (wp,
	     "\t\t<meta http-equiv=\"Content-Type\" content=\"application/xhtml+xml; charset=%s\" />\n",
	     live_translate ("lang_charset.set"));
  websWrite (wp,
	     "\t\t<script type=\"text/javascript\" src=\"../common.js\"></script>\n");
  websWrite (wp,
	     "\t\t<script type=\"text/javascript\" src=\"../lang_pack/english.js\"></script>\n");
#ifdef HAVE_LANGUAGE
  websWrite (wp,
	     "\t\t<script type=\"text/javascript\" src=\"../lang_pack/language.js\"></script>\n");
#endif
  websWrite (wp,
	     "\t\t<link type=\"text/css\" rel=\"stylesheet\" href=\"help.css\">\n");
  websWrite (wp, "\t\t<title>%s", live_translate ("share.help"));
  websWrite (wp, " - %s</title>\n", live_translate (htitle));
  websWrite (wp, "\t</head>\n");

}

void
ej_show_timeoptions (webs_t wp, int argc, char_t ** argv)	//Eko
{

  char timediffs[39][8] =
    { "-12", "-11", "-10", "-09.5", "-09", "-08", "-07", "-06", "-05",
    "-04.5", "-04",
    "-03.5", "-03", "-02", "-01", "+00",
    "+01", "+02", "+03", "+03.5", "+04", "+04.5", "+05", "+05.5", "+05.75",
    "+06",
    "+06.5", "+07", "+08", "+09", "+09.5", "+10", "+10.5", "+11", "+11.5",
    "+12", "+12.75", "+13", "+14"
  };

  char timezones[39][8] =
    { "-12:00", "-11:00", "-10:00", "-09:30", "-09:00", "-08:00", "-07:00",
    "-06:00", "-05:00", "-04:30", "-04:00", "-03:30", "-03:00", "-02:00",
    "-01:00", "",
    "+01:00", "+02:00", "+03:00", "+03:30", "+04:00", "+04:30", "+05:00",
    "+05:30",
    "+05:45", "+06:00", "+06:30", "+07:00", "+08:00", "+09:00", "+09:30",
    "+10:00", "+10:30", "+11:00", "+11:30", "+12:00", "+12:45", "+13:00",
    "+14:00"
  };

  int i;

  for (i = 0; i < 39; i++)
    {
      websWrite (wp,
		 "<option value=\"%s\" %s>UTC%s</option>\");\n",
		 timediffs[i], nvram_match ("time_zone",
					    timediffs[i]) ?
		 "selected=\"selected\"" : "", timezones[i]);

    }

}




void
ej_show_wanipinfo (webs_t wp, int argc, char_t ** argv)	//Eko
{
  char *wan_ipaddr;
  int wan_link;

  if (nvram_match ("wl0_mode", "wet") || nvram_match ("wl0_mode", "apstawet")
      || nvram_match ("wan_proto", "disabled"))
    {
      websWrite (wp, ": %s", live_translate ("share.disabled"));
      return;
    }

  wan_link = check_wan_link (0);

  if (nvram_match ("wan_proto", "pptp"))
    {
      wan_ipaddr =
	wan_link ? nvram_safe_get ("pptp_get_ip") :
	nvram_safe_get ("wan_ipaddr");
    }
  else if (!strcmp (nvram_safe_get ("wan_proto"), "pppoe"))
    {
      wan_ipaddr = wan_link ? nvram_safe_get ("wan_ipaddr") : "0.0.0.0";
    }
  else if (nvram_match ("wan_proto", "l2tp"))
    {
      wan_ipaddr =
	wan_link ? nvram_safe_get ("l2tp_get_ip") :
	nvram_safe_get ("wan_ipaddr");
    }
  else
    {
      wan_ipaddr = nvram_safe_get ("wan_ipaddr");
    }

  websWrite (wp, "&nbsp;IP: %s", wan_ipaddr);

  return;
}


/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_selected("wan_proto", "dhcp",); %> produces: selected="selected"
 * <% nvram_selected_js("wan_proto", "dhcp"); %> produces: selected=\"selected\"
 * <% nvram_selected("wan_proto", "static"); %> does not produce
 */
void
ej_nvram_selected (webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
  if (argc < 2);
  {
    websError (wp, 400, "Insufficient args\n");
    return;
  }
#endif

  if (nvram_match (argv[0], argv[1]))
    {
      websWrite (wp, "selected=\"selected\"");
    }
  return;
}

void
ej_nvram_selected_js (webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
  if (argc < 2);
  {
    websError (wp, 400, "Insufficient args\n");
    return;
  }
#endif

  if (nvram_match (argv[0], argv[1]))
    {
      websWrite (wp, "selected=\\\"selected\\\"");
    }
  return;
}



void
ej_getrebootflags (webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_RB500
  websWrite (wp, "1");
#elif HAVE_MAGICBOX
  websWrite (wp, "2");
#elif HAVE_FONERA
  websWrite (wp, "2");
#elif HAVE_MERAKI
  websWrite (wp, "2");
#elif HAVE_LS2
  websWrite (wp, "2");
#elif HAVE_LS5
  websWrite (wp, "2");
#elif HAVE_WHRAG108
  websWrite (wp, "2");
#elif HAVE_TW6600
  websWrite (wp, "2");
#elif HAVE_CA8
  websWrite (wp, "2");
#elif HAVE_GATEWORX
  websWrite (wp, "1");
#elif HAVE_X86
  websWrite (wp, "1");
#else
  websWrite (wp, "0");
#endif
}

void
ej_tran (webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
  if (argc != 1)
    return;
#endif
  websWrite (wp, "<script type=\"text/javascript\">Capture(%s)</script>",
	     argv[0]);
  return;
}

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_checked("wan_proto", "dhcp"); %> produces: checked="checked"
 * <% nvram_checked_js("wan_proto", "dhcp"); %> produces: checked=\"checked\"
 * <% nvram_checked("wan_proto", "static"); %> does not produce
 */
void
ej_nvram_checked (webs_t wp, int argc, char_t ** argv)
{

#ifdef FASTWEB
  if (argc < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif

  if (nvram_match (argv[0], argv[1]))
    {
      websWrite (wp, "checked=\"checked\"");
    }

  return;
}

void
ej_nvram_checked_js (webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
  if (argc < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif

  if (nvram_match (argv[0], argv[1]))
    {
      websWrite (wp, "checked=\\\"checked\\\"");
    }

  return;
}

void
ej_make_time_list (webs_t wp, int argc, char_t ** argv)
{
  int i, st, en;
  char ic[16];

#ifndef FASTWEB
  if (argc < 3)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
#endif

  st = atoi (argv[1]);
  en = atoi (argv[2]);

  for (i = st; i <= en; i++)
    {
      sprintf (ic, "%d", i);
      websWrite (wp, "<option value=\"%d\" %s >%02d</option>\n", i,
		 nvram_match (argv[0], ic) ? "selected=\"selected\"" : "", i);
    }

  return;
}

#ifdef HAVE_CPUTEMP
void
ej_get_cputemp (webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_GATEWORX
  int TEMP_MUL = 100;
  if (getRouterBrand () == ROUTER_BOARD_GATEWORX_SWAP)
    TEMP_MUL = 200;

  FILE *fp =
    fopen
    ("/sys/devices/platform/IXP4XX-I2C.0/i2c-adapter:i2c-0/0-0028/temp_input",
     "rb");
#else
#define TEMP_MUL 1000
#ifdef HAVE_X86
  FILE *fp = fopen ("/sys/devices/platform/i2c-1/1-0048/temp1_input", "rb");
#else
  FILE *fp = fopen ("/sys/devices/platform/i2c-0/0-0048/temp1_input", "rb");
#endif
#endif

  if (fp == NULL)
    {
      websWrite (wp, "%s", live_translate ("status_router.notavail"));	//no i2c lm75 found
      return;
    }
  int temp;
  fscanf (fp, "%d", &temp);
  fclose (fp);
  int high = temp / TEMP_MUL;
  int low = (temp - (high * TEMP_MUL)) / (TEMP_MUL / 10);
  websWrite (wp, "%d.%d &#176;C", high, low);	//no i2c lm75 found
}


void
ej_show_cpu_temperature (webs_t wp, int argc, char_t ** argv)
{
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\"><script type=\"text/javascript\">Capture(status_router.cputemp)</script></div>\n");
  websWrite (wp, "<span id=\"cpu_temp\">");
  ej_get_cputemp (wp, argc, argv);
  websWrite (wp, "</span>&nbsp;\n");
  websWrite (wp, "</div>\n");
}
#else
void
ej_get_cputemp (webs_t wp, int argc, char_t ** argv)
{
  return;
}

void
ej_show_cpu_temperature (webs_t wp, int argc, char_t ** argv)
{
  return;
}
#endif


#ifdef HAVE_VOLT
void
ej_get_voltage (webs_t wp, int argc, char_t ** argv)
{
  FILE *fp =
    fopen ("/sys/devices/platform/IXP4XX-I2C.0/i2c-adapter:i2c-0/0-0028/volt",
	   "rb");

  if (fp == NULL)
    {
      websWrite (wp, "%s", live_translate ("status_router.notavail"));	//no i2c lm75 found
      return;
    }
  int temp;
  fscanf (fp, "%d", &temp);
  fclose (fp);
//temp*=564;
  int high = temp / 1000;
  int low = (temp - (high * 1000)) / 100;
  websWrite (wp, "%d.%d V", high, low);	//no i2c lm75 found
}


void
ej_show_voltage (webs_t wp, int argc, char_t ** argv)
{
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\"><script type=\"text/javascript\">Capture(status_router.inpvolt)</script></div>\n");
  websWrite (wp, "<span id=\"voltage\">");
  ej_get_voltage (wp, argc, argv);
  websWrite (wp, "</span>&nbsp;\n");
  websWrite (wp, "</div>\n");
}
#else
void
ej_get_voltage (webs_t wp, int argc, char_t ** argv)
{
  return;
}

void
ej_show_voltage (webs_t wp, int argc, char_t ** argv)
{
  return;
}
#endif

#ifdef HAVE_MSSID

static void
showencstatus (webs_t wp, char *prefix)
{
  char akm[64];
  sprintf (akm, "%s_akm", prefix);
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\"><script type=\"text/javascript\">Capture(share.encrypt)</script>&nbsp;-&nbsp;<script type=\"text/javascript\">Capture(share.intrface)</script>&nbsp;%s</div>\n",
	     prefix);
  websWrite (wp, "<script type=\"text/javascript\">");
  if (nvram_match (akm, "disabled"))
    {
      websWrite (wp, "Capture(share.disabled)");
      websWrite (wp, "</script>");
    }
  else
    {
      websWrite (wp, "Capture(share.enabled)");
      websWrite (wp, "</script>,&nbsp;");

      if (nvram_match (akm, "psk"))
	websWrite (wp, "WPA Personal");
      if (nvram_match (akm, "wpa"))
	websWrite (wp, "WPA Enterprise");
      if (nvram_match (akm, "psk2"))
	websWrite (wp, "WPA2 Personal");
      if (nvram_match (akm, "wpa2"))
	websWrite (wp, "WPA2 Enterprise");
      if (nvram_match (akm, "psk psk2"))
	websWrite (wp, "WPA2 Personal Mixed");
      if (nvram_match (akm, "wpa wpa2"))
	websWrite (wp, "WPA Enterprise Mixed");
      if (nvram_match (akm, "radius"))
	websWrite (wp, "RADIUS");
      if (nvram_match (akm, "wep"))
	websWrite (wp, "WEP");
    }

  websWrite (wp, "\n</div>\n");
  return;
}

void
ej_get_txpower (webs_t wp, int argc, char_t ** argv)
{
#ifndef HAVE_MADWIFI
  websWrite (wp, "%s mW", nvram_safe_get ("wl0_txpwr"));
#else
  char m[32];
  strncpy (m, nvram_safe_get ("wifi_display"), 4);
  m[4] = 0;
  websWrite (wp, "%d dBm", wifi_gettxpower (m));
#endif
}

void
ej_getencryptionstatus (webs_t wp, int argc, char_t ** argv)
{
  char *mode = nvram_safe_get ("wifi_display");
  showencstatus (wp, mode);
}

void
ej_getwirelessstatus (webs_t wp, int argc, char_t ** argv)
{
#ifndef HAVE_MADWIFI
  char *mode = "wl0_mode";
#else
  char mode[32];
  char m[32];
  strncpy (m, nvram_safe_get ("wifi_display"), 4);
  m[4] = 0;
  sprintf (mode, "%s_mode", m);

#endif
  if (nvram_match (mode, "wet") || nvram_match (mode, "sta")
      || nvram_match (mode, "infra"))
    websWrite (wp,
	       "<script type=\"text/javascript\">Capture(info.ap)</script>");
  else if (nvram_match (mode, "apsta") || nvram_match (mode, "apstawet"))
    {
      websWrite (wp,
		 "<script type=\"text/javascript\">Capture(info.ap)</script>");
      websWrite (wp, " & ");
      websWrite (wp,
		 "<script type=\"text/javascript\">Capture(status_wireless.legend3)</script>");
    }
  else
    websWrite (wp,
	       "<script type=\"text/javascript\">Capture(status_wireless.legend3)</script>");
}
#endif
void
ej_getwirelessssid (webs_t wp, int argc, char_t ** argv)
{
#ifndef HAVE_MADWIFI
  tf_webWriteESCNV (wp, "wl0_ssid");
#else
  char ssid[32];
  sprintf (ssid, "%s_ssid", nvram_safe_get ("wifi_display"));
  tf_webWriteESCNV (wp, ssid);
#endif
}

void
ej_getwirelessmode (webs_t wp, int argc, char_t ** argv)
{
#ifndef HAVE_MADWIFI
  char *mode = "wl0_mode";
#else
  char mode[32];
  char m[32];
  strncpy (m, nvram_safe_get ("wifi_display"), 4);
  m[4] = 0;
  sprintf (mode, "%s_mode", m);
#endif
  websWrite (wp, "<script type=\"text/javascript\">");
  if (nvram_match (mode, "wet"))
    websWrite (wp, "Capture(wl_basic.clientBridge)");
  if (nvram_match (mode, "ap"))
    websWrite (wp, "Capture(wl_basic.ap)");
  if (nvram_match (mode, "sta"))
    websWrite (wp, "Capture(wl_basic.client)");
  if (nvram_match (mode, "infra"))
    websWrite (wp, "Capture(wl_basic.adhoc)");
  if (nvram_match (mode, "apsta"))
    websWrite (wp, "Capture(wl_basic.repeater)");
  if (nvram_match (mode, "apstawet"))
    websWrite (wp, "Capture(wl_basic.repeaterbridge)");
  if (nvram_match (mode, "wdssta"))
    websWrite (wp, "Capture(wl_basic.wdssta)");
  if (nvram_match (mode, "wdsap"))
    websWrite (wp, "Capture(wl_basic.wdsap)");
  websWrite (wp, "</script>&nbsp;\n");
}

void
ej_getwirelessnetmode (webs_t wp, int argc, char_t ** argv)
{
#ifndef HAVE_MADWIFI
#ifndef HAVE_MSSID
  char *mode = "wl_net_mode";
#else
  char *mode = "wl0_net_mode";
#endif
#else
  char mode[32];
  char m[32];
  strncpy (m, nvram_safe_get ("wifi_display"), 4);
  m[4] = 0;
  sprintf (mode, "%s_net_mode", m);
#endif
  websWrite (wp, "<script type=\"text/javascript\">");
  if (nvram_match (mode, "disabled"))
    websWrite (wp, "Capture(share.disabled)");
  if (nvram_match (mode, "mixed"))
    websWrite (wp, "Capture(wl_basic.mixed)");
  if (nvram_match (mode, "g-only"))
    websWrite (wp, "Capture(wl_basic.g)");
  if (nvram_match (mode, "b-only"))
    websWrite (wp, "Capture(wl_basic.b)");
  if (nvram_match (mode, "n-only"))
    websWrite (wp, "Capture(wl_basic.n)");
  if (nvram_match (mode, "a-only"))
    websWrite (wp, "Capture(wl_basic.a)");
  websWrite (wp, "</script>&nbsp;\n");
}




void
ej_show_openvpn_status (webs_t wp, int argc, char_t ** argv)
{
  websWrite (wp,
	     "<fieldset>\n<legend><script type=\"text/javascript\">Capture(share.state)</script></legend>\n");

  system2 ("/etc/openvpnstate.sh > /tmp/.temp");
  FILE *in = fopen ("/tmp/.temp", "r");
  while (!feof (in))
    {
      int b = getc (in);
      if (b != EOF)
	wfputc (b, wp);
    }
  fclose (in);
  websWrite (wp, "</fieldset><br />");
  websWrite (wp,
	     "<fieldset>\n<legend><script type=\"text/javascript\">Capture(share.statu)</script></legend>\n");
  system2 ("/etc/openvpnstatus.sh > /tmp/.temp");
  in = fopen ("/tmp/.temp", "r");
  while (!feof (in))
    {
      int b = getc (in);
      if (b != EOF)
	wfputc (b, wp);
    }
  fclose (in);
  websWrite (wp, "</fieldset><br />");
  websWrite (wp,
	     "<fieldset>\n<legend><script type=\"text/javascript\">Capture(log.legend)</script></legend>\n");
  system2 ("/etc/openvpnlog.sh > /tmp/.temp");
  in = fopen ("/tmp/.temp", "r");
  while (!feof (in))
    {
      int b = getc (in);
      if (b != EOF)
	wfputc (b, wp);
    }
  fclose (in);
  websWrite (wp, "</fieldset><br />");

}



void
ej_get_radio_state (webs_t wp, int argc, char_t ** argv)
{
  int radiooff = -1;

#ifdef HAVE_MADWIFI
  char *ifname = nvram_safe_get ("wifi_display");
  if (strlen (ifname) > 0)
    {
      int state = get_radiostate (ifname);
      switch (state)
	{
	case 1:
	  websWrite (wp, "%s", live_translate ("wl_basic.radio_on"));
	  break;
	case -1:
	  websWrite (wp, "%s", live_translate ("share.unknown"));
	  break;
	default:		// 1: software disabled, 2: hardware disabled, 3: both are disabled
	  websWrite (wp, "%s", live_translate ("wl_basic.radio_off"));
	  break;
	}
    }
  else
    {
      websWrite (wp, "%s", live_translate ("share.unknown"));
    }
#else
  wl_ioctl (get_wdev (), WLC_GET_RADIO, &radiooff, sizeof (int));

  switch ((radiooff & WL_RADIO_SW_DISABLE))
    {
    case 0:
      websWrite (wp, "%s", live_translate ("wl_basic.radio_on"));
      break;
    case -1:
      websWrite (wp, "%s", live_translate ("share.unknown"));
      break;
    default:			// 1: software disabled, 2: hardware disabled, 3: both are disabled
      websWrite (wp, "%s", live_translate ("wl_basic.radio_off"));
      break;
    }
#endif
}


void
ej_dumparptable (webs_t wp, int argc, char_t ** argv)
{
  FILE *f;
  FILE *host;
  FILE *conn;
  char buf[256];
  char hostname[128];
  char ip[16];
  char ip2[20];
  char fullip[18];
  char mac[18];
  char landev[16];
  int count = 0;
  int conn_count = 0;

  if ((f = fopen ("/proc/net/arp", "r")) != NULL)
    {
      while (fgets (buf, sizeof (buf), f))
	{
	  if (sscanf (buf, "%15s %*s %*s %17s %*s %s", ip, mac, landev) != 3)
	    continue;
	  if ((strlen (mac) != 17)
	      || (strcmp (mac, "00:00:00:00:00:00") == 0))
	    continue;
	  if (strcmp (landev, nvram_safe_get ("wan_iface")) == 0)
	    continue;		//skip all but LAN arp entries
	  strcpy (hostname, "*");	//set name to *

/* count open connections per IP */
	  if ((conn = fopen ("/proc/net/ip_conntrack", "r")) != NULL)
	    {
	      strcpy (ip2, ip);
	      strcat (ip2, " ");

	      while (fgets (buf, sizeof (buf), conn))
		{
		  if (strstr (buf, ip2))
		    conn_count++;
		}
	      fclose (conn);
	    }

/* end count */

/* do nslookup */

//  struct servent *servp;
//  char buf1[256];
//  
//  getHostName (buf1, ip);
//  if (strcmp(buf1, "unknown"))
//      strcpy (hostname, buf1);
/* end nslookup */

/* look into hosts file for hostnames  (static leases) */
	  if ((host = fopen ("/tmp/hosts", "r")) != NULL
	      && !strcmp (hostname, "*"))
	    {
	      while (fgets (buf, sizeof (buf), host))
		{
		  sscanf (buf, "%15s %*s", fullip);

		  if (!strcmp (ip, fullip))
		    {
		      sscanf (buf, "%*15s %s", hostname);
		    }
		}
	      fclose (host);
	    }
/* end hosts file lookup */

/* check  for dnsmasq leases in /tmp/dnsmasq.leases and /jffs/ if hostname is still unknown */

	  if (!strcmp (hostname, "*") && nvram_match ("dhcp_dnsmasq", "1")
	      && nvram_match ("dhcpd_usenvram", "0"))
	    {
	      if (!(host = fopen ("/tmp/dnsmasq.leases", "r")))
		host = fopen ("/jffs/dnsmasq.leases", "r");

	      if (host)
		{

		  while (fgets (buf, sizeof (buf), host))
		    {
		      sscanf (buf, "%*s %*s %15s %*s", fullip);

		      if (strcmp (ip, fullip) == 0)
			{
			  sscanf (buf, "%*s %*s %*s %s", hostname);
			}
		    }
		  fclose (host);
		}
	    }
/* end dnsmasq.leases check */

/* check nvram for dnsmasq leases in nvram if hostname is still unknown */

	  if (!strcmp (hostname, "*") && nvram_match ("dhcp_dnsmasq", "1")
	      && nvram_match ("dhcpd_usenvram", "1"))
	    {
	      sscanf (nvram_nget ("dnsmasq_lease_%s", ip), "%*s %*s %*s %s",
		      hostname);
	    }
/* end nvram check */


	  websWrite (wp, "%c'%s','%s','%s','%d'", (count ? ',' : ' '),
		     hostname, ip, mac, conn_count);
	  ++count;
	  conn_count = 0;
	}
      fclose (f);
    }
}

#ifdef HAVE_EOP_TUNNEL

void
ej_show_eop_tunnels (webs_t wp, int argc, char_t ** argv)
{

  int tun;
  char temp[32];

  for (tun = 1; tun < 11; tun++)
    {

      websWrite (wp, "<fieldset>\n");
      websWrite (wp,
		 "<legend><script type=\"text/javascript\">Capture(eoip.tunnel)</script> %d</legend>\n",
		 tun);
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.srv)</script></div>\n");
      sprintf (temp, "oet%d_en", tun);
      websWrite (wp,
		 "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idoet%d', true)\" /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
		 temp, (nvram_match (temp, "1") ? "checked=\"checked\"" : ""),
		 tun);
      websWrite (wp,
		 "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idoet%d', false)\" /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
		 temp, (nvram_match (temp, "0") ? "checked=\"checked\"" : ""),
		 tun);
      websWrite (wp, "</div>\n");
      websWrite (wp, "<div id=\"idoet%d\">\n", tun);
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.remoteIP)</script></div>\n");
      websWrite (wp,
		 "<input type=\"hidden\" name=\"oet%d_rem\" value=\"0.0.0.0\"/>\n",
		 tun);
      sprintf (temp, "oet%d_rem", tun);
      websWrite (wp,
		 "<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_0\" onblur=\"valid_range(this,0,255,eoip.remoteIP)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_1\" onblur=\"valid_range(this,0,255,eoip.tunnelID)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_2\" onblur=\"valid_range(this,0,255,eoip.tunnelID)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_3\" onblur=\"valid_range(this,1,254,eoip.tunnelID)\" value=\"%d\" />\n",
		 temp, get_single_ip (nvram_safe_get (temp), 0), temp,
		 get_single_ip (nvram_safe_get (temp), 1), temp,
		 get_single_ip (nvram_safe_get (temp), 2), temp,
		 get_single_ip (nvram_safe_get (temp), 3));
      websWrite (wp, "</div>\n");
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.tunnelID)</script></div>\n");
      sprintf (temp, "oet%d_id", tun);
      websWrite (wp,
		 "<input size=\"4\" maxlength=\"3\" class=\"num\" name=\"%s\" onblur=\"valid_range(this,0,999,eoip.tunnelID)\" value=\"%s\" />\n",
		 temp, nvram_get (temp));
      websWrite (wp, "</div>\n");
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.comp)</script></div>\n");
      sprintf (temp, "oet%d_comp", tun);
      websWrite (wp,
		 "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
		 temp,
		 (nvram_match (temp, "1") ? "checked=\"checked\"" : ""));
      websWrite (wp,
		 "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
		 temp,
		 (nvram_match (temp, "0") ? "checked=\"checked\"" : ""));
      websWrite (wp, "</div>\n");
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.passtos)</script></div>\n");
      sprintf (temp, "oet%d_pt", tun);
      websWrite (wp,
		 "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
		 temp,
		 (nvram_match (temp, "1") ? "checked=\"checked\"" : ""));
      websWrite (wp,
		 "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
		 temp,
		 (nvram_match (temp, "0") ? "checked=\"checked\"" : ""));
      websWrite (wp, "</div>\n");
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.frag)</script></div>\n");
      sprintf (temp, "oet%d_fragment", tun);
      websWrite (wp,
		 "<input size=\"4\" maxlength=\"4\" class=\"num\" name=\"%s\" onblur=\"valid_range(this,0,1500,eoip.frag)\" value=\"%s\" />\n",
		 temp, nvram_get (temp));
      websWrite (wp, "</div>\n");
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.mssfix)</script></div>\n");
      sprintf (temp, "oet%d_mssfix", tun);
      websWrite (wp,
		 "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
		 temp,
		 (nvram_match (temp, "1") ? "checked=\"checked\"" : ""));
      websWrite (wp,
		 "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
		 temp,
		 (nvram_match (temp, "0") ? "checked=\"checked\"" : ""));
      websWrite (wp, "</div>\n");
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.shaper)</script></div>\n");
      sprintf (temp, "oet%d_shaper", tun);
      websWrite (wp,
		 "<input size=\"6\" maxlength=\"6\" class=\"num\" name=\"%s\" onblur=\"valid_range(this,0,100000,eoip.shaper)\" value=\"%s\" />\n",
		 temp, nvram_get (temp));
      websWrite (wp, "</div>\n");
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.bridging)</script></div>\n");
      sprintf (temp, "oet%d_bridged", tun);
      websWrite (wp,
		 "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'isbridged%d', true)\" /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
		 temp, (nvram_match (temp, "1") ? "checked=\"checked\"" : ""),
		 tun);
      websWrite (wp,
		 " <input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'isbridged%d', false)\" /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
		 temp, (nvram_match (temp, "0") ? "checked=\"checked\"" : ""),
		 tun);
      websWrite (wp, "</div>\n");
      websWrite (wp, "<div id=\"idbridged%d\">\n", tun);
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(share.ip)</script></div>\n");
      websWrite (wp,
		 "<input type=\"hidden\" name=\"oet%d_ip\" value=\"0.0.0.0\"/>\n",
		 tun);
      sprintf (temp, "oet%d_ip", tun);
      websWrite (wp,
		 "<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_0\" onblur=\"valid_range(this,0,255,share.ip)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_1\" onblur=\"valid_range(this,0,255,share.ip)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_2\" onblur=\"valid_range(this,0,255,share.ip)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_3\" onblur=\"valid_range(this,1,254,share.ip)\" value=\"%d\" />\n",
		 temp, get_single_ip (nvram_safe_get (temp), 0), temp,
		 get_single_ip (nvram_safe_get (temp), 1), temp,
		 get_single_ip (nvram_safe_get (temp), 2), temp,
		 get_single_ip (nvram_safe_get (temp), 3));
      websWrite (wp, "</div>\n");
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(share.subnet)</script></div>\n");
      websWrite (wp,
		 "<input type=\"hidden\" name=\"oet%d_netmask\" value=\"0.0.0.0\"/>\n",
		 tun);
      sprintf (temp, "oet%d_netmask", tun);
      websWrite (wp,
		 "<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_0\" onblur=\"valid_range(this,0,255,share.subnet)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_1\" onblur=\"valid_range(this,0,255,share.subnet)\" value=\"%d\" />.<input size=\3\" maxlength=\"3\" class=\"num\" name=\"%s_2\" onblur=\"valid_range(this,0,255,share.subnet)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_3\" onblur=\"valid_range(this,0,254,share.subnet)\" value=\"%d\" />\n",
		 temp, get_single_ip (nvram_safe_get (temp), 0), temp,
		 get_single_ip (nvram_safe_get (temp), 1), temp,
		 get_single_ip (nvram_safe_get (temp), 2), temp,
		 get_single_ip (nvram_safe_get (temp), 3));
      websWrite (wp, "</div>\n");
      websWrite (wp, "</div>\n");
      websWrite (wp, "</div>\n");
      websWrite (wp, "</fieldset><br/>\n");
    }
}

#endif



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
int
tf_webWriteJS (webs_t wp, const char *s)
{
  char buf[512];
  int n;
  int r;

  n = 0;
  r = 0;
  for (; *s; s++)
    {
      if (*s == '<')
        {
        sprintf (buf + n, "&lt;");
	  n += 4;
        }
        else
      if (*s == '>')
        {
        sprintf (buf + n, "&gt;");
	  n += 4;
        }
        else
      if ((*s != '"') && (*s != '\\') && (*s != '/') && (*s != '*') && (*s != '\'') && (isprint (*s)))
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

#ifdef HAVE_UPNP
// changed by steve
// writes javascript-string safe text


//      <% tf_upnp(); %>
//      returns all "forward_port#" nvram entries containing upnp port forwardings
void
ej_tf_upnp (webs_t wp, int argc, char_t ** argv)
{
  int i;
  int len, pos, count;
  char *temp;

  if (nvram_match ("upnp_enable", "1"))
    {
      for (i = 0; i < 50; i++)
	{
	  websWrite (wp, (i > 0) ? ",'" : "'");

// fix: some entries are missing the desc. - this breaks the upnp.asp page, so we add ,*
	  temp = nvram_nget ("forward_port%d", i);
	  count = 0;
	  len = strlen (temp);

	  for (pos = len; pos != 0; pos--)
	    {
	      if (temp[pos] == ',')
		count++;
	    }

	  tf_webWriteJS (wp, temp);
	  if (count == 2)
	    websWrite (wp, ",*");

	  websWrite (wp, "'");
	}
    }
}

// end changed by steve
#endif
