/*
 * dd-wrt.c
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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <broadcom.h>
#include <cymac.h>
#include <wlutils.h>
#include <bcmparams.h>
#include <dirent.h>

#include <syslog.h>  //for debugging


static unsigned int oldclocks[9] =
  { 192, 200, 216, 228, 240, 252, 264, 280, 300 };
static unsigned int newclocks[9] =
  { 183, 187, 198, 200, 216, 225, 233, 237, 250 };


void
ej_show_clocks (int eid, webs_t wp, int argc, char_t ** argv)
{
  int tab = getcpurev ();
  unsigned int *c;
  if (tab == 7)
    c = oldclocks;
  else if (tab == 8)
    c = newclocks;
  else
    return;
  int i;
  for (i = 0; i < 9; i++)
    {
      char clock[16];
      sprintf (clock, "%d", c[i]);
      websWrite (wp, "<option value=\"%d\" %s >%d Mhz</option>\n", c[i],
		 nvram_match ("overclocking",
			      clock) ? "selected=\"selected\"" : "", c[i]);

    }
}

void
ej_show_routing (int eid, webs_t wp, int argc, char_t ** argv)
{
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"gateway\\\" %s >\" + share.gateway + \"</option>\");</script>\n",
	     nvram_selmatch (wp, "wk_mode",
			     "gateway") ? "selected=\\\"selected\\\"" : "");
#ifdef HAVE_BIRD
  websWrite (wp, "<option value=\"bgp\" %s >BGP</option>\n",
	     nvram_selmatch (wp, "wk_mode",
			     "bgp") ? "selected=\"selected\"" : "");
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"router\\\" %s >\" + route.rip2_mod + \"</option>\");</script>\n",
	     nvram_selmatch (wp, "wk_mode",
			     "router") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"ospf\\\" %s >\" + route.ospf_mod + \"</option>\");</script>\n",
	     nvram_selmatch (wp, "wk_mode",
			     "ospf") ? "selected=\\\"selected\\\"" : "");
#else
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"router\\\" %s >\" + share.router + \"</option>\");</script>\n",
	     nvram_selmatch (wp, "wk_mode",
			     "router") ? "selected=\\\"selected\\\"" : "");
#endif
  return;

}

void
ej_show_connectiontype (int eid, webs_t wp, int argc, char_t ** argv)
{

  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"disabled\\\" %s >\" + share.disabled + \"</option>\");</script>\n",
	     nvram_selmatch (wp, "wan_proto",
			     "disabled") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"static\\\" %s >\" + idx.static_ip + \"</option>\");</script>\n",
	     nvram_selmatch (wp, "wan_proto",
			     "static") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"dhcp\\\" %s >\" + idx.dhcp + \"</option>\");</script>\n",
	     nvram_selmatch (wp, "wan_proto",
			     "dhcp") ? "selected=\\\"selected\\\"" : "");
#ifdef HAVE_PPPOE
  websWrite (wp, "<option value=\"pppoe\" %s >PPPoE</option>\n",
	     nvram_selmatch (wp, "wan_proto",
			     "pppoe") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_PPTP
  websWrite (wp, "<option value=\"pptp\" %s >PPTP</option>\n",
	     nvram_selmatch (wp, "wan_proto",
			     "pptp") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_L2TP
  websWrite (wp, "<option value=\"l2tp\" %s >L2TP</option>\n",
	     nvram_selmatch (wp, "wan_proto",
			     "l2tp") ? "selected=\"selected\"" : "");
#endif
#ifdef HAVE_HEARTBEAT
  websWrite (wp,
	     "<option value=\"heartbeat\" %s >HeartBeat Signal</option>\n",
	     nvram_selmatch (wp, "wan_proto",
			     "heartbeat") ? "selected=\"selected\"" : "");
#endif
  return;
}

void
ej_show_infopage (int eid, webs_t wp, int argc, char_t ** argv)
{
/*
#ifdef HAVE_NEWMEDIA
websWrite(wp,"<dl>\n");
websWrite(wp,"<dd class=\"definition\">GGEW net GmbH</dd>\n");
websWrite(wp,"<dd class=\"definition\">Dammstrasse 68</dd>\n");
websWrite(wp,"<dd class=\"definition\">64625 Bensheim</dd>\n");
websWrite(wp,"<dd class=\"definition\"><a href=\"http://ggew-net.de\"><img src=\"images/ggewlogo.gif\" border=\"0\"/></a></dd>\n");
websWrite(wp,"<dd class=\"definition\"> </dd>\n");
websWrite(wp,"<dd class=\"definition\"><a href=\"http://ggew-net.de\"/></dd>\n");
websWrite(wp,"<dd class=\"definition\"> </dd>\n");
websWrite(wp,"<dd class=\"definition\">In Kooperation mit NewMedia-NET GmbH</dd>\n");
websWrite(wp,"<dd class=\"definition\"><a href=\"http://www.newmedia-net.de\"/></dd>\n");
websWrite(wp,"</dl>\n");
#endif*/
  return;
}


void
ej_dumpmeminfo (int eid, webs_t wp, int argc, char_t ** argv)
{
  FILE *fcpu = fopen ("/proc/meminfo", "r");
  if (fcpu == NULL)
    {
      return;
    }
  char buf[128];
  int n = 0;
rep:;
  if (n == EOF)
    {
      fclose (fcpu);
      return;
    }
  if (n)
    websWrite (wp, "'%s'", buf);
  n = fscanf (fcpu, "%s", buf);
  if (n != EOF)
    websWrite (wp, ",");
  goto rep;
}

#ifdef HAVE_RB500
void
ej_get_clkfreq (int eid, webs_t wp, int argc, char_t ** argv)
{
  FILE *fp = fopen ("/proc/cpuinfo", "rb");
  if (fp == NULL)
    {
      websWrite (wp, "unknown");
      return;
    }
  int cnt = 0;
  int b = 0;
  while (b != EOF)
    {
      b = getc (fp);
      if (b == ':')
	cnt++;

      if (cnt == 4)
	{
	  getc (fp);
	  char cpuclk[4];
	  cpuclk[0] = getc (fp);
	  cpuclk[1] = getc (fp);
	  cpuclk[2] = getc (fp);
	  cpuclk[3] = 0;
	  websWrite (wp, cpuclk);
	  fclose (fp);
	  return;
	}
    }

  fclose (fp);
  websWrite (wp, "unknown");
  return;
}

#elif HAVE_XSCALE
void
ej_get_clkfreq (int eid, webs_t wp, int argc, char_t ** argv)
{
  FILE *fp = fopen ("/proc/cpuinfo", "rb");
  if (fp == NULL)
    {
      websWrite (wp, "unknown");
      return;
    }
  int cnt = 0;
  int b = 0;
  while (b != EOF)
    {
      b = getc (fp);
      if (b == ':')
	cnt++;
      if (cnt == 2)
	{
	  getc (fp);
	  char cpuclk[4];
	  cpuclk[0] = getc (fp);
	  cpuclk[1] = getc (fp);
	  cpuclk[2] = getc (fp);
	  cpuclk[3] = 0;
	  websWrite (wp, cpuclk);
	  fclose (fp);
	  return;
	}
    }

  fclose (fp);
  websWrite (wp, "unknown");
  return;
}

#else


void
ej_get_clkfreq (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *clk = nvram_get ("clkfreq");
  if (clk == NULL)
    {
      websWrite (wp, "125");
      return;
    }
  char buf[64];
  strcpy (buf, clk);
  int i = 0;
  while (buf[i++] != 0)
    {
      if (buf[i] == ',')
	buf[i] = 0;
    }
  websWrite (wp, buf);
  return;
}
#endif

void
ej_show_cpuinfo (int eid, webs_t wp, int argc, char_t ** argv)
{
  FILE *fcpu = fopen ("/proc/cpuinfo", "r");
  if (fcpu == NULL)
    {
      websWrite (wp, "Not Detected!\n");
      return;
    }
  char buf[256];
  int i;
  for (i = 0; i < 256; i++)
    {
      int c = getc (fcpu);
      if (c == EOF)
	{
	  websWrite (wp, "Not Detected!\n");
	  fclose (fcpu);
	  return;
	}
      if (c == ':')
	break;
    }
  getc (fcpu);
  for (i = 0; i < 256; i++)
    {
      int c = getc (fcpu);
      if (c == EOF)
	{
	  websWrite (wp, "Not Detected!\n");
	  fclose (fcpu);
	  return;
	}
      if (c == 0xa || c == 0xd)
	break;
      buf[i] = c;
    }
  buf[i] = 0;
  websWrite (wp, buf);
  fclose (fcpu);
  return;
}

#define ASSOCLIST_TMP	"/tmp/.wl_assoclist"
#define RSSI_TMP	"/tmp/.rssi"
#define ASSOCLIST_CMD	"wl assoclist"
#define RSSI_CMD	"wl rssi"
#define NOISE_CMD	"wl noise"


void
ej_show_wds_subnet (int eid, webs_t wp, int argc, char_t ** argv)
{
  int index = -1;
  if (ejArgs (argc, argv, "%d", &index) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
  if (nvram_invmatch ("wl_br1_enable", "1"))
    return;
  char buf[16];
  sprintf (buf, "wl_wds%d_enable", index);
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"2\\\" %s >\" + wds.subnet + \"</option>\");</script>\n",
	     nvram_selmatch (wp, buf,
			     "2") ? "selected=\\\"selected\\\"" : "");
  return;
}



#ifdef HAVE_SKYTRON
void
ej_active_wireless2 (int eid, webs_t wp, int argc, char_t ** argv)
{
  int rssi = 0, noise = 0;
  FILE *fp, *fp2;
  char *mode;
  char mac[30];
  char list[2][30];
  char line[80];
  char cmd[80];

  unlink (ASSOCLIST_TMP);
  unlink (RSSI_TMP);

  mode = nvram_safe_get ("wl_mode");
  snprintf (cmd, sizeof (cmd), "%s > %s", ASSOCLIST_CMD, ASSOCLIST_TMP);
  system (cmd);			// get active wireless mac

  int connected = 0;
  if ((fp = fopen (ASSOCLIST_TMP, "r")))
    {
      while (fgets (line, sizeof (line), fp) != NULL)
	{
	  if (sscanf (line, "%s %s", list[0], mac) != 2)	// assoclist 00:11:22:33:44:55
	    continue;

	  if (strcmp (list[0], "assoclist"))
	    break;

	  rssi = 0;
	  noise = 0;
	  // get rssi value
	  if (strcmp (mode, "ap"))
	    snprintf (cmd, sizeof (cmd), "%s > %s", RSSI_CMD, RSSI_TMP);
	  else
	    snprintf (cmd, sizeof (cmd), "%s \"%s\" > %s", RSSI_CMD, mac,
		      RSSI_TMP);
	  system (cmd);

	  // get noise value if not ap mode
	  if (strcmp (mode, "ap"))
	    snprintf (cmd, sizeof (cmd), "%s >> %s", NOISE_CMD, RSSI_TMP);

	  system (cmd);		// get RSSI value for mac

	  fp2 = fopen (RSSI_TMP, "r");
	  if (fgets (line, sizeof (line), fp2) != NULL)
	    {

	      // get rssi
	      if (sscanf (line, "%s %s %d", list[0], list[1], &rssi) != 3)
		continue;

	      // get noise for client/wet mode
	      if (strcmp (mode, "ap") &&
		  fgets (line, sizeof (line), fp2) != NULL &&
		  sscanf (line, "%s %s %d", list[0], list[1], &noise) != 3)
		continue;

	      fclose (fp2);
	    }
	  if (nvram_match ("maskmac", "1"))
	    {
	      mac[0] = 'x';
	      mac[1] = 'x';
	      mac[3] = 'x';
	      mac[4] = 'x';
	      mac[6] = 'x';
	      mac[7] = 'x';
	      mac[9] = 'x';
	      mac[10] = 'x';
	    }
	  if (strcmp (mode, "ap") != 0)
	    {
	      connected = 1;
	      websWrite (wp, "<tr>\n");
	      websWrite (wp,
			 "<td bgcolor=\"#B2B2B2\" valign=\"center\" align=\"right\" width=\"200\" height=\"25\"><font face=\"Arial\" color=\"#000000\" size=\"2\"><b>Verbindungsstatus</b></font></td>\n");
	      websWrite (wp, "<td bgcolor=\"#B2B2B2\"></td>\n");
	      websWrite (wp, "<td bgcolor=\"#FFFFFF\"></td>\n");
	      websWrite (wp,
			 "<td colspan=\"2\" bgcolor=\"#FFFFFF\" valign=\"center\" align=\"left\"><font face=\"Arial\" color=\"#000000\" size=\"2\">Verbunden</font></td>\n");
	      websWrite (wp, "</tr>\n");
	      websWrite (wp, "<tr>\n");
	      websWrite (wp,
			 "<td bgcolor=\"#B2B2B2\" valign=\"center\" align=\"right\" width=\"200\" height=\"25\"><font face=\"Arial\" color=\"#000000\" size=\"2\">Signal</font></td>\n");
	      websWrite (wp, "<td bgcolor=\"#B2B2B2\"></td>\n");
	      websWrite (wp, "<td bgcolor=\"#FFFFFF\"></td>\n");
	      websWrite (wp,
			 "<td colspan=\"2\" bgcolor=\"#FFFFFF\" valign=\"center\" align=\"left\"><font face=\"Arial\" color=\"#000000\" size=\"2\">%d dBm</font></td>\n",
			 rssi);
	      websWrite (wp, "</tr>\n");
	      websWrite (wp, "<tr>\n");
	      websWrite (wp,
			 "<td bgcolor=\"#B2B2B2\" valign=\"center\" align=\"right\" width=\"200\" height=\"25\"><font face=\"Arial\" color=\"#000000\" size=\"2\">Rauschen</font></td>\n");
	      websWrite (wp, "<td bgcolor=\"#B2B2B2\"></td>\n");
	      websWrite (wp, "<td bgcolor=\"#FFFFFF\"></td>\n");
	      websWrite (wp,
			 "<td colspan=\"2\" bgcolor=\"#FFFFFF\" valign=\"center\" align=\"left\"><font face=\"Arial\" color=\"#000000\" size=\"2\">%d dBm</font></td>\n",
			 noise);
	      websWrite (wp, "</tr>\n");
	    }
	}
    }

  unlink (ASSOCLIST_TMP);
  unlink (RSSI_TMP);
  if (!connected)
    {
      connected = 1;
      websWrite (wp, "<tr>\n");
      websWrite (wp,
		 "<td bgcolor=\"#B2B2B2\" valign=\"center\" align=\"right\" width=\"200\" height=\"25\"><font face=\"Arial\" color=\"#000000\" size=\"2\"><b>Verbindungsstatus</b></font></td>\n");
      websWrite (wp, "<td bgcolor=\"#B2B2B2\"></td>\n");
      websWrite (wp, "<td bgcolor=\"#FFFFFF\"></td>\n");
      websWrite (wp,
		 "<td colspan=\"2\" bgcolor=\"#FFFFFF\" valign=\"center\" align=\"left\"><font face=\"Arial\" color=\"#000000\" size=\"2\">Nicht Verbunden</font></td>\n");
      websWrite (wp, "</tr>\n");

    }

  return 0;
}
#endif


int
macro_add (char *a)
{
  cprintf ("adding %s\n", a);

  char *count;
  int c;
  char buf[20];
  count = nvram_safe_get (a);
  cprintf ("count = %s\n", count);
  if (count != NULL && strlen (count) > 0)
    {
      c = atoi (count);
      if (c > -1)
	{
	  c++;
	  sprintf (buf, "%d", c);
	  cprintf ("set %s to %s\n", a, buf);
	  nvram_set (a, buf);
	}
    }
  return 0;
}

int
macro_rem (char *a, char *nv)
{
  char *count;
  int c, i, cnt;
  char buf[20];
  char *buffer, *b;
  cnt = 0;
  count = nvram_safe_get (a);
  if (count != NULL && strlen (count) > 0)
    {
      c = atoi (count);
      if (c > 0)
	{
	  c--;
	  sprintf (buf, "%d", c);
	  nvram_set (a, buf);
	  buffer = nvram_safe_get (nv);
	  if (buffer != NULL)
	    {
	      b = malloc (strlen (buffer) + 1);
	      for (i = 0; i < strlen (buffer); i++)
		{
		  if (buffer[i] == ' ')
		    cnt++;
		  if (cnt == c)
		    break;
		  b[i] = buffer[i];
		}
	      b[i] = 0;
	      nvram_set (nv, b);
	    }

	}
    }
  return 0;
}

int
forward_remove (webs_t wp)
{
  return macro_rem ("forward_entries", "forward_port");
}

int
forward_add (webs_t wp)
{
  return macro_add ("forward_entries");
}

int
lease_remove (webs_t wp)
{
  return macro_rem ("static_leasenum", "static_leases");
}

int
lease_add (webs_t wp)
{
  return macro_add ("static_leasenum");
}


int
forwardspec_remove (webs_t wp)
{
  return macro_rem ("forwardspec_entries", "forward_spec");
}

int
forwardspec_add (webs_t wp)
{
  return macro_add ("forwardspec_entries");
}

int
trigger_remove (webs_t wp)
{
  return macro_rem ("trigger_entries", "port_trigger");
}

int
trigger_add (webs_t wp)
{
  return macro_add ("trigger_entries");
}

void
ej_show_paypal (int eid, webs_t wp, int argc, char_t ** argv)
{
#ifndef CONFIG_BRANDING
  websWrite (wp, "<a href=\"http://www.dd-wrt.com/\">DD-WRT</a>");
  websWrite (wp,
	     "<form action=\"https://www.paypal.com/cgi-bin/webscr\" method=\"post\" target=\"_blank\">");
  websWrite (wp, "<input type=\"hidden\" name=\"cmd\" value=\"_xclick\" />");
  websWrite (wp,
	     "<input type=\"hidden\" name=\"business\" value=\"paypal@dd-wrt.com\" />");
  websWrite (wp,
	     "<input type=\"hidden\" name=\"item_name\" value=\"DD-WRT Development Support\" />");
  websWrite (wp, "<input type=\"hidden\" name=\"no_note\" value=\"1\" />");
  websWrite (wp,
	     "<input type=\"hidden\" name=\"currency_code\" value=\"EUR\" />");
  websWrite (wp, "<input type=\"hidden\" name=\"tax\" value=\"0\" />");
  websWrite (wp,
	     "<input type=\"image\" src=\"images/paypal.gif\" border=\"0\" name=\"submit\" width=\"62\" height=\"31\" />");
  websWrite (wp, "</form>");
  websWrite (wp,
	     "<br><script type=\"text/javascript\">Capture(donate.mb)</script><br>\n");
  websWrite (wp,
	     "<a href=\"https://www.moneybookers.com/app/send.pl\" target=\"_blank\">\n");
  if (!strcmp (DIST, "micro"))
    websWrite (wp,
	       "<img style=\"border-width: 1px; border-color: #8B8583;\" src=\"http://www.moneybookers.com/images/banners/88_en_interpayments.gif\" width=88 height=31 border=0>\n");
  else
    websWrite (wp,
	       "<img style=\"border-width: 1px; border-color: #8B8583;\" src=\"images/88_en_interpayments.gif\" width=88 height=31 border=0>\n");

  websWrite (wp, "</a>\n");
#endif
  return;
}

void
filterstring (char *str, char character)
{
  if (str == NULL)
    return;
  int c;
  int i;
  int len = strlen (str);
  c = 0;
  for (i = 0; i < len; i++)
    {
      if (str[i] != character)
	str[c++] = str[i];
    }
  str[c++] = 0;
}

char *
buildmac (char *in)
{
  char mac[20];
  char *outmac;
  outmac = malloc (20);
  strncpy (mac, in, 20);
  filterstring (mac, ':');
  filterstring (mac, '-');
  filterstring (mac, ' ');
  if (strlen (mac) != 12)
    {
      free (outmac);
      return NULL;		// error. invalid mac
    }
  int i;
  int c = 0;
  for (i = 0; i < 12; i += 2)
    {
      outmac[c++] = mac[i];
      outmac[c++] = mac[i + 1];
      if (i < 10)
	outmac[c++] = ':';
    }
  outmac[c++] = 0;
  return outmac;
}

#ifdef HAVE_RADLOCAL

int
raduser_add (webs_t wp)
{
  int radcount = 0;
  char *radc = nvram_get ("iradius_count");
  if (radc != NULL)
    radcount = atoi (radc);
  radcount++;
  char count[16];
  sprintf (count, "%d", radcount);
  nvram_set ("iradius_count", count);
  return 0;
}

void
ej_show_iradius_check (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *sln = nvram_safe_get ("iradius_count");
  if (sln == NULL || strlen (sln) == 0)
    return;
  int leasenum = atoi (sln);
  int i;
  for (i = 0; i < leasenum; i++)
    {
      websWrite (wp, "if(F._iradius%d_active)\n", i);
      websWrite (wp, "if(F._iradius%d_active.checked == true)\n", i);
      websWrite (wp, "F.iradius%d_active.value=1\n", i);
      websWrite (wp, "else\n");
      websWrite (wp, "F.iradius%d_active.value=0\n", i);

      websWrite (wp, "if(F._iradius%d_delete)\n", i);
      websWrite (wp, "if(F._iradius%d_delete.checked == true)\n", i);
      websWrite (wp, "F.iradius%d_delete.value=1\n", i);
      websWrite (wp, "else\n");
      websWrite (wp, "F.iradius%d_delete.value=0\n", i);
    }


}
void
ej_show_iradius (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *sln = nvram_safe_get ("iradius_count");
  if (sln == NULL || strlen (sln) == 0)
    return;
  int leasenum = atoi (sln);
  if (leasenum == 0)
    return;
  int i;
  char username[32];
  char *o, *userlist;
  cprintf ("get collection\n");
  char *u = nvram_get_collection ("iradius");
  cprintf ("collection result %s", u);
  if (u != NULL)
    {
      userlist = (char *) malloc (strlen (u) + 1);
      strcpy (userlist, u);
      free (u);
      o = userlist;
    }
  else
    {
      userlist = NULL;
      o = NULL;
    }
  cprintf ("display = chain\n");
  for (i = 0; i < leasenum; i++)
    {
      snprintf (username, 31, "iradius%d_name", i);
      char *sep = NULL;
      if (userlist)
	sep = strsep (&userlist, " ");
      websWrite (wp, "<tr><td>\n");
      websWrite (wp, "<input name=\"%s\" type=\"hidden\" />", username);
      websWrite (wp,
		 "<input name=\"%s\" value=\"%s\" size=\"25\" maxlength=\"63\" />\n",
		 username, sep != NULL ? sep : "");
      websWrite (wp, "</td>\n");
      if (userlist)
	sep = strsep (&userlist, " ");

      char active[32];
      snprintf (active, 31, "iradius%d_active", i);

      websWrite (wp, "<td>\n");
      websWrite (wp, "<input name=\"%s\" type=\"hidden\" />", active);
      websWrite (wp,
		 "<input type=\"checkbox\" value=\"%s\" name=\"_%s\" %s />\n",
		 sep, active, sep != NULL ? strcmp (sep,
						    "1") ==
		 0 ? "checked=\"checked\"" : "" : "");
      websWrite (wp, "</td>\n");
      websWrite (wp, "<td>\n");
      snprintf (active, 31, "iradius%d_delete", i);
      websWrite (wp, "<input name=\"%s\" type=\"hidden\" />", active);
      websWrite (wp, "<input type=\"checkbox\" name=\"_%s\"/>\n", active);
      websWrite (wp, "</td></tr>\n");
    }
  if (o != NULL)
    free (o);
  return;
}


void
validate_iradius (webs_t wp, char *value, struct variable *v)
{
  char username[32] = "iradiusxxx_name";
  char active[32] = "iradiusxxx_active";
  char del[32] = "iradiusxxx_delete";

  char *sln = nvram_safe_get ("iradius_count");
  if (sln == NULL || strlen (sln) == 0)
    return;
  int leasenum = atoi (sln);
  if (leasenum == 0)
    return;
  char *leases;
  int i;
  leases = (char *) malloc ((128 * leasenum) + 1);
  memset (leases, 0, (128 * leasenum) + 1);
  int leasen = 0;
  cprintf ("build mac list\n");
  for (i = 0; i < leasenum; i++)
    {
      snprintf (del, 31, "iradius%d_delete", i);
      char *d = websGetVar (wp, del, "");
      cprintf ("radius delete = %s\n", d);
      if (strcmp (d, "1") == 0)
	continue;

      snprintf (username, 31, "iradius%d_name", i);
      strcat (leases, websGetVar (wp, username, ""));
      strcat (leases, " ");

      snprintf (active, 31, "iradius%d_active", i);
      strcat (leases, websGetVar (wp, active, ""));
      strcat (leases, " ");
      leasen++;
    }

  cprintf ("done %s\n", leases);
  nvram_store_collection ("iradius", leases);
  cprintf ("stored\n");
  char nr[16];
  sprintf (nr, "%d", leasen);
  nvram_set ("iradius_count", nr);
  nvram_commit ();
  free (leases);
}




#endif

#ifdef HAVE_CHILLILOCAL

int
user_remove (webs_t wp)
{
  return macro_rem ("fon_usernames", "fon_userlist");
}

int
user_add (webs_t wp)
{
  return macro_add ("fon_usernames");
  //validate_userlist(wp);
}

void
ej_show_userlist (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *sln = nvram_safe_get ("fon_usernames");
  if (sln == NULL || strlen (sln) == 0)
    return;
  int leasenum = atoi (sln);
  if (leasenum == 0)
    return;
  int i;
  char username[32];
  char password[32];
  char *u = nvram_safe_get ("fon_userlist");
  char *userlist = (char *) malloc (strlen (u) + 1);
  strcpy (userlist, u);
  char *o = userlist;
  for (i = 0; i < leasenum; i++)
    {
      snprintf (username, 31, "fon_user%d_name", i);
      char *sep = strsep (&userlist, "=");
      websWrite (wp, "<tr><td>\n");
      websWrite (wp,
		 "<input name=\"%s\" value=\"%s\" size=\"25\" maxlength=\"63\" />\n",
		 username, sep != NULL ? sep : "");
      websWrite (wp, "</td>\n");
      sep = strsep (&userlist, " ");
      snprintf (password, 31, "fon_user%d_password", i);
      websWrite (wp, "<td>\n");
      websWrite (wp,
		 "<input type=\"password\" name=\"%s\" value=\"blahblahblah\" size=\"25\" maxlength=\"63\" />\n",
		 password);
      websWrite (wp, "</td></tr>\n");
    }
  free (o);
  return;
}


void
validate_userlist (webs_t wp, char *value, struct variable *v)
{
  char username[32] = "fon_userxxx_name";
  char password[32] = "fon_userxxx_password";
  char *sln = nvram_safe_get ("fon_usernames");
  if (sln == NULL || strlen (sln) == 0)
    return;
  int leasenum = atoi (sln);
  if (leasenum == 0)
    return;
  char *leases;
  int i;
  leases = (char *) malloc ((128 * leasenum) + 1);
  memset (leases, 0, (128 * leasenum) + 1);

  for (i = 0; i < leasenum; i++)
    {
      snprintf (username, 31, "fon_user%d_name", i);
      strcat (leases, websGetVar (wp, username, ""));
      strcat (leases, "=");
      snprintf (password, 31, "fon_user%d_password", i);
      strcat (leases, websGetVar (wp, password, ""));
      strcat (leases, " ");
    }
  nvram_set ("fon_userlist", leases);
  nvram_commit ();
  free (leases);
}


#endif



void
validate_staticleases (webs_t wp, char *value, struct variable *v)
{
  char lease_hwaddr[32] = "leasexxx_hwaddr";
  char lease_hostname[32] = "leasexxx_hostname";
  char lease_ip[32] = "leasexxx_ip";
  char *sln = nvram_safe_get ("static_leasenum");
  char *hwaddr;
  if (sln == NULL || strlen (sln) == 0)
    return;
  int leasenum = atoi (sln);
  if (leasenum == 0)
    return;
  char *leases;
  int i;
  leases = (char *) malloc ((54 * leasenum) + 1);
  memset (leases, 0, (54 * leasenum) + 1);

  for (i = 0; i < leasenum; i++)
    {
      snprintf (lease_hwaddr, 31, "lease%d_hwaddr", i);
      hwaddr = websGetVar (wp, lease_hwaddr, "");
      char *mac = buildmac (hwaddr);
      if (mac == NULL)
	{
	  free (leases);
	  websError (wp, 400, "%s is not a valid mac adress\n", hwaddr);
	  return;
	}
      strcat (leases, mac);
      free (mac);
      strcat (leases, "=");
      snprintf (lease_hostname, 31, "lease%d_hostname", i);
      strcat (leases, websGetVar (wp, lease_hostname, ""));
      strcat (leases, "=");
      snprintf (lease_ip, 31, "lease%d_ip", i);
      strcat (leases, websGetVar (wp, lease_ip, ""));
      strcat (leases, " ");
    }
  nvram_set ("static_leases", leases);
  nvram_commit ();
  free (leases);
}

void
ej_show_staticleases (int eid, webs_t wp, int argc, char_t ** argv)
{
  int i;
//cprintf("get static leasenum");

  char *sln = nvram_safe_get ("static_leasenum");
//cprintf("check null");
  if (sln == NULL || strlen (sln) == 0)
    return;
//cprintf("atoi");

  int leasenum = atoi (sln);
//cprintf("leasenum==0");
  if (leasenum == 0)
    return;
//cprintf("get leases");
  char *nvleases = nvram_safe_get ("static_leases");
  char *leases = (char *) malloc (strlen (nvleases) + 1);
  char *originalpointer = leases;	//strsep destroys the pointer by moving it
  strcpy (leases, nvleases);
  for (i = 0; i < leasenum; i++)
    {
      char *sep = strsep (&leases, "=");
      websWrite (wp,
		 "<tr><td><input name=\"lease%d_hwaddr\" value=\"%s\" size=\"18\" maxlength=\"18\" onblur=\"valid_name(this,share.mac,SPACE_NO)\" /></td>",
		 i, sep != NULL ? sep : "");
      sep = strsep (&leases, "=");
      websWrite (wp,
		 "<td><input name=\"lease%d_hostname\" value=\"%s\" size=\"24\" maxlength=\"24\" onblur=\"valid_name(this,share.hostname,SPACE_NO)\" /></td>",
		 i, sep != NULL ? sep : "");
      sep = strsep (&leases, " ");
      websWrite (wp,
		 "<td><input name=\"lease%d_ip\" value=\"%s\" size=\"15\" maxlength=\"15\" class=\"num\" onblur=\"valid_name(this,share.ip,SPACE_NO)\" /></td></tr>\n",
		 i, sep != NULL ? sep : "");
    }
  free (originalpointer);
  return;
}


void
ej_show_control (int eid, webs_t wp, int argc, char_t ** argv)
{
#ifdef CONFIG_BRANDING
  websWrite (wp, "Control Panel");
#else
  websWrite (wp, "DD-WRT Control Panel");
#endif
  return;
}


#ifndef HAVE_AQOS
void
ej_show_default_level (int eid, webs_t wp, int argc, char_t ** argv)
{
  return;
}

#else
void
ej_show_default_level (int eid, webs_t wp, int argc, char_t ** argv)
{
  websWrite (wp, "<fieldset>\n");
  websWrite (wp,
	     "<legend><script type=\"text/javascript\">Capture(qos.legend6)</script></legend>\n");
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\"><script type=\"text/javascript\">Capture(qos.bandwith)</script></div>\n");
  websWrite (wp,
	     "<input type=\"num\" name=\"default_level\" size=\"6\" value=\"%s\" />\n",
	     nvram_safe_get ("default_level"));
  websWrite (wp, "</div>\n");
  websWrite (wp, "</fieldset><br />\n");
  return;
}
#endif



#ifndef HAVE_MSSID

int
set_security (webs_t wp)
{
  char *var = websGetVar (wp, "security_varname", "security_mode");
  nvram_set (var, websGetVar (wp, var, "disabled"));
  return 0;
}

char *
selmatch (char *var, char *is, char *ret)
{
  if (nvram_match (var, is))
    return ret;
  return "";
}

void
ej_show_security (int eid, webs_t wp, int argc, char_t ** argv)
{
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.secmode)</script></div>\n");
  websWrite (wp,
	     "<select name=\"security_mode\" onchange=\"SelMode('security_mode',this.form.security_mode.selectedIndex,this.form)\">\n");
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"disabled\\\" %s >\" + share.disabled + \"</option>\");</script>\n",
	     selmatch ("security_mode", "disabled", "selected"));
  websWrite (wp, "<option value=\"psk\" %s>WPA Pre-Shared Key</option>\n",
	     selmatch ("security_mode", "psk", "selected"));
  websWrite (wp, "<option value=\"wpa\" %s>WPA RADIUS</option>\n",
	     selmatch ("security_mode", "wpa", "selected"));
  if (!nvram_match ("wl0_mode", "wet") && nvram_match ("wl_wds1_enable", "0") && nvram_match ("wl_wds2_enable", "0") && nvram_match ("wl_wds3_enable", "0") && nvram_match ("wl_wds4_enable", "0") && nvram_match ("wl_wds5_enable", "0") && nvram_match ("wl_wds6_enable", "0") && nvram_match ("wl_wds7_enable", "0") && nvram_match ("wl_wds8_enable", "0") && nvram_match ("wl_wds9_enable", "0") && nvram_match ("wl_wds10_enable", "0"))	//botho 10/04/06 : if wireless client bridge mode selected or WDS activated => we don't display WPA2 security modes
    {
      websWrite (wp,
		 "<option value=\"psk2\" %s>WPA2 Pre-Shared Key Only</option>\n",
		 selmatch ("security_mode", "psk2", "selected"));
      websWrite (wp, "<option value=\"wpa2\" %s>WPA2 RADIUS Only</option>\n",
		 selmatch ("security_mode", "wpa2", "selected"));
    }
  websWrite (wp,
	     "<option value=\"psk psk2\" %s>WPA2 Pre-Shared Key Mixed</option>\n",
	     selmatch ("security_mode", "psk psk2", "selected"));
  websWrite (wp, "<option value=\"wpa wpa2\" %s>WPA2 RADIUS Mixed</option>\n",
	     selmatch ("security_mode", "wpa wpa2", "selected"));
  websWrite (wp, "<option value=\"radius\" %s>RADIUS</option>\n",
	     selmatch ("security_mode", "radius", "selected"));
  websWrite (wp, "<option value=\"wep\" %s>WEP</option></select>\n",
	     selmatch ("security_mode", "wep", "selected"));
  websWrite (wp, "</div>\n");
  ej_show_wpa_setting (eid, wp, argc, argv);
  return;
}
#else
#ifdef HAVE_MADWIFI
struct wifi_channels
{
  int channel;
  int freq;
};
extern struct wifi_channels *list_channels (char *devnr);
//extern int getchannelcount (void);
extern int getdevicecount (void);
#endif


void
rep (char *in, char from, char to)
{
  int i;
  for (i = 0; i < strlen (in); i++)
    if (in[i] == from)
      in[i] = to;

}

int
set_security (webs_t wp)
{
  char *var = websGetVar (wp, "security_varname", "security_mode");
  cprintf ("set security to %s\n", var);
  cprintf ("security var = %s\n", websGetVar (wp, var, "disabled"));
  char *var2 = websGetVar (wp, var, "disabled");
//rep(var,'X','.');
  nvram_set (var, var2);
  return 0;
}

char *
selmatch (char *var, char *is, char *ret)
{
  if (nvram_match (var, is))
    return ret;
  return "";
}

static void
show_security_prefix (int eid, webs_t wp, int argc, char_t ** argv,
		      char *prefix)
{
  char var[80];
//char p2[80];
  cprintf ("show security prefix\n");
  sprintf (var, "%s_security_mode", prefix);
//strcpy(p2,prefix);
//rep(p2,'X','.');
//websWrite (wp, "<input type=\"hidden\" name=\"%s_security_mode\"/>\n",p2);
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp, "<div class=\"label\">Security Mode</div>\n");
  websWrite (wp,
	     "<select name=\"%s_security_mode\" onchange=\"SelMode('%s_security_mode',this.form.%s_security_mode.selectedIndex,this.form)\">\n",
	     prefix, prefix, prefix);
  websWrite (wp, "<option value=\"disabled\" %s>Disable</option>\n",
	     selmatch (var, "disabled", "selected"));
  websWrite (wp, "<option value=\"psk\" %s>WPA Pre-Shared Key</option>\n",
	     selmatch (var, "psk", "selected"));
  websWrite (wp, "<option value=\"wpa\" %s>WPA RADIUS</option>\n",
	     selmatch (var, "wpa", "selected"));
  websWrite (wp,
	     "<option value=\"psk2\" %s>WPA2 Pre-Shared Key Only</option>\n",
	     selmatch (var, "psk2", "selected"));
  websWrite (wp, "<option value=\"wpa2\" %s>WPA2 RADIUS Only</option>\n",
	     selmatch (var, "wpa2", "selected"));
  websWrite (wp,
	     "<option value=\"psk psk2\" %s>WPA2 Pre-Shared Key Mixed</option>\n",
	     selmatch (var, "psk psk2", "selected"));
  websWrite (wp, "<option value=\"wpa wpa2\" %s>WPA2 RADIUS Mixed</option>\n",
	     selmatch (var, "wpa wpa2", "selected"));
  websWrite (wp, "<option value=\"radius\" %s>RADIUS</option>\n",
	     selmatch (var, "radius", "selected"));
  websWrite (wp, "<option value=\"wep\" %s>WEP</option></select>\n",
	     selmatch (var, "wep", "selected"));
  websWrite (wp, "</div>\n");
  rep (prefix, 'X', '.');
  cprintf ("ej show wpa\n");
  ej_show_wpa_setting (eid, wp, argc, argv, prefix);

}

static void
ej_show_security_single (int eid, webs_t wp, int argc, char_t ** argv,
			 char *prefix)
{
  char *next;
  char var[80];
  char ssid[80];
  char vif[16];
  char mac[16];
  sprintf (mac, "%s_hwaddr", prefix);
  sprintf (vif, "%s_vifs", prefix);
  char *vifs = nvram_safe_get (vif);
  if (vifs == NULL)
    return 0;
  sprintf (ssid, "%s_ssid", prefix);
  websWrite (wp, "<fieldset>\n");
  //cprintf("getting %s %s\n",ssid,nvram_safe_get(ssid));
  websWrite (wp,
	     "<legend>Physical Interface %s SSID [%s] HWAddr [%s]</legend>\n",
	     prefix, nvram_safe_get (ssid), nvram_safe_get (mac));
  show_security_prefix (eid, wp, argc, argv, prefix);
  websWrite (wp, "</fieldset>\n");
  foreach (var, vifs, next)
  {
    sprintf (ssid, "%s_ssid", var);
    websWrite (wp, "<br />\n");
    websWrite (wp, "<fieldset>\n");
    //cprintf("getting %s %s\n",ssid,nvram_safe_get(ssid));
    websWrite (wp, "<legend>Virtual Interface %s SSID [%s]</legend>\n", var,
	       nvram_get (ssid));
    rep (var, '.', 'X');
    show_security_prefix (eid, wp, argc, argv, var);
    websWrite (wp, "</fieldset>\n");
  }

}

void
ej_show_security (int eid, webs_t wp, int argc, char_t ** argv)
{
#ifndef HAVE_MADWIFI
  return ej_show_security_single (eid, wp, argc, argv, "wl0");
#else
  int c = getdevicecount ();
  int i;
  for (i = 0; i < c; i++)
    {
      char buf[16];
      sprintf (buf, "ath%d", i);
      ej_show_security_single (eid, wp, argc, argv, buf);
    }
  return;
#endif
}

#endif



int
dhcpfwd (webs_t wp)
{
  int ret = 0;
  char *enable;

  enable = websGetVar (wp, "dhcpfwd_enable", NULL);
  nvram_set ("dhcpfwd_enable", enable);

  return ret;
}

int
wan_proto (webs_t wp)
{
  int ret = 0;
  char *enable;

  enable = websGetVar (wp, "wan_proto", NULL);
  nvram_set ("wan_proto", enable);

  return ret;
}


void
ej_show_dhcpd_settings (int eid, webs_t wp, int argc, char_t ** argv)
{
  int i;
  if (nvram_match ("wl_mode", "wet"))	//dhcpd settings disabled in client bridge mode, so we wont display it
    return;
  websWrite (wp,
	     "<fieldset><legend><script type=\"text/javascript\">Capture(idx.dhcp_legend)</script></legend>\n");
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_type)</script></div>\n");
  websWrite (wp,
	     "<select class=\"num\" size=\"1\" name=\"dhcpfwd_enable\" onchange=SelDHCPFWD(this.form.dhcpfwd_enable.selectedIndex,this.form)>\n");
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"0\\\" %s >\" + idx.dhcp_srv + \"</option>\");</script>\n",
	     nvram_match ("dhcpfwd_enable",
			  "0") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"1\\\" %s >\" + idx.dhcp_fwd + \"</option>\");</script>\n",
	     nvram_match ("dhcpfwd_enable",
			  "1") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp, "</select>\n");
  websWrite (wp, "</div>\n");
  if (nvram_match ("dhcpfwd_enable", "1"))
    {
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_srv)</script></div>\n");
      char *ipfwd = nvram_safe_get ("dhcpfwd_ip");
      websWrite (wp,
		 "<input type=\"hidden\" name=\"dhcpfwd_ip\" value=\"4\" /><input class=\"num\" maxlength=\"3\" size=\"3\" name=\"dhcpfwd_ip_0\" onblur=\"valid_range(this,0,255,idx.dhcp_srv)\" value=\"%d\" />.<input class=\"num\" maxlength=\"3\" size=\"3\" name=\"dhcpfwd_ip_1\" onblur=\"valid_range(this,0,255,idx.dhcp_srv)\" value=\"%d\" />.<input class=\"num\" maxlength=\"3\" name=\"dhcpfwd_ip_2\" size=\"3\" onblur=\"valid_range(this,0,255,idx.dhcp_srv)\" value=\"%d\" />.<input class=\"num\" maxlength=\"3\" name=\"dhcpfwd_ip_3\" size=\"3\" onblur=\"valid_range(this,0,254,idx.dhcp_srv)\" value=\"%d\"\" /></div>\n",
		 get_single_ip (ipfwd, 0), get_single_ip (ipfwd, 1),
		 get_single_ip (ipfwd, 2), get_single_ip (ipfwd, 3));
    }
  else
    {
      char buf[20];
      prefix_ip_get ("lan_ipaddr", buf, 1);
      websWrite (wp, "<div class=\"setting\">\n");
//      char *nv = nvram_safe_get ("wan_wins");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_srv)</script></div><input class=\"spaceradio\" type=\"radio\" name=\"lan_proto\" value=\"dhcp\" onclick=SelDHCP('dhcp',this.form) %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
		 nvram_match ("lan_proto",
			      "dhcp") ? "checked=\"checked\"" : "");
      websWrite (wp,
		 "<input class=\"spaceradio\" type=\"radio\" name=\"lan_proto\" value=\"static\" onclick=\"SelDHCP('static',this.form)\" %s /><script type=\"text/javascript\">Capture(share.disable)</script></div><input type=\"hidden\" name=\"dhcp_check\" /><div class=\"setting\">\n",
		 nvram_match ("lan_proto",
			      "static") ? "checked=\"checked\"" : "");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_start)</script></div>%s",
		 buf);
      websWrite (wp,
		 "<input class=\"num\" name=\"dhcp_start\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,1,254,idx.dhcp_start)\" value=\"%s\" />",
		 nvram_safe_get ("dhcp_start"));
      websWrite (wp, "</div>\n");
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_maxusers)</script></div><input class=\"num\" name=\"dhcp_num\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,1,253,idx.dhcp_maxusers)\" value=\"%s\" /></div>\n",
		 nvram_safe_get ("dhcp_num"));
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_lease)</script></div><input class=\"num\" name=\"dhcp_lease\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,0,9999,idx.dhcp_lease)\" value=\"%s\" > <script type=\"text/javascript\">Capture(share.minutes)</script></input></div>\n",
		 nvram_safe_get ("dhcp_lease"));
      if (nvram_invmatch ("wan_proto", "static"))
	{
	  websWrite (wp, "<div class=\"setting\">\n");
	  websWrite (wp,
		     "<div class=\"label\"><script type=\"text/javascript\">Capture(idx_static.dns)</script> 1</div>");
	  websWrite (wp,
		     "<input type=\"hidden\" name=\"wan_dns\" value=\"4\" />");
	  for (i = 0; i < 4; i++)
	    websWrite (wp,
		       "<input class=\"num\" name=\"wan_dns0_%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,%d,idx_static.dns)\" value=\"%d\" />%s",
		       i, i == 3 ? 254 : 255, get_dns_ip ("wan_dns", 0, i),
		       i < 3 ? "." : "");

	  websWrite (wp, "\n</div>\n<div class=\"setting\">\n");
	  websWrite (wp,
		     "<div class=\"label\"><script type=\"text/javascript\">Capture(idx_static.dns)</script> 2</div>");
	  for (i = 0; i < 4; i++)
	    websWrite (wp,
		       "<input class=\"num\" name=\"wan_dns1_%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,%d,idx_static.dns)\" value=\"%d\" />%s",
		       i, i == 3 ? 254 : 255, get_dns_ip ("wan_dns", 1, i),
		       i < 3 ? "." : "");

	  websWrite (wp, "\n</div>\n<div class=\"setting\">\n");
	  websWrite (wp,
		     "<div class=\"label\"><script type=\"text/javascript\">Capture(idx_static.dns)</script> 3</div>");
	  for (i = 0; i < 4; i++)
	    websWrite (wp,
		       "<input class=\"num\" name=\"wan_dns2_%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,%d,idx_static.dns)\" value=\"%d\" />%s",
		       i, i == 3 ? 254 : 255, get_dns_ip ("wan_dns", 2, i),
		       i < 3 ? "." : "");
	  websWrite (wp, "\n</div>");
	}
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp, "<div class=\"label\">WINS</div>\n");
      websWrite (wp,
		 "<input type=\"hidden\" name=\"wan_wins\" value=\"4\" />\n");
      char *wins = nvram_safe_get ("wan_wins");
      for (i = 0; i < 4; i++)
	{
	  websWrite (wp,
		     "<input class=\"num\" name=\"wan_wins_%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,%d,&#34;WINS&#34;)\" value=\"%d\" />%s",
		     i, i == 3 ? 254 : 255, get_single_ip (wins, i),
		     i < 3 ? "." : "");
	}

      websWrite (wp, "</div>\n<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dhcp_dnsmasq)</script></div>\n");
      websWrite (wp,
		 "<input type=\"checkbox\" name=\"_dhcp_dnsmasq\" value=\"1\" onclick=\"setDNSMasq(this.form)\" %s />\n",
		 nvram_match ("dhcp_dnsmasq",
			      "1") ? "checked=\"checked\"" : "");
      websWrite (wp, "</div>\n<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.dns_dnsmasq)</script></div>\n");
      websWrite (wp,
		 "<input type=\"checkbox\" name=\"_dns_dnsmasq\" value=\"1\" %s />\n",
		 nvram_match ("dns_dnsmasq",
			      "1") ? "checked=\"checked\"" : "");
      websWrite (wp, "</div>\n<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(idx.auth_dnsmasq)</script></div>\n");
      websWrite (wp,
		 "<input type=\"checkbox\" name=\"_auth_dnsmasq\" value=\"1\" %s />\n",
		 nvram_match ("auth_dnsmasq",
			      "1") ? "checked=\"checked\"" : "");
      websWrite (wp, "</div>\n");
    }

  websWrite (wp, "</fieldset><br />\n");
  return;
}


static void
showOption (webs_t wp, char *propname, char *nvname)
{
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\"><script type=\"text/javascript\">Capture(%s)</script></div><select name=\"%s\">\n",
	     propname, nvname);
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"0\\\" %s>\" + share.disabled + \"</option>\");</script>\n",
	     nvram_match (nvname, "0") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"1\\\" %s>\" + share.enabled + \"</option>\");</script>\n</select>\n",
	     nvram_match (nvname, "1") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp, "</div>\n");

}

static void
showDynOption (webs_t wp, char *propname, char *nvname, char *options[],
	       char *names[])
{
  int i;
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\">%s</div><select name=\"%s\">\n",
	     propname, nvname);
  for (i = 0; options[i] != NULL; i++)
    {
      websWrite (wp,
		 "<option value=\"%s\" %s>Off</option>\n",
		 names[i], nvram_match (nvname,
					options[i]) ? "selected=\"selected\""
		 : "");
    }
  websWrite (wp, "</div>\n");

}

#ifdef HAVE_MSSID

#ifdef HAVE_MADWIFI


/*
0x10 (FCC)
0x20 (DOC)
0x30 (ETSI)
0x31 (Spain)
0x32 (France)
0x40 (MKK-Japan)
0xFF (debug)

*/

#ifdef REGDOMAIN_OVERRIDE

struct regdomain
{
  char *name;
  int code;
};

static struct regdomain regdomains[] = {
  {"UNDEFINED", 0x00},
  {"USA1 (FCC)", 0x10},		//FCC
  {"USA2", 0x3A},
  {"DOC", 0x20},
  {"ETSI", 0x30},
  {"SPAIN", 0x31},
  {"FRANCE", 0x32},
  {"JAPAN", 0x40},
  {"WORLD0", 0x60},
  {"WORLD1", 0x61},
  {"WORLD2", 0x62},
  {"WORLD3", 0x63},
  {"WORLD4", 0x64},
  {"WORLD5", 0x65},
  {NULL, 0}
};
#endif

#endif



void
show_channel (webs_t wp, char *dev, char *prefix, int type)
{
  char wl_mode[16];
  sprintf (wl_mode, "%s_mode", prefix);
  char wl_net_mode[16];
  sprintf (wl_net_mode, "%s_net_mode", prefix);
  if (nvram_match (wl_mode, "ap") || nvram_match (wl_mode, "apsta")
      || nvram_match (wl_mode, "wdsap") || nvram_match (wl_mode, "infra"))
    {
      char wl_channel[16];
      sprintf (wl_channel, "%s_channel", prefix);
      char wl_wchannel[16];
      sprintf (wl_wchannel, "%s_wchannel", prefix);
      char wl_nbw[16];
      sprintf (wl_wchannel, "%s_nbw", prefix);

      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label4)</script></div><select name=\"%s\" onfocus=\"check_action(this,0)\"><script type=\"text/javascript\">\n",
		 wl_channel);
#ifdef HAVE_MADWIFI
      struct wifi_channels *chan;
      char cn[32];
      chan = list_channels (prefix);
      if (chan == NULL)
	chan = list_channels (dev);
      if (chan != NULL)
	{
	  //int cnt = getchannelcount ();
	  websWrite (wp,
		     "document.write(\"<option value=\\\"0\\\" %s>\" + share.auto + \"</option>\");\n",
		     nvram_match (wl_channel,
				  "0") ? "selected=\\\"selected\\\"" : "");
	  int i = 0;
	  while (chan[i].freq != -1)
	    {
	      cprintf ("%d\n", chan[i].channel);
	      cprintf ("%d\n", chan[i].freq);

	      sprintf (cn, "%d", chan[i].channel);
	      websWrite (wp,
			 "document.write(\"<option value=\\\"%s\\\" %s>%s - %dMhz</option>\");\n",
			 cn, nvram_match (wl_channel,
					  cn) ? "selected=\\\"selected\\\"" :
			 "", cn, chan[i].freq);
	      //free (chan[i].freq);
	      i++;
	    }
	  free (chan);
	}
#else
      if (type == 1 && !nvram_match (wl_net_mode, "g-only")
	  && !nvram_match (wl_net_mode, "a-only")
	  && !nvram_match (wl_net_mode, "bg-mixed")
	  && nvram_match (wl_nbw, "40"))
	{
	  int ch = atoi (nvram_safe_get ("wl0_wchannel"));
	  websWrite (wp, "var max_channel = 2;\n");
	  websWrite (wp, "var wl0_channel = '%s';\n",
		     nvram_safe_get (wl_wchannel));
	  websWrite (wp,
		     "var freq = new Array(\"Auto\",\"2.412\",\"2.417\",\"2.422\",\"2.427\",\"2.432\",\"2.437\",\"2.442\",\"2.447\",\"2.452\",\"2.457\",\"2.462\",\"2.467\",\"2.472\",\"2.484\");\n");
	  char *sel = "";
	  if (nvram_match ("wl0_nctrlsb", "lower"))
	    sel = "selected";

	  websWrite (wp,
		     " document.write(\"<option value=%d %s>%d - \"+freq[%d]+\"GHz</option>\");\n",
		     ch - 2, sel, ch - 2, ch - 2);
	  if (nvram_match ("wl0_nctrlsb", "upper"))
	    sel = "selected";
	  websWrite (wp,
		     " document.write(\"<option value=%d %s>%d - \"+freq[%d]+\"GHz</option>\");\n",
		     ch + 2, sel, ch + 2, ch + 2);

	}
      else
	{

	  unsigned int chanlist[128];
	  int chancount = getchannels (chanlist);

	  websWrite (wp, "var max_channel = %d;\n", chancount);
	  websWrite (wp, "var wl0_channel = '%s';\n",
		     nvram_safe_get (wl_channel));
	  websWrite (wp, "var offset = %d;\n", chanlist[0]);
	  websWrite (wp, "var buf = \"\";");
	  websWrite (wp, "var freq = new Array(\"Auto\",");
	  int i;
	  for (i = 0; i < chancount; i++)
	    {
	      float ofs;
	      if (chanlist[i] < 25)
		ofs = 2.407f;
	      else
		ofs = 5.000f;
	      ofs += (float) (chanlist[i] * 0.005f);
	      websWrite (wp, "\"%0.3f\"", ofs);
	      if (i != chancount - 1)
		websWrite (wp, ",");
	    }
	  websWrite (wp, ");\n");
	  websWrite (wp, "	for(i=0; i<=max_channel ; i++){\n");
	  websWrite (wp,
		     "		if(i == wl0_channel)	buf = \"selected\";\n");
	  websWrite (wp,
		     "		else			buf = \"\";\n");
	  websWrite (wp, "		if(i==0)\n");
	  websWrite (wp,
		     "		 document.write(\"<option value=\"+i+\" \"+buf+\">\" + share.auto + \"</option>\");\n");
	  websWrite (wp, "		else\n");
	  websWrite (wp,
		     "		 document.write(\"<option value=\"+i+\" \"+buf+\">\"+(i+offset-1)+\" - \"+freq[i]+\"GHz</option>\");\n");
	  websWrite (wp, "}\n");
	}
#endif
      websWrite (wp, "</script></select></div>\n");
    }
}


void
show_netmode (webs_t wp, char *prefix)
{
  char wl_net_mode[16];
  sprintf (wl_net_mode, "%s_net_mode", prefix);

  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label2)</script></div><select name=\"%s\" onchange=\"SelWL(this.form.%s.selectedIndex,this.form)\">\n",
	     wl_net_mode, wl_net_mode);
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"disabled\\\" %s>\" + share.disabled + \"</option>\");</script>\n",
	     nvram_match (wl_net_mode,
			  "disabled") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"mixed\\\" %s>\" + wl_basic.mixed + \"</option>\");</script>\n",
	     nvram_match (wl_net_mode,
			  "mixed") ? "selected=\\\"selected\\\"" : "");
  if (has_mimo ())
    {
      websWrite (wp,
		 "<script type=\"text/javascript\">document.write(\"<option value=\\\"bg-mixed\\\" %s>\" + wl_basic.bg + \"</option>\");</script>\n",
		 nvram_match (wl_net_mode,
			      "bg-mixed") ? "selected=\\\"selected\\\"" : "");
    }
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"b-only\\\" %s>\" + wl_basic.b + \"</option>\");</script>\n",
	     nvram_match (wl_net_mode,
			  "b-only") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"g-only\\\" %s>\" + wl_basic.g + \"</option>\");</script>\n",
	     nvram_match (wl_net_mode,
			  "g-only") ? "selected=\\\"selected\\\"" : "");
  if (has_mimo ())
    {
      websWrite (wp,
		 "<script type=\"text/javascript\">document.write(\"<option value=\\\"n-only\\\" %s>\" + wl_basic.n + \"</option>\");</script>\n",
		 nvram_match (wl_net_mode,
			      "n-only") ? "selected=\\\"selected\\\"" : "");
    }
#ifndef HAVE_MADWIFI
  if (nvram_match ("wl0_phytypes", "ga") || nvram_match ("wl0_phytypes", "a"))
#endif
    websWrite (wp,
	       "<script type=\"text/javascript\">document.write(\"<option value=\\\"a-only\\\" %s>\" + wl_basic.a + \"</option>\");</script>\n",
	       nvram_match (wl_net_mode,
			    "a-only") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp, "</select>\n");
  websWrite (wp, "</div>\n");
}


int
show_virtualssid (webs_t wp, char *prefix)
{
  char *next;
  char var[80];
  char ssid[80];
  char vif[16];
//  char mac[16];
  sprintf (vif, "%s_vifs", prefix);
  char *vifs = nvram_safe_get (vif);
  if (vifs == NULL)
    return 0;
#ifndef HAVE_MADWIFI
  if (!nvram_match ("wl0_mode", "ap") && !nvram_match ("wl0_mode", "apsta"))
    return 0;
#endif
  int count = 1;
  websWrite (wp,
	     "<h2><script type=\"text/javascript\">Capture(wl_basic.h2_vi)</script></h2>\n");
  foreach (var, vifs, next)
  {
//    sprintf (mac, "%s_hwaddr", var);  ?? doesn't exists
    sprintf (ssid, "%s_ssid", var);
    websWrite (wp,
	       "<fieldset><legend>Interface %s SSID [%s]</legend>\n",
	       var, nvram_safe_get (ssid));
//             var, nvram_safe_get (ssid), nvram_safe_get (mac));
    websWrite (wp, "<div class=\"setting\">\n");
    websWrite (wp,
	       "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label3)</script></div>\n");

    websWrite (wp,
	       "<input name=\"%s_ssid\" size=\"20\" maxlength=\"32\" onblur=\"valid_name(this,wl_basic.label3)\" value=\"%s\" /></div>\n",
	       var, nvram_safe_get (ssid));
    websWrite (wp, "<div class=\"setting\">\n");
    websWrite (wp,
	       "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label5)</script></div>");
    sprintf (ssid, "%s_closed", var);
    websWrite (wp,
	       "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_closed\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
	       var, nvram_match (ssid, "0") ? "checked=\"checked\"" : "");
    websWrite (wp,
	       "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_closed\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>\n",
	       var, nvram_match (ssid, "1") ? "checked=\"checked\"" : "");
    websWrite (wp, "</div>\n");
#ifdef HAVE_MADWIFI
    char wl_mode[16];
    sprintf (wl_mode, "%s_mode", var);
    websWrite (wp,
	       "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label)</script></div><select name=\"%s\" >\n",
	       wl_mode);
    websWrite (wp,
	       "<script type=\"text/javascript\">document.write(\"<option value=\\\"ap\\\" %s >\" + wl_basic.ap + \"</option>\");</script>\n",
	       nvram_match (wl_mode,
			    "ap") ? "selected=\\\"selected\\\"" : "");
    websWrite (wp,
	       "<script type=\"text/javascript\">document.write(\"<option value=\\\"wdssta\\\" %s >\" + wl_basic.wdssta + \"</option>\");</script>\n",
	       nvram_match (wl_mode,
			    "wdssta") ? "selected=\\\"selected\\\"" : "");
    websWrite (wp,
	       "<script type=\"text/javascript\">document.write(\"<option value=\\\"wdsap\\\" %s >\" + wl_basic.wdsap + \"</option>\");</script>\n",
	       nvram_match (wl_mode,
			    "wdsap") ? "selected=\\\"selected\\\"" : "");
    websWrite (wp, "</select>\n");
    websWrite (wp, "</div>\n");
#endif
//mode
//    show_netmode (wp, var);
//    show_channel (wp, prefix, var);
    sprintf (ssid, "%s_ap_isolate", var);
    showOption (wp, "wl_adv.label11", ssid);

    websWrite (wp,
	       "<div class=\"setting\">\n<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.network)</script></div>\n");
    sprintf (ssid, "%s_bridged", var);
    websWrite (wp,
	       "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s_bridged\" %s><script type=\"text/javascript\">Capture(wl_basic.unbridged)</script></input>&nbsp;\n",
	       var, nvram_match (ssid, "0") ? "checked=\"checked\"" : "");
    websWrite (wp,
	       "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s_bridged\" %s><script type=\"text/javascript\">Capture(wl_basic.bridged)</script></input>\n",
	       var, nvram_match (ssid, "1") ? "checked=\"checked\"" : "");
    websWrite (wp, "</div>\n");
    if (nvram_match (ssid, "0"))
      {
//      websWrite (wp, "<legend>Network Settings</legend>\n");
	websWrite (wp, "<div class=\"setting\">\n");
	websWrite (wp,
		   "<div class=\"label\"><script type=\"text/javascript\">Capture(share.ip)</script></div>\n");
	char ip[32];
	sprintf (ip, "%s_ipaddr", var);
	char *ipv = nvram_safe_get (ip);
	websWrite (wp,
		   "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,1,223,share.ip)\" name=\"%s_ipaddr_0\" value=\"%d\" />.",
		   var, get_single_ip (ipv, 0));
	websWrite (wp,
		   "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.ip)\" name=\"%s_ipaddr_1\" value=\"%d\" />.",
		   var, get_single_ip (ipv, 1));
	websWrite (wp,
		   "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.ip)\" name=\"%s_ipaddr_2\" value=\"%d\" />.",
		   var, get_single_ip (ipv, 2));
	websWrite (wp,
		   "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,'IP')\" name=\"%s_ipaddr_3\" value=\"%d\" />\n",
		   var, get_single_ip (ipv, 3));
	websWrite (wp, "</div>\n");
	websWrite (wp, "<div class=\"setting\">\n");
	websWrite (wp,
		   "<div class=\"label\"><script type=\"text/javascript\">Capture(share.subnet)</script></div>\n");
	sprintf (ip, "%s_netmask", var);
	ipv = nvram_safe_get (ip);

	websWrite (wp,
		   "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_netmask_0\" value=\"%d\" />.",
		   var, get_single_ip (ipv, 0));
	websWrite (wp,
		   "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_netmask_1\" value=\"%d\" />.",
		   var, get_single_ip (ipv, 1));
	websWrite (wp,
		   "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_netmask_2\" value=\"%d\" />.",
		   var, get_single_ip (ipv, 2));
	websWrite (wp,
		   "<input class=\"num\" maxlength=\"3\" size=\"3\" onblur=\"valid_range(this,0,255,share.subnet)\" name=\"%s_netmask_3\" value=\"%d\" />.",
		   var, get_single_ip (ipv, 3));
	websWrite (wp, "</div>\n");
      }
    websWrite (wp, "</fieldset><br />\n");
    count++;
  }
#ifdef HAVE_MADWIFI
  if (count < 16)
#else
  if (count < WL_MAXBSSCFG)
#endif
    websWrite (wp, "<div class=\"center\">\n");
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<input class=\\\"center\\\" type=\\\"button\\\" value=\\\"\" + sbutton.add + \"\\\" onclick=\\\"vifs_add_submit(this.form,'%s')\\\" />\");</script>\n",
	     prefix);
  if (count > 1)
    websWrite (wp,
	       "<script type=\"text/javascript\">document.write(\"<input class=\\\"center\\\" type=\\\"button\\\" value=\\\"\" + sbutton.remove + \"\\\" onclick=\\\"vifs_remove_submit(this.form,'%s')\\\" />\");</script>\n",
	       prefix);
  websWrite (wp, "</div><br />\n");

  return 0;
}

int
get_vifcount (char *prefix)
{
  char *next;
  char var[80];
  char wif[16];
  sprintf (wif, "%s_vifs", prefix);
  char *vifs = nvram_safe_get (wif);
  if (vifs == NULL)
    return 0;
  int count = 0;
  foreach (var, vifs, next)
  {
    count++;
  }
  return count;
}


int
add_vifs_single (char *prefix, int device)
{
  int count = get_vifcount (prefix);
  if (count == 16)
    return 0;
  char vif[16];
  sprintf (vif, "%s_vifs", prefix);
  char *vifs = nvram_safe_get (vif);
  if (vifs == NULL)
    return 0;
  char *n = (char *) malloc (strlen (vifs) + 8);
  char v[80];
  char v2[80];
#ifdef HAVE_MADWIFI
//  char *cou[] = { "a", "b", "c", "d", "e", "f" };
  sprintf (v, "ath%d.%d", device, count + 1);
#else
  sprintf (v, "wl%d.%d", device, count + 1);
#endif
  if (strlen (vifs) == 0)
    sprintf (n, "%s", v);
  else
    sprintf (n, "%s %s", vifs, v);
  sprintf (v2, "%s_closed", v);
  nvram_set (v2, "0");
  sprintf (v2, "%s_mode", v);
  nvram_set (v2, "ap");

  sprintf (v2, "%s_ap_isolate", v);
  nvram_set (v2, "0");
  sprintf (v2, "%s_ssid", v);
  nvram_set (v2, "default");
  sprintf (v2, "%s_vifs", prefix);
  nvram_set (v2, n);
  sprintf (v2, "%s_bridged", v);
  nvram_set (v2, "1");
  sprintf (v2, "%s_ipaddr", v);
  nvram_set (v2, "0.0.0.0");
  sprintf (v2, "%s_netmask", v);
  nvram_set (v2, "0.0.0.0");

  sprintf (v2, "%s_gtk_rekey", v);
  nvram_set (v2, "3600");

  sprintf (v2, "%s_radius_port", v);
  nvram_set (v2, "1812");

  sprintf (v2, "%s_radius_ipaddr", v);
  nvram_set (v2, "0.0.0.0");

  //nvram_commit ();
  free (n);
  return 0;
}

int
add_vifs (webs_t wp)
{
  char *prefix = websGetVar (wp, "iface", NULL);
  if (prefix == NULL)
    return 0;
  int devcount = prefix[strlen (prefix) - 1] - '0';
  return add_vifs_single (prefix, devcount);
}

int
remove_vifs_single (char *prefix)
{
  char *next;

  char var[80];
  char wif[16];
  sprintf (wif, "%s_vifs", prefix);
  int o = -1;
  char *vifs = nvram_safe_get (wif);
  char copy[128];
  strcpy (copy, vifs);
  int i;

  for (i = 0; i < strlen (copy); i++)
    {
      if (copy[i] == 0x20)
	o = i;
    }
  if (o == -1)
    {
      nvram_set (wif, "");
    }
  else
    {
      copy[o] = 0;
      nvram_set (wif, copy);
    }
  //nvram_commit ();
  return 0;
}

int
remove_vifs (webs_t wp)
{
  char *prefix = websGetVar (wp, "iface", NULL);
  return remove_vifs_single (prefix);
}
#endif

void
copytonv (webs_t wp, char *n)
{
  char *wl = websGetVar (wp, n, NULL);
  cprintf ("copy value %s which is [%s] to nvram\n", n, wl);
  if (wl)
    nvram_set (n, wl);
}

static void
save_prefix (webs_t wp, char *prefix)
{
  char n[80];
  sprintf (n, "%s_ssid", prefix);
  copytonv (wp, n);
  if (!strcmp (prefix, "wl0"))
    {
      char *wl = websGetVar (wp, n, NULL);
      cprintf ("copy value %s which is [%s] to nvram\n", n, wl);
      if (wl)
	nvram_set ("wl_ssid", wl);
    }
  sprintf (n, "%s_distance", prefix);
  copytonv (wp, n);
#ifdef HAVE_MADWIFI
  sprintf (n, "%s_txpwr", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_regdomain", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_turbo", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_xr", prefix);
  copytonv (wp, n);
//  sprintf (n, "%s_xchanmode", prefix);
//  copytonv (wp, n);
//  sprintf (n, "%s_outdoor", prefix);
//  copytonv (wp, n);
  sprintf (n, "%s_diversity", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_txantenna", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_rxantenna", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_channelbw", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_xr", prefix);
  copytonv (wp, n);
#endif
  sprintf (n, "%s_closed", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_bridged", prefix);
  copytonv (wp, n);

  char addr[32];
  sprintf (n, "%s_ipaddr", prefix);
  if (get_merge_ipaddr (wp, n, addr))
    nvram_set (n, addr);

  sprintf (n, "%s_netmask", prefix);
  if (get_merge_ipaddr (wp, n, addr))
    nvram_set (n, addr);

  copytonv (wp, n);
  sprintf (n, "%s_ap_isolate", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_mode", prefix);
  copytonv (wp, n);
  if (!strcmp (prefix, "wl0"))
    {
      char *wl = websGetVar (wp, n, NULL);
      cprintf ("copy value %s which is [%s] to nvram\n", n, wl);
      if (wl)
	nvram_set ("wl_mode", wl);
#ifndef HAVE_MADWIFI
      if (strcmp (wl, "ap") && strcmp (wl, "apsta"))
	{
	  nvram_set ("wl0_vifs", "");
	}
#endif
    }
  if (!strcmp (prefix, "wl0"))
    {
      sprintf (n, "%s_net_mode", prefix);
      copytonv (wp, n);
      convert_wl_gmode (nvram_safe_get (n));
    }
  sprintf (n, "%s_nbw", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_nctrlsb", prefix);
  copytonv (wp, n);



  sprintf (n, "%s_channel", prefix);
  if (!strcmp (prefix, "wl0"))
    {
      char *wl = websGetVar (wp, n, NULL);
      cprintf ("copy value %s which is [%s] to nvram\n", n, wl);
      if (wl)
	nvram_set ("wl_channel", wl);
    }
  copytonv (wp, n);

  sprintf (n, "%s_wchannel", prefix);
  if (!strcmp (prefix, "wl0"))
    {
      char *wl = websGetVar (wp, n, NULL);
      cprintf ("copy value %s which is [%s] to nvram\n", n, wl);
      if (wl)
	nvram_set ("wl_wchannel", wl);
    }

  copytonv (wp, n);

}

int
wireless_save (webs_t wp)
{
#ifdef HAVE_MSSID
  char *next;
  char var[80];
  char vif[16];
#ifndef HAVE_MADWIFI
  save_prefix (wp, "wl0");
  char *vifs = nvram_safe_get ("wl0_vifs");
#else
  int c = getdevicecount ();
  int i;
  for (i = 0; i < c; i++)
    {
      sprintf (vif, "ath%d_vifs", i);
      char buf[16];
      sprintf (buf, "ath%d", i);
      save_prefix (wp, buf);
      char *vifs = nvram_safe_get (vif);

#endif
      if (vifs == NULL)
	return 0;
      int count = 0;
      foreach (var, vifs, next)
      {
	save_prefix (wp, var);
      }
#ifdef HAVE_MADWIFI
    }
#endif
  //nvram_commit ();
#endif
  return 0;
}

void
ej_showad (int eid, webs_t wp, int argc, char_t ** argv)
{
#ifndef HAVE_FON
#ifndef CONFIG_BRANDING
#ifdef HAVE_CHILLI
//  if (nvram_invmatch ("fon_enable", "1"))
//    websWrite (wp,
//             "<a href=\"fon.cgi\"><img src=\"images/turn.gif\" border=0 /></a>");
#endif
#endif
#endif


#ifndef HAVE_NOAD
/*
if (nvram_match("wanup","1"))
{
websWrite(wp,"<script type=\"text/javascript\"><!--\n");
websWrite(wp,"google_ad_client = \"pub-8308593183433068\";\n");
websWrite(wp,"google_ad_width = 728;\n");
websWrite(wp,"google_ad_height = 90;\n");
websWrite(wp,"google_ad_format = \"728x90_as\";\n");
websWrite(wp,"google_ad_type = \"text_image\";\n");
websWrite(wp,"google_ad_channel =\"8866414571\";\n");
websWrite(wp,"google_color_border = \"333333\";\n");
websWrite(wp,"google_color_bg = \"000000\";\n");
websWrite(wp,"google_color_link = \"FFFFFF\";\n");
websWrite(wp,"google_color_url = \"999999\";\n");
websWrite(wp,"google_color_text = \"CCCCCC\";\n");
websWrite(wp,"//--></script>\n");
websWrite(wp,"<script type=\"text/javascript\"\n");
websWrite(wp,"  src=\"http://pagead2.googlesyndication.com/pagead/show_ads.js\">\n");
websWrite(wp,"</script>\n");
}*/
#endif
  return;
}


#ifdef HAVE_MSSID

void
ej_show_wireless_single (webs_t wp, char *prefix)
{
  char wl_mode[16];
  char wl_macaddr[16];
  char wl_ssid[16];
  sprintf (wl_mode, "%s_mode", prefix);
  sprintf (wl_macaddr, "%s_hwaddr", prefix);
  sprintf (wl_ssid, "%s_ssid", prefix);


//wireless mode
  websWrite (wp,
	     "<h2><script type=\"text/javascript\">Capture(wl_basic.h2_v24)</script></h2>\n");
  websWrite (wp, "<fieldset>\n");
  websWrite (wp,
	     "<legend>Interface %s - SSID [%s] HWAddr [%s]</legend>\n",
	     prefix, nvram_safe_get (wl_ssid), nvram_safe_get (wl_macaddr));
  websWrite (wp, "<div>\n");
  char power[16];
  char maxpower[16];
#ifdef HAVE_MADWIFI
  char wl_regdomain[16];
  sprintf (wl_regdomain, "%s_regdomain", prefix);

#ifdef REGDOMAIN_OVERRIDE
  websWrite (wp,
	     "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.regdom)</script></div><select name=\"%s\">\n",
	     wl_regdomain);
  int domcount = 0;
  while (regdomains[domcount].name != NULL)
    {
      char domcode[16];
      sprintf (domcode, "%d", regdomains[domcount].code);
      websWrite (wp, "<option value=\"%d\" %s>%s</option>\n",
		 regdomains[domcount].code, nvram_match (wl_regdomain,
							 domcode) ?
		 " selected=\"selected\"" : "", regdomains[domcount].name);
      domcount++;
    }
  websWrite (wp, "</select>\n");
  websWrite (wp, "</div>\n");
#endif
//power adjustment
  sprintf (power, "%s_txpwr", prefix);
  sprintf (maxpower, "%s_maxpower", prefix);
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.TXpower)</script></div><input class=\"num\" name=\"%s\" size=\"6\" maxlength=\"3\" value=\"%s\"/> mW (Max: %s)\n",
	     power, nvram_safe_get (power), nvram_safe_get (maxpower));
  websWrite (wp, "</div>\n");

#endif

#ifdef HAVE_MADWIFI
  if (!strcmp (prefix, "ath0"))	//show client only on first interface
#endif
    {
      websWrite (wp,
		 "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label)</script></div><select name=\"%s\" >\n",
		 wl_mode);
      websWrite (wp,
		 "<script type=\"text/javascript\">document.write(\"<option value=\\\"ap\\\" %s >\" + wl_basic.ap + \"</option>\");</script>\n",
		 nvram_match (wl_mode,
			      "ap") ? "selected=\\\"selected\\\"" : "");
      websWrite (wp,
		 "<script type=\"text/javascript\">document.write(\"<option value=\\\"sta\\\" %s >\" + wl_basic.client + \"</option>\");</script>\n",
		 nvram_match (wl_mode,
			      "sta") ? "selected=\\\"selected\\\"" : "");
      websWrite (wp,
		 "<script type=\"text/javascript\">document.write(\"<option value=\\\"infra\\\" %s >\" + wl_basic.adhoc + \"</option>\");</script>\n",
		 nvram_match (wl_mode,
			      "infra") ? "selected=\\\"selected\\\"" : "");
#ifndef HAVE_MADWIFI
      websWrite (wp,
		 "<script type=\"text/javascript\">document.write(\"<option value=\\\"wet\\\" %s >\" + wl_basic.clientBridge + \"</option>\");</script>\n",
		 nvram_match (wl_mode,
			      "wet") ? "selected=\\\"selected\\\"" : "");
      websWrite (wp,
		 "<script type=\"text/javascript\">document.write(\"<option value=\\\"apsta\\\" %s >\" + wl_basic.repeater + \"</option>\");</script>\n",
		 nvram_match (wl_mode,
			      "apsta") ? "selected=\\\"selected\\\"" : "");
#else
      websWrite (wp,
		 "<script type=\"text/javascript\">document.write(\"<option value=\\\"wdssta\\\" %s >\" + wl_basic.wdssta + \"</option>\");</script>\n",
		 nvram_match (wl_mode,
			      "wdssta") ? "selected=\\\"selected\\\"" : "");
      websWrite (wp,
		 "<script type=\"text/javascript\">document.write(\"<option value=\\\"wdsap\\\" %s >\" + wl_basic.wdsap + \"</option>\");</script>\n",
		 nvram_match (wl_mode,
			      "wdsap") ? "selected=\\\"selected\\\"" : "");
#endif
      websWrite (wp, "</select>\n");
      websWrite (wp, "</div>\n");
    }
#ifdef HAVE_MADWIFI
  else
    {
      websWrite (wp,
		 "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label)</script></div><select name=\"%s\">\n",
		 wl_mode);
      websWrite (wp,
		 "<script type=\"text/javascript\">document.write(\"<option value=\\\"ap\\\" %s >\" + wl_basic.ap + \"</option>\");</script>\n",
		 nvram_match (wl_mode,
			      "ap") ? "selected=\\\"selected\\\"" : "");
      websWrite (wp,
		 "<script type=\"text/javascript\">document.write(\"<option value=\\\"wdssta\\\" %s >\" + wl_basic.wdssta + \"</option>\");</script>\n",
		 nvram_match (wl_mode,
			      "wdssta") ? "selected=\\\"selected\\\"" : "");
      websWrite (wp,
		 "<script type=\"text/javascript\">document.write(\"<option value=\\\"wdsap\\\" %s >\" + wl_basic.wdsap + \"</option>\");</script>\n",
		 nvram_match (wl_mode,
			      "wdsap") ? "selected=\\\"selected\\\"" : "");
      websWrite (wp, "</select>\n");
      websWrite (wp, "</div>\n");
    }
#endif
//writeless net mode
  show_netmode (wp, prefix);
  //turbo options
#ifdef HAVE_MADWIFI
  char wl_turbo[16];
//  char wl_xchanmode[16];
//  char wl_outdoor[16];
  char wl_diversity[16];
  char wl_rxantenna[16];
  char wl_txantenna[16];
  char wl_width[16];
  char wl_xr[16];
  sprintf (wl_turbo, "%s_turbo", prefix);
//  sprintf (wl_xchanmode, "%s_xchanmode", prefix);
//  sprintf (wl_outdoor, "%s_outdoor", prefix);
  sprintf (wl_diversity, "%s_diversity", prefix);
  sprintf (wl_rxantenna, "%s_rxantenna", prefix);
  sprintf (wl_txantenna, "%s_txantenna", prefix);
  sprintf (wl_width, "%s_channelbw", prefix);
  sprintf (wl_xr, "%s_xr", prefix);

  showOption (wp, "wl_basic.turbo", wl_turbo);
  showOption (wp, "wl_basic.extrange", wl_xr);
//  showOption (wp, "wl_basic.extchannel", wl_xchanmode);
//  showOption (wp, "wl_basic.outband", wl_outdoor);
  showOption (wp, "wl_basic.diversity", wl_diversity);
  websWrite (wp,
	     "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_width)</script></div><select name=\"%s\" >\n",
	     wl_width);
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"20\\\" %s >\" + share.full + \"</option>\");</script>\n",
	     nvram_match (wl_width, "20") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"10\\\" %s >\" + share.half + \"</option>\");</script>\n",
	     nvram_match (wl_width, "10") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"5\\\" %s >\" + share.quarter + \"</option>\");</script>\n",
	     nvram_match (wl_width, "5") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp, "</select>\n");
  websWrite (wp, "</div>\n");

  websWrite (wp,
	     "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label12)</script></div><select name=\"%s\" >\n",
	     wl_txantenna);
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"0\\\" %s >\" + wl_basic.diversity + \"</option>\");</script>\n",
	     nvram_match (wl_txantenna,
			  "0") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.primary + \"</option>\");</script>\n",
	     nvram_match (wl_txantenna,
			  "1") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.secondary + \"</option>\");</script>\n",
	     nvram_match (wl_txantenna,
			  "2") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp, "</select>\n");
  websWrite (wp, "</div>\n");


  websWrite (wp,
	     "<div class=\"setting\"><div class=\"label\"><script type=\"text/javascript\">Capture(wl_adv.label13)</script></div><select name=\"%s\" >\n",
	     wl_rxantenna);
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"0\\\" %s >\" + wl_basic.diversity + \"</option>\");</script>\n",
	     nvram_match (wl_rxantenna,
			  "0") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"1\\\" %s >\" + wl_basic.primary + \"</option>\");</script>\n",
	     nvram_match (wl_rxantenna,
			  "1") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp,
	     "<script type=\"text/javascript\">document.write(\"<option value=\\\"2\\\" %s >\" + wl_basic.secondary + \"</option>\");</script>\n",
	     nvram_match (wl_rxantenna,
			  "2") ? "selected=\\\"selected\\\"" : "");
  websWrite (wp, "</select>\n");
  websWrite (wp, "</div>\n");

#endif

  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label3)</script></div><input name=\"%s\" size=\"20\" maxlength=\"32\" onblur=\"valid_name(this,wl_basic.label3)\" value=\"%s\" /></div>\n",
	     wl_ssid, nvram_safe_get (wl_ssid));

  if (nvram_match (wl_mode, "ap") || nvram_match (wl_mode, "apsta")
      || nvram_match (wl_mode, "wdsap") || nvram_match (wl_mode, "infra"))
    {

      if (has_mimo ()
	  && (nvram_match ("wl0_net_mode", "n-only")
	      || nvram_match ("wl0_net_mode", "mixed")))
	{

	  websWrite (wp, "<div class=\"setting\">\n");
	  websWrite (wp,
		     "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_width)</script></div>\n");
	  websWrite (wp, "<select name=\"wl0_nbw\">\n");
	  websWrite (wp,
		     "<script type=\"text/javascript\">document.write(\"<option value=\\\"0\\\" %s >\" + share.auto + \"</option>\");</script>\n",
		     nvram_match ("wl0_nbw",
				  "0") ? "selected=\\\"selected\\\"" : "");
	  websWrite (wp, "<option value=\"20\" %s>20 Mhz</option>",
		     nvram_match ("wl0_nbw",
				  "20") ? "selected=\\\"selected\\\"" : "");
	  websWrite (wp, "<option value=\"40\" %s>40 Mhz</option>",
		     nvram_match ("wl0_nbw",
				  "40") ? "selected=\\\"selected\\\"" : "");
	  websWrite (wp, "</select>\n");
	  websWrite (wp, "</div>\n");

	  websWrite (wp, "<div class=\"setting\">\n");
	  websWrite (wp,
		     "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.channel_wide)</script></div>\n");
	  websWrite (wp, "<select name=\"wl0_wchannel\" ></select>\n");
	  websWrite (wp, "</div>\n");

	  show_channel (wp, prefix, prefix, 1);
	}
      else

	show_channel (wp, prefix, prefix, 0);

      char wl_closed[16];
      sprintf (wl_closed, "%s_closed", prefix);
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label5)</script></div>\n");
      websWrite (wp,
		 "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.enable)</script></input>&nbsp;\n",
		 wl_closed, nvram_match (wl_closed,
					 "0") ? "checked=\"checked\"" : "");
      websWrite (wp,
		 "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s><script type=\"text/javascript\">Capture(share.disable)</script></input>\n",
		 wl_closed, nvram_match (wl_closed,
					 "1") ? "checked=\"checked\"" : "");
      websWrite (wp, "</div>\n");
    }


// ACK timing
  sprintf (power, "%s_distance", prefix);
  websWrite (wp, "<br />\n");
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\"><script type=\"text/javascript\">Capture(wl_basic.label6)</script></div>\n");
  websWrite (wp,
	     "<input class=\"num\" name=\"%s\" size=\"8\" maxlength=\"8\" onblur=\"valid_range(this,0,99999999,wl_basic.label6)\" value=\"%s\" />\n",
	     power, nvram_safe_get (power));
  websWrite (wp,
	     "<span class=\"default\"><script type=\"text/javascript\">document.write(\"(\" + share.deflt + \": 2000 \" + share.meters + \")\")</script></span>\n");
  websWrite (wp, "</div>\n");
//end ACK timing

  websWrite (wp, "</fieldset>\n");
  websWrite (wp, "<br />\n");
  show_virtualssid (wp, prefix);
}

void
ej_show_wireless (int eid, webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_MADWIFI
  int c = getdevicecount ();
  int i;
  for (i = 0; i < c; i++)
    {
      char buf[16];
      sprintf (buf, "ath%d", i);
      ej_show_wireless_single (wp, buf);
    }
#else
  ej_show_wireless_single (wp, "wl0");
#endif
  return;
}

void
changeKeySize (webs_t wp)
{
  security_save (wp);
}

void
show_preshared (webs_t wp, char *prefix)
{
  char var[80];
  cprintf ("show preshared");
  sprintf (var, "%s_crypto", prefix);
  websWrite (wp, "<div><div class=\"setting\">\n");
  websWrite (wp, "<div class=\"label\">WPA Algorithms</div>\n");
  websWrite (wp, "<select name=\"%s_crypto\">\n", prefix);
  websWrite (wp, "<option value=\"tkip\" %s>TKIP</option>\n",
	     selmatch (var, "tkip", "selected=\"selected\""));
  websWrite (wp, "<option value=\"aes\" %s>AES</option>\n",
	     selmatch (var, "aes", "selected=\"selected\""));
  websWrite (wp, "<option value=\"tkip+aes\" %s>TKIP+AES</option>\n",
	     selmatch (var, "tkip+aes", "selected=\"selected\""));
  websWrite (wp, "</select>\n");
  websWrite (wp, "</div>\n");
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.shared_key)</script></div>\n");
  sprintf (var, "%s_wpa_psk", prefix);
  websWrite (wp,
	     "<input name=\"%s_wpa_psk\" maxlength=\"64\" size=\"32\" value=\"%s\" />\n",
	     prefix, nvram_safe_get (var));
  websWrite (wp, "</div>\n");
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\"><script type=\"text/javascript\">Capture(wpa.rekey)</script></div>\n");
  sprintf (var, "%s_wpa_gtk_rekey", prefix);
  websWrite (wp,
	     "<input name=\"%s_wpa_gtk_rekey\" maxlength=\"5\" size=\"5\" onblur=\"valid_range(this,0,99999,wpa.rekey)\" value=\"%s\" />\n",
	     prefix, nvram_safe_get (var));
  websWrite (wp,
	     "<span class=\"default\"><script type=\"text/javascript\">document.write(\"(\" + share.deflt + \": 3600, \" + share.range + \": 1 - 99999)\")</script></span>\n",
	     prefix, nvram_safe_get (var));
  websWrite (wp, "</div>\n");
  websWrite (wp, "</div>\n");
}

void
show_radius (webs_t wp, char *prefix)
{
  char var[80];
  cprintf ("show radius\n");
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label3)</script></div>\n");
  websWrite (wp,
	     "<input type=\"hidden\" name=\"%s_radius_ipaddr\" value=\"4\" />\n",
	     prefix);
  sprintf (var, "%s_radius_ipaddr", prefix);
  char *rad = nvram_safe_get (var);
  websWrite (wp,
	     "<input size=\"3\" maxlength=\"3\" name=\"%s_radius_ipaddr_0\" onblur=\"valid_range(this,0,255,radius.label3)\" class=\"num\" value=\"%d\" />.",
	     prefix, get_single_ip (rad, 0));
  websWrite (wp,
	     "<input size=\"3\" maxlength=\"3\" name=\"%s_radius_ipaddr_1\" onblur=\"valid_range(this,0,255,radius.label3)\" class=\"num\" value=\"%d\" />.",
	     prefix, get_single_ip (rad, 1));
  websWrite (wp,
	     "<input size=\"3\" maxlength=\"3\" name=\"%s_radius_ipaddr_2\" onblur=\"valid_range(this,0,255,radius.label3)\" class=\"num\" value=\"%d\" />.",
	     prefix, get_single_ip (rad, 2));
  websWrite (wp,
	     "<input size=\"3\" maxlength=\"3\" name=\"%s_radius_ipaddr_3\" onblur=\"valid_range(this,1,254,radius.label3)\" class=\"num\" value=\"%d\" />\n",
	     prefix, get_single_ip (rad, 3));
  websWrite (wp, "</div>\n");
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label4)</script></div>\n");
  sprintf (var, "%s_radius_port", prefix);
  websWrite (wp,
	     "<input name=\"%s_radius_port\" size=\"3\" maxlength=\"5\" onblur=\"valid_range(this,1,65535,radius.label4)\" value=\"%s\" /></div>\n",
	     prefix, nvram_safe_get (var));
  websWrite (wp,
	     "<span class=\"default\"><script type=\"text/javascript\">document.write(\"(\" + share.deflt + \": 1812)\")</script></span>\n");
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\"><script type=\"text/javascript\">Capture(radius.label7)</script></div>\n");
  sprintf (var, "%s_radius_key", prefix);
  websWrite (wp,
	     "<input name=\"%s_radius_key\" size=\"20\" maxlength=\"79\" value=\"%s\" /></div>\n",
	     prefix, nvram_safe_get (var));
}

void
show_wparadius (webs_t wp, char *prefix)
{
  char var[80];
  websWrite (wp, "<div>\n");
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp, "<div class=\"label\">WPA Algorithms</div>\n");
  websWrite (wp, "<select name=\"%s_crypto\">\n", prefix);
  sprintf (var, "%s_crypto", prefix);
  websWrite (wp, "<option value=\"tkip\" %s>TKIP</option>\n",
	     selmatch (var, "tkip", "selected"));
  websWrite (wp, "<option value=\"aes\" %s>AES</option>\n",
	     selmatch (var, "aes", "selected"));
  websWrite (wp, "<option value=\"tkip+aes\" %s>TKIP+AES</option>\n",
	     selmatch (var, "tkip+aes", "selected"));
  websWrite (wp, "</select></div>\n");
  show_radius (wp, prefix);
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp, "<div class=\"label\">Key Renewal Timeout</div>\n");
  sprintf (var, "%s_wpa_gtk_rekey", prefix);
  websWrite (wp,
	     "<input name=\"%s_wpa_gtk_rekey\" maxlength=\"5\" size=\"10\" onblur=\"valid_range(this,0,99999,'rekey interval')\" value=\"%s\" />",
	     prefix, nvram_safe_get (var));
  websWrite (wp, "seconds</div>\n");
  websWrite (wp, "</div>\n");
}

void
show_wep (webs_t wp, char *prefix)
{
  char var[80];
  char *bit;
  cprintf ("show wep\n");
  websWrite (wp,
	     "<div><div class=\"setting\"><div class=\"label\">Default Transmit Key</div>");
  websWrite (wp, "<input type=\"hidden\" name=\"%s_WEP_key\" />", prefix);
  websWrite (wp,
	     "<input type=\"hidden\" name=\"%s_wep\" value=restricted\" />",
	     prefix);
  sprintf (var, "%s_key", prefix);
  websWrite (wp,
	     "<input type=\"radio\" value=\"1\" name=\"%s_key\" %s /> 1\n",
	     prefix, selmatch (var, "1", "checked"));
  websWrite (wp,
	     "<input type=\"radio\" value=\"2\" name=\"%s_key\" %s /> 2\n",
	     prefix, selmatch (var, "2", "checked"));
  websWrite (wp,
	     "<input type=\"radio\" value=\"3\" name=\"%s_key\" %s /> 3\n",
	     prefix, selmatch (var, "3", "checked"));
  websWrite (wp,
	     "<input type=\"radio\" value=\"4\" name=\"%s_key\" %s /> 4\n",
	     prefix, selmatch (var, "4", "checked"));
  websWrite (wp, "</div>");
  websWrite (wp,
	     "<div class=\"setting\"><div class=\"label\">WEP Encryption</div>");

  sprintf (var, "%s_wep_bit", prefix);
  bit = nvram_safe_get (var);

  cprintf ("bit %s\n", bit);

  websWrite (wp,
	     "<select name=\"%s_wep_bit\" size=\"1\" onchange=keyMode(this.form)>",
	     prefix);
  websWrite (wp, "<option value=\"64\" %s>64 bits 10 hex digits</option>",
	     selmatch (var, "64", "selected"));
  websWrite (wp, "<option value=\"128\" %s>128 bits 26 hex digits</option>",
	     selmatch (var, "128", "selected"));
  websWrite (wp,
	     "</select></div><div class=\"setting\"><div class=\"label\">Passphrase</div>\n");

  websWrite (wp,
	     "<input name=%s_passphrase maxLength=\"16\" size=\"20\" value=\"%s\" />",
	     prefix, get_wep_value ("passphrase", bit, prefix));

  websWrite (wp,
	     "<input type=\"hidden\" value=\"Null\" name=\"generateButton\" />\n");
  if (strcmp (bit, "64"))
    websWrite (wp,
	       "<input type=\"button\" value=\"Generate\" onclick=generateKey64(this.form,\"%s\") name=wepGenerate /></div>",
	       prefix);
  else
    websWrite (wp,
	       "<input type=\"button\" value=\"Generate\" onclick=generateKey128(this.form,\"%s\") name=wepGenerate /></div>",
	       prefix);

  websWrite (wp, "<div class=\"setting\"><div class=\"label\">Key 1</div>\n");
  websWrite (wp, "<input name=%s_key1 size=\"36\" value=\"%s\" /></div>\n",
	     prefix, get_wep_value ("key1", bit, prefix));
  websWrite (wp, "<div class=\"setting\"><div class=\"label\">Key 2</div>\n");
  websWrite (wp, "<input name=%s_key2 size=\"36\" value=\"%s\" /></div>\n",
	     prefix, get_wep_value ("key2", bit, prefix));
  websWrite (wp, "<div class=\"setting\"><div class=\"label\">Key 3</div>\n");
  websWrite (wp, "<input name=%s_key3 size=\"36\" value=\"%s\" /></div>\n",
	     prefix, get_wep_value ("key3", bit, prefix));
  websWrite (wp, "<div class=\"setting\"><div class=\"label\">Key 4</div>\n");
  websWrite (wp, "<input name=%s_key4 size=\"36\" value=\"%s\" /></div>\n",
	     prefix, get_wep_value ("key4", bit, prefix));
  websWrite (wp, "</div>\n");
}

#endif

void
validate_wds (webs_t wp, char *value, struct variable *v)
{
#ifdef HAVE_MSSID
  int h, i, devcount = 1;	//changed from 2 to 3
#else
  int h, i, devcount = 3;	//changed from 2 to 3
#endif
  struct variable wds_variables[] = {
  {longname: "WDS Mac", argv:NULL},
  {longname: "WDS IP Address", argv:NULL},
  {longname: "WDS Subnet Mask", argv:NULL},
  {longname: "WDS Gateway", argv:NULL},
  {longname: "WDS Description", argv:NULL},
  };

  char *val = NULL;
  char wds[32] = "";
  char wdsif_var[32] = "";
  char enabled_var[32];
  char hwaddr_var[32] = "";
  char ipaddr_var[32] = "";
  char netmask_var[32] = "";
  char desc_var[32] = "";
  char hwaddr[18] = "";
  char ipaddr[16] = "";
  char netmask[16] = "";
  char desc[48] = "";
  char wds_if[32] = { 0 };
  char wds_list[199] = "";


  nvram_set ("wl0_wds", "");
  snprintf (wds, 31, "wl_br1");
  snprintf (enabled_var, 31, "%s_enable", wds);
  cprintf ("wds_validate\n");
  /* validate separate br1 bridge params */
  if (nvram_match (enabled_var, "1"))
    {

      memset (ipaddr, 0, sizeof (ipaddr));
      memset (netmask, 0, sizeof (netmask));

      // disable until validated
      nvram_set (enabled_var, "0");

      // subnet params validation
      for (i = 0; i < 4; i++)
	{

	  snprintf (ipaddr_var, 31, "%s_%s%d", wds, "ipaddr", i);
	  val = websGetVar (wp, ipaddr_var, NULL);
	  if (val)
	    {
	      strcat (ipaddr, val);
	      if (i < 3)
		strcat (ipaddr, ".");
	    }
	  else
	    break;

	  snprintf (netmask_var, 31, "%s_%s%d", wds, "netmask", i);
	  val = websGetVar (wp, netmask_var, NULL);
	  if (val)
	    {
	      strcat (netmask, val);

	      if (i < 3)
		strcat (netmask, ".");
	    }
	  else
	    break;
	}

      if (!valid_ipaddr (wp, ipaddr, &wds_variables[1]) ||
	  !valid_netmask (wp, netmask, &wds_variables[2]))
	return;

      snprintf (ipaddr_var, 31, "%s_%s", wds, "ipaddr");
      snprintf (netmask_var, 31, "%s_%s", wds, "netmask");

      nvram_set (enabled_var, "1");
      nvram_set (ipaddr_var, ipaddr);
      nvram_set (netmask_var, netmask);
    }
  else
    nvram_set (enabled_var, "0");


  for (h = 1; h <= MAX_WDS_DEVS; h++)
    {
      memset (hwaddr, 0, sizeof (hwaddr));
      memset (desc, 0, sizeof (desc));
      snprintf (wds, 31, "wl_wds%d", h);
      snprintf (enabled_var, 31, "%s_enable", wds);

      for (i = 0; i < 6; i++)
	{

	  snprintf (hwaddr_var, 31, "%s_%s%d", wds, "hwaddr", i);
	  val = websGetVar (wp, hwaddr_var, NULL);

	  if (val)
	    {
	      strcat (hwaddr, val);
	      if (i < 5)
		strcat (hwaddr, ":");
	    }
	}

      if (!valid_hwaddr (wp, hwaddr, &wds_variables[0]))
	{
	  return;
	}

      snprintf (hwaddr_var, 31, "%s_%s", wds, "hwaddr");
      nvram_set (hwaddr_var, hwaddr);

      snprintf (desc_var, 31, "%s_%s", wds, "desc");
      val = websGetVar (wp, desc_var, NULL);
      if (val)
	{
	  strcat (desc, val);
	  nvram_set (desc_var, desc);
	}

      /* <lonewolf> */
      snprintf (desc_var, 31, "%s_%s", wds, "ospf");
      val = websGetVar (wp, desc_var, "");
      if (val)
	nvram_set (desc_var, val);
      /* </lonewolf> */

      if (strcmp (hwaddr, "00:00:00:00:00:00")
	  && nvram_invmatch (enabled_var, "0"))
	{
	  snprintf (wds_list, 199, "%s %s", wds_list, hwaddr);
	}

      if (nvram_match (enabled_var, "1"))
	{

	  memset (ipaddr, 0, sizeof (ipaddr));
	  memset (netmask, 0, sizeof (netmask));

	  // disable until validated
	  nvram_set (enabled_var, "0");

	  // subnet params validation
	  for (i = 0; i < 4; i++)
	    {

	      snprintf (ipaddr_var, 31, "%s_%s%d", wds, "ipaddr", i);
	      val = websGetVar (wp, ipaddr_var, NULL);
	      if (val)
		{
		  strcat (ipaddr, val);
		  if (i < 3)
		    strcat (ipaddr, ".");
		}
	      else
		break;

	      snprintf (netmask_var, 31, "%s_%s%d", wds, "netmask", i);
	      val = websGetVar (wp, netmask_var, NULL);
	      if (val)
		{
		  strcat (netmask, val);

		  if (i < 3)
		    strcat (netmask, ".");
		}
	      else
		break;
	    }

	  if (!valid_ipaddr (wp, ipaddr, &wds_variables[1]) ||
	      !valid_netmask (wp, netmask, &wds_variables[2]))
	    {
	      continue;
	    }

	  snprintf (ipaddr_var, 31, "%s_%s", wds, "ipaddr");
	  snprintf (netmask_var, 31, "%s_%s", wds, "netmask");

	  nvram_set (enabled_var, "1");
	  nvram_set (ipaddr_var, ipaddr);
	  nvram_set (netmask_var, netmask);
	}

      /* keep the wds devices in sync w enabled entries */
      snprintf (wdsif_var, 31, "%s_if", wds);
      if (!nvram_match (enabled_var, "0"))
	{
#ifdef HAVE_MSSID
	  snprintf (wds_if, 31, "wds0.%d", (devcount++));
#else
	  snprintf (wds_if, 31, "wds0.491%d", 50 + (devcount++));
#endif
	  nvram_set (wdsif_var, wds_if);
	}
      else
	nvram_unset (wdsif_var);

    }

  nvram_set ("wl0_wds", wds_list);
}

void
ej_get_wds_mac (int eid, webs_t wp, int argc, char_t ** argv)
{
  int mac = -1, wds_idx = -1, mac_idx = -1;
  char *c, wds_var[32] = "";


  if (ejArgs (argc, argv, "%d %d", &wds_idx, &mac_idx) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  else if (wds_idx < 1 || wds_idx > MAX_WDS_DEVS)
    return;
  else if (mac_idx < 0 || mac_idx > 5)
    return;

  snprintf (wds_var, 31, "wl_wds%d_hwaddr", wds_idx);

  c = nvram_safe_get (wds_var);

  if (c)
    {
      mac = get_single_mac (c, mac_idx);
      websWrite (wp, "%02X", mac);
    }
  else
    websWrite (wp, "00");

  return;

}

void
ej_get_wds_ip (int eid, webs_t wp, int argc, char_t ** argv)
{
  int ip = -1, wds_idx = -1, ip_idx = -1;
  char *c, wds_var[32] = "";


  if (ejArgs (argc, argv, "%d %d", &wds_idx, &ip_idx) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  else if (wds_idx < 1 || wds_idx > MAX_WDS_DEVS)
    return;
  else if (ip_idx < 0 || ip_idx > 3)
    return;

  snprintf (wds_var, 31, "wl_wds%d_ipaddr", wds_idx);

  c = nvram_safe_get (wds_var);

  if (c)
    {
      ip = get_single_ip (c, ip_idx);
      websWrite (wp, "%d", ip);
    }
  else
    websWrite (wp, "0");

  return;

}

void
ej_get_wds_netmask (int eid, webs_t wp, int argc, char_t ** argv)
{
  int nm = -1, wds_idx = -1, nm_idx = -1;
  char *c, wds_var[32] = "";


  if (ejArgs (argc, argv, "%d %d", &wds_idx, &nm_idx) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  else if (wds_idx < 1 || wds_idx > 6)
    return;
  else if (nm_idx < 0 || nm_idx > 3)
    return;

  snprintf (wds_var, 31, "wl_wds%d_netmask", wds_idx);

  c = nvram_safe_get (wds_var);

  if (c)
    {
      nm = get_single_ip (c, nm_idx);
      websWrite (wp, "%d", nm);
    }
  else
    websWrite (wp, "255");

  return;

}


void
ej_get_wds_gw (int eid, webs_t wp, int argc, char_t ** argv)
{
  int gw = -1, wds_idx = -1, gw_idx = -1;
  char *c, wds_var[32] = "";


  if (ejArgs (argc, argv, "%d %d", &wds_idx, &gw_idx) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  else if (wds_idx < 1 || wds_idx > MAX_WDS_DEVS)
    return;
  else if (gw_idx < 0 || gw_idx > 3)
    return;

  snprintf (wds_var, 31, "wl_wds%d_gw", wds_idx);

  c = nvram_safe_get (wds_var);

  if (c)
    {
      gw = get_single_ip (c, gw_idx);
      websWrite (wp, "%d", gw);
    }
  else
    websWrite (wp, "0");

  return;

}

void
ej_get_br1_ip (int eid, webs_t wp, int argc, char_t ** argv)
{
  int ip = -1, ip_idx = -1;
  char *c;


  if (ejArgs (argc, argv, "%d", &ip_idx) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
  else if (ip_idx < 0 || ip_idx > 3)
    return;

  c = nvram_safe_get ("wl_br1_ipaddr");

  if (c)
    {
      ip = get_single_ip (c, ip_idx);
      websWrite (wp, "%d", ip);
    }
  else
    websWrite (wp, "0");

  return;

}

void
ej_get_br1_netmask (int eid, webs_t wp, int argc, char_t ** argv)
{
  int nm = -1, nm_idx = -1;
  char *c;


  if (ejArgs (argc, argv, "%d", &nm_idx) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
  else if (nm_idx < 0 || nm_idx > 3)
    return;

  c = nvram_safe_get ("wl_br1_netmask");

  if (c)
    {
      nm = get_single_ip (c, nm_idx);
      websWrite (wp, "%d", nm);
    }
  else
    websWrite (wp, "255");

  return;

}

void
ej_get_currate (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *dev = NULL;
  int rate = 0;

  if (wl_probe ("eth2"))
    dev = "eth1";
  else
    dev = "eth2";

  wl_ioctl (dev, WLC_GET_RATE, &rate, sizeof (rate));

  if (rate > 0)
    {
      if (rate == 11)
	websWrite (wp, "5.5");
      else
	websWrite (wp, "%d", rate / 2);
    }
  else
    websWrite (wp, "unknown");

  return;

}

#define UPTIME_TMP	"/tmp/.uptime"
void
ej_get_uptime (int eid, webs_t wp, int argc, char_t ** argv)
{
  char uptime[200] = { 0 }, cmd[200] =
  {
  0};
  FILE *fp;
  unlink (UPTIME_TMP);

  snprintf (cmd, 254, "uptime 2>&1 > %s", UPTIME_TMP);
  system (cmd);

  if ((fp = fopen (UPTIME_TMP, "r")) != NULL)
    fgets (uptime, sizeof (uptime), fp);
  else
    return;
  int i = 0;
  while (uptime[i++] != 0 && i < 199)
    {
      if (uptime[i] == 0xa || uptime[i] == 0xd)
	uptime[i] = 0;
    }
  websWrite (wp, "%s", uptime);

  fclose (fp);

  unlink (UPTIME_TMP);

  return;

}

void
ej_get_curchannel (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *dev = NULL;
  channel_info_t ci;

  if (wl_probe ("eth2"))
    dev = "eth1";
  else
    dev = "eth2";

  ci.target_channel = 0;
  wl_ioctl (dev, WLC_GET_CHANNEL, &ci, sizeof (ci));
  if (ci.target_channel > 0)
    {
      websWrite (wp, "%d", ci.target_channel);
    }
  else
    websWrite (wp, "0");

  return;

}

#ifdef HAVE_MADWIFI
#include <sys/types.h>
#include <sys/file.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <ctype.h>
#include <getopt.h>
#include <err.h>


#include <linux/wireless.h>
#include "net80211/ieee80211.h"
#include "net80211/ieee80211_crypto.h"
#include "net80211/ieee80211_ioctl.h"
static const char *
ieee80211_ntoa (const uint8_t mac[IEEE80211_ADDR_LEN])
{
  static char a[18];
  int i;

  i = snprintf (a, sizeof (a), "%02x:%02x:%02x:%02x:%02x:%02x",
		mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return (i < 17 ? NULL : a);
}

static u_int
rssi2dbm (u_int rssi)
{
  return rssi - 95;
}

void
ej_active_wireless_if (int eid, webs_t wp, int argc, char_t ** argv,
		       char *ifname)
{
  unsigned char buf[24 * 1024];
  unsigned char *cp;
  int s, len;
  struct iwreq iwr;
  s = socket (AF_INET, SOCK_DGRAM, 0);
  if (s < 0)
    {
      fprintf (stderr, "socket(SOCK_DRAGM)\n");
      return;
    }
  (void) memset (&iwr, 0, sizeof (iwr));
  (void) strncpy (iwr.ifr_name, ifname, sizeof (iwr.ifr_name));
  iwr.u.data.pointer = (void *) buf;
  iwr.u.data.length = sizeof (buf);
  if (ioctl (s, IEEE80211_IOCTL_STA_INFO, &iwr) < 0)
    {
      close (s);
      return;
    }
  len = iwr.u.data.length;
  if (len < sizeof (struct ieee80211req_sta_info))
    {
      close (s);
      return;
    }
  int cnt = 0;
  cp = buf;
  do
    {
      struct ieee80211req_sta_info *si;
      uint8_t *vp;

      si = (struct ieee80211req_sta_info *) cp;
      vp = (u_int8_t *) (si + 1);

      if (cnt)
	websWrite (wp, ",");
      cnt++;
      websWrite (wp, "'%s','%d','%d','%d'", ieee80211_ntoa (si->isi_macaddr),
		 rssi2dbm (si->isi_rssi), si->isi_noise,
		 rssi2dbm (si->isi_rssi) - (si->isi_noise));

      cp += si->isi_len, len -= si->isi_len;
    }
  while (len >= sizeof (struct ieee80211req_sta_info));
  close (s);


}
extern char *getiflist (void);


void
ej_active_wireless (int eid, webs_t wp, int argc, char_t ** argv)
{
  int c = getdevicecount ();
  char devs[32];
  int i;
  for (i = 0; i < c; i++)
    {
      sprintf (devs, "ath%d", i);
//fprintf(stderr,"show ifname %s\n",devs);
      ej_active_wireless_if (eid, wp, argc, argv, devs);
      char vif[32];
      sprintf (vif, "%s_vifs", devs);
      char var[80], *next;
      char *vifs = nvram_safe_get (vif);
      if (vifs != NULL)
	foreach (var, vifs, next)
	{
//    fprintf(stderr,"show ifname %s\n",var);
	  ej_active_wireless_if (eid, wp, argc, argv, var);
	}
    }
}

#else
#define RSSI_TMP	"/tmp/.rssi"
#define ASSOCLIST_CMD	"wl assoclist"
#define RSSI_CMD	"wl rssi"
#define NOISE_CMD	"wl noise"

void
ej_active_wireless (int eid, webs_t wp, int argc, char_t ** argv)
{
  int rssi = 0, noise = 0;
  FILE *fp2;
  char *mode;
  char mac[30];
  char list[2][30];
  char line[80];
  char cmd[80];
  int macmask;
  if (ejArgs (argc, argv, "%d", &macmask) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  unlink (RSSI_TMP);
  int cnt = 0;
  mode = nvram_safe_get ("wl_mode");
  unsigned char buf[WLC_IOCTL_MAXLEN];
  memset (buf, 0, WLC_IOCTL_MAXLEN);
  char *iface = get_wdev ();
  int r = getassoclist (iface, buf);
  if (r < 0)
    return;
  struct maclist *maclist = (struct maclist *) buf;
  int i;
  for (i = 0; i < maclist->count; i++)
    {
      ether_etoa ((uint8 *) & maclist->ea[i], mac);

      rssi = 0;
      noise = 0;
      // get rssi value
      if (strcmp (mode, "ap") && strcmp (mode, "apsta"))
	snprintf (cmd, sizeof (cmd), "%s > %s", RSSI_CMD, RSSI_TMP);
      else
	snprintf (cmd, sizeof (cmd), "%s \"%s\" > %s", RSSI_CMD, mac,
		  RSSI_TMP);
      system (cmd);

      // get noise value if not ap mode
      if (strcmp (mode, "ap"))
	snprintf (cmd, sizeof (cmd), "%s >> %s", NOISE_CMD, RSSI_TMP);
      system (cmd);		// get RSSI value for mac

      fp2 = fopen (RSSI_TMP, "r");
      if (fgets (line, sizeof (line), fp2) != NULL)
	{

	  // get rssi
#ifdef HAVE_MSSID
	  if (sscanf (line, "%d", &rssi) != 1)
	    continue;

//           noise=getNoise(iface);

	  if (strcmp (mode, "ap") &&
	      fgets (line, sizeof (line), fp2) != NULL &&
	      sscanf (line, "%d", &noise) != 1)
	    continue;
#else
	  if (sscanf (line, "%s %s %d", list[0], list[1], &rssi) != 3)
	    continue;
//          noise=getNoise(iface);
	  if (strcmp (mode, "ap") &&
	      fgets (line, sizeof (line), fp2) != NULL &&
	      sscanf (line, "%s %s %d", list[0], list[1], &noise) != 3)
	    continue;
#endif
	  // get noise for client/wet mode

	  fclose (fp2);
	}
      if (nvram_match ("maskmac", "1") && macmask)
	{
	  mac[0] = 'x';
	  mac[1] = 'x';
	  mac[3] = 'x';
	  mac[4] = 'x';
	  mac[6] = 'x';
	  mac[7] = 'x';
	  mac[9] = 'x';
	  mac[10] = 'x';
	}
      if (cnt)
	websWrite (wp, ",");
      cnt++;
      if (!strcmp (mode, "ap"))
	{
	  char *ref = nvram_get ("noise_reference");
	  noise = -98;
	  if (ref)
	    noise = atoi (ref);
	}
      websWrite (wp, "'%s','%d','%d','%d'", mac, rssi, noise, rssi - noise);
    }
  unlink (RSSI_TMP);

  return;
}
#endif

#define WDS_RSSI_TMP	"/tmp/.rssi"
#define WDS_CMD			"wl wds"

void
ej_active_wds (int eid, webs_t wp, int argc, char_t ** argv)
{
#ifndef HAVE_MADWIFI
  int rssi = 0, i;
  FILE *fp2;
  char *mode;
  char mac[30];
  char list[2][30];
  char line[80];
  char cmd[80];
//  char title[30];
  char wdsvar[30];
  char desc[30];
  int cnt = 0;
  int macmask;
  if (ejArgs (argc, argv, "%d", &macmask) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  unlink (WDS_RSSI_TMP);

  mode = nvram_safe_get ("wl_mode");

  if (strcmp (mode, "ap") && strcmp (mode, "apsta"))
    return;
  unsigned char buf[WLC_IOCTL_MAXLEN];
  char *iface = get_wdev ();
  int r = getwdslist (iface, buf);
  if (r < 0)
    return;
  struct maclist *maclist = (struct maclist *) buf;
  int e;
  for (e = 0; e < maclist->count; e++)
    {

      ether_etoa ((uint8 *) & maclist->ea[e], mac);


      rssi = 0;

      for (i = 1; i <= 10; i++)
	{
	  snprintf (wdsvar, 30, "wl_wds%d_hwaddr", i);
	  if (nvram_match (wdsvar, mac))
	    {
	      snprintf (wdsvar, 30, "wl_wds%d_desc", i);
	      snprintf (desc, sizeof (desc), "%s", nvram_get (wdsvar));
	      if (!strcmp (nvram_get (wdsvar), ""))
		strcpy (desc, "&nbsp;");
	    }
	}

      snprintf (cmd, sizeof (cmd), "%s \"%s\" > %s", RSSI_CMD, mac, RSSI_TMP);
      system (cmd);

      fp2 = fopen (RSSI_TMP, "r");
      if (fgets (line, sizeof (line), fp2) != NULL)
	{

	  // get rssi
#ifdef HAVE_MSSID
	  if (sscanf (line, "%d", &rssi) != 1)
	    continue;
#else
	  if (sscanf (line, "%s %s %d", list[0], list[1], &rssi) != 3)
	    continue;
#endif
	  fclose (fp2);
	}
      if (nvram_match ("maskmac", "1") && macmask)
	{
	  mac[0] = 'x';
	  mac[1] = 'x';
	  mac[3] = 'x';
	  mac[4] = 'x';
	  mac[6] = 'x';
	  mac[7] = 'x';
	  mac[9] = 'x';
	  mac[10] = 'x';
	}
      if (cnt)
	websWrite (wp, ",");
      cnt++;
      char *ref = nvram_get ("noise_reference");
      int noise = -98;
      if (ref)
	noise = atoi (ref);
      websWrite (wp,
		 "\"%s\",\"%s\",\"%d\",\"%d\",\"%d\"",
		 mac, desc, rssi, noise, rssi - noise);
    }

  unlink (WDS_RSSI_TMP);
#endif
  return;
}


void
ej_get_wdsp2p (int eid, webs_t wp, int argc, char_t ** argv)
{
  int index = -1, ip[4] = { 0, 0, 0, 0 }, netmask[4] =
  {
  0, 0, 0, 0};
  char nvramvar[32] = { 0 };

  if (ejArgs (argc, argv, "%d", &index) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
  if (nvram_selmatch (wp, "wk_mode", "ospf") &&
      nvram_selmatch (wp, "expert_mode", "1") &&
      nvram_selmatch (wp, "wl_wds1_enable", "1"))
    {
      char buf[16];
      sprintf (buf, "wl_wds%d_ospf", index);
      websWrite (wp,
		 "<input name=\"%s\" size=\"2\" maxlength=\"5\" value=\"%s\" />\n",
		 buf, nvram_safe_get (buf));
    }

  snprintf (nvramvar, 31, "wl_wds%d_ipaddr", index);
  sscanf (nvram_safe_get (nvramvar), "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2],
	  &ip[3]);
  snprintf (nvramvar, 31, "wl_wds%d_netmask", index);
  sscanf (nvram_safe_get (nvramvar), "%d.%d.%d.%d", &netmask[0], &netmask[1],
	  &netmask[2], &netmask[3]);
  snprintf (nvramvar, 31, "wl_wds%d_enable", index);

  // set netmask to a suggested default if blank
  if (netmask[0] == 0 &&
      netmask[1] == 0 && netmask[2] == 0 && netmask[3] == 0)
    {
      netmask[0] = 255;
      netmask[1] = 255;
      netmask[2] = 255;
      netmask[3] = 252;
    }

  if (nvram_match (nvramvar, "1"))
    {
      websWrite (wp, "\
	<div class=\"setting\">\n\
	          <input type=\"hidden\" name=\"wl_wds%d_ipaddr\" value=\"4\">\n\
	          <div class=\"label\"><script type=\"text/javascript\">Capture(share.ip)</script></div>\n\
	          <input size=\"3\" maxlength=\"3\" name=\"wl_wds%d_ipaddr0\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">\.<input size=\"3\" maxlength=\"3\" name=\"wl_wds%d_ipaddr1\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">\.<input size=\"3\" maxlength=\"3\" name=\"wl_wds%d_ipaddr2\" value=\"%d\" onblur=\"valid_range(this,0,255,'IP')\" class=\"num\">\.<input size=\"3\" maxlength=\"3\" name=\"wl_wds%d_ipaddr3\" value=\"%d\" onblur=\"valid_range(this,1,254,'IP')\" class=\"num\">\n\
       </div>\n", index, index, ip[0], index, ip[1], index, ip[2], index, ip[3], index);

      websWrite (wp, "\
       	  <div class=\"setting\">\n\
       	  <div class=\"label\"><script type=\"text/javascript\">Capture(share.subnet)</script></div>\n\
	  <input type=\"hidden\" name=\"wl_wds%d_netmask\" value=\"4\">\n\
	  <input name=\"wl_wds%d_netmask0\" value=\"%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,255,'IP')\" class=num>\.<input name=\"wl_wds%d_netmask1\" value=\"%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,255,'IP')\" class=num>\.<input name=\"wl_wds%d_netmask2\" value=\"%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,255,'IP')\" class=num>\.<input name=\"wl_wds%d_netmask3\" value=\"%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,255,'IP')\" class=num>\n\
          </div>\n", index, index, netmask[0], index, netmask[1], index, netmask[2], index, netmask[3]);

    }

  return;

}

void
save_wds (webs_t wp)
{
  char *wds_enable_val, wds_enable_var[32] = { 0 };
  int h = 0;

  for (h = 1; h <= MAX_WDS_DEVS; h++)
    {
      sprintf (wds_enable_var, "wl_wds%d_enable", h);
      wds_enable_val = websGetVar (wp, wds_enable_var, NULL);
      nvram_set (wds_enable_var, wds_enable_val);
    }

  wds_enable_val = websGetVar (wp, "wl_br1_enable", NULL);
  nvram_set ("wl_br1_enable", wds_enable_val);

  return;

}


char *
get_filter_services (void)
{
  static char services[8192] = "", svcs_var[32] = "filter_services0";
  int index = 1;
char temop[2048]="start ";
  while (strlen (nvram_safe_get (svcs_var)) > 0 && index < 8)
    {
      strcat (services, nvram_safe_get (svcs_var));
temop = strcpy (temop, nvram_safe_get (svcs_var));
syslog (LOG_DEBUG, "serv=%s\n", temop);
      snprintf (svcs_var, 31, "filter_services%d", index);
      index++;


    }

  return services;
}

void
ej_get_services_options (int eid, webs_t wp, int argc, char_t ** argv)
{
  char word[1024], *next, *services;
  char delim[] = "<&nbsp;>";

//services = nvram_safe_get("filter_services");
  services = get_filter_services ();

  split (word, services, next, delim)
  {
    int len = 0;
    char *name, *prot, *port;
    char protocol[100], ports[100];
    int from = 0, to = 0;
    //int proto;

    if ((name = strstr (word, "$NAME:")) == NULL ||
	(prot = strstr (word, "$PROT:")) == NULL ||
	(port = strstr (word, "$PORT:")) == NULL)
      continue;

    /* $NAME */
    if (sscanf (name, "$NAME:%3d:", &len) != 1)
      continue;

    strncpy (name, name + sizeof ("$NAME:nnn:") - 1, len);
    name[len] = '\0';

    /* $PROT */
    if (sscanf (prot, "$PROT:%3d:", &len) != 1)
      continue;

    strncpy (protocol, prot + sizeof ("$PROT:nnn:") - 1, len);
    protocol[len] = '\0';

    /* $PORT */
    if (sscanf (port, "$PORT:%3d:", &len) != 1)
      continue;

    strncpy (ports, port + sizeof ("$PORT:nnn:") - 1, len);
    ports[len] = '\0';

    if (sscanf (ports, "%d:%d", &from, &to) != 2)
      continue;

    //cprintf("match:: name=%s, protocol=%s, ports=%s\n",
    //      word, protocol, ports);

    websWrite (wp, "<option value=\"%s\">%s</option>", name, name);

  }

  return;
}

int
get_svc (char *svc, char *protocol, char *ports)
{
  char word[1024], *next, *services;
  char delim[] = "<&nbsp;>";

//      services = nvram_safe_get("filter_services");
  services = get_filter_services ();
syslog (LOG_DEBUG, "services=%s\n", services);
  split (word, services, next, delim)
  {
    int len = 0;
    char *name, *prot, *port;
    int from = 0, to = 0;

    if ((name = strstr (word, "$NAME:")) == NULL ||
	(prot = strstr (word, "$PROT:")) == NULL ||
	(port = strstr (word, "$PORT:")) == NULL)
      continue;

    /* $NAME */
    if (sscanf (name, "$NAME:%3d:", &len) != 1)
      return -1;

    strncpy (name, name + sizeof ("$NAME:nnn:") - 1, len);
    name[len] = '\0';
syslog (LOG_DEBUG, "name=%s\n", name);
    if (strcasecmp (svc, name))
      continue;

    /* $PROT */
    if (sscanf (prot, "$PROT:%3d:", &len) != 1)
      return -1;

    strncpy (protocol, prot + sizeof ("$PROT:nnn:") - 1, len);
    protocol[len] = '\0';
syslog (LOG_DEBUG, "protocol=%s\n", protocol);
    /* $PORT */
    if (sscanf (port, "$PORT:%3d:", &len) != 1)
      return -1;

    strncpy (ports, port + sizeof ("$PORT:nnn:") - 1, len);
    ports[len] = '\0';
syslog (LOG_DEBUG, "ports=%s\n", ports);
    if (sscanf (ports, "%d:%d", &from, &to) != 2)
      return -1;


    if (strcasecmp (svc, name) == 0)
      return 0;
  }

  return -1;
}

int
qos_add_svc (webs_t wp)
{
  char protocol[100] = { 0 }, ports[100] =
  {
  0};
  char *add_svc = websGetVar (wp, "add_svc", NULL);
  char *svqos_svcs = nvram_safe_get ("svqos_svcs");
  char new_svcs[4096] = { 0 };
  int i = 0;
syslog (LOG_DEBUG, "add_svc1=%s\n", add_svc);
  memset (new_svcs, 0, sizeof (new_svcs));

  if (get_svc (add_svc, protocol, ports))
    return -1;
syslog (LOG_DEBUG, "passed check 1\n");
  if (strcmp (protocol, "l7") == 0)
    {
      for (i = 0; i < strlen (add_svc); i++)
	add_svc[i] = tolower (add_svc[i]);
    }
syslog (LOG_DEBUG, "add_svc2=%s\n", add_svc);
  /* if this service exists, return an error */
  if (strstr (svqos_svcs, add_svc))
    return -1;
syslog (LOG_DEBUG, "passed check 2\n");
  if (strlen (svqos_svcs) > 0)
    snprintf (new_svcs, 4095, "%s %s %s %s 30 |", svqos_svcs, add_svc,
	      protocol, ports);
  else
    snprintf (new_svcs, 4095, "%s %s %s 30 |", add_svc, protocol, ports);

  if (strlen (new_svcs) >= sizeof (new_svcs))
    return -1;
syslog (LOG_DEBUG, "passed check 3\n");
  nvram_set ("svqos_svcs", new_svcs);
  nvram_commit ();

  return 0;
}


int
qos_add_ip (webs_t wp)
{
  char *add_ip0 = websGetVar (wp, "svqos_ipaddr0", NULL);
  char *add_ip1 = websGetVar (wp, "svqos_ipaddr1", NULL);
  char *add_ip2 = websGetVar (wp, "svqos_ipaddr2", NULL);
  char *add_ip3 = websGetVar (wp, "svqos_ipaddr3", NULL);
  char *add_nm = websGetVar (wp, "svqos_netmask", NULL);
  char add_ip[19] = { 0 };
  char *svqos_ips = nvram_safe_get ("svqos_ips");
  char new_ip[4096] = { 0 };

  memset (new_ip, 0, sizeof (new_ip));

  snprintf (add_ip, 19, "%s.%s.%s.%s/%s", add_ip0, add_ip1, add_ip2, add_ip3,
	    add_nm);

  /* if this ip exists, return an error */
  if (strstr (svqos_ips, add_ip))
    return -1;

  snprintf (new_ip, 4095, "%s %s 30 |", svqos_ips, add_ip);

  if (strlen (new_ip) >= sizeof (new_ip))
    return -1;

  nvram_set ("svqos_ips", new_ip);
  nvram_commit ();

  return 0;
}

int
qos_add_mac (webs_t wp)
{
  char *add_mac0 = websGetVar (wp, "svqos_hwaddr0", NULL);
  char *add_mac1 = websGetVar (wp, "svqos_hwaddr1", NULL);
  char *add_mac2 = websGetVar (wp, "svqos_hwaddr2", NULL);
  char *add_mac3 = websGetVar (wp, "svqos_hwaddr3", NULL);
  char *add_mac4 = websGetVar (wp, "svqos_hwaddr4", NULL);
  char *add_mac5 = websGetVar (wp, "svqos_hwaddr5", NULL);
  char add_mac[19] = { 0 };
  char *svqos_macs = nvram_safe_get ("svqos_macs");
  char new_mac[4096] = { 0 };

  memset (new_mac, 0, sizeof (new_mac));

  snprintf (add_mac, 18, "%s:%s:%s:%s:%s:%s", add_mac0, add_mac1, add_mac2,
	    add_mac3, add_mac4, add_mac5);

  /* if this mac exists, return an error */
  if (strstr (svqos_macs, add_mac))
    return -1;

  snprintf (new_mac, 4095, "%s %s 30 |", svqos_macs, add_mac);

  if (strlen (new_mac) >= sizeof (new_mac))
    return -1;

  nvram_set ("svqos_macs", new_mac);
  nvram_commit ();

  return 0;
}

int
qos_save (webs_t wp)
{
  char svqos_var[4096] = { 0 };
  char field[32] = { 0 };
  char *name, *data, *level, *delete;
  int no_svcs = atoi (websGetVar (wp, "svqos_nosvcs", NULL));
  int no_ips = atoi (websGetVar (wp, "svqos_noips", NULL));
  int no_macs = atoi (websGetVar (wp, "svqos_nomacs", NULL));
  int i = 0, j = 0;

  /* reused wshaper fields - see src/router/rc/wshaper.c */
  snprintf (field, 31, "wshaper_enable");
  data = websGetVar (wp, field, NULL);
  nvram_set ("wshaper_enable", data);


  if (strcmp (data, "0") == 0)
    return -1;

  snprintf (field, 31, "enable_game");
  data = websGetVar (wp, field, NULL);
  nvram_set ("enable_game", data);

  snprintf (field, 31, "default_level");
  data = websGetVar (wp, field, NULL);
  nvram_set ("default_level", data);

  snprintf (field, 31, "wshaper_downlink");
  data = websGetVar (wp, field, NULL);
  nvram_set ("wshaper_downlink", data);

  snprintf (field, 31, "wshaper_uplink");
  data = websGetVar (wp, field, NULL);
  nvram_set ("wshaper_uplink", data);

  snprintf (field, 31, "wshaper_dev");
  data = websGetVar (wp, field, NULL);
  nvram_set ("wshaper_dev", data);

  snprintf (field, 31, "qos_type");
  data = websGetVar (wp, field, NULL);
  nvram_set ("qos_type", data);

  nvram_commit ();

  memset (svqos_var, 0, sizeof (svqos_var));

  /* services priorities */
  for (i = 0; i < no_svcs; i++)
    {
      char protocol[100], ports[100];

      memset (protocol, 0, 100);
      memset (ports, 0, 10);

      snprintf (field, 31, "svqos_svcdel%d", i);
      delete = websGetVar (wp, field, NULL);

      if (delete && strlen (delete) > 0)
	continue;

      snprintf (field, 31, "svqos_svcname%d", i);
      name = websGetVar (wp, field, NULL);

      snprintf (field, 31, "svqos_svcprio%d", i);
      level = websGetVar (wp, field, NULL);

      if (get_svc (name, protocol, ports))
	continue;

      if (strcmp (protocol, "l7") == 0)
	{
	  for (j = 0; j < strlen (name); j++)
	    name[j] = tolower (name[j]);
	}

      if (strlen (svqos_var) > 0)
	sprintf (svqos_var, "%s %s %s %s %s |", svqos_var, name, protocol,
		 ports, level);
      else
	sprintf (svqos_var, "%s %s %s %s |", name, protocol, ports, level);

    }

  if (strlen (svqos_var) <= sizeof (svqos_var))
    nvram_set ("svqos_svcs", svqos_var);
  nvram_commit ();
  memset (svqos_var, 0, sizeof (svqos_var));

  /* IP priorities */
  for (i = 0; i < no_ips; i++)
    {

      snprintf (field, 31, "svqos_ipdel%d", i);
      delete = websGetVar (wp, field, NULL);

      if (delete && strlen (delete) > 0)
	continue;

      snprintf (field, 31, "svqos_ip%d", i);
      data = websGetVar (wp, field, NULL);

      snprintf (field, 31, "svqos_ipprio%d", i);
      level = websGetVar (wp, field, NULL);

      if (strlen (svqos_var) > 0)
	sprintf (svqos_var, "%s %s %s |", svqos_var, data, level);
      else
	sprintf (svqos_var, "%s %s |", data, level);

    }

  if (strlen (svqos_var) <= sizeof (svqos_var))
    nvram_set ("svqos_ips", svqos_var);
  nvram_commit ();
  memset (svqos_var, 0, sizeof (svqos_var));

  /* MAC priorities */
  for (i = 0; i < no_macs; i++)
    {
      snprintf (field, 31, "svqos_macdel%d", i);
      delete = websGetVar (wp, field, NULL);

      if (delete && strlen (delete) > 0)
	continue;

      snprintf (field, 31, "svqos_mac%d", i);
      data = websGetVar (wp, field, NULL);

      snprintf (field, 31, "svqos_macprio%d", i);
      level = websGetVar (wp, field, NULL);

      if (strlen (svqos_var) > 0)
	sprintf (svqos_var, "%s %s %s |", svqos_var, data, level);
      else
	sprintf (svqos_var, "%s %s |", data, level);

    }

  if (strlen (svqos_var) <= sizeof (svqos_var))
    nvram_set ("svqos_macs", svqos_var);
  nvram_commit ();

  /* adm6996 LAN port priorities */
  nvram_set ("svqos_port1prio", websGetVar (wp, "svqos_port1prio", NULL));
  nvram_set ("svqos_port2prio", websGetVar (wp, "svqos_port2prio", NULL));
  nvram_set ("svqos_port3prio", websGetVar (wp, "svqos_port3prio", NULL));
  nvram_set ("svqos_port4prio", websGetVar (wp, "svqos_port4prio", NULL));

  nvram_set ("svqos_port1bw", websGetVar (wp, "svqos_port1bw", NULL));
  nvram_set ("svqos_port2bw", websGetVar (wp, "svqos_port2bw", NULL));
  nvram_set ("svqos_port3bw", websGetVar (wp, "svqos_port3bw", NULL));
  nvram_set ("svqos_port4bw", websGetVar (wp, "svqos_port4bw", NULL));

  nvram_commit ();

  return 0;
}

void
ej_get_clone_wmac (int eid, webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_RB500
  return 0;
#else

  char *c;
  int mac, which;
  int dofree = 0;
  if (ejArgs (argc, argv, "%d", &which) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  if (nvram_match ("def_whwaddr", "00:00:00:00:00:00"))
    {
      c = strdup (nvram_safe_get ("et0macaddr"));
      dofree = 1;
      if (c)
	{
	  MAC_ADD (c);
	  MAC_ADD (c);
	}
    }
  else
    c = nvram_safe_get ("def_whwaddr");

  if (c)
    {
      mac = get_single_mac (c, which);
      websWrite (wp, "%02X", mac);
      if (dofree)
	free (c);
    }
  else
    websWrite (wp, "00");

  return;
#endif
}



/* todo stylesheet compatible code */
/* lonewolf additions */

// Note that there is no VLAN #16.  It's just a convieniant way of denoting a "Tagged" port
void
ej_port_vlan_table (int eid, webs_t wp, int argc, char_t ** argv)
{
  /*
     vlans[x][y] where
     x 0-15 are VLANS
     x 16 is tagging, 17 is auto-negotiation, 18 is 100/10 Mbit, and 19 is Full/Half duplex
     y 0-4 are switch ports (port 5 is set automaticly)
     y 5 it the bridge device (x 16 dosn't apply)
   */

  int i, j, vlans[20][6], tmp, wl_br;
  char *c, *next, buff[32], portvlan[32];

  for (i = 0; i < 20; i++)
    for (j = 0; j < 6; j++)
      vlans[i][j] = -1;

  wl_br = -1;

  for (i = 0; i < 8; i++)
    {
      if (i < 5)
	snprintf (buff, 31, "port%dvlans", i);
      else if (i == 5)
	snprintf (buff, 31, "%s", "lan_ifnames");
      else
	snprintf (buff, 31, "ub%d_ifnames", i - 5);

      c = nvram_safe_get (buff);

      if (c)
	{
	  foreach (portvlan, c, next)
	  {
	    if (portvlan[0] == 'e' && portvlan[1] == 't' && portvlan[2] == 'h'
		&& portvlan[3] == '1')
	      wl_br = i - 5;
	    if (ISDIGIT (portvlan, 1)
		|| (portvlan[0] == 'v' && portvlan[1] == 'l'
		    && portvlan[2] == 'a' && portvlan[3] == 'n'))
	      {
		if (ISDIGIT (portvlan, 1))
		  tmp = atoi (portvlan);
		else
		  {
		    portvlan[0] = portvlan[4];
		    portvlan[1] = portvlan[5];
		    portvlan[2] = '\0';
		    if (ISDIGIT (portvlan, 1))
		      tmp = atoi (portvlan);
		    else
		      continue;
		  }

		if (i < 5)
		  {
		    vlans[tmp][i] = 1;
		  }
		else
		  {
		    vlans[tmp][5] = i - 5;
		  }
	      }
	  }
	}
    }

  for (i = 0; i < 20; i++)
    {
      websWrite (wp, "              <tr>\n");
      websWrite (wp, "<td>");

      switch (i)
	{
	case 16:
	  websWrite (wp,
		     "<script type=\"text/javascript\">Capture(vlan.tagged)</script>");
	  break;
	case 17:
	  websWrite (wp,
		     "<script type=\"text/javascript\">Capture(vlan.negociate)</script>");
	  break;
	case 18:
	  websWrite (wp, "100 Mbit");
	  break;
	case 19:
	  websWrite (wp, "Full-Duplex");
	  break;
	default:
	  snprintf (buff, 31, "%d", i);
	  websWrite (wp, buff);
	  break;
	}

      websWrite (wp, "</td>\n");

      for (j = 0; j < 5; j++)
	{
	  snprintf (buff, 31, "\"port%dvlan%d\"", j, i);
	  websWrite (wp, "<td");

	  if (j % 2 == 0)
	    websWrite (wp, " bgcolor=\"#CCCCCC\"");


	  websWrite (wp,
		     " height=\"20\"><div align=\"center\"><input type=\"checkbox\" value=\"on\" name=");
	  websWrite (wp, buff);

	  if (i < 17 || i > 19)
	    {
	      if (vlans[i][j] == 1)
		websWrite (wp, " checked=\"checked\"");
	    }
	  else
	    {
	      if (vlans[i][j] == -1)
		websWrite (wp, " checked=\"checked\"");
	    }

	  if (i < 17)
	    {
	      websWrite (wp, " onclick=");
	      snprintf (buff, sizeof (buff),
			"\"SelVLAN(this.form,'port%d')\"", j);
	      websWrite (wp, buff);
	    }
	  else if (i == 17)
	    {
	      websWrite (wp, " onclick=");
	      snprintf (buff, sizeof (buff),
			"\"SelSpeed(this.form,'port%d')\"", j);
	      websWrite (wp, buff);
	    }
	  websWrite (wp, " /></div></td>\n");
	}

      if (i < 16)
	{
	  websWrite (wp, "			<td><select name=");
	  snprintf (buff, 31, "\"vlan%d\"", i);
	  websWrite (wp, buff);
	  websWrite (wp,
		     "><script type=\"text/javascript\">document.write(\"<option value=\\\"-1\\\"");
	  if (vlans[i][5] < 0)
	    websWrite (wp, " selected=\\\"selected\\\"");
	  websWrite (wp,
		     ">\" + share.none + \"</option>\");</script><option value=\"0\"");
	  if (vlans[i][5] == 0)
	    websWrite (wp, " selected=\"selected\"");
	  websWrite (wp, ">LAN</option></select></td>\n");
	}
      else
	{
	  websWrite (wp, "<td>&nbsp;</td>\n");
	}

      websWrite (wp, "</tr>\n");

      if (i == 16 || i == 19)
	{
	  websWrite (wp, "<tr height=\"5\"><td>&nbsp;</td></tr>\n");
	}
    }

  websWrite (wp, "<tr>\n");
  websWrite (wp,
	     "<td><script type=\"text/javascript\">Capture(share.wireless)</script></td>\n");

  websWrite (wp,
	     "<td colspan=\"6\"><select name=\"wireless\"><script type=\"text/javascript\">document.write(\"<option value=\\\"-1\\\"");
  if (wl_br < 0)
    websWrite (wp, " selected=\\\"selected\\\"");
  websWrite (wp,
	     ">\" + share.none + \"</option>\");</script><option value=\"0\"");
  if (wl_br == 0)
    websWrite (wp, " selected=\"selected\"");
  websWrite (wp, ">LAN</option></select></td>\n");
  websWrite (wp, "</tr>\n");

  websWrite (wp, "<tr height=\"5\"><td>&nbsp;</td></tr>\n");

  websWrite (wp, "<tr>\n");
  websWrite (wp,
	     "<td><script type=\"text/javascript\">Capture(vlan.aggregation)</script></td>\n");

  websWrite (wp,
	     "<td colspan=\"6\"><select name=\"trunking\"><script type=\"text/javascript\">document.write(\"<option value=\\\"0\\\">\" + share.no + \"</option>\");</script><script type=\"text/javascript\">document.write(\"<option value=\\\"1\\\"");

  c = nvram_safe_get ("trunking");

  snprintf (buff, 5, "%s", c);

  if (atoi (buff) == 1)
    websWrite (wp, " selected=\\\"selected\\\"");

  websWrite (wp,
	     ">\" + vlan.trunk + \"</option>\");</script></select></td>\n");
  websWrite (wp, "              </tr>");

  return;
}

/* Note: VLAN #16 designates tagging.  There is no VLAN #16 (only 0-15) */

int
port_vlan_table_save (webs_t wp)
{
  int port = 0, vlan = 0, vlans[20], i;
  char portid[32], portvlan[64], *portval, buff[32], *c, *next, br0vlans[64],
    br1vlans[64], br2vlans[64];

  strcpy (portvlan, "");

  for (vlan = 0; vlan < 20; vlan++)
    vlans[vlan] = 0;

  vlans[16] = 1;

  for (port = 0; port < 5; port++)
    {
      for (vlan = 0; vlan < 20; vlan++)
	{
	  snprintf (portid, 31, "port%dvlan%d", port, vlan);
	  portval = websGetVar (wp, portid, "");

	  if (vlan < 17 || vlan > 19)
	    i = (strcmp (portval, "on") == 0);
	  else
	    i = (strcmp (portval, "on") != 0);

	  if (i)
	    {
	      if (strlen (portvlan) > 0)
		strcat (portvlan, " ");

	      snprintf (buff, 4, "%d", vlan);
	      strcat (portvlan, buff);
	      vlans[vlan] = 1;
	    }
	}

      snprintf (portid, 31, "port%dvlans", port);
      nvram_set (portid, portvlan);
      strcpy (portvlan, "");
    }

  /* done with ports 0-4, now set up #5 automaticly */
  /* if a VLAN is used, it also gets assigned to port #5 */
  for (vlan = 0; vlan < 17; vlan++)
    {
      if (vlans[vlan])
	{
	  if (strlen (portvlan) > 0)
	    strcat (portvlan, " ");

	  snprintf (buff, 4, "%d", vlan);
	  strcat (portvlan, buff);
	}
    }

  nvram_set ("port5vlans", portvlan);

  strcpy (br0vlans, "");
  c = nvram_safe_get ("lan_ifnames");
  if (c)
    {
      foreach (portid, c, next)
      {
	if (!(strncmp (portid, "vlan", 4) == 0)
	    && !(strncmp (portid, "eth1", 4) == 0))
	  {
	    if (strlen (br0vlans) > 0)
	      strcat (br0vlans, " ");
	    strcat (br0vlans, portid);
	  }
      }
    }

  strcpy (br1vlans, "");
  c = nvram_safe_get ("ub1_ifnames");
  if (c)
    {
      foreach (portid, c, next)
      {
	if (!(strncmp (portid, "vlan", 4) == 0)
	    && !(strncmp (portid, "eth1", 4) == 0))
	  {
	    if (strlen (br1vlans) > 0)
	      strcat (br1vlans, " ");
	    strcat (br1vlans, portid);
	  }
      }
    }

  strcpy (br2vlans, "");
  c = nvram_safe_get ("ub2_ifnames");
  if (c)
    {
      foreach (portid, c, next)
      {
	if (!(strncmp (portid, "vlan", 4) == 0)
	    && !(strncmp (portid, "eth1", 4) == 0))
	  {
	    if (strlen (br2vlans) > 0)
	      strcat (br2vlans, " ");
	    strcat (br2vlans, portid);
	  }
      }
    }

  for (i = 0; i < 16; i++)
    {
      snprintf (buff, 31, "vlan%d", i);
      portval = websGetVar (wp, buff, "");

      switch (atoi (portval))
	{
	case 0:
	  if (strlen (br0vlans) > 0)
	    strcat (br0vlans, " ");
	  strcat (br0vlans, buff);
	  break;
	case 1:
	  if (strlen (br1vlans) > 0)
	    strcat (br1vlans, " ");
	  strcat (br1vlans, buff);
	  break;
	case 2:
	  if (strlen (br2vlans) > 0)
	    strcat (br2vlans, " ");
	  strcat (br2vlans, buff);
	  break;
	}
    }

  strcpy (buff, "");

  switch (atoi (websGetVar (wp, "wireless", "")))
    {
    case 0:
      if (strlen (br0vlans) > 0)
	strcat (br0vlans, " ");
      strcat (br0vlans, get_wdev ());
      break;
    case 1:
      if (strlen (br1vlans) > 0)
	strcat (br1vlans, " ");
      strcat (br1vlans, get_wdev ());
      break;
    case 2:
      if (strlen (br2vlans) > 0)
	strcat (br2vlans, " ");
      strcat (br2vlans, get_wdev ());
      break;
    }

  snprintf (buff, 3, "%s", websGetVar (wp, "trunking", ""));

  nvram_set ("lan_ifnames", br0vlans);
  //nvram_set("ub1_ifnames", br1vlans);
  //nvram_set("ub2_ifnames", br2vlans);
  nvram_set ("trunking", buff);
  nvram_set ("vlans", "1");

  nvram_commit ();

  return 0;
}

/*
int ej_if_config_table(int eid, webs_t wp)
{
	int ret=0, dhcp=0, routing=0, firewall=0, ip[4], nm[4], i=0, j=0, found_wan=0;
	char *c, *next, *next2, all_ifnames[256], br_ifnames[256], ifnames[256], buff[256], buff2[64];
	char wan_ifname[10], *ip_aliases, ip_alias[24], dhcpd_opts[4][24];

	strcpy(br_ifnames, nvram_safe_get("lan_ifnames"));
	strcpy(wan_ifname, nvram_safe_get("wan_ifname"));

	if(strcmp(wan_ifname, nvram_safe_get("wan_ifnames")) != 0)
	{
		strcat(br_ifnames, " ");
		strcat(br_ifnames, nvram_safe_get("wan_ifnames"));
	}

	strcpy(all_ifnames, "eth1");
	strcpy(ifnames, nvram_safe_get("port5vlans"));
	if(strlen(ifnames) < 1)
		strcat(all_ifnames, " eth0");
	foreach(buff, ifnames, next)
	{
		if(atoi(buff) > 15)
			continue;
		snprintf(buff2, 63, " vlan%s", buff);
		strcat(all_ifnames, buff2);
	}

	strcpy(ifnames, "");

	foreach(buff, all_ifnames, next)
	{
		i = 1;

		foreach(buff2, br_ifnames, next2)
		{
			if(strcmp(buff, buff2) == 0)
				i = 0;
		}

		if(i)
		{
			if(strlen(ifnames) > 1)
				strcat(ifnames, " ");

			strcat(ifnames, buff);

			if(strcmp(wan_ifname, buff) == 0)
				found_wan = 1;
		}
	}

	if(!found_wan)
	{
		if(strlen(ifnames) > 1)
			strcat(ifnames, " ");
		strcat(ifnames, wan_ifname);
	}

	websWrite(wp, "<input type=hidden name=ifnames value=\"");
	websWrite(wp, ifnames);
	websWrite(wp, "\">\n");
	foreach(buff2, ifnames, next)
	{
		if(strcmp(buff2, "none") == 0)
			continue;
		dhcp = 0;
		routing = 0;
		firewall = 0;

		for (i=0;i<4;i++)
			strcpy(dhcpd_opts[i], "");

		c = nvram_safe_get("dhcpd_ifnames");
		if (c)
		{
			foreach(buff, c, next2)
			{
				if (strcmp(buff, buff2) == 0)
					dhcp = 1;
			}
		}

		c = nvram_safe_get("dhcpc_ifnames");
		if (c)
		{
			foreach(buff, c, next2)
			{
				if (strcmp(buff, buff2) == 0)
					dhcp = 2;
			}
		}

		c = nvram_safe_get("no_firewall_if");
		if (c)
		{
			foreach(buff, c, next2)
			{
				if (strcmp(buff, buff2) != 0)
					firewall = 1;
			}
		}

		c = nvram_safe_get("no_route_if");
		if (c)
		{
			foreach(buff, c, next2)
			{
				if (strcmp(buff, buff2) != 0)
					routing = 1;
			}
		}

		if (dhcp == 1)
		{
			snprintf(buff, sizeof(buff), "dhcpd_%s_conf", buff2);
			c = nvram_safe_get(buff);

			i = 0;
			if (c)
			{
				foreach(buff, c, next2)
				{
					snprintf(dhcpd_opts[i], sizeof(dhcpd_opts[i]), "%s", buff);
					i++;
					if (i>3)
						break;
				}
			}
		}

		ret += websWrite(wp, "			<TR>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF height=30 colspan=5><B>");
		if(strcmp(buff2, "eth1") == 0)
			ret += websWrite(wp, "WLAN");
		else
			ret += websWrite(wp, buff2);
		ret += websWrite(wp, "</B></TD>\n");
		ret += websWrite(wp, "			</TR>\n");
		ret += websWrite(wp, "			<TR>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF height=30><input type=radio value=0 name=");
		snprintf(buff, sizeof(buff), "%s_dhcp", buff2);
		ret += websWrite(wp, buff);
		if(dhcp == 0)
			ret += websWrite(wp, " checked");
		ret += websWrite(wp, " onClick=SelMode(this.form,\"");
		ret += websWrite(wp, buff2);
		ret += websWrite(wp, "\",this.value)>No DHCP</TD>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF height=30><input type=radio value=1 name=");
		snprintf(buff, sizeof(buff), "%s_dhcp", buff2);
		ret += websWrite(wp, buff);
		if(dhcp == 1)
			ret += websWrite(wp, " checked");
		ret += websWrite(wp, " onClick=SelMode(this.form,\"");
		ret += websWrite(wp, buff2);
		ret += websWrite(wp, "\",this.value)>DHCP Server</TD>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 0px solid\" align=center valign=middle bgColor=#FFFFFF height=30><input type=radio value=2 name=");
		snprintf(buff, sizeof(buff), "%s_dhcp", buff2);
		ret += websWrite(wp, buff);
		if(dhcp == 2)
			ret += websWrite(wp, " checked");
		ret += websWrite(wp, " onClick=SelMode(this.form,\"");
		ret += websWrite(wp, buff2);
		ret += websWrite(wp, "\",this.value)>DHCP Client</TD>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF height=30><input type=checkbox value=on name=");
		if(routing)
			snprintf(buff, sizeof(buff), "%s_route checked", buff2);
		else
			snprintf(buff, sizeof(buff), "%s_route", buff2);
		ret += websWrite(wp, buff);
		ret += websWrite(wp, ">Routing</TD>\n");

		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF height=30><input type=radio name=is_wan value=");
		ret += websWrite(wp, buff2);
		if(strcmp(buff2, wan_ifname) == 0)
		{
			ret += websWrite(wp, " checked");
		}
		ret += websWrite(wp, " onClick=SelWAN(this.form,this.value)>WAN</TD>\n");
		ret += websWrite(wp, "			</TR>\n");
		ret += websWrite(wp, "			<TR>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF height=30 colspan=2>IP Address<br>");

		snprintf(buff, sizeof(buff), "%s_ipaddr", buff2);
		c = nvram_safe_get(buff);

		for(i=0;i<4;i++)
			ip[i] = 0;

		if(c)
		{
			snprintf(buff, sizeof(buff), "%s", c);
			next2 = strtok(buff, ".");
			i = 0;
			while((next2 != NULL) && (i < 4))
			{
				ip[i] = atoi(next2);
				i++;
				next2 = strtok(NULL, ".");
			}
		}

		for(i=0;i<4;i++)
			if(ip[i] < 0 || ip[i] > 255)
				ip[i] = 0;

		snprintf(buff, sizeof(buff), "<input type=text name=%s_ip1 value=%d size=2 maxlength=3 onblur=valid_range(this,0,223,\"IP\")>.<input type=text name=%s_ip2 value=%d size=2 maxlength=3 onblur=valid_range(this,0,255,\"IP\")>.", buff2, ip[0], buff2, ip[1]);
		ret += websWrite(wp, buff);
		snprintf(buff, sizeof(buff), "<input type=text name=%s_ip3 value=%d size=2 maxlength=3 onblur=valid_range(this,0,255,\"IP\")>.<input type=text name=%s_ip4 value=%d size=2 maxlength=3 onblur=valid_range(this,0,255,\"IP\")>", buff2, ip[2], buff2, ip[3]);
		ret += websWrite(wp, buff);
		ret += websWrite(wp, "</TD>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF height=30 colspan=2>Netmask<br>");

		snprintf(buff, sizeof(buff), "%s_netmask", buff2);
		c = nvram_safe_get(buff);

		for(i=0;i<4;i++)
			nm[i] = 0;

		if(c)
		{
			snprintf(buff, sizeof(buff), "%s", c);
			next2 = strtok(buff, ".");
			i = 0;
			while((next2 != NULL) && (i < 4))
			{
				strcpy(buff, next2);
				nm[i] = atoi(buff);
				i++;
				next2 = strtok(NULL, ".");
			}
		}

		for(i=0;i<4;i++)
			if(nm[i] < 0 || nm[i] > 255)
				nm[i] = 0;

		if(nm[0] == 0)
		{
			for(i=0;i<3;i++)
				nm[i] = 255;
			nm[3] = 0;
		}

		snprintf(buff, sizeof(buff), "<input type=text name=%s_nm1 value=%d size=2 maxlength=3 onblur=valid_range(this,128,255,\"Netmask\")>.<input type=text name=%s_nm2 value=%d size=2 maxlength=3 onblur=valid_range(this,128,255,\"Netmask\")>.", buff2, nm[0], buff2, nm[1]);
		ret += websWrite(wp, buff);
		snprintf(buff, sizeof(buff), "<input type=text name=%s_nm3 value=%d size=2 maxlength=3 onblur=valid_range(this,128,255,\"Netmask\")>.<input type=text name=%s_nm4 value=%d size=2 maxlength=3 onblur=valid_range(this,128,255,\"Netmask\")>", buff2, nm[2], buff2, nm[3]);
		ret += websWrite(wp, buff);
		ret += websWrite(wp, "</TD>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=center valign=middle bgColor=#FFFFFF height=30><input type=checkbox value=on name=");
		snprintf(buff, sizeof(buff), "%s_firewall", buff2);
		ret += websWrite(wp, buff);
		if(firewall)
			ret += websWrite(wp, " checked");
		ret += websWrite(wp, ">Firewall</TD>\n");
		ret += websWrite(wp, "			</TR>\n");

		snprintf(buff, sizeof(buff), "%s_alias", buff2);
		ip_aliases = nvram_safe_get(buff);
		i=1;

		foreach(ip_alias, ip_aliases, c)
		{
			ret += websWrite(wp, "			<TR>\n");
			ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF height=30 colspan=3>");
			snprintf(buff, sizeof(buff), "Alias %d: %s", i, ip_alias);
			ret += websWrite(wp, buff);
			ret += websWrite(wp, "</TD>\n");
			ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=center valign=middle bgColor=#FFFFFF height=30><input type=button value=\"Delete\" onclick=DelIP(this.form,");
			snprintf(buff, sizeof(buff), "\"%s\",\"%s\",\"%d\"", buff2, ip_alias, i);
			ret += websWrite(wp, buff);
			ret += websWrite(wp, ") name=");
			snprintf(buff, sizeof(buff), "%s:%d", buff2, i);
			ret += websWrite(wp, buff);
			ret += websWrite(wp, "></TD>\n");
			ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=center valign=middle bgColor=#FFFFFF height=30>&nbsp;</TD>");
			ret += websWrite(wp, "			</TR>\n");
			i++;
		}

		ret += websWrite(wp, "			<TR>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF height=30 colspan=2>Alias IP Address<br>");

		snprintf(buff, sizeof(buff), "<input type=text name=%s_add_ip1 size=2 maxlength=3 onblur=valid_range(this,0,223,\"IP\")>.<input type=text name=%s_add_ip2 size=2 maxlength=3 onblur=valid_range(this,0,255,\"IP\")>.", buff2, buff2);
		ret += websWrite(wp, buff);
		snprintf(buff, sizeof(buff), "<input type=text name=%s_add_ip3 size=2 maxlength=3 onblur=valid_range(this,0,255,\"IP\")>.<input type=text name=%s_add_ip4 size=2 maxlength=3 onblur=valid_range(this,0,255,\"IP\")>", buff2, buff2);
		ret += websWrite(wp, buff);
		ret += websWrite(wp, "</TD>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF height=30 colspan=2>Alias Netmask<br>");

		snprintf(buff, sizeof(buff), "<input type=text name=%s_add_nm1 size=2 maxlength=3 onblur=valid_range(this,128,255,\"Netmask\")>.<input type=text name=%s_add_nm2 size=2 maxlength=3 onblur=valid_range(this,128,255,\"Netmask\")>.", buff2, buff2);
		ret += websWrite(wp, buff);
		snprintf(buff, sizeof(buff), "<input type=text name=%s_add_nm3 size=2 maxlength=3 onblur=valid_range(this,128,255,\"Netmask\")>.<input type=text name=%s_add_nm4 size=2 maxlength=3 onblur=valid_range(this,128,255,\"Netmask\")>", buff2, buff2);
		ret += websWrite(wp, buff);
		ret += websWrite(wp, "</TD>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=center valign=middle bgColor=#FFFFFF height=30><input type=Button value=\"Add\" onclick=AddIP(this.form)></TD>\n");
		ret += websWrite(wp, "			</TR>\n");
		ret += websWrite(wp, "			<TR>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF height=30 colspan=2>DHCP Start IP<br>");

		next2 = strtok(dhcpd_opts[0], ".");
		i = 0;
		j = ip[3];
		while((next2 != NULL) && (i < 4))
		{
			if (atoi(next2) >= 0 && atoi(next2) <= 255 && (i > 0 || (atoi(next2) <= 223 && atoi(next2) > 0)))
				ip[i] = atoi(next2);
			i++;
			next2 = strtok(NULL, ".");
		}

		if  (j == ip[3])
		{
			if (ip[3] < 100)
				ip[3] = 100;
			else if (ip[3] < 200)
				ip[3] = 200;
			else
				ip[3] = 1;
		}

		if(nm[0] < 255)
			snprintf(buff, sizeof(buff), "<input type=text name=%s_dhcpd_ip1 value=%d size=2 maxlength=3 onblur=valid_range(this,0,223,\"IP\")>.", buff2, ip[0]);
		else
			snprintf(buff, sizeof(buff), "%d.", ip[0]);
		ret += websWrite(wp, buff);
		if(nm[1] < 255)
			snprintf(buff, sizeof(buff), "<input type=text name=%s_dhcpd_ip2 value=%d size=2 maxlength=3 onblur=valid_range(this,0,255,\"IP\")>.", buff2, ip[1]);
		else
			snprintf(buff, sizeof(buff), "%d.", ip[1]);
		ret += websWrite(wp, buff);
		if(nm[2] < 255)
			snprintf(buff, sizeof(buff), "<input type=text name=%s_dhcpd_ip3 value=%d size=2 maxlength=3 onblur=valid_range(this,0,255,\"IP\")>.", buff2, ip[2]);
		else
			snprintf(buff, sizeof(buff), "%d.", ip[2]);
		ret += websWrite(wp, buff);
		snprintf(buff, sizeof(buff), "<input type=text name=%s_dhcpd_ip4 value=%d", buff2, ip[3]);
		ret += websWrite(wp, buff);
		ret += websWrite(wp, " size=2 maxlength=3 onblur=valid_range(this,0,255,\"IP\")></TD>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF height=30>Max Clients<br><input type=text size=2 maxlength=3 name=");
		snprintf(buff, sizeof(buff), "%s_dhcpd_max value=", buff2);
		ret += websWrite(wp, buff);
		snprintf(buff, sizeof(buff), "dhcpd_%s_max", buff2);
		i = atoi(dhcpd_opts[1]);
		if(i < 1 || i > 16777216)
			i = 50;
		snprintf(buff, sizeof(buff), "%d", i);
		ret += websWrite(wp, buff);
		ret += websWrite(wp, "></TD>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF height=30>Lease Time (sec)<br><input type=text size=6 maxlength=8 name=");
		snprintf(buff, sizeof(buff), "%s_dhcpd_time value=", buff2);
		ret += websWrite(wp, buff);
		snprintf(buff, sizeof(buff), "dhcpd_%s_time", buff2);
		i = atoi(dhcpd_opts[2]);
		if(i < 0 || i > 99999999)
			i = 0;
		snprintf(buff, sizeof(buff), "%d", i);
		ret += websWrite(wp, buff);
		ret += websWrite(wp, "></TD>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=center valign=middle bgColor=#FFFFFF height=30>Server<br>Options</TD>\n");
		ret += websWrite(wp, "			</TR>\n");
		ret += websWrite(wp, "			<TR>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF height=30 colspan=4>FQDN <input type=text name=");
		snprintf(buff, sizeof(buff), "%s_fqdn value=\"", buff2);
		ret += websWrite(wp, buff);
		snprintf(buff, sizeof(buff), "dhcpc_%s_name", buff2);
		ret += websWrite(wp, nvram_safe_get(buff));
		ret += websWrite(wp, "\"></TD>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle valign=center valign=middle bgColor=#FFFFFF height=30>Client<br>Options</TD>\n");
		ret += websWrite(wp, "			</TR>\n");
		ret += websWrite(wp, "			<TR>\n");
		ret += websWrite(wp, "				<TD height=30 colspan=5>&nbsp;</TD>\n");
		ret += websWrite(wp, "			</TR>\n");
	}

	ret += websWrite(wp, "			<TR>\n");
	ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle valign=center valign=middle bgColor=#FFFFFF height=30 colspan=4>None</TD>\n");
	ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle valign=center valign=middle bgColor=#FFFFFF height=30><input type=radio name=is_wan value=none");
	if(strcmp("none", wan_ifname) == 0)
	{
		ret += websWrite(wp, " checked");
	}
	ret += websWrite(wp, ">WAN</TD>\n");
	ret += websWrite(wp, "			</TR>\n");
	ret += websWrite(wp, "			<TR>\n");
	ret += websWrite(wp, "				<TD height=30 colspan=5>&nbsp;</TD>\n");
	ret += websWrite(wp, "			</TR>\n");

	return ret;
}

int if_config_table_save(webs_t wp)
{
	int ret=0, dhcp=0, ip[4], nm[4], i=0, j=0, cidr=0;
	char *c, *next, *next2, all_ifnames[256], br_ifnames[256], ifnames[256];
	char dhcpd_ifnames[256], dhcpc_ifnames[256], no_route_if[384], no_firewall_if[384];
	char buff[512], buff2[64], buff3[64], wan_ifname[10];

	strcpy(br_ifnames, nvram_safe_get("lan_ifnames"));
	strcpy(wan_ifname, nvram_safe_get("wan_ifname"));

	if(strcmp(wan_ifname, nvram_safe_get("wan_ifnames")) != 0)
	{
		strcat(br_ifnames, " ");
		strcat(br_ifnames, nvram_safe_get("wan_ifnames"));
	}
      <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />

      <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
	strcpy(all_ifnames, "eth1");
	strcpy(ifnames, nvram_safe_get("port5vlans"));
	if(strlen(ifnames) < 1)
		strcat(all_ifnames, " eth0");
	foreach(buff, ifnames, next)
	{
		if(atoi(buff) > 15)
			continue;
		snprintf(buff2, 63, " vlan%s", buff);
		strcat(all_ifnames, buff2);
	}

	strcpy(ifnames, "");

	foreach(buff, all_ifnames, next)
	{
		i = 1;

		foreach(buff2, br_ifnames, next2)
		{
			if(strcmp(buff, buff2) == 0)
				i = 0;
		}

		if(i)
		{
			if(strlen(ifnames) > 1)
				strcat(ifnames, " ");

			strcat(ifnames, buff);
		}
	}

	snprintf(wan_ifname, sizeof(wan_ifname), "%s", websGetVar(wp, "is_wan", ""));
	nvram_set("wan_ifname", wan_ifname);
	nvram_set("wan_ifnames", wan_ifname);

	strcpy(dhcpd_ifnames, "");
	strcpy(dhcpc_ifnames, "");
	strcpy(no_route_if, "");
	strcpy(no_firewall_if, "");

	foreach(buff2, ifnames, next)
	{
		if(strcmp(buff2, wan_ifname) == 0)
			continue;

		snprintf(buff, sizeof(buff), "dhcpc_%s_name", buff2);
		nvram_unset(buff);
		snprintf(buff, sizeof(buff), "dhcpd_%s_conf", buff2);
		nvram_unset(buff);
		snprintf(buff, sizeof(buff), "%s_ipaddr", buff2);
		nvram_unset(buff);
		snprintf(buff, sizeof(buff), "%s_netmask", buff2);
		nvram_unset(buff);

		snprintf(buff, sizeof(buff), "%s_route", buff2);
		if (strcmp(websGetVar(wp, buff, ""), "on") != 0)
		{
			if (strlen(no_route_if) > 1)
				strcat(no_route_if, " ");
			strcat(no_route_if, buff2);
		}

		snprintf(buff, sizeof(buff), "%s_firewall", buff2);
		if (strcmp(websGetVar(wp, buff, ""), "on") != 0)
		{
			if (strlen(no_firewall_if) > 1)
				strcat(no_firewall_if, " ");
			strcat(no_firewall_if, buff2);
		}

		strcpy(buff, "");

		for(i=0; i<4; i++)
		{
			ip[i] = 0;
			snprintf(buff3, sizeof(buff3), "%s_ip%d", buff2, (i+1));
			c = websGetVar(wp, buff3, "");

			if (c)
			{
				ip[i] = atoi(c);

				if (atoi(c) > 255 || atoi(c) < 0 || (i == 0 && atoi(c) > 223))
					ip[i] = 0;
			}

			snprintf(buff3, sizeof(buff3), "%d", ip[i]);
			strcat(buff, buff3);
			if (i < 3)
				strcat(buff, ".");
		}

		if (ip[0] == 0 || ip[0] > 223)
		{
			ip[0]=ip[1]=ip[2]=ip[3]=0;
			strcpy(buff, "0.0.0.0");
		}

		snprintf(buff3, sizeof(buff3), "%s_ipaddr", buff2);
		nvram_set(buff3, buff);

		strcpy(buff, "");

		for(i=0; i<4; i++)
		{
			snprintf(buff3, sizeof(buff3), "%s_nm%d", buff2, (i+1));
			c = websGetVar(wp, buff3, "");

			nm[i] = atoi(c);

			if (atoi(c) > 255 || atoi(c) < 0)
				nm[i] = 0;

			snprintf(buff3, sizeof(buff3), "%d", nm[i]);
			strcat(buff, buff3);
			if (i < 3)      <meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1" />

      <meta http-equiv="Content-Type" content="application/xhtml+xml; charset=iso-8859-1" />
				strcat(buff, ".");
		}

		if (nm[0] == 0 || nm[3] == 255)
		{
			nm[0]=nm[1]=nm[2]=255;
			nm[3]=0;
			strcpy(buff, "255.255.255.0");
		}

		snprintf(buff3, sizeof(buff3), "%s_netmask", buff2);
		nvram_set(buff3, buff);

		strcpy(buff, "");

		snprintf(buff, sizeof(buff), "%s_dhcp", buff2);
		dhcp = atoi(websGetVar(wp, buff, ""));

		if (dhcp == 2)
		{
			if (strlen(dhcpc_ifnames) > 1)
				strcat(dhcpc_ifnames, " ");
			strcat(dhcpc_ifnames, buff2);
			snprintf(buff, sizeof(buff), "dhcpc_%s_name", buff2);
			snprintf(buff3, sizeof(buff3), "%s_fqdn", buff2);
			nvram_set(buff, websGetVar(wp, buff3, ""));
		}
		else if (dhcp == 1)
		{
			if (strlen(dhcpd_ifnames) > 1)
				strcat(dhcpd_ifnames, " ");
			strcat(dhcpd_ifnames, buff2);

			strcpy(buff, "");
			strcpy(buff3, "");
			for(i=0; i<4; i++)
			{
				snprintf(buff3, sizeof(buff3), "%s_dhcpd_ip%d", buff2, (i+1));
				c = websGetVar(wp, buff3, "");

				if (strlen(c) < 1 || atoi(c) > 255 || atoi(c) < 0 || (i == 0 && (atoi(c) < 1 || atoi(c) > 223)))
					snprintf(buff3, sizeof(buff3), "%d", ip[i]);
				else
					snprintf(buff3, sizeof(buff3), "%s", c);
				strcat(buff, buff3);
				if (i < 3)
					strcat(buff, ".");
				else
					strcat(buff, " ");
			}

			snprintf(buff3, sizeof(buff3), "%s_dhcpd_max", buff2);
			c = websGetVar(wp, buff3, "");
			if (strlen(c) < 1 || atoi(c) < 1 || atoi(c) > 16777216)
				strcat(buff, "50");
			else
				strcat(buff, c);
			strcat(buff, " ");

			snprintf(buff3, sizeof(buff3), "%s_dhcpd_time", buff2);
			c = websGetVar(wp, buff3, "");
			if (strlen(c) < 1 || atoi(c) < 5 || atoi(c) > 99999999)
				strcat(buff, "86400");
			else
				strcat(buff, c);
			strcat(buff, " ");

			strcat(buff, "0");

			snprintf(buff3, sizeof(buff3), "dhcpd_%s_conf", buff2);
			nvram_set(buff3, buff);
		}

		snprintf(buff, sizeof(buff), "%s_add_ip1", buff2);
		c = websGetVar(wp, buff, "");
		if (atoi(c) > 0 && atoi(c) < 224)
		{
			snprintf(buff, sizeof(buff), "%s", c);
			for(i=2; i<5; i++)
			{
				snprintf(buff3, sizeof(buff3), "%s_add_ip%d", buff2, i);
				c = websGetVar(wp, buff3, "");
				strcat(buff, ".");
				if (atoi(c) >= 0 && atoi(c) < 256)
					strcat(buff, c);
				else
					if (i < 4)
						strcat(buff, "0");
					else
						strcat(buff, "1");
			}

			strcpy(buff3, buff);

			cidr = 0;
			for(i=0; i<4; i++)
			{
				snprintf(buff, sizeof(buff), "%s_add_nm%d", buff2, i+1);
				c = websGetVar(wp, buff, "");
				if (atoi(c) >= 0 && atoi(c) < 256 && (i > 0 || atoi(c) > 0))
					for(j=0; j<8; j++)
						cidr += ((atoi(c) << j) & 255) >> 7;
				else
					if (i < 3)
						cidr += 8;
			}
			snprintf(buff, sizeof(buff), "/%d", cidr);
			strcat(buff3, buff);

			snprintf(buff, sizeof(buff), "%s_alias", buff2);
			c = nvram_safe_get(buff);
			snprintf(buff, sizeof(buff), "%s", c);
			if (strlen(buff) > 1)
				strcat(buff, " ");
			strcat(buff, buff3);
			snprintf(buff3, sizeof(buff3), "%s_alias", buff2);
			nvram_set(buff3, buff);
		}
	}

	nvram_set("no_firewall_if", no_firewall_if);
	nvram_set("no_route_if", no_route_if);
	nvram_set("dhcpc_ifnames", dhcpc_ifnames);
	nvram_set("dhcpd_ifnames", dhcpd_ifnames);

	return ret;
}
*/
/*
int alias_delete_ip(webs_t wp)
{
	char delif[16], delip[24], buff[512], buff2[24], buff3[512], *next;
	int delid = 0;

	if (atoi(websGetVar(wp, "mode", "")) != 1)
		return if_config_table_save(wp);

	strcpy(buff3, "");
	strcpy(delif, websGetVar(wp, "delif", ""));
	strcpy(delip, websGetVar(wp, "delip", ""));
	delid = atoi(websGetVar(wp, "delid", ""));

	snprintf(buff2, sizeof(buff2), "%s_alias", delif);
	snprintf(buff, sizeof(buff), "%s", nvram_safe_get(buff2));

	foreach(buff2, buff, next)
	{
		if (strcmp(buff2, delip) != 0)
		{
			if (strlen(buff3) > 0)
				strcat(buff3, " ");
			strcat(buff3, buff2);
		}
	}
	snprintf(buff2, sizeof(buff2), "%s_alias", delif);
	nvram_set(buff2, buff3);

	return if_config_table_save(wp);
}
*/
/* end lonewolf additions */

void
ej_get_qossvcs2 (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *qos_svcs = nvram_safe_get ("svqos_svcs");
  char name[32], type[32], data[32], level[32];
  int no_svcs = 0, i = 0;

// calc # of services
// no_svcs = strspn(qos_svcs,"|");

  while ((qos_svcs = strpbrk (qos_svcs, "|")))
    {
      no_svcs++;
      qos_svcs++;
    }

// write HTML data

  websWrite (wp,
	     "<input type=\"hidden\" name=\"svqos_nosvcs\" value=\"%d\" />",
	     no_svcs);

  qos_svcs = nvram_safe_get ("svqos_svcs");

  /* services format is "name type data level | name type data level |" ..etc */
  for (i = 0; i < no_svcs && qos_svcs && qos_svcs[0]; i++)
    {
      if (sscanf (qos_svcs, "%31s %31s %31s %31s ", name, type, data, level) <
	  4)
	break;

      websWrite (wp, "<tr>\n\
					<td>\n\
						<input type=\"checkbox\" name=\"svqos_svcdel%d\" />\n\
						<input type=\"hidden\" name=\"svqos_svcname%d\" value=\"%s\" />\n\
						<input type=\"hidden\" name=\"svqos_svctype%d\" value=\"%s\" />\n\
					</td>\n\
					<td><em>%s</em></td>\n\
					<td >\n\
						<select name=\"svqos_svcprio%d\"> \n\
							<script type=\"text/javascript\">document.write(\"<option value=\\\"100\\\" %s >\" + qos.prio_x + \"</option>\");</script>\n\
							<script type=\"text/javascript\">document.write(\"<option value=\\\"10\\\" %s >\" + qos.prio_p + \"</option>\");</script>\n\
							<script type=\"text/javascript\">document.write(\"<option value=\\\"20\\\" %s >\" + qos.prio_e + \"</option>\");</script>\n\
							<script type=\"text/javascript\">document.write(\"<option value=\\\"30\\\" %s >\" + qos.prio_s + \"</option>\");</script>\n\
							<script type=\"text/javascript\">document.write(\"<option value=\\\"40\\\" %s >\" + qos.prio_b + \"</option>\");</script>\n\
						</select>\n\
		           </td>\n\
				</tr>\n", i, i, name, i, type, name, i, strcmp (level, "100") == 0 ? "selected=\\\"selected\\\"" : "", strcmp (level, "10") == 0 ? "selected=\\\"selected\\\"" : "", strcmp (level, "20") == 0 ? "selected=\\\"selected\\\"" : "", strcmp (level, "30") == 0 ? "selected=\\\"selected\\\"" : "", strcmp (level, "40") == 0 ? "selected=\\\"selected\\\"" : "");

      qos_svcs = strpbrk (++qos_svcs, "|");
      qos_svcs++;

    }

  return;
}

#ifndef HAVE_AQOS
void
ej_get_qosips2 (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *qos_ips = nvram_safe_get ("svqos_ips");
  char ip[32], level[32];
  int no_ips = 0, i = 0;

  // calc # of ips
  while ((qos_ips = strpbrk (qos_ips, "|")))
    {
      no_ips++;
      qos_ips++;
    }
  websWrite (wp, "<tr>\n\
  					<th><script type=\"text/javascript\">Capture(share.del)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(qos.ipmask)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(share.priority)</script></th>\n\
  				</tr>\n");

  // write HTML data

  websWrite (wp,
	     "<input type=\"hidden\" name=\"svqos_noips\" value=\"%d\" />",
	     no_ips);

  qos_ips = nvram_safe_get ("svqos_ips");

  /* IP format is "data level | data level |" ..etc */
  for (i = 0; i < no_ips && qos_ips && qos_ips[0]; i++)
    {
      if (sscanf (qos_ips, "%31s %31s ", ip, level) < 2)
	break;

      websWrite (wp, "<tr>\n\
					<td>\n\
						<input type=\"checkbox\" name=\"svqos_ipdel%d\" />\n\
						<input type=\"hidden\" name=\"svqos_ip%d\" value=\"%s\" />\n\
					</td>\n\
					<td><em>%s</em></td>\n\
					<td>\n\
						<select name=\"svqos_ipprio%d\"> \n\
							<script type=\"text/javascript\">document.write(\"<option value=\\\"100\\\" %s >\" + qos.prio_x + \"</option>\");</script>\n\
							<script type=\"text/javascript\">document.write(\"<option value=\\\"10\\\" %s >\" + qos.prio_p + \"</option>\");</script>\n\
							<script type=\"text/javascript\">document.write(\"<option value=\\\"20\\\" %s >\" + qos.prio_e + \"</option>\");</script>\n\
							<script type=\"text/javascript\">document.write(\"<option value=\\\"30\\\" %s >\" + qos.prio_s + \"</option>\");</script>\n\
							<script type=\"text/javascript\">document.write(\"<option value=\\\"40\\\" %s >\" + qos.prio_b + \"</option>\");</script>\n\
						</select>\n\
					</td>\n\
				</tr>\n", i, i, ip, ip, i, strcmp (level, "100") == 0 ? "selected=\\\"selected\\\"" : "", strcmp (level, "10") == 0 ? "selected=\\\"selected\\\"" : "", strcmp (level, "20") == 0 ? "selected=\\\"selected\\\"" : "", strcmp (level, "30") == 0 ? "selected=\\\"selected\\\"" : "", strcmp (level, "40") == 0 ? "selected=\\\"selected\\\"" : "");

      qos_ips = strpbrk (++qos_ips, "|");
      qos_ips++;

    }

  return;
}
#else
void
ej_get_qosips2 (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *qos_ips = nvram_safe_get ("svqos_ips");
  char ip[32], level[32];
  int no_ips = 0, i = 0;

// calc # of ips
  while ((qos_ips = strpbrk (qos_ips, "|")))
    {
      no_ips++;
      qos_ips++;
    }
  websWrite (wp, "<tr>\n\
  					<th><script type=\"text/javascript\">Capture(share.del)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(qos.ipmask)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(qos.maxrate_b)</script></th>\n\
  				</tr>\n");

// write HTML data

  websWrite (wp,
	     "<input type=\"hidden\" name=\"svqos_noips\" value=\"%d\" />",
	     no_ips);

  qos_ips = nvram_safe_get ("svqos_ips");

  /* IP format is "data level | data level |" ..etc */
  for (i = 0; i < no_ips && qos_ips && qos_ips[0]; i++)
    {
      if (sscanf (qos_ips, "%31s %31s ", ip, level) < 2)
	break;

      websWrite (wp, "<tr>\n\
					<td>\n\
						<input type=\"checkbox\" name=\"svqos_ipdel%d\" />\n\
						<input type=\"hidden\" name=\"svqos_ip%d\" value=\"%s\" />\n\
					</td>\n\
					<td><em>%s</em></td>\n\
					<td>\n\
						<input name=\"svqos_ipprio%d\" class=\"num\" size=\"5\" maxlength=\"5\" value=\"%s\" /> \n\
						</select>\n\
					</td>\n\
				</tr>\n", i, i, ip, ip, i, level);

      qos_ips = strpbrk (++qos_ips, "|");
      qos_ips++;

    }

  return;
}
#endif
#ifndef HAVE_AQOS
void
ej_get_qosmacs2 (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *qos_macs = nvram_safe_get ("svqos_macs");
  char mac[32], level[32];
  int no_macs = 0, i = 0;


  // calc # of ips
  while ((qos_macs = strpbrk (qos_macs, "|")))
    {
      no_macs++;
      qos_macs++;
    }

  websWrite (wp, "<tr>\n\
  					<th><script type=\"text/javascript\">Capture(share.del)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(share.mac)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(share.priority)</script></th>\n\
  				</tr>\n");

  // write HTML data
  websWrite (wp,
	     "<input type=\"hidden\" name=\"svqos_nomacs\" value=\"%d\" />",
	     no_macs);

  qos_macs = nvram_safe_get ("svqos_macs");

  /* IP format is "data level | data level |" ..etc */
  for (i = 0; i < no_macs && qos_macs && qos_macs[0]; i++)
    {
      if (sscanf (qos_macs, "%31s %31s ", mac, level) < 2)
	break;

      websWrite (wp, "<tr>\n\
					<td>\n\
						<input type=\"checkbox\" name=\"svqos_macdel%d\" />\n\
						<input type=\"hidden\" name=\"svqos_mac%d\" value=\"%s\" />\n\
					</td>\n\
					<td><em>%s</em></td>\n\
					<td>\n\
						<select name=\"svqos_macprio%d\"> \n\
							<script type=\"text/javascript\">document.write(\"<option value=\\\"100\\\" %s >\" + qos.prio_x + \"</option>\");</script>\n\
							<script type=\"text/javascript\">document.write(\"<option value=\\\"10\\\" %s >\" + qos.prio_p + \"</option>\");</script>\n\
							<script type=\"text/javascript\">document.write(\"<option value=\\\"20\\\" %s >\" + qos.prio_e + \"</option>\");</script>\n\
							<script type=\"text/javascript\">document.write(\"<option value=\\\"30\\\" %s >\" + qos.prio_s + \"</option>\");</script>\n\
							<script type=\"text/javascript\">document.write(\"<option value=\\\"40\\\" %s >\" + qos.prio_b + \"</option>\");</script>\n\
						</select>\n\
					</td>\n\
				</tr>\n", i, i, mac, mac, i, strcmp (level, "100") == 0 ? "selected=\\\"selected\\\"" : "", strcmp (level, "10") == 0 ? "selected=\\\"selected\\\"" : "", strcmp (level, "20") == 0 ? "selected=\\\"selected\\\"" : "", strcmp (level, "30") == 0 ? "selected=\\\"selected\\\"" : "", strcmp (level, "40") == 0 ? "selected=\\\"selected\\\"" : "");

      qos_macs = strpbrk (++qos_macs, "|");
      qos_macs++;

    }

  return;
}

#else
void
ej_get_qosmacs2 (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *qos_macs = nvram_safe_get ("svqos_macs");
  char mac[32], level[32];
  int no_macs = 0, i = 0;


  // calc # of ips
  while ((qos_macs = strpbrk (qos_macs, "|")))
    {
      no_macs++;
      qos_macs++;
    }
  websWrite (wp, "<tr>\n\
  					<th><script type=\"text/javascript\">Capture(share.del)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(share.mac)</script></th>\n\
  					<th><script type=\"text/javascript\">Capture(qos.maxrate_b)</script></th>\n\
  				</tr>\n");

  // write HTML data
  websWrite (wp,
	     "<input type=\"hidden\" name=\"svqos_nomacs\" value=\"%d\" />",
	     no_macs);

  qos_macs = nvram_safe_get ("svqos_macs");

  /* IP format is "data level | data level |" ..etc */
  for (i = 0; i < no_macs && qos_macs && qos_macs[0]; i++)
    {
      if (sscanf (qos_macs, "%31s %31s ", mac, level) < 2)
	break;

      websWrite (wp, "<tr>\n\
					<td>\n\
						<input type=\"checkbox\" name=\"svqos_macdel%d\" />\n\
						<input type=\"hidden\" name=\"svqos_mac%d\" value=\"%s\" />\n\
					</td>\n\
					<td><em>%s</em></td>\n\
					<td>\n\
						<input name=\"svqos_macprio%d\" class=\"num\" size=\"5\" maxlength=\"5\" value=\"%s\" /> \n\
					</td>\n\
				</tr>\n", i, i, mac, mac, i, level);

      qos_macs = strpbrk (++qos_macs, "|");
      qos_macs++;

    }

  return;
}
#endif



/* Added by Botho 03.April.06 */
void
ej_dumpip_conntrack (int eid, webs_t wp, int argc, char_t ** argv)
{
  int ip_count = 0;
  FILE *fp;
  int c;

  fp = fopen ("/proc/net/ip_conntrack", "rb");
  if (fp == NULL)
    return;
  while (!feof (fp))
    {
      c = getc (fp);
      if (c == EOF)
	break;
      if (c == 0xa)
	ip_count++;
    }

  websWrite (wp, "%d", ip_count);

  fclose (fp);

  return;
}


/* BEGIN  Added by Botho 21.April.06 */
void
ej_js_include (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *lang = nvram_get ("language");

  do_file ("common.js", wp);
  do_file ("lang_pack/english.js", wp);

  if (lang != NULL)
    {
      char l[60];
      sprintf (l, "lang_pack/%s.js", lang);
      do_file (l, wp);
    }

  return;
}
extern websRomPageIndexType websRomPageIndex[];

void
ej_css_include (int eid, webs_t wp, int argc, char_t ** argv)
{

  char *style = nvram_get ("router_style");

  do_file ("style/common.css", wp);

  if (style == NULL || strlen (style) == 0)
    do_file ("style/cyan/style.css", wp);
  else
    {

      char l[60];
      char filename[60];
      sprintf (l, "style/%s", style);
      int i = 0;
      while (websRomPageIndex[i].path != NULL)
	{
	  if (!strncmp (websRomPageIndex[i].path, l, strlen (l)))
	    {
	      websWrite (wp, "\n/* %s */\n\n", websRomPageIndex[i].path);
	      do_file (websRomPageIndex[i].path, wp);
	      break;
	    }
	  i++;
	}

    }

  return;

}

/* END  Added by Botho 21.April.06 */

/* BEGIN  Added by Botho 10.May.06 */
void
ej_show_wan_to_switch (int eid, webs_t wp, int argc, char_t ** argv)
{
  if (nvram_match ("wan_proto", "disabled") || !nvram_match ("wl_mode", "ap"))	//WAN disabled OR Wirelles is not AP
    {
      websWrite (wp, "<fieldset>\n\
							<legend><script type=\"text/javascript\">Capture(idx.legend2)</script></legend>\n\
								<div class=\"setting\">\n\
									<div class=\"label\"><script type=\"text/javascript\">Capture(idx.wantoswitch)</script></div>\n\
									<input class=\"spaceradio\" type=\"checkbox\" name=\"_fullswitch\" value=\"1\" %s />\n\
								</div>\n\
							</fieldset><br />\n", nvram_match ("fullswitch", "1") ? "checked=\"checked\"" : "");
    }

  return;
}

/* END  Added by Botho 10.May.06 */
