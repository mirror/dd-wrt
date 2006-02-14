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
 * $id
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

#define ASSOCLIST_TMP	"/tmp/.wl_assoclist"
#define RSSI_TMP	"/tmp/.rssi"
#define ASSOCLIST_CMD	"wl assoclist"
#define RSSI_CMD	"wl rssi"
#define NOISE_CMD	"wl noise"







int
ej_show_wds_subnet (int eid, webs_t wp, int argc, char_t ** argv)
{
  int index = -1;
  if (ejArgs (argc, argv, "%d", &index) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return -1;
    }
  if (nvram_invmatch ("wl_br1_enable", "1"))
    return 0;
  char buf[16];
  sprintf (buf, "wl_wds%d_enable", index);
  websWrite (wp, "<OPTION value=2 %s >Subnet</OPTION>\n",
	     nvram_selmatch (wp, buf, "2") ? "selected" : "");
  return 0;
}



#ifdef HAVE_SKYTRON
int
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
cprintf("adding %s\n",a);

  char *count;
  int c;
  char buf[20];
  count = nvram_safe_get (a);
cprintf("count = %s\n",count);
  if (count != NULL && strlen (count) > 0)
    {
      c = atoi (count);
      if (c > -1)
	{
	  c++;
	  sprintf (buf, "%d", c);
	  cprintf("set %s to %s\n",a,buf);
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

int
ej_show_paypal (int eid, webs_t wp, int argc, char_t ** argv)
{
#ifndef CONFIG_BRANDING
  websWrite (wp, "<a href=\"http://www.dd-wrt.com/\">DD-WRT</a>");
  websWrite (wp,
	     "<form action=\"https://www.paypal.com/cgi-bin/webscr\" method=\"post\" target=\"_blank\">");
  websWrite (wp, "<input type=\"hidden\" name=\"cmd\" value=\"_xclick\">");
  websWrite (wp,
	     "<input type=\"hidden\" name=\"business\" value=\"sebastian.gottschall@advis.de\">");
  websWrite (wp,
	     "<input type=\"hidden\" name=\"item_name\" value=\"DD-WRT Development Support\">");
  websWrite (wp, "<input type=\"hidden\" name=\"no_note\" value=\"1\">");
  websWrite (wp,
	     "<input type=\"hidden\" name=\"currency_code\" value=\"EUR\">");
  websWrite (wp, "<input type=\"hidden\" name=\"tax\" value=\"0\">");
  websWrite (wp,
	     "<input type=\"image\" src=\"images/paypal.gif\" border=\"0\" name=\"submit\" width=\"62\" height=\"31\">");
  websWrite (wp, "</form>");
#endif
  return 0;
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
#ifdef HAVE_FON

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

int
ej_show_userlist (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *sln = nvram_safe_get ("fon_usernames");
  if (sln == NULL || strlen (sln) == 0)
    return 0;
  int leasenum = atoi (sln);
  if (leasenum == 0)
    return 0;
int i;
char username[32];
char password[32];
char *u = nvram_safe_get("fon_userlist");
char *userlist = (char *) malloc (strlen (u) + 1);
strcpy(userlist,u);
char *o = userlist;
  for (i = 0; i < leasenum; i++)
    {
    snprintf (username, 31, "fon_user%d_name", i);
    char *sep = strsep (&userlist, "=");
    websWrite(wp,"<tr><td>\n");
    websWrite(wp,"<input class=\"num\" name=\"%s\" value=\"%s\" size=\"18\" maxlength=\"63\" />\n",username,sep!=NULL?sep:"");
    websWrite(wp,"</td>\n");
    sep = strsep (&userlist, " ");    
    snprintf (password, 31, "fon_user%d_password", i);
    websWrite(wp,"<td>\n");
    websWrite(wp,"<input type=\"password\" name=\"%s\" value=\"blahblahblah\" size=\"18\" maxlength=\"63\" />\n",password);
    websWrite(wp,"</td></tr>\n");
    }
free(o);    
return 0;
}


void
validate_userlist (webs_t wp, char *value, struct variable *v)
{
  char username[32] = "fon_userxxx_name";
  char password[32] = "fon_userxxx_password";
  char *sln = nvram_safe_get ("fon_usernames");
  char *uname;
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

int
ej_show_staticleases (int eid, webs_t wp, int argc, char_t ** argv)
{
  int i;
//cprintf("get static leasenum");

  char *sln = nvram_safe_get ("static_leasenum");
//cprintf("check null");
  if (sln == NULL || strlen (sln) == 0)
    return 0;
//cprintf("atoi");

  int leasenum = atoi (sln);
//cprintf("leasenum==0");
  if (leasenum == 0)
    return 0;
//cprintf("get leases");
  char *nvleases = nvram_safe_get ("static_leases");
  char *leases = (char *) malloc (strlen (nvleases) + 1);
  char *originalpointer = leases;	//strsep destroys the pointer by moving it
  strcpy (leases, nvleases);
  for (i = 0; i < leasenum; i++)
    {
      char *sep = strsep (&leases, "=");
      websWrite (wp,
		 "<tr><td><input name=\"lease%d_hwaddr\" value=\"%s\" size=\"18\" maxlength=\"18\" class=\"num\" /></td>",
		 i, sep != NULL ? sep : "");
      sep = strsep (&leases, "=");
      websWrite (wp,
		 "<td><input name=\"lease%d_hostname\" value=\"%s\" size=\"16\" maxlength=\"16\" class=\"num\" /></td>",
		 i, sep != NULL ? sep : "");
      sep = strsep (&leases, " ");
      websWrite (wp,
		 "<td><input name=\"lease%d_ip\" value=\"%s\" size=\"15\" maxlength=\"15\" class=\"num\" /></td></tr>\n",
		 i, sep != NULL ? sep : "");
    }
  free (originalpointer);
  return 0;
}


int
ej_show_control (int eid, webs_t wp, int argc, char_t ** argv)
{
#ifdef CONFIG_BRANDING
  websWrite (wp, "Control Panel");
#else
  websWrite (wp, "DD-WRT Control Panel");
#endif
  return 0;
}


#ifndef HAVE_AQOS
int
ej_show_default_level (int eid, webs_t wp, int argc, char_t ** argv)
{
  return 0;
}

#else
int
ej_show_default_level (int eid, webs_t wp, int argc, char_t ** argv)
{
  websWrite (wp, "<fieldset>\n");
  websWrite (wp, "<legend>Default Bandwith Level</legend>\n");
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp, "<div class=\"label\">Bandwith in Kbits</div>\n");
  websWrite (wp,
	     "<input type=\"num\" name=\"default_level\" size=\"6\" value=\"%s\" />\n",
	     nvram_safe_get ("default_level"));
  websWrite (wp, "</div>\n");
  websWrite (wp, "</fieldset><br />\n");
  return 0;
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

int
ej_show_security (int eid, webs_t wp, int argc, char_t ** argv)
{
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp, "<div class=\"label\">Security Mode</div>\n");
  websWrite (wp,
	     "<select name=\"security_mode\" onChange=SelMode(\"security_mode\",this.form.security_mode.selectedIndex,this.form)>\n");
  websWrite (wp, "<OPTION value=\"disabled\" %s>Disable</OPTION>\n",
	     selmatch ("security_mode", "disabled", "selected"));
  websWrite (wp, "<OPTION value=\"psk\" %s>WPA Pre-Shared Key</OPTION>\n",
	     selmatch ("security_mode", "psk", "selected"));
  websWrite (wp, "<OPTION value=\"wpa\" %s>WPA RADIUS</OPTION>\n",
	     selmatch ("security_mode", "wpa", "selected"));
  websWrite (wp,
	     "<OPTION value=\"psk2\" %s>WPA2 Pre-Shared Key Only</OPTION>\n",
	     selmatch ("security_mode", "psk2", "selected"));
  websWrite (wp, "<OPTION value=\"wpa2\" %s>WPA2 RADIUS Only</OPTION>\n",
	     selmatch ("security_mode", "wpa2", "selected"));
  websWrite (wp,
	     "<OPTION value=\"psk psk2\" %s>WPA2 Pre-Shared Key Mixed</OPTION>\n",
	     selmatch ("security_mode", "psk psk2", "selected"));
  websWrite (wp, "<OPTION value=\"wpa wpa2\" %s>WPA2 RADIUS Mixed</OPTION>\n",
	     selmatch ("security_mode", "wpa wpa2", "selected"));
  websWrite (wp, "<option value=\"radius\" %s>RADIUS</option>\n",
	     selmatch ("security_mode", "radius", "selected"));
  websWrite (wp, "<option value=\"wep\" %s>WEP</option></select>\n",
	     selmatch ("security_mode", "wep", "selected"));
  websWrite (wp, "</div>\n");
  ej_show_wpa_setting (eid, wp, argc, argv);
  return 0;
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
	     "<select name=\"%s_security_mode\" onChange=SelMode(\"%s_security_mode\",this.form.%s_security_mode.selectedIndex,this.form)>\n",
	     prefix, prefix, prefix);
  websWrite (wp, "<OPTION value=\"disabled\" %s>Disable</OPTION>\n",
	     selmatch (var, "disabled", "selected"));
  websWrite (wp, "<OPTION value=\"psk\" %s>WPA Pre-Shared Key</OPTION>\n",
	     selmatch (var, "psk", "selected"));
  websWrite (wp, "<OPTION value=\"wpa\" %s>WPA RADIUS</OPTION>\n",
	     selmatch (var, "wpa", "selected"));
  websWrite (wp,
	     "<OPTION value=\"psk2\" %s>WPA2 Pre-Shared Key Only</OPTION>\n",
	     selmatch (var, "psk2", "selected"));
  websWrite (wp, "<OPTION value=\"wpa2\" %s>WPA2 RADIUS Only</OPTION>\n",
	     selmatch (var, "wpa2", "selected"));
  websWrite (wp,
	     "<OPTION value=\"psk psk2\" %s>WPA2 Pre-Shared Key Mixed</OPTION>\n",
	     selmatch (var, "psk psk2", "selected"));
  websWrite (wp, "<OPTION value=\"wpa wpa2\" %s>WPA2 RADIUS Mixed</OPTION>\n",
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

static int
ej_show_security_single (int eid, webs_t wp, int argc, char_t ** argv,
			 char *prefix)
{
  char *next;
  char var[80];
  char ssid[80];
  char vif[16];
  sprintf (vif, "%s_vifs", prefix);
  char *vifs = nvram_safe_get (vif);
  if (vifs == NULL)
    return 0;
  int count = 1;
  sprintf (ssid, "%s_ssid", prefix);
  websWrite (wp, "<fieldset>\n");
  //cprintf("getting %s %s\n",ssid,nvram_safe_get(ssid));
  websWrite (wp, "<legend>Base Interface %d SSID [%s]</legend>\n", count,
	     nvram_get (ssid));
  show_security_prefix (eid, wp, argc, argv, prefix);
  websWrite (wp, "</fieldset>\n");
  foreach (var, vifs, next)
  {
    sprintf (ssid, "%s_ssid", var);
    websWrite (wp, "<fieldset>\n");
    //cprintf("getting %s %s\n",ssid,nvram_safe_get(ssid));
    websWrite (wp, "<legend>Virtual Interface %d SSID [%s]</legend>\n", count,
	       nvram_get (ssid));
    rep (var, '.', 'X');
    show_security_prefix (eid, wp, argc, argv, var);
    websWrite (wp, "</fieldset>\n");
    count++;
  }

}

int
ej_show_security (int eid, webs_t wp, int argc, char_t ** argv)
{
#ifndef HAVE_MADWIFI
  return ej_show_security_single (eid, wp, argc, argv, "wl0")
#else
  int c = getdevicecount ();
  int i;
  for (i = 0; i < c; i++)
    {
      char buf[16];
      sprintf (buf, "ath%d", i);
      ej_show_security_single (eid, wp, argc, argv, buf);
    }
  return 0;
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


int
ej_show_dhcpd_settings (int eid, webs_t wp, int argc, char_t ** argv)
{
  int i;
  if (nvram_match ("wl_mode", "wet"))	//dhcpd settings disabled in client bridge mode, so we wont display it
    return 0;
  websWrite (wp,
	     "<fieldset><legend>Network Address Server Settings (DHCP)</legend>\n");
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp, "<div class=\"label\">DHCP Type</div>\n");
  websWrite (wp,
	     "<select class=\"num\" size=\"1\" name=\"dhcpfwd_enable\" onchange=SelDHCPFWD(this.form.dhcpfwd_enable.selectedIndex,this.form)>\n");
  websWrite (wp, "<option value=\"0\" %s>DHCP Server</option>",
	     nvram_match ("dhcpfwd_enable", "0") ? "selected" : "");
  websWrite (wp, "<option value=\"1\" %s>DHCP Forwarder</option>",
	     nvram_match ("dhcpfwd_enable", "1") ? "selected" : "");
  websWrite (wp, "</select>\n");
  websWrite (wp, "</div>\n");
  if (nvram_match ("dhcpfwd_enable", "1"))
    {
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp, "<div class=\"label\">DHCP Server</div>\n");
      char *ipfwd = nvram_safe_get ("dhcpfwd_ip");
      websWrite (wp,
		 "<input type=\"hidden\" name=\"dhcpfwd_ip\" value=\"4\" /><input class=\"num\" maxlength=\"3\" size=\"3\" name=\"dhcpfwd_ip_0\" onblur=\"valid_range(this,0,255,'IP')\" value='%d' />.<input class=\"num\" maxlength=\"3\" size=\"3\" name=\"dhcpfwd_ip_1\" onblur=\"valid_range(this,0,255,'IP')\" value='%d' />.<input class=\"num\" maxlength=\"3\" name=\"dhcpfwd_ip_2\" size=\"3\" onblur=\"valid_range(this,0,255,'IP')\" value='%d' />.<input class=\"num\" maxlength=\"3\" name=\"dhcpfwd_ip_3\" size=\"3\" onblur=\"valid_range(this,0,254,'IP')\" value='%d' /></div>\n",
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
		 "<div class=\"label\">DHCP Server</div><input type=\"radio\" name=\"lan_proto\" value=\"dhcp\" onclick=SelDHCP('dhcp',this.form) %s>Enable</input>\n",
		 nvram_match ("lan_proto", "dhcp") ? "checked" : "");
      websWrite (wp,
		 "<input type=\"radio\" name=\"lan_proto\" value=\"static\" onclick=\"SelDHCP('static',this.form)\" %s>Disable</input></div><input type=\"hidden\" name=\"dhcp_check\" /><div class=\"setting\">\n",
		 nvram_match ("lan_proto", "static") ? "checked" : "");
      websWrite (wp, "<div class=\"label\">Starting IP Address</div>%s", buf);
      websWrite (wp,
		 "<input class=\"num\" name=\"dhcp_start\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,1,254,&#34;DHCP starting IP&#34;)\" value='%s' />",
		 nvram_safe_get ("dhcp_start"));
      websWrite (wp, "</div>\n");
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\">Maximum DHCP Users</div><input class=\"num\" name=\"dhcp_num\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,1,253,'Number of DHCP users')\" value='%s' /></div>\n",
		 nvram_safe_get ("dhcp_num"));
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\">Client Lease Time</div><input class=\"num\" name=\"dhcp_lease\" size=\"4\" maxlength=\"4\" onblur=\"valid_range(this,0,9999,'DHCP Lease Time')\" value='%s'> minutes</input></div>\n",
		 nvram_safe_get ("dhcp_lease"));
      if (nvram_invmatch ("wan_proto", "static"))
	{
	  websWrite (wp, "<div class=\"setting\">\n");
	  websWrite (wp, "<div class=\"label\">Static DNS 1</div>");
	  websWrite (wp,
		     "<input type=\"hidden\" name=\"wan_dns\" value=\"4\" />");
	  for (i = 0; i < 4; i++)
	    websWrite (wp,
		       "<input class=\"num\" name=\"wan_dns0_%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,%d,&#34;DNS&#34;)\" value='%d' />%s",
		       i, i == 3 ? 254 : 255, get_dns_ip ("wan_dns", 0, i),
		       i < 3 ? "." : "");

	  websWrite (wp, "\n</div>\n<div class=\"setting\">\n");
	  websWrite (wp, "<div class=\"label\">Static DNS 2</div>");
	  for (i = 0; i < 4; i++)
	    websWrite (wp,
		       "<input class=\"num\" name=\"wan_dns1_%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,%d,&#34;DNS&#34;)\" value='%d' />%s",
		       i, i == 3 ? 254 : 255, get_dns_ip ("wan_dns", 1, i),
		       i < 3 ? "." : "");

	  websWrite (wp, "\n</div>\n<div class=\"setting\">\n");
	  websWrite (wp, "<div class=\"label\">Static DNS 3</div>");
	  for (i = 0; i < 4; i++)
	    websWrite (wp,
		       "<input class=\"num\" name=\"wan_dns2_%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,%d,&#34;DNS&#34;)\" value='%d' />%s",
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
		     "<input class=\"num\" name=\"wan_wins_%d\" size=\"3\" maxlength=\"3\" onblur=\"valid_range(this,0,%d,&#34;WINS&#34;)\" value='%d' />%s",
		     i, i == 3 ? 254 : 255, get_single_ip (wins, i),
		     i < 3 ? "." : "");
	}

      websWrite (wp, "</div>\n<div class=\"setting\">\n");
      websWrite (wp, "<div class=\"label\">Use DNSMasq for DHCP</div>\n");
      websWrite (wp,
		 "<input type=\"checkbox\" name=\"_dhcp_dnsmasq\" value=\"1\" %s />\n",
		 nvram_match ("dhcp_dnsmasq", "1") ? "checked" : "");
      websWrite (wp, "</div>\n");
    }

  websWrite (wp, "</fieldset><br />\n");
  return 0;
}


static void
showOption (webs_t wp, char *propname, char *nvname)
{
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\">%s</div><select name=\"%s\">\n",
	     propname, nvname);
  websWrite (wp,
	     "<option value=\"0\" %s>Off</option>\n",
	     nvram_match (nvname, "0") ? "selected" : "");
  websWrite (wp,
	     "<option value=\"1\" %s>On</option></select>\n",
	     nvram_match (nvname, "1") ? "selected" : "");
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
					options[i]) ? "selected" : "");
    }
  websWrite (wp, "</div>\n");

}

#ifdef HAVE_MSSID
int
show_virtualssid (webs_t wp, char *prefix)
{
  char *next;
  char var[80];
  char ssid[80];
  char vif[16];
  sprintf (vif, "%s_vifs", prefix);
  char *vifs = nvram_safe_get (vif);
  if (vifs == NULL)
    return 0;
  int count = 1;
  websWrite (wp, "<fieldset>\n");
  websWrite (wp, "<h2>Virtual Interfaces</h2>\n");
  foreach (var, vifs, next)
  {
    sprintf (ssid, "%s_ssid", var);
    websWrite (wp, "<fieldset><legend>Interface %d</legend>\n", count);
    websWrite (wp, "<div class=\"setting\">\n");
    websWrite (wp,
	       "<div class=\"label\">Wireless Network Name (SSID)</div>\n");

    websWrite (wp,
	       "<input name=\"%s_ssid\" size=\"20\" maxLength=\"32\" onBlur=\"valid_name(this,'SSID')\" value=\"%s\" /></div>\n",
	       var, nvram_safe_get (ssid));
    websWrite (wp, "<div class=\"setting\">\n");
    websWrite (wp, "<div class=\"label\">Wireless SSID Broadcast</div>");
    sprintf (ssid, "%s_closed", var);
    websWrite (wp,
	       "<input type=\"radio\" value=\"0\" name=\"%s_closed\" %s>Enable</input>\n",
	       var, nvram_match (ssid, "0") ? "checked" : "");
    websWrite (wp,
	       "<input type=\"radio\" value=\"1\" name=\"%s_closed\" %s>Disable</input>\n",
	       var, nvram_match (ssid, "1") ? "checked" : "");
    websWrite (wp, "</div>\n");
//mode 
#ifdef HAVE_MADWIFI
    websWrite (wp, "<div class=\"setting\">\n");
    websWrite (wp,
	       "<div class=\"label\">Wireless Mode</div><select name=\"%s_mode\">\n",
	       var);
    sprintf (ssid, "%s_mode", var);

    websWrite (wp,
	       "<option value=\"ap\" %s>AP</option>\n",
	       nvram_match (ssid, "ap") ? "selected" : "");
    websWrite (wp,
	       "<option value=\"sta\" %s>Client</option></select>\n",
	       nvram_match (ssid, "sta") ? "selected" : "");
    websWrite (wp, "</div>\n");
#endif
    sprintf (ssid, "%s_ap_isolate", var);
    showOption (wp, "AP Isolation", ssid);
    websWrite (wp, "</fieldset>\n");
    count++;
  }
  if (count < WL_MAXBSSCFG)
    websWrite (wp,
	       "<input type=\"button\" value=\"Add\" onClick=vifs_add_submit(this.form,'%s') />\n",
	       prefix);
  if (count > 1)
    websWrite (wp,
	       "<input type=\"button\" value=\"Remove\" onClick=vifs_remove_submit(this.form,'%s') />\n",
	       prefix);
  websWrite (wp, "</fieldset>\n");

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
  char *cou[] = { "a", "b", "c", "d", "e", "f" };
  sprintf (v, "ath%s%d", cou[device], count + 1);
#else
  sprintf (v, "wl%d.%d", device, count + 1);
#endif
  if (strlen (vifs) == 0)
    sprintf (n, "%s", v);
  else
    sprintf (n, "%s %s", vifs, v);
  sprintf (v2, "%s_closed", v);
  nvram_set (v2, "0");
  sprintf (v2, "%s_ap_isolate", v);
  nvram_set (v2, "0");
  sprintf (v2, "%s_ssid", v);
  nvram_set (v2, "default");
  sprintf (v2, "%s_vifs", prefix);
  nvram_set (v2, n);
  //nvram_commit ();
  free (n);
  return 0;
}

int
add_vifs (webs_t wp)
{
  char *prefix = websGetVar (wp, "interface", NULL);
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
  int count = get_vifcount (prefix) - 1;
  int o = count;

  char *vifs = nvram_safe_get (wif);
  char *n = (char *) malloc (strlen (vifs));
  n[0] = 0;
  if (vifs == NULL)
    return 0;
  if (count)
    {
      foreach (var, vifs, next)
      {
	if (o == count)
	  strcat (n, var);
	else
	  {
	    strcat (n, " ");
	    strcat (n, var);
	  }
      }
    }
  nvram_set (wif, n);
  //nvram_commit ();
  free (n);
  return 0;
}

int
remove_vifs (webs_t wp)
{
  char *prefix = websGetVar (wp, "interface", NULL);
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
#ifdef HAVE_MADWIFI
  sprintf (n, "%s_regdomain", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_turbo", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_xchanmode", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_outdoor", prefix);
  copytonv (wp, n);
#endif
  sprintf (n, "%s_closed", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_ap_isolate", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_mode", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_net_mode", prefix);
  copytonv (wp, n);
  sprintf (n, "%s_channel", prefix);
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
  save_prefix (wp, "wl");
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
int ej_showad(int eid, webs_t wp, int argc,char_t **argv)
{
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
return 0;
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




int
ej_show_wireless_single (webs_t wp, char *prefix)
{
  char wl_mode[16];
  char wl_net_mode[16];
  sprintf (wl_mode, "%s_mode", prefix);
  sprintf (wl_net_mode, "%s_net_mode", prefix);

//wireless mode
  websWrite (wp, "<h2>Wireless Physical Interface %s</h2>\n", prefix);
  websWrite (wp, "<div>\n");
#ifdef HAVE_MADWIFI
  char wl_regdomain[16];
  sprintf (wl_regdomain, "%s_regdomain", prefix);


  websWrite (wp,
	     "<div class=\"setting\"><div class=\"label\">Regulatory Domain</div><select name=\"%s\">\n",
	     wl_regdomain);
  int domcount = 0;
  while (regdomains[domcount].name != NULL)
    {
      char domcode[16];
      sprintf (domcode, "%d", regdomains[domcount].code);
      websWrite (wp, "<option value=\"%d\" %s>%s</option>\n",
		 regdomains[domcount].code, nvram_match (wl_regdomain, domcode) ? "selected" : "",regdomains[domcount].name);
      domcount++;
    }
  websWrite (wp, "</select>\n");
  websWrite (wp, "</div>\n");
//power adjustment
  char power[16];
  sprintf(power,"%s_txpwr",prefix);
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp, "<div class=\"label\">TX Power</div><input class=\"num\" name=\"%s\" size=\"6\" maxLength=\"3\" value='%s'/> mW (Default: 28)\n",power,nvram_safe_get(power));
  websWrite (wp,"</div>\n");

  sprintf(power,"%s_sens",prefix);
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp, "<div class=\"label\">Sensitivity Range</div><input class=\"num\" name=\"%s\" size=\"6\" maxLength=\"3\" value='%s'/> Km (Default: 20)\n",power,nvram_safe_get(power));
  websWrite (wp,"</div>\n");

#endif



  websWrite (wp,
	     "<div class=\"setting\"><div class=\"label\">Wireless Mode</div><select name=\"%s\">\n",
	     wl_mode);
  websWrite (wp, "<option value=\"ap\" %s>AP</option>\n",
	     nvram_match (wl_mode, "ap") ? "selected" : "");
  websWrite (wp, "<option value=\"sta\" %s>Client</option>\n",
	     nvram_match (wl_mode, "sta") ? "selected" : "");
#ifndef HAVE_MADWIFI
  websWrite (wp, "<option value=\"wet\" %s>Client Bridge</option>\n",
	     nvram_match (wl_mode, "wet") ? "selected" : "");
  websWrite (wp, "<option value=\"infra\" %s>Adhoc</option>\n",
	     nvram_match (wl_mode, "infra") ? "selected" : "");
  websWrite (wp, "<option value=\"apsta\" %s>Repeater</option>\n",
	     nvram_match (wl_mode, "apsta") ? "selected" : "");
#endif
  websWrite (wp, "</select>\n");
  websWrite (wp, "</div>\n");
//writeless net mode

  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\">Wireless Network Mode</div><select name=\"%s\" onChange=\"SelWL(this.form.%s.selectedIndex,this.form)\">\n",
	     wl_net_mode, wl_net_mode);
  websWrite (wp, "<option value=\"disabled\" %s>Disabled</option>\n",
	     nvram_match (wl_net_mode, "disabled") ? "selected" : "");
  websWrite (wp, "<option value=\"mixed\" %s>Mixed</option>\n",
	     nvram_match (wl_net_mode, "mixed") ? "selected" : "");
  websWrite (wp, "<option value=\"b-only\" %s>B-Only</option>\n",
	     nvram_match (wl_net_mode, "b-only") ? "selected" : "");
  websWrite (wp, "<option value=\"g-only\" %s>G-Only</option>\n",
	     nvram_match (wl_net_mode, "g-only") ? "selected" : "");
#ifdef HAVE_MADWIFI
  websWrite (wp, "<option value=\"a-only\" %s>A-Only</option>\n",
	     nvram_match (wl_net_mode, "a-only") ? "selected" : "");
#endif
  websWrite (wp, "</select>\n");
  websWrite (wp, "</div>\n");
//turbo options
#ifdef HAVE_MADWIFI
  char wl_turbo[16];
  char wl_xchanmode[16];
  char wl_outdoor[16];
  sprintf (wl_turbo, "%s_turbo", prefix);
  sprintf (wl_xchanmode, "%s_xchanmode", prefix);
  sprintf (wl_outdoor, "%s_outdoor", prefix);
  showOption (wp, "Turbo Mode", wl_turbo);
  showOption (wp, "Extended Channel Mode", wl_xchanmode);
  showOption (wp, "Outdoor Band", wl_outdoor);
#endif

  char wl_ssid[16];
  sprintf (wl_ssid, "%s_ssid", prefix);
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp,
	     "<div class=\"label\">Wireless Network Name (SSID)</div><input name=\"%s\" size=\"20\" maxLength=\"32\" onBlur=\"valid_name(this,'SSID')\" value='%s' /></div>\n",
	     wl_ssid, nvram_safe_get (wl_ssid));

  char wl_channel[16];
  sprintf (wl_channel, "%s_channel", prefix);

  if (nvram_match (wl_mode, "ap") || nvram_match (wl_mode, "apsta"))
    {
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp,
		 "<div class=\"label\">Wireless Channel</div><select name=\"%s\" onFocus=\"check_action(this,0)\"><script language=\"javascript\">\n",
		 wl_channel);
#ifdef HAVE_MADWIFI
      struct wifi_channels *chan;
      char cn[32];
      chan = list_channels (prefix);
      //int cnt = getchannelcount ();
      websWrite (wp,
		 "document.write(\"<option value=0 %s>Auto</option>\");\n",
		 nvram_match (wl_channel, "0") ? "selected" : "");
      int i = 0;
      while (chan[i].freq != -1)
	{
	  cprintf("%d\n",chan[i].channel);
	  cprintf("%d\n",chan[i].freq);
	  
	  sprintf (cn, "%d", chan[i].channel);
	  websWrite (wp,
		     "document.write(\"<option value=%s %s>%s - %dMhz</option>\");\n",
		     cn, nvram_match (wl_channel, cn) ? "selected" : "",
		     cn, chan[i].freq);
	  //free (chan[i].freq);
	  i++;
	}
      free (chan);
#else
      websWrite (wp, "var max_channel = 14;\n");
      websWrite (wp, "var wl_channel = '%s';\n", nvram_safe_get (wl_channel));
      websWrite (wp, "var buf = \"\";");
      websWrite (wp,
		 "var freq = new Array(\"Auto\",\"2.412\",\"2.417\",\"2.422\",\"2.427\",\"2.432\",\"2.437\",\"2.442\",\"2.447\",\"2.452\",\"2.457\",\"2.462\",\"2.467\",\"2.472\",\"2.484\");\n");
      websWrite (wp, "	for(i=0; i<=max_channel ; i++){\n");
      websWrite (wp,
		 "		if(i == wl_channel)	buf = \"selected\";\n");
      websWrite (wp, "		else			buf = \"\";\n");
      websWrite (wp, "		if(i==0)\n");
      websWrite (wp,
		 "		 document.write(\"<option value=\"+i+\" \"+buf+\">Auto</option>\");\n");
      websWrite (wp, "		else\n");
      websWrite (wp,
		 "		 document.write(\"<option value=\"+i+\" \"+buf+\">\"+i+\" - \"+freq[i]+\"GHz</option>\");\n");
      websWrite (wp, "}\n");

#endif


      char wl_closed[16];
      sprintf (wl_closed, "%s_closed", prefix);
      websWrite (wp, "</script></select></div>\n");
      websWrite (wp, "<div class=\"setting\">\n");
      websWrite (wp, "<div class=\"label\">Wireless SSID Broadcast</div>\n");
      websWrite (wp,
		 "<input type=\"radio\" value=\"0\" name=\"%s\" %s>Enable</input>\n",
		 wl_closed, nvram_match (wl_closed, "0") ? "checked" : "");
      websWrite (wp,
		 "<input type=\"radio\" value=\"1\" name=\"%s\" %s>Disable</input>\n",
		 wl_closed, nvram_match (wl_closed, "1") ? "checked" : "");
      websWrite (wp, "</div>\n");
    }
  websWrite (wp, "</div>\n");
  websWrite (wp, "<br />\n");
  return show_virtualssid (wp, prefix);
}

int
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
  ej_show_wireless_single ("wl0");
#endif
  return 0;
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
	     selmatch (var, "tkip", "selected"));
  websWrite (wp, "<option value=\"aes\" %s>AES</option>\n",
	     selmatch (var, "aes", "selected"));
  websWrite (wp, "<option value=\"tkip+aes\" %s>TKIP+AES</option>\n",
	     selmatch (var, "tkip+aes", "selected"));
  websWrite (wp, "</select>\n");
  websWrite (wp, "</div>\n");
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp, "<div class=\"label\">WPA Shared Key</div>\n");
  sprintf (var, "%s_wpa_psk", prefix);
  websWrite (wp,
	     "<input name=\"%s_wpa_psk\" maxlength=\"64\" size=\"32\" value=\"%s\" />\n",
	     prefix, nvram_safe_get (var));
  websWrite (wp, "</div>\n");
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp, "<div class=\"label\">Group Key Renewal</div>\n");
  sprintf (var, "%s_wpa_gtk_rekey", prefix);
  websWrite (wp,
	     "<input name=\"%s_wpa_gtk_rekey\" maxlength=\"5\" size=\"10\" onblur=valid_range(this,0,99999,'rekey interval') value=\"%s\"/> seconds\n",
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
  websWrite (wp, "<div class=\"label\">RADIUS Server Address</div>\n");
  websWrite (wp,
	     "<input type=\"hidden\" name=\"%s_radius_ipaddr\" value=\"4\" />\n",
	     prefix);
  sprintf (var, "%s_radius_ipaddr", prefix);
  websWrite (wp,
	     "<input size=\"3\" maxlength=\"3\" name=\"%s_radius_ipaddr_0\" onblur=valid_range(this,0,255,'IP') class=\"num\" value=\"%d\" />.",
	     prefix, get_single_ip (var, 0));
  websWrite (wp,
	     "<input size=\"3\" maxlength=\"3\" name=\"%s_radius_ipaddr_1\" onblur=valid_range(this,0,255,'IP') class=\"num\" value=\"%d\" />.",
	     prefix, get_single_ip (var, 1));
  websWrite (wp,
	     "<input size=\"3\" maxlength=\"3\" name=\"%s_radius_ipaddr_2\" onblur=valid_range(this,0,255,'IP') class=\"num\" value=\"%d\" />.",
	     prefix, get_single_ip (var, 2));
  websWrite (wp,
	     "<input size=\"3\" maxlength=\"3\" name=\"%s_radius_ipaddr_3\" onblur=valid_range(this,1,254,'IP') class=\"num\" value=\"%d\" />\n",
	     prefix, get_single_ip (var, 3));
  websWrite (wp, "</div>\n");
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp, "<div class=\"label\">RADIUS Port</div>\n");
  sprintf (var, "%s_radius_port", prefix);
  websWrite (wp,
	     "<input name=\"%s_radius_port\" size=\"3\" maxlength=\"5\" onblur=valid_range(this,1,65535,'Port') value=\"%s\" /></div>\n",
	     prefix, nvram_safe_get (var));
  websWrite (wp, "<div class=\"setting\">\n");
  websWrite (wp, "<div class=\"label\">Shared Key</div>\n");
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
	     "<input name=\"%s_wpa_gtk_rekey\" maxlength=\"5\" size=\"10\" onblur=valid_range(this,0,99999,'rekey interval') value=\"%s\" />",
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
