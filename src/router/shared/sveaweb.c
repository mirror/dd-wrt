
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


void
validate_wds (webs_t wp, char *value, struct variable *v)
{
  int h, i, devcount = 3;	//changed from 2 to 3
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
	  snprintf (wds_if, 31, "wds0.4915%d", devcount++);
	  nvram_set (wdsif_var, wds_if);
	}
      else
	nvram_unset (wdsif_var);

    }

  nvram_set ("wl0_wds", wds_list);
}

int
ej_get_wds_mac (int eid, webs_t wp, int argc, char_t ** argv)
{
  int mac = -1, wds_idx = -1, mac_idx = -1, ret = 0;
  char *c, wds_var[32] = "";


  if (ejArgs (argc, argv, "%d %d", &wds_idx, &mac_idx) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return -1;
    }

  else if (wds_idx < 1 || wds_idx > MAX_WDS_DEVS)
    return -1;
  else if (mac_idx < 0 || mac_idx > 5)
    return -1;

  snprintf (wds_var, 31, "wl_wds%d_hwaddr", wds_idx);

  c = nvram_safe_get (wds_var);

  if (c)
    {
      mac = get_single_mac (c, mac_idx);
      ret += websWrite (wp, "%02X", mac);
    }
  else
    ret += websWrite (wp, "00");

  return ret;

}

int
ej_get_wds_ip (int eid, webs_t wp, int argc, char_t ** argv)
{
  int ip = -1, wds_idx = -1, ip_idx = -1, ret = 0;
  char *c, wds_var[32] = "";


  if (ejArgs (argc, argv, "%d %d", &wds_idx, &ip_idx) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return -1;
    }

  else if (wds_idx < 1 || wds_idx > MAX_WDS_DEVS)
    return -1;
  else if (ip_idx < 0 || ip_idx > 3)
    return -1;

  snprintf (wds_var, 31, "wl_wds%d_ipaddr", wds_idx);

  c = nvram_safe_get (wds_var);

  if (c)
    {
      ip = get_single_ip (c, ip_idx);
      ret += websWrite (wp, "%d", ip);
    }
  else
    ret += websWrite (wp, "0");

  return ret;

}

int
ej_get_wds_netmask (int eid, webs_t wp, int argc, char_t ** argv)
{
  int nm = -1, wds_idx = -1, nm_idx = -1, ret = 0;
  char *c, wds_var[32] = "";


  if (ejArgs (argc, argv, "%d %d", &wds_idx, &nm_idx) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return -1;
    }

  else if (wds_idx < 1 || wds_idx > 6)
    return -1;
  else if (nm_idx < 0 || nm_idx > 3)
    return -1;

  snprintf (wds_var, 31, "wl_wds%d_netmask", wds_idx);

  c = nvram_safe_get (wds_var);

  if (c)
    {
      nm = get_single_ip (c, nm_idx);
      ret += websWrite (wp, "%d", nm);
    }
  else
    ret += websWrite (wp, "255");

  return ret;

}


int
ej_get_wds_gw (int eid, webs_t wp, int argc, char_t ** argv)
{
  int gw = -1, wds_idx = -1, gw_idx = -1, ret = 0;
  char *c, wds_var[32] = "";


  if (ejArgs (argc, argv, "%d %d", &wds_idx, &gw_idx) < 2)
    {
      websError (wp, 400, "Insufficient args\n");
      return -1;
    }

  else if (wds_idx < 1 || wds_idx > MAX_WDS_DEVS)
    return -1;
  else if (gw_idx < 0 || gw_idx > 3)
    return -1;

  snprintf (wds_var, 31, "wl_wds%d_gw", wds_idx);

  c = nvram_safe_get (wds_var);

  if (c)
    {
      gw = get_single_ip (c, gw_idx);
      ret += websWrite (wp, "%d", gw);
    }
  else
    ret += websWrite (wp, "0");

  return ret;

}

int
ej_get_br1_ip (int eid, webs_t wp, int argc, char_t ** argv)
{
  int ip = -1, ip_idx = -1, ret = 0;
  char *c;


  if (ejArgs (argc, argv, "%d", &ip_idx) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return -1;
    }
  else if (ip_idx < 0 || ip_idx > 3)
    return -1;

  c = nvram_safe_get ("wl_br1_ipaddr");

  if (c)
    {
      ip = get_single_ip (c, ip_idx);
      ret += websWrite (wp, "%d", ip);
    }
  else
    ret += websWrite (wp, "0");

  return ret;

}

int
ej_get_br1_netmask (int eid, webs_t wp, int argc, char_t ** argv)
{
  int nm = -1, nm_idx = -1, ret = 0;
  char *c;


  if (ejArgs (argc, argv, "%d", &nm_idx) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return -1;
    }
  else if (nm_idx < 0 || nm_idx > 3)
    return -1;

  c = nvram_safe_get ("wl_br1_netmask");

  if (c)
    {
      nm = get_single_ip (c, nm_idx);
      ret += websWrite (wp, "%d", nm);
    }
  else
    ret += websWrite (wp, "255");

  return ret;

}

int
ej_get_currate (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *dev = NULL;
  int ret = 0, rate = 0;

  if (wl_probe ("eth2"))
    dev = "eth1";
  else
    dev = "eth2";

  wl_ioctl (dev, WLC_GET_RATE, &rate, sizeof (rate));

  if (rate > 0)
    {
      ret += websWrite (wp, "%d", rate / 2);
    }
  else
    ret += websWrite (wp, "unknown");

  return ret;

}

#define UPTIME_TMP	"/tmp/.uptime"
int
ej_get_uptime (int eid, webs_t wp, int argc, char_t ** argv)
{
  char uptime[200] = { 0 }, cmd[200] =
  {
  0};
  FILE *fp;
  int ret = -1;
  unlink (UPTIME_TMP);

  snprintf (cmd, 254, "uptime 2>&1 > %s", UPTIME_TMP);
  system (cmd);

  if ((fp = fopen (UPTIME_TMP, "r")) != NULL)
    fgets (uptime, sizeof (uptime), fp);
  else
    return -1;

  ret = websWrite (wp, "%s", uptime);

  fclose (fp);

  unlink (UPTIME_TMP);

  return ret;

}

int
ej_get_curchannel (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *dev = NULL;
  int ret = 0;
  channel_info_t ci;

  if (wl_probe ("eth2"))
    dev = "eth1";
  else
    dev = "eth2";

  ci.target_channel = 0;
  wl_ioctl (dev, WLC_GET_CHANNEL, &ci, sizeof (ci));
  if (ci.target_channel > 0)
    {
      ret += websWrite (wp, "%d", ci.target_channel);
    }
  else
    ret += websWrite (wp, "0");

  return ret;

}

#define ASSOCLIST_TMP	"/tmp/.wl_assoclist"
#define RSSI_TMP	"/tmp/.rssi"
#define ASSOCLIST_CMD	"wl assoclist"
#define RSSI_CMD	"wl rssi"
#define NOISE_CMD	"wl noise"

int
ej_active_wireless (int eid, webs_t wp, int argc, char_t ** argv)
{
  int rssi = 0, noise = 0;
  FILE *fp, *fp2;
  char *mode;
  char mac[30];
  char list[2][30];
  char line[80];
  char cmd[80];
  char title[20];
  int macmask;
  if (ejArgs (argc, argv, "%d", &macmask) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return -1;
    }

  unlink (ASSOCLIST_TMP);
  unlink (RSSI_TMP);
  int cnt = 0;
  mode = nvram_safe_get ("wl_mode");
  snprintf (cmd, sizeof (cmd), "%s > %s", ASSOCLIST_CMD, ASSOCLIST_TMP);
  system (cmd);			// get active wireless mac

  if (strcmp (mode, "ap") != 0)
    strcpy (title, "AP Signal");
  else
    strcpy (title, "Clients");

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
	  if (!cnt)
	    {
	      cnt++;
	      websWrite (wp, "<div class=\"setting\">\n");
	      websWrite (wp, "<h3>%s</h3>\n", title);
	      websWrite (wp, "<table class=\"table\">\n");
	      websWrite (wp, "<tr>\n");
	      websWrite (wp, "<th>MAC</th>\n");
	      websWrite (wp, "<th>Signal</th>\n");
	      websWrite (wp, "<th>Noise</th>\n");
	      websWrite (wp, "<th>SNR</th>\n");
	      websWrite (wp, "</tr>\n");
	    }
	  websWrite (wp, "<tr>\n");
	  if (strcmp (mode, "ap") != 0)
	    {
	      websWrite (wp, "<td>%s</td><td>%d</td><td>%d</td><td>%d</td>\n",
			 mac, rssi, noise, rssi - noise);
	    }
	  else
	    {
	      websWrite (wp, "<td>%s</td><td>%d</td><td>%d</td><td>%d</td>\n",
			 mac, rssi, -100, rssi - (-100));
	    }
	  websWrite (wp, "</tr>\n");
	}
      // One less Top10-Wanted leak (belanger[AT]pobox.com)
      fclose (fp);
    }
  if (cnt)
    websWrite (wp, "</table></div>\n");

  unlink (ASSOCLIST_TMP);
  unlink (RSSI_TMP);

  return 0;
}

#define WDS_LIST_TMP	"/tmp/.wl_wdslist"
#define WDS_RSSI_TMP	"/tmp/.rssi"
#define WDS_CMD	"wl wds"

int
ej_active_wds (int eid, webs_t wp, int argc, char_t ** argv)
{
  int rssi = 0, i;
  FILE *fp, *fp2;
  char *mode;
  char mac[30];
  char list[2][30];
  char line[80];
  char cmd[80];
  char title[30];
  char wdsvar[30];
  char desc[12];
  int cnt = 0;
  int macmask;
  if (ejArgs (argc, argv, "%d", &macmask) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return -1;
    }

  unlink (WDS_LIST_TMP);
  unlink (WDS_RSSI_TMP);

  mode = nvram_safe_get ("wl_mode");
  snprintf (cmd, sizeof (cmd), "%s > %s", WDS_CMD, WDS_LIST_TMP);
  system (cmd);			// get active wireless mac

  if (strcmp (mode, "ap") == 0)
    strcpy (title, "WDS Signal");
  else
    return -1;


  if ((fp = fopen (WDS_LIST_TMP, "r")))
    {
      while (fgets (line, sizeof (line), fp) != NULL)
	{
	  if (sscanf (line, "%s %s", list[0], mac) != 2)	// "XX:XX:XX:XX:XX:XX XX:XX:XX:XX:XX:XX" etc
	    continue;

	  if (strcmp (list[0], "wds"))
	    break;

	  rssi = 0;

	  for (i = 0; i < 10; i++)
	    {
	      snprintf (wdsvar, 30, "wl_wds%d_hwaddr", i);
	      if (nvram_match (wdsvar, mac))
		{
		  snprintf (wdsvar, 30, "wl_wds%d_desc", i);
		  snprintf (desc, sizeof (desc), "%s", nvram_get (wdsvar));
		  snprintf (title, sizeof (title), "WDS Signal (%s):", desc);
		  if (!strcmp (nvram_get (wdsvar), ""))
		    strcpy (title, "WDS Signal:");
		}
	    }

	  snprintf (cmd, sizeof (cmd), "%s \"%s\" > %s", RSSI_CMD, mac,
		    RSSI_TMP);
	  system (cmd);

	  fp2 = fopen (RSSI_TMP, "r");
	  if (fgets (line, sizeof (line), fp2) != NULL)
	    {

	      // get rssi
	      if (sscanf (line, "%s %s %d", list[0], list[1], &rssi) != 3)
		continue;

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
	  if (!cnt)
	    {
	      cnt++;
	      websWrite (wp, "<div class=\"setting\">\n");
	      websWrite (wp, "<h3>WDS Nodes</h3>\n");
	      websWrite (wp, "<table class=\"table\">\n");
	      websWrite (wp, "<tr>\n");
	      websWrite (wp, "<th>MAC</th>\n");
	      websWrite (wp, "<th>Signal</th>\n");
	      websWrite (wp, "<th>Noise</th>\n");
	      websWrite (wp, "<th>SNR</th>\n");
	      websWrite (wp, "</tr>\n");
	    }

	  websWrite (wp,
		     "<tr><td>%s %s</td><td>%d</td><td>%d</td><td>%d</td></tr>\n",
		     title, mac, rssi, -100, rssi - (-100));
	}
      // One less Top10-Wanted leak (belanger[AT]pobox.com)
      fclose (fp);
    }
  if (cnt)
    websWrite (wp, "</table></div>\n");

  unlink (WDS_LIST_TMP);
  unlink (WDS_RSSI_TMP);

  return 0;
}

int
ej_get_wdsp2p (int eid, webs_t wp, int argc, char_t ** argv)
{
  int index = -1, ip[4] = { 0, 0, 0, 0 }, netmask[4] =
  {
  0, 0, 0, 0};
  char nvramvar[32] = { 0 };

  if (ejArgs (argc, argv, "%d", &index) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return -1;
    }
  if (nvram_selmatch (wp, "wk_mode", "ospf") &&
      nvram_selmatch (wp, "expert_mode", "1") &&
      nvram_selmatch (wp, "wl_wds1_enable", "1"))
    {
      char buf[16];
      sprintf (buf, "wl_wds%d_ospf", index);
      websWrite (wp,
		 "<input name=\"%s\" size=\"2\" maxlength=\"5\" value='%s' />\n",
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
#ifdef KROMOGUI
      websWrite (wp, "\
	<div class=\"setting\">\n\
	          <input type=hidden name=wl_wds%d_ipaddr value=4>\n\
	          <input size=3 maxlength=3 name=wl_wds%d_ipaddr0 value=\"%d\" onBlur=valid_range(this,0,255,\"IP\") class=num>\n\
                  <input size=3 maxlength=3 name=wl_wds%d_ipaddr1 value=\"%d\" onBlur=valid_range(this,0,255,\"IP\") class=num>\n\
                  <input size=3 maxlength=3 name=wl_wds%d_ipaddr2 value=\"%d\" onBlur=valid_range(this,0,255,\"IP\") class=num>\n\
                  <input size=3 maxlength=3 name=wl_wds%d_ipaddr3 value=\"%d\" onBlur=valid_range(this,1,254,\"IP\") class=num>\n\
       </div>\n", index, index, ip[0], index, ip[1], index, ip[2], index, ip[3], index);

      websWrite (wp, "\
       	  <div class=\"setting\">\n\
	  <input type=\"hidden\" name=\"wl_wds%d_netmask\" value=\"4\">\n\
	  <input name=\"wl_wds%d_netmask0\" value=\"%d\" size=\"3\" maxlength=\"3\" onBlur=valid_range(this,0,255,\"IP\") class=num>\n\
	  <input name=\"wl_wds%d_netmask1\" value=\"%d\" size=\"3\" maxlength=\"3\" onBlur=valid_range(this,0,255,\"IP\") class=num>\n\
	  <input name=\"wl_wds%d_netmask2\" value=\"%d\" size=\"3\" maxlength=\"3\" onBlur=valid_range(this,0,255,\"IP\") class=num>\n\
	  <input name=\"wl_wds%d_netmask3\" value=\"%d\" size=\"3\" maxlength=\"3\" onBlur=valid_range(this,0,255,\"IP\") class=num>\n\
          </div>\n", index, index, netmask[0], index, netmask[1], index, netmask[2], index, netmask[3]);


#else

      websWrite (wp, "\
	<tr>\n\
          <td width=156 bgColor=#e7e7e7 colSpan=3 height=25>&nbsp;</td>\n\
          <td width=8 background=image/UI_04.gif height=25>&nbsp;</td>\n\
          <td colSpan=3 height=25>&nbsp;</td>\n\
          <td width=101 height=25>&nbsp;IP Address:</td>\n\
          <td width=296 height=25>&nbsp;\n\
	          <input type=hidden name=wl_wds%d_ipaddr value=4>\n\
	          <input size=3 maxlength=3 name=wl_wds%d_ipaddr0 value=\"%d\" onBlur=valid_range(this,0,255,\"IP\") class=num>\n\
                  <input size=3 maxlength=3 name=wl_wds%d_ipaddr1 value=\"%d\" onBlur=valid_range(this,0,255,\"IP\") class=num>\n\
                  <input size=3 maxlength=3 name=wl_wds%d_ipaddr2 value=\"%d\" onBlur=valid_range(this,0,255,\"IP\") class=num>\n\
                  <input size=3 maxlength=3 name=wl_wds%d_ipaddr3 value=\"%d\" onBlur=valid_range(this,1,254,\"IP\") class=num></td>\n\
          <td width=13 height=25>&nbsp;</td>\n\
          <td width=15 background=image/UI_05.gif height=25>&nbsp;</td>\n\
       </tr>\n", index, index, ip[0], index, ip[1], index, ip[2], index, ip[3], index);

      websWrite (wp, "\
        <TR>\n\
          <TD width=156 bgColor=#e7e7e7 colSpan=3 height=25>&nbsp;</TD>\n\
          <TD width=8 background=image/UI_04.gif height=24>&nbsp;</TD>\n\
          <TD colSpan=3 height=25>&nbsp;</TD>\n\
          <TD width=101 height=24><FONT style=\"FONT-SIZE: 8pt\"><SPAN>Subnet Mask:</SPAN></FONT></TD>\n\
          <TD width=296 height=24>&nbsp;\n\
	  <input type=\"hidden\" name=\"wl_wds%d_netmask\" value=\"4\">\n\
	  <input name=\"wl_wds%d_netmask0\" value=\"%d\" size=\"3\" maxlength=\"3\" onBlur=valid_range(this,0,255,\"IP\") class=num>\n\
	  <input name=\"wl_wds%d_netmask1\" value=\"%d\" size=\"3\" maxlength=\"3\" onBlur=valid_range(this,0,255,\"IP\") class=num>\n\
	  <input name=\"wl_wds%d_netmask2\" value=\"%d\" size=\"3\" maxlength=\"3\" onBlur=valid_range(this,0,255,\"IP\") class=num>\n\
	  <input name=\"wl_wds%d_netmask3\" value=\"%d\" size=\"3\" maxlength=\"3\" onBlur=valid_range(this,0,255,\"IP\") class=num>\n\
	  </TD>\n\
          <TD width=13 height=24>&nbsp;</TD>\n\
          <TD width=15 background=image/UI_05.gif height=24>&nbsp;</TD>\n\
       </TR>\n", index, index, netmask[0], index, netmask[1], index, netmask[2], index, netmask[3]);

      websWrite (wp, "\
        <TR>\n\
          <TD width=156 bgColor=#e7e7e7 colSpan=3 height=25>&nbsp;</TD>\n\
          <TD width=8 background=image/UI_04.gif height=24>&nbsp;</TD>\n\
          <TD colSpan=3 height=25>&nbsp;</TD>\n\
          <TD width=101 height=24>&nbsp;</TD>\n\
          <TD width=296 height=24><HR>\n</TD>\n\
          <TD width=13 height=24>&nbsp;</TD>\n\
          <TD width=15 background=image/UI_05.gif height=24>&nbsp;</TD>\n\
       </TR>");
#endif
    }

  return 0;

}

int
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

  return 0;
}

#ifndef KROMOGUI
int
ej_get_qossvcs (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *qos_svcs = nvram_safe_get ("svqos_svcs");
  char name[32], type[32], data[32], level[32];
  int no_svcs = 0, i = 0, ret = -1;


  // calc # of services
//      no_svcs = strspn(qos_svcs,"|");

  while ((qos_svcs = strpbrk (qos_svcs, "|")))
    {
      no_svcs++;
      qos_svcs++;
    }

  // write HTML data

  websWrite (wp, "<input type=hidden name='svqos_nosvcs' value='%d'>",
	     no_svcs);

  qos_svcs = nvram_safe_get ("svqos_svcs");

  /* services format is "name type data level | name type data level |" ..etc */
  for (i = 0; i < no_svcs && qos_svcs && qos_svcs[0]; i++)
    {
      if (sscanf (qos_svcs, "%31s %31s %31s %31s ", name, type, data, level) <
	  4)
	break;

      ret = websWrite (wp, "<TR>\n\
			   <TD align=right bgColor=#e7e7e7 height=5></TD>\n\
			   <TD width=8 background=image/UI_04.gif height=5></TD>\n\
			   <TD height=5></TD>\n\
			   <TD colSpan=2>\n\
		           <TABLE>\n\
			   <TR>\n\
			   <TD width=100 height=5><input type=checkbox name=\"svqos_svcdel%d\"></TD>\n\
			   <TD width=297 height=25>\n\
		           <TABLE>\n\
		           <TR>\n\
			   <CENTER>\n\
			   <input type=hidden name=\"svqos_svcname%d\" value=\"%s\">\n\
			   <input type=hidden name=\"svqos_svctype%d\" value=\"%s\">\n\
			   <TD align=middle width=80><B>%s</B></TD>\n\
			   <TD align=middle width=80>\n\
			   <SELECT name=\"svqos_svcprio%d\"> \n\
			   <option value=\"10\" %s>Premium</option>\n\
		           <option value=\"20\" %s>Express</option>\n\
			   <option value=\"30\" %s>Standard</option>\n\
		           <option value=\"40\" %s>Bulk</option>\n\
			   </SELECT>\n\
		           </TD>\n\
			   <TD align=middle width=80><B>&nbsp;</B></TD>\n\
			   </CENTER>\n\
			   </TR>\n\
			   </TABLE>\n\
			   </TD>\n\
			   </TR>\n\
		           </TABLE>\n\
			   </TD>\n\
			   </TR>\n", i, i, name, i, type, name, i, strcmp (level, "10") == 0 ? "selected" : "", strcmp (level, "20") == 0 ? "selected" : "", strcmp (level, "30") == 0 ? "selected" : "", strcmp (level, "40") == 0 ? "selected" : "");

      qos_svcs = strpbrk (++qos_svcs, "|");
      qos_svcs++;

    }

  return ret;
}

int
ej_get_qosips (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *qos_ips = nvram_safe_get ("svqos_ips");
  char ip[32], level[32];
  int no_ips = 0, i = 0, ret = -1;

  // calc # of ips
  while ((qos_ips = strpbrk (qos_ips, "|")))
    {
      no_ips++;
      qos_ips++;
    }

  // write HTML data

  websWrite (wp, "<input type=hidden name='svqos_noips' value='%d'>", no_ips);

  qos_ips = nvram_safe_get ("svqos_ips");

  /* IP format is "data level | data level |" ..etc */
  for (i = 0; i < no_ips && qos_ips && qos_ips[0]; i++)
    {
      if (sscanf (qos_ips, "%31s %31s ", ip, level) < 2)
	break;

      ret = websWrite (wp, "<TR>\n\
			   <TD align=right bgColor=#e7e7e7 height=5></TD>\n\
			   <TD width=8 background=image/UI_04.gif height=5></TD>\n\
			   <TD height=5></TD>\n\
			   <TD colSpan=2>\n\
		           <TABLE>\n\
			   <TR>\n\
			   <TD width=90 height=5><input type=checkbox name=\"svqos_ipdel%d\"></TD>\n\
			   <input type=hidden name=\"svqos_ip%d\" value=\"%s\">\n\
			   <TD width=297 height=25>\n\
		           <TABLE>\n\
		           <TR>\n\
			   <CENTER>\n\
			   <TD align=middle width=90><B>%s</B></TD>\n\
			   <TD align=middle width=80>\n\
			   <SELECT name=\"svqos_ipprio%d\"> \n\
			   <option value=\"10\" %s>Premium</option>\n\
		           <option value=\"20\" %s>Express</option>\n\
			   <option value=\"30\" %s>Standard</option>\n\
		           <option value=\"40\" %s>Bulk</option>\n\
			   </SELECT>\n\
		           </TD>\n\
			   <TD align=middle width=80><B>&nbsp;</B></TD>\n\
			   </CENTER>\n\
			   </TR>\n\
			   </TABLE>\n\
			   </TD>\n\
			   </TR>\n\
		           </TABLE>\n\
			   </TD>\n\
			   </TR>\n", i, i, ip, ip, i, strcmp (level, "10") == 0 ? "selected" : "", strcmp (level, "20") == 0 ? "selected" : "", strcmp (level, "30") == 0 ? "selected" : "", strcmp (level, "40") == 0 ? "selected" : "");

      qos_ips = strpbrk (++qos_ips, "|");
      qos_ips++;

    }

  return ret;
}


int
ej_get_qosmacs (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *qos_macs = nvram_safe_get ("svqos_macs");
  char mac[32], level[32];
  int no_macs = 0, i = 0, ret = -1;


  // calc # of ips
  while ((qos_macs = strpbrk (qos_macs, "|")))
    {
      no_macs++;
      qos_macs++;
    }

  // write HTML data
  websWrite (wp, "<input type=hidden name='svqos_nomacs' value='%d'>",
	     no_macs);

  qos_macs = nvram_safe_get ("svqos_macs");

  /* IP format is "data level | data level |" ..etc */
  for (i = 0; i < no_macs && qos_macs && qos_macs[0]; i++)
    {
      if (sscanf (qos_macs, "%31s %31s ", mac, level) < 2)
	break;

      ret = websWrite (wp, "<TR>\n\
			   <TD align=right bgColor=#e7e7e7 height=5></TD>\n\
			   <TD width=8 background=image/UI_04.gif height=5></TD>\n\
			   <TD height=5></TD>\n\
			   <TD colSpan=2>\n\
		           <TABLE>\n\
			   <TR>\n\
			   <TD width=100 height=5><input type=checkbox name=\"svqos_macdel%d\"></TD>\n\
			   <input type=hidden name=\"svqos_mac%d\" value=\"%s\">\n\
			   <TD width=297 height=25>\n\
		           <TABLE>\n\
		           <TR>\n\
			   <CENTER>\n\
			   <TD align=middle width=80><B>%s</B></TD>\n\
			   <TD align=middle width=80>\n\
			   <SELECT name=\"svqos_macprio%d\"> \n\
			   <option value=\"10\" %s>Premium</option>\n\
		           <option value=\"20\" %s>Express</option>\n\
			   <option value=\"30\" %s>Standard</option>\n\
		           <option value=\"40\" %s>Bulk</option>\n\
			   </SELECT>\n\
		           </TD>\n\
			   <TD align=middle width=80><B>&nbsp;</B></TD>\n\
			   </CENTER>\n\
			   </TR>\n\
			   </TABLE>\n\
			   </TD>\n\
			   </TR>\n\
		           </TABLE>\n\
			   </TD>\n\
			   </TR>\n", i, i, mac, mac, i, strcmp (level, "10") == 0 ? "selected" : "", strcmp (level, "20") == 0 ? "selected" : "", strcmp (level, "30") == 0 ? "selected" : "", strcmp (level, "40") == 0 ? "selected" : "");

      qos_macs = strpbrk (++qos_macs, "|");
      qos_macs++;

    }

  return ret;
}
#endif

char *
get_filter_services (void)
{
  static char services[8192] = "", svcs_var[32] = "filter_services0";
  int index = 1;

  while (strlen (nvram_safe_get (svcs_var)) > 0 && index < 8)
    {
      strcat (services, nvram_safe_get (svcs_var));
      snprintf (svcs_var, 31, "filter_services%d", index);
      index++;


    }

  return services;
}

int
ej_get_services_options (int eid, webs_t wp, int argc, char_t ** argv)
{
  int ret = 0;
  char word[1024], *next, *services;
  char delim[] = "<&nbsp;>";

//      services = nvram_safe_get("filter_services");
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

    ret = websWrite (wp, "<option value=\"%s\">%s</option>", name, name);

  }

  return ret;
}

int
get_svc (char *svc, char *protocol, char *ports)
{
  char word[1024], *next, *services;
  char delim[] = "<&nbsp;>";

//      services = nvram_safe_get("filter_services");
  services = get_filter_services ();
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

    if (strcasecmp (svc, name))
      continue;

    /* $PROT */
    if (sscanf (prot, "$PROT:%3d:", &len) != 1)
      return -1;

    strncpy (protocol, prot + sizeof ("$PROT:nnn:") - 1, len);
    protocol[len] = '\0';

    /* $PORT */
    if (sscanf (port, "$PORT:%3d:", &len) != 1)
      return -1;

    strncpy (ports, port + sizeof ("$PORT:nnn:") - 1, len);
    ports[len] = '\0';

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

  memset (new_svcs, 0, sizeof (new_svcs));

  if (get_svc (add_svc, protocol, ports))
    return -1;

  if (strcmp (protocol, "l7") == 0)
    {
      for (i = 0; i < strlen (add_svc); i++)
	add_svc[i] = tolower (add_svc[i]);
    }

  /* if this service exists, return an error */
  if (strstr (svqos_svcs, add_svc))
    return -1;

  if (strlen (svqos_svcs) > 0)
    snprintf (new_svcs, 4095, "%s %s %s %s 30 |", svqos_svcs, add_svc,
	      protocol, ports);
  else
    snprintf (new_svcs, 4095, "%s %s %s 30 |", add_svc, protocol, ports);

  if (strlen (new_svcs) >= sizeof (new_svcs))
    return -1;

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

  snprintf (add_ip, 18, "%s.%s.%s.%s/%s", add_ip0, add_ip1, add_ip2, add_ip3,
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

int
ej_get_clone_wmac (int eid, webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_RB500
  return 0;
#else

  int ret = 0;
  char *c;
  int mac, which;
  int dofree = 0;
  if (ejArgs (argc, argv, "%d", &which) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return -1;
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
      ret += websWrite (wp, "%02X", mac);
      if (dofree)
	free (c);
    }
  else
    ret += websWrite (wp, "00");

  return ret;
#endif
}



/* todo stylesheet compatible code */
/* lonewolf additions */

// Note that there is no VLAN #16.  It's just a convieniant way of denoting a "Tagged" port
int
ej_port_vlan_table (int eid, webs_t wp, int argc, char_t ** argv)
{
  /*
     vlans[x][y] where
     x 0-15 are VLANS
     x 16 is tagging, 17 is auto-negotiation, 18 is 100/10 Mbit, and 19 is Full/Half duplex
     y 0-4 are switch ports (port 5 is set automaticly)
     y 5 it the bridge device (x 16 dosn't apply)
   */

  int ret = 0, i, j, vlans[20][6], tmp, wl_br;
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
      ret += websWrite (wp, "              <TR>\n");
      ret +=
	websWrite (wp,
		   "			<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF height=30><B>");

      switch (i)
	{
	case 16:
	  ret += websWrite (wp, "Tagged");
	  break;
	case 17:
	  ret += websWrite (wp, "Auto-Negotiate");
	  break;
	case 18:
	  ret += websWrite (wp, "100 Mbit");
	  break;
	case 19:
	  ret += websWrite (wp, "Full-Duplex");
	  break;
	default:
	  snprintf (buff, 31, "%d", i);
	  ret += websWrite (wp, buff);
	  break;
	}

      ret += websWrite (wp, "</B></TD>\n");

      for (j = 0; j < 5; j++)
	{
	  snprintf (buff, 31, "port%dvlan%d", j, i);
	  ret +=
	    websWrite (wp,
		       "			<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#");

	  if (j % 2 == 0)
	    ret += websWrite (wp, "CCCCCC");
	  else
	    ret += websWrite (wp, "FFFFFF");

	  ret +=
	    websWrite (wp,
		       " height=30><B><INPUT type=\"checkbox\" value=\"on\" name=");
	  ret += websWrite (wp, buff);

	  if (i < 17 || i > 19)
	    {
	      if (vlans[i][j] == 1)
		ret += websWrite (wp, " checked");
	    }
	  else
	    {
	      if (vlans[i][j] == -1)
		ret += websWrite (wp, " checked");
	    }

	  ret += websWrite (wp, " onClick=");
	  if (i < 17)
	    snprintf (buff, sizeof (buff),
		      "\"SelVLAN(this.form,\"port%d\")\"", j);
	  else if (i == 17)
	    snprintf (buff, sizeof (buff),
		      "\"SelSpeed(this.form,\"port%d\")\"", j);

	  ret += websWrite (wp, buff);
	  ret += websWrite (wp, "></B></TD>\n");
	}

      if (i < 16)
	{
	  ret +=
	    websWrite (wp,
		       "			<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF height=30><select name=");
	  snprintf (buff, 31, "vlan%d", i);
	  ret += websWrite (wp, buff);
	  ret += websWrite (wp, "><option value=\"-1\"");
	  if (vlans[i][5] < 0)
	    ret += websWrite (wp, " selected");
	  ret += websWrite (wp, ">None</option><option value=\"0\"");
	  if (vlans[i][5] == 0)
	    ret += websWrite (wp, " selected");
	  ret += websWrite (wp, ">LAN</option></select></TD>\n");
	}
      else
	{
	  ret += websWrite (wp, "                  <TD>&nbsp;</TD>\n");
	}

      ret += websWrite (wp, "              </TR>\n");

      if (i == 16 || i == 19)
	{
	  ret +=
	    websWrite (wp,
		       "              <TR height=25><TD>&nbsp;</TD></TR>\n");
	}
    }

  ret += websWrite (wp, "              <TR>\n");
  ret +=
    websWrite (wp,
	       "			<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF colspan=6><B>WIRELESS</B></TD>\n");
  ret +=
    websWrite (wp,
	       "			<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF><select name=wireless><option value=\"-1\"");
  if (wl_br < 0)
    ret += websWrite (wp, " selected");
  ret += websWrite (wp, ">None</option><option value=\"0\"");
  if (wl_br == 0)
    ret += websWrite (wp, " selected");
  ret += websWrite (wp, ">LAN</option></select></TD>\n");
  ret += websWrite (wp, "              </TR>\n");

  ret += websWrite (wp, "              <TR height=50>\n");
  ret += websWrite (wp, "                  <TD>&nbsp;</TD>\n");
  ret += websWrite (wp, "              </TR>\n");

  ret += websWrite (wp, "              <TR>\n");
  ret +=
    websWrite (wp,
	       "                  <TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF colspan=4><B>Link Aggregation<br>on Ports 3 & 4</B></TD>\n");
  ret +=
    websWrite (wp,
	       "                  <TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF colspan=3 align=left><select name=trunking><option value=0>No</option><option value=1");

  c = nvram_safe_get ("trunking");

  snprintf (buff, 5, "%s", c);

  if (atoi (buff) == 1)
    ret += websWrite (wp, " selected");

  ret += websWrite (wp, ">Trunk</option></select></TD>\n");
  ret += websWrite (wp, "              </TR>");

  return ret;
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
      strcat (br0vlans, "eth1");
      break;
    case 1:
      if (strlen (br1vlans) > 0)
	strcat (br1vlans, " ");
      strcat (br1vlans, "eth1");
      break;
    case 2:
      if (strlen (br2vlans) > 0)
	strcat (br2vlans, " ");
      strcat (br2vlans, "eth1");
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

		snprintf(buff, sizeof(buff), "<input type=text name=%s_ip1 value=%d size=2 maxlength=3 onBlur=valid_range(this,0,223,\"IP\")>.<input type=text name=%s_ip2 value=%d size=2 maxlength=3 onBlur=valid_range(this,0,255,\"IP\")>.", buff2, ip[0], buff2, ip[1]);
		ret += websWrite(wp, buff);
		snprintf(buff, sizeof(buff), "<input type=text name=%s_ip3 value=%d size=2 maxlength=3 onBlur=valid_range(this,0,255,\"IP\")>.<input type=text name=%s_ip4 value=%d size=2 maxlength=3 onBlur=valid_range(this,0,255,\"IP\")>", buff2, ip[2], buff2, ip[3]);
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

		snprintf(buff, sizeof(buff), "<input type=text name=%s_nm1 value=%d size=2 maxlength=3 onBlur=valid_range(this,128,255,\"Netmask\")>.<input type=text name=%s_nm2 value=%d size=2 maxlength=3 onBlur=valid_range(this,128,255,\"Netmask\")>.", buff2, nm[0], buff2, nm[1]);
		ret += websWrite(wp, buff);
		snprintf(buff, sizeof(buff), "<input type=text name=%s_nm3 value=%d size=2 maxlength=3 onBlur=valid_range(this,128,255,\"Netmask\")>.<input type=text name=%s_nm4 value=%d size=2 maxlength=3 onBlur=valid_range(this,128,255,\"Netmask\")>", buff2, nm[2], buff2, nm[3]);
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

		snprintf(buff, sizeof(buff), "<input type=text name=%s_add_ip1 size=2 maxlength=3 onBlur=valid_range(this,0,223,\"IP\")>.<input type=text name=%s_add_ip2 size=2 maxlength=3 onBlur=valid_range(this,0,255,\"IP\")>.", buff2, buff2);
		ret += websWrite(wp, buff);
		snprintf(buff, sizeof(buff), "<input type=text name=%s_add_ip3 size=2 maxlength=3 onBlur=valid_range(this,0,255,\"IP\")>.<input type=text name=%s_add_ip4 size=2 maxlength=3 onBlur=valid_range(this,0,255,\"IP\")>", buff2, buff2);
		ret += websWrite(wp, buff);
		ret += websWrite(wp, "</TD>\n");
		ret += websWrite(wp, "				<TD style=\"BORDER-RIGHT: 1px solid; BORDER-TOP: 1px solid; BORDER-LEFT: 1px solid; BORDER-BOTTOM: 1px solid\" align=center valign=middle bgColor=#FFFFFF height=30 colspan=2>Alias Netmask<br>");

		snprintf(buff, sizeof(buff), "<input type=text name=%s_add_nm1 size=2 maxlength=3 onBlur=valid_range(this,128,255,\"Netmask\")>.<input type=text name=%s_add_nm2 size=2 maxlength=3 onBlur=valid_range(this,128,255,\"Netmask\")>.", buff2, buff2);
		ret += websWrite(wp, buff);
		snprintf(buff, sizeof(buff), "<input type=text name=%s_add_nm3 size=2 maxlength=3 onBlur=valid_range(this,128,255,\"Netmask\")>.<input type=text name=%s_add_nm4 size=2 maxlength=3 onBlur=valid_range(this,128,255,\"Netmask\")>", buff2, buff2);
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
			snprintf(buff, sizeof(buff), "<input type=text name=%s_dhcpd_ip1 value=%d size=2 maxlength=3 onBlur=valid_range(this,0,223,\"IP\")>.", buff2, ip[0]);
		else
			snprintf(buff, sizeof(buff), "%d.", ip[0]);
		ret += websWrite(wp, buff);
		if(nm[1] < 255)
			snprintf(buff, sizeof(buff), "<input type=text name=%s_dhcpd_ip2 value=%d size=2 maxlength=3 onBlur=valid_range(this,0,255,\"IP\")>.", buff2, ip[1]);
		else
			snprintf(buff, sizeof(buff), "%d.", ip[1]);
		ret += websWrite(wp, buff);
		if(nm[2] < 255)
			snprintf(buff, sizeof(buff), "<input type=text name=%s_dhcpd_ip3 value=%d size=2 maxlength=3 onBlur=valid_range(this,0,255,\"IP\")>.", buff2, ip[2]);
		else
			snprintf(buff, sizeof(buff), "%d.", ip[2]);
		ret += websWrite(wp, buff);
		snprintf(buff, sizeof(buff), "<input type=text name=%s_dhcpd_ip4 value=%d", buff2, ip[3]);
		ret += websWrite(wp, buff);
		ret += websWrite(wp, " size=2 maxlength=3 onBlur=valid_range(this,0,255,\"IP\")></TD>\n");
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
			if (i < 3)
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

#ifdef KROMOGUI
int
ej_get_qossvcs2 (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *qos_svcs = nvram_safe_get ("svqos_svcs");
  char name[32], type[32], data[32], level[32];
  int no_svcs = 0, i = 0, ret = -1;


  // calc # of services
//      no_svcs = strspn(qos_svcs,"|");

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

      ret = websWrite (wp, "<tr>\n\
					<td>\n\
						<input type=\"checkbox\" name=\"svqos_svcdel%d\" />\n\
						<input type=\"hidden\" name=\"svqos_svcname%d\" value=\"%s\" />\n\
						<input type=\"hidden\" name=\"svqos_svctype%d\" value=\"%s\" />\n\
					</td>\n\
					<td><em>%s</em></td>\n\
					<td >\n\
						<select name=\"svqos_svcprio%d\"> \n\
							<option value=\"10\" %s>Premium</option>\n\
							<option value=\"20\" %s>Express</option>\n\
							<option value=\"30\" %s>Standard</option>\n\
							<option value=\"40\" %s>Bulk</option>\n\
						</select>\n\
		           </td>\n\
				</tr>\n", i, i, name, i, type, name, i, strcmp (level, "10") == 0 ? "selected" : "", strcmp (level, "20") == 0 ? "selected" : "", strcmp (level, "30") == 0 ? "selected" : "", strcmp (level, "40") == 0 ? "selected" : "");

      qos_svcs = strpbrk (++qos_svcs, "|");
      qos_svcs++;

    }

  return ret;
}

#ifndef HAVE_AQOS
int
ej_get_qosips2 (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *qos_ips = nvram_safe_get ("svqos_ips");
  char ip[32], level[32];
  int no_ips = 0, i = 0, ret = -1;

  // calc # of ips
  while ((qos_ips = strpbrk (qos_ips, "|")))
    {
      no_ips++;
      qos_ips++;
    }
  websWrite (wp, "<tr><th>Delete</th><th>IP/Mask</th><th>Priority</th></tr>");

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

      ret = websWrite (wp, "<tr>\n\
					<td>\n\
						<input type=\"checkbox\" name=\"svqos_ipdel%d\" />\n\
						<input type=\"hidden\" name=\"svqos_ip%d\" value=\"%s\" />\n\
					<</td>\n\
					<td><em>%s</em></td>\n\
					<td>\n\
						<select name=\"svqos_ipprio%d\"> \n\
							<option value=\"10\" %s>Premium</option>\n\
							<option value=\"20\" %s>Express</option>\n\
							<option value=\"30\" %s>Standard</option>\n\
							<option value=\"40\" %s>Bulk</option>\n\
						</select>\n\
					</td>\n\
				</tr>\n", i, i, ip, ip, i, strcmp (level, "10") == 0 ? "selected" : "", strcmp (level, "20") == 0 ? "selected" : "", strcmp (level, "30") == 0 ? "selected" : "", strcmp (level, "40") == 0 ? "selected" : "");

      qos_ips = strpbrk (++qos_ips, "|");
      qos_ips++;

    }

  return ret;
}
#else
int
ej_get_qosips2 (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *qos_ips = nvram_safe_get ("svqos_ips");
  char ip[32], level[32];
  int no_ips = 0, i = 0, ret = -1;

  // calc # of ips
  while ((qos_ips = strpbrk (qos_ips, "|")))
    {
      no_ips++;
      qos_ips++;
    }
  websWrite (wp,
	     "<tr><th>Delete</th><th>IP/Mask</th><th>Max Kbits</th></tr>");

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

      ret = websWrite (wp, "<tr>\n\
					<td>\n\
						<input type=\"checkbox\" name=\"svqos_ipdel%d\" />\n\
						<input type=\"hidden\" name=\"svqos_ip%d\" value=\"%s\" />\n\
					<</td>\n\
					<td><em>%s</em></td>\n\
					<td>\n\
						<input name=\"svqos_ipprio%d\" class=\"num\" size=\"5\" maxlength=\"5\" value=\"%s\" /> \n\
						</select>\n\
					</td>\n\
				</tr>\n", i, i, ip, ip, i, level);

      qos_ips = strpbrk (++qos_ips, "|");
      qos_ips++;

    }

  return ret;
}
#endif
#ifndef HAVE_AQOS
int
ej_get_qosmacs2 (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *qos_macs = nvram_safe_get ("svqos_macs");
  char mac[32], level[32];
  int no_macs = 0, i = 0, ret = -1;


  // calc # of ips
  while ((qos_macs = strpbrk (qos_macs, "|")))
    {
      no_macs++;
      qos_macs++;
    }

  websWrite (wp,
	     "<tr><th>Delete</th><th>MAC Address</th><th>Priority</th></tr>");

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

      ret = websWrite (wp, "<tr>\n\
					<td>\n\
						<input type=\"checkbox\" name=\"svqos_macdel%d\" />\n\
						<input type=\"hidden\" name=\"svqos_mac%d\" value=\"%s\" />\n\
					</td>\n\
					<td><em>%s</em></td>\n\
					<td>\n\
						<select name=\"svqos_macprio%d\"> \n\
							<option value=\"10\" %s>Premium</option>\n\
							<option value=\"20\" %s>Express</option>\n\
							<option value=\"30\" %s>Standard</option>\n\
							<option value=\"40\" %s>Bulk</option>\n\
						</select>\n\
					</td>\n\
				</tr>\n", i, i, mac, mac, i, strcmp (level, "10") == 0 ? "selected" : "", strcmp (level, "20") == 0 ? "selected" : "", strcmp (level, "30") == 0 ? "selected" : "", strcmp (level, "40") == 0 ? "selected" : "");

      qos_macs = strpbrk (++qos_macs, "|");
      qos_macs++;

    }

  return ret;
}

#else
int
ej_get_qosmacs2 (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *qos_macs = nvram_safe_get ("svqos_macs");
  char mac[32], level[32];
  int no_macs = 0, i = 0, ret = -1;


  // calc # of ips
  while ((qos_macs = strpbrk (qos_macs, "|")))
    {
      no_macs++;
      qos_macs++;
    }
  websWrite (wp,
	     "<tr><th>Delete</th><th>Mac Address</th><th>Max Kbits</th></tr>");

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

      ret = websWrite (wp, "<tr>\n\
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

  return ret;
}
#endif


#endif
