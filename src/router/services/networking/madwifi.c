/*
 * madwifi.c
 *
 * Copyright (C) 2005 - 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
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

#ifdef HAVE_MADWIFI
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

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

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <bcmnvram.h>
#include <bcmutils.h>
#include <shutils.h>
#include <utils.h>
#include <unistd.h>

#include "wireless.h"
#include "net80211/ieee80211.h"
#include "net80211/ieee80211_crypto.h"
#include "net80211/ieee80211_ioctl.h"
#include <iwlib.h>

static int socket_handle = -1;

static int
getsocket (void)
{

  if (socket_handle < 0)
    {
      socket_handle = socket (AF_INET, SOCK_DGRAM, 0);
      if (socket_handle < 0)
	err (1, "socket(SOCK_DGRAM)");
    }
  return socket_handle;
}

#define IOCTL_ERR(x) [x - SIOCIWFIRSTPRIV] "ioctl[" #x "]"
static int
set80211priv (struct iwreq *iwr, const char *ifname, int op, void *data,
	      size_t len)
{
#define	N(a)	(sizeof(a)/sizeof(a[0]))

  memset (iwr, 0, sizeof (struct iwreq));
  strncpy (iwr->ifr_name, ifname, IFNAMSIZ);
  if (len < IFNAMSIZ)
    {
      /*
       * Argument data fits inline; put it there.
       */
      memcpy (iwr->u.name, data, len);
    }
  else
    {
      /*
       * Argument data too big for inline transfer; setup a
       * parameter block instead; the kernel will transfer
       * the data for the driver.
       */
      iwr->u.data.pointer = data;
      iwr->u.data.length = len;
    }

  if (ioctl (getsocket (), op, iwr) < 0)
    {
      static const char *opnames[] = {
	IOCTL_ERR (IEEE80211_IOCTL_SETPARAM),
	IOCTL_ERR (IEEE80211_IOCTL_GETPARAM),
	IOCTL_ERR (IEEE80211_IOCTL_SETMODE),
	IOCTL_ERR (IEEE80211_IOCTL_GETMODE),
	IOCTL_ERR (IEEE80211_IOCTL_SETWMMPARAMS),
	IOCTL_ERR (IEEE80211_IOCTL_GETWMMPARAMS),
	IOCTL_ERR (IEEE80211_IOCTL_SETCHANLIST),
	IOCTL_ERR (IEEE80211_IOCTL_GETCHANLIST),
	IOCTL_ERR (IEEE80211_IOCTL_CHANSWITCH),
	IOCTL_ERR (IEEE80211_IOCTL_GETCHANINFO),
	IOCTL_ERR (IEEE80211_IOCTL_SETOPTIE),
	IOCTL_ERR (IEEE80211_IOCTL_GETOPTIE),
	IOCTL_ERR (IEEE80211_IOCTL_SETMLME),
	IOCTL_ERR (IEEE80211_IOCTL_SETKEY),
	IOCTL_ERR (IEEE80211_IOCTL_DELKEY),
	IOCTL_ERR (IEEE80211_IOCTL_ADDMAC),
	IOCTL_ERR (IEEE80211_IOCTL_DELMAC),
	IOCTL_ERR (IEEE80211_IOCTL_WDSADDMAC),
#ifdef OLD_MADWIFI
	IOCTL_ERR (IEEE80211_IOCTL_WDSDELMAC),
#else
	IOCTL_ERR (IEEE80211_IOCTL_WDSSETMAC),
#endif
      };
      op -= SIOCIWFIRSTPRIV;
      if (0 <= op && op < N (opnames))
	perror (opnames[op]);
      else
	perror ("ioctl[unknown???]");
      return -1;
    }
  return 0;
#undef N
}

int
do80211priv (const char *ifname, int op, void *data, size_t len)
{
  struct iwreq iwr;

  if (set80211priv (&iwr, ifname, op, data, len) < 0)
    return -1;
  if (len < IFNAMSIZ)
    memcpy (data, iwr.u.name, len);
  return iwr.u.data.length;
}

static int
set80211param (char *iface, int op, int arg)
{
  struct iwreq iwr;

  memset (&iwr, 0, sizeof (iwr));
  strncpy (iwr.ifr_name, iface, IFNAMSIZ);
  iwr.u.mode = op;
  memcpy (iwr.u.name + sizeof (__u32), &arg, sizeof (arg));

  if (ioctl (getsocket (), IEEE80211_IOCTL_SETPARAM, &iwr) < 0)
    {
      perror ("ioctl[IEEE80211_IOCTL_SETPARAM]");
      return -1;
    }
  return 0;
}

extern int br_add_interface (const char *br, const char *dev);

static int
setsysctrl (const char *dev, const char *control, u_long value)
{
  char buffer[256];

  sysprintf ("echo %li > /proc/sys/dev/%s/%s", value, dev, control);

  return 0;
}

static void
setdistance (char *device, int distance, int chanbw)
{

  if (distance >= 0)
    {
      int slottime = (distance / 300) + ((distance % 300) ? 1 : 0);
      int acktimeout = slottime * 2 + 3;
      int ctstimeout = slottime * 2 + 3;

      // printf("Setting distance on interface %s to %i meters\n", device,
      // distance);
      setsysctrl (device, "slottime", slottime);
      setsysctrl (device, "acktimeout", acktimeout);
      setsysctrl (device, "ctstimeout", ctstimeout);
    }
}

// returns the number of installed atheros devices/cards

static char iflist[1024];

char *
getiflist (void)
{
  return iflist;
}

static void
destroy_wds (char *ifname)
{
  int s;

  for (s = 1; s <= 10; s++)
    {
      char dev[16];

      sprintf (dev, "wds%s.%d", ifname, s);
      if (ifexists (dev))
	{
	  br_del_interface ("br0", dev);
	  eval ("wlanconfig", dev, "destroy");
	}
    }

}

static void
deconfigure_single (int count)
{
  char *next;
  char dev[16];
  char var[80];
  char wifivifs[16];

  sprintf (wifivifs, "ath%d_vifs", count);
  sprintf (dev, "ath%d", count);
  char vifs[128];

  sprintf (vifs, "%s.1 %s.2 %s.3 %s.4 %s.5 %s.6 %s.7 %s.8 %s.9", dev, dev,
	   dev, dev, dev, dev, dev, dev, dev);
  int s;

  for (s = 1; s <= 10; s++)
    {
      sprintf (dev, "wdsath%d.%d", count, s);
      if (ifexists (dev))
	{
	  br_del_interface ("br0", dev);
	  eval ("ifconfig", dev, "down");
	}
    }
  sprintf (dev, "ath%d", count);
  if (ifexists (dev))
    {
      br_del_interface ("br0", dev);
      eval ("ifconfig", dev, "down");
    }
  foreach (var, vifs, next)
  {
    if (ifexists (var))
      {
	eval ("ifconfig", var, "down");
      }
  }
  sprintf (dev, "ath%d", count);
#ifdef OLD_MADWIFI
  destroy_wds (dev);
#endif

  if (ifexists (dev))
    eval ("wlanconfig", dev, "destroy");

  foreach (var, vifs, next)
  {
    if (ifexists (var))
      {
	eval ("wlanconfig", var, "destroy");
      }
  }

}

void
deconfigure_wifi (void)
{

  memset (iflist, 0, 1024);
  killall ("wrt-radauth", SIGTERM);
  killall ("hostapd", SIGTERM);
  killall ("wpa_supplicant", SIGTERM);
  sleep (1);
  killall ("wrt-radauth", SIGKILL);
  killall ("hostapd", SIGKILL);
  killall ("wpa_supplicant", SIGKILL);

  int c = getdevicecount ();
  int i;

  for (i = 0; i < c; i++)
    deconfigure_single (i);
}

static int need_commit = 0;

static int
getMaxPower (char *ifname)
{
  char buf[128];

  sprintf (buf, "iwlist %s txpower|grep \"Maximum Power:\" > /tmp/.power",
	   ifname);
  system2 (buf);
  FILE *in = fopen ("/tmp/.power", "rb");

  if (in == NULL)
    return 1000;
  char buf2[16];
  int max;

  fscanf (in, "%s %s %d", buf, buf2, &max);
  fclose (in);
  return max;
}

/*
 * MADWIFI Encryption Setup 
 */
void
setupSupplicant (char *prefix, char *ssidoverride)
{
if (!isregistered())
    return;
  char akm[16];
  char bridged[32];
  char wmode[16];

  sprintf (akm, "%s_akm", prefix);
  sprintf (wmode, "%s_mode", prefix);
  sprintf (bridged, "%s_bridged", prefix);
  if (nvram_match (akm, "wep"))
    {
      char key[16];
      int cnt = 1;
      int i;
      char bul[8];

      for (i = 1; i < 5; i++)
	{
	  char *athkey = nvram_nget ("%s_key%d", prefix, i);

	  if (athkey != NULL && strlen (athkey) > 0)
	    {
	      sprintf (bul, "[%d]", cnt++);
	      eval ("iwconfig", prefix, "key", bul, athkey);	// setup wep
	      // encryption 
	      // key
	    }
	}
      sprintf (bul, "[%s]", nvram_nget ("%s_key", prefix));
      eval ("iwconfig", prefix, "key", bul);
      // eval ("iwpriv", prefix, "authmode", "2");
    }
  else if (nvram_match (akm, "psk") ||
	   nvram_match (akm, "psk2") || nvram_match (akm, "psk psk2"))
    {
      char fstr[32];
      char psk[16];

      sprintf (fstr, "/tmp/%s_wpa_supplicant.conf", prefix);
      FILE *fp = fopen (fstr, "wb");

      fprintf (fp, "ap_scan=1\n");
      fprintf (fp, "fast_reauth=1\n");
      fprintf (fp, "eapol_version=1\n");
      // fprintf (fp, "ctrl_interface_group=0\n");
      // fprintf (fp, "ctrl_interface=/var/run/wpa_supplicant\n");

      fprintf (fp, "network={\n");
      if (!ssidoverride)
	ssidoverride = nvram_nget ("%s_ssid", prefix);
      fprintf (fp, "\tssid=\"%s\"\n", ssidoverride);
      // fprintf (fp, "\tmode=0\n");
      fprintf (fp, "\tscan_ssid=1\n");
      fprintf (fp, "\tkey_mgmt=WPA-PSK\n");

      sprintf (psk, "%s_crypto", prefix);
      if (nvram_match (psk, "aes"))
	{
	  fprintf (fp, "\tpairwise=CCMP\n");
	  fprintf (fp, "\tgroup=CCMP\n");
	}
      if (nvram_match (psk, "tkip"))
	{
	  fprintf (fp, "\tpairwise=TKIP\n");
	  fprintf (fp, "\tgroup=TKIP\n");
	}
      if (nvram_match (psk, "tkip+aes"))
	{
	  fprintf (fp, "\tpairwise=CCMP TKIP\n");
	  fprintf (fp, "\tgroup=CCMP TKIP\n");
	}
      if (nvram_match (akm, "psk"))
	fprintf (fp, "\tproto=WPA\n");
      if (nvram_match (akm, "psk2"))
	fprintf (fp, "\tproto=RSN\n");
      if (nvram_match (akm, "psk psk2"))
	fprintf (fp, "\tproto=WPA RSN\n");

      fprintf (fp, "\tpsk=\"%s\"\n", nvram_nget ("%s_wpa_psk", prefix));
      fprintf (fp, "}\n");
      fclose (fp);
      sprintf (psk, "-i%s", prefix);
      if ((nvram_match (wmode, "wdssta") || nvram_match (wmode, "wet"))
	  && nvram_match (bridged, "1"))
	eval ("wpa_supplicant", "-b", getBridge (prefix), "-B",
	      "-Dmadwifi", psk, "-c", fstr);
      else
	eval ("wpa_supplicant", "-B", "-Dmadwifi", psk, "-c", fstr);
    }
  else if (nvram_match (akm, "8021X"))
    {
      char fstr[32];
      char psk[64];
      char ath[64];

      sprintf (fstr, "/tmp/%s_wpa_supplicant.conf", prefix);
      FILE *fp = fopen (fstr, "wb");

      fprintf (fp, "ap_scan=1\n");
      fprintf (fp, "fast_reauth=1\n");
      fprintf (fp, "eapol_version=1\n");
      // fprintf (fp, "ctrl_interface_group=0\n");
      // fprintf (fp, "ctrl_interface=/var/run/wpa_supplicant\n");
      fprintf (fp, "network={\n");
      if (!ssidoverride)
	ssidoverride = nvram_nget ("%s_ssid", prefix);
      fprintf (fp, "\tssid=\"%s\"\n", ssidoverride);
      fprintf (fp, "\tscan_ssid=1\n");
      if (nvram_prefix_match ("8021xtype", prefix, "tls"))
	{
	  fprintf (fp, "\tkey_mgmt=IEEE8021X\n");
	  fprintf (fp, "\teap=TLS\n");
	  fprintf (fp, "\tidentity=\"%s\"\n",
		   nvram_prefix_get ("tls8021xuser", prefix));
	  sprintf (psk, "/tmp/%s", prefix);
	  mkdir (psk);
	  sprintf (psk, "/tmp/%s/ca.pem", prefix);
	  sprintf (ath, "%s_tls8021xca", prefix);
	  write_nvram (psk, ath);
	  sprintf (psk, "/tmp/%s/user.pem", prefix);
	  sprintf (ath, "%s_tls8021xpem", prefix);
	  write_nvram (psk, ath);

	  sprintf (psk, "/tmp/%s/user.prv", prefix);
	  sprintf (ath, "%s_tls8021xprv", prefix);
	  write_nvram (psk, ath);
	  fprintf (fp, "\tca_cert=\"/tmp/%s/ca.pem\"\n", prefix);
	  fprintf (fp, "\tclient_cert=\"/tmp/%s/user.pem\"\n", prefix);
	  fprintf (fp, "\tprivate_key=\"/tmp/%s/user.prv\"\n", prefix);
	  fprintf (fp, "\tprivate_key_passwd=\"%s\"\n",
		   nvram_prefix_get ("tls8021xpasswd", prefix));
	  fprintf (fp, "\teapol_flags=3\n");
	}
      if (nvram_prefix_match ("8021xtype", prefix, "peap"))
	{
	  fprintf (fp, "\tkey_mgmt=IEEE8021X\n");
	  fprintf (fp, "\teap=PEAP\n");
	  fprintf (fp, "\tphase2=\"auth=MSCHAPV2\"\n");
	  fprintf (fp, "\tidentity=\"%s\"\n",
		   nvram_prefix_get ("peap8021xuser", prefix));
	  fprintf (fp, "\tpassword=\"%s\"\n",
		   nvram_prefix_get ("peap8021xpasswd", prefix));
	  sprintf (psk, "/tmp/%s", prefix);
	  mkdir (psk);
	  sprintf (psk, "/tmp/%s/ca.pem", prefix);
	  sprintf (ath, "%s_peap8021xca", prefix);
	  write_nvram (psk, ath);
	  fprintf (fp, "\tca_cert=\"/tmp/%s/ca.pem\"\n", prefix);
	}
      if (nvram_prefix_match ("8021xtype", prefix, "ttls"))
	{
	  fprintf (fp, "\tkey_mgmt=WPA-EAP\n");
	  fprintf (fp, "\teap=TTLS\n");
	  fprintf (fp, "\tidentity=\"%s\"\n",
		   nvram_prefix_get ("ttls8021xuser", prefix));
	  fprintf (fp, "\tpassword=\"%s\"\n",
		   nvram_prefix_get ("ttls8021xpasswd", prefix));
	  if (strlen (nvram_nget ("%s_ttls8021xca", prefix)) > 0)
	    {
	      sprintf (psk, "/tmp/%s", prefix);
	      mkdir (psk);
	      sprintf (psk, "/tmp/%s/ca.pem", prefix);
	      sprintf (ath, "%s_ttls8021xca", prefix);
	      write_nvram (psk, ath);
	      fprintf (fp, "\tca_cert=\"/tmp/%s/ca.pem\"\n", prefix);
	    }
	}
      if (nvram_prefix_match ("8021xtype", prefix, "leap"))
	{
	  fprintf (fp, "\tkey_mgmt=WPA-EAP\n");
	  fprintf (fp, "\teap=LEAP\n");
	  fprintf (fp, "\tauth_alg=LEAP\n");
	  fprintf (fp, "\tproto=WPA RSN\n");
	  fprintf (fp, "\tpairwise=CCMP TKIP\n");
	  fprintf (fp, "\tgroup=CCMP TKIP\n");
	  fprintf (fp, "\tidentity=\"%s\"\n",
		   nvram_prefix_get ("peap8021xuser", prefix));
	  fprintf (fp, "\tpassword=\"%s\"\n",
		   nvram_prefix_get ("peap8021xpasswd", prefix));
	  // sprintf (psk, "/tmp/%s", prefix);
	  // mkdir (psk);
	  // sprintf (psk, "/tmp/%s/ca.pem", prefix);
	  // sprintf (ath, "%s_peap8021xca", prefix);
	  // write_nvram (psk, ath);
	  // fprintf (fp, "\tca_cert=\"/tmp/%s/ca.pem\"\n", prefix);
	}
      fprintf (fp, "}\n");
      fclose (fp);
      sprintf (psk, "-i%s", prefix);
      eval ("iwpriv", prefix, "hostroaming", "2");
      if (nvram_match (bridged, "1")
	  && (nvram_match (wmode, "wdssta") || nvram_match (wmode, "wet")))
	eval ("wpa_supplicant", "-b", nvram_safe_get ("lan_ifname"),
	      "-B", "-Dmadwifi", psk, "-c", fstr);
      else
	eval ("wpa_supplicant", "-B", "-Dmadwifi", psk, "-c", fstr);
    }
  else
    {
      eval ("iwconfig", prefix, "key", "off");
      // eval ("iwpriv", prefix, "authmode", "0");
    }

}
void
supplicant_main (int argc, char *argv[])
{
  setupSupplicant (argv[1], argv[2]);
}

void
setupHostAP (char *prefix, int iswan)
{
if (!isregistered())
    return;
  char psk[32];
  char akm[16];

  sprintf (akm, "%s_akm", prefix);
  if (nvram_match (akm, "wpa") || nvram_match (akm, "wpa2")
      || nvram_match (akm, "wpa wpa2") || nvram_match (akm, "radius"))
    {
      if (iswan == 0)
	return;
    }
  if (nvram_match (akm, "psk") ||
      nvram_match (akm, "psk2") ||
      nvram_match (akm, "psk psk2") || nvram_match (akm, "wep"))
    {
      if (iswan == 1)
	return;
    }
  // wep key support
  if (nvram_match (akm, "wep"))
    {
      int cnt = 1;
      int i;
      char bul[8];

      for (i = 1; i < 5; i++)
	{
	  char *athkey = nvram_nget ("%s_key%d", prefix, i);

	  if (athkey != NULL && strlen (athkey) > 0)
	    {
	      sprintf (bul, "[%d]", cnt++);
	      eval ("iwconfig", prefix, "key", bul, athkey);	// setup wep
	      // encryption 
	      // key
	    }
	}
      sprintf (bul, "[%s]", nvram_nget ("%s_key", prefix));
      eval ("iwconfig", prefix, "key", bul);
      // eval ("iwpriv", prefix, "authmode", "2");
    }
  else if (nvram_match (akm, "psk") ||
	   nvram_match (akm, "psk2") ||
	   nvram_match (akm, "psk psk2") ||
	   nvram_match (akm, "wpa") ||
	   nvram_match (akm, "wpa2") || nvram_match (akm, "wpa wpa2"))
    {
      char fstr[32];

      sprintf (fstr, "/tmp/%s_hostap.conf", prefix);
      FILE *fp = fopen (fstr, "wb");

      fprintf (fp, "interface=%s\n", prefix);
      // sprintf(buf, "rsn_preauth_interfaces=%s\n", "br0");
      if (nvram_nmatch ("1", "%s_bridged", prefix))
	fprintf (fp, "bridge=%s\n", getBridge (prefix));

      fprintf (fp, "driver=madwifi\n");
      fprintf (fp, "logger_syslog=-1\n");
      fprintf (fp, "logger_syslog_level=2\n");
      fprintf (fp, "logger_stdout=-1\n");
      fprintf (fp, "logger_stdout_level=2\n");
      fprintf (fp, "debug=0\n");
      fprintf (fp, "dump_file=/tmp/hostapd.dump\n");
      // fprintf (fp, "eap_server=0\n");
      // fprintf (fp, "own_ip_addr=127.0.0.1\n");
      fprintf (fp, "eapol_version=1\n");
      fprintf (fp, "eapol_key_index_workaround=0\n");
      if (nvram_match (akm, "psk") || nvram_match (akm, "wpa"))
	fprintf (fp, "wpa=1\n");
      if (nvram_match (akm, "psk2") || nvram_match (akm, "wpa2"))
	fprintf (fp, "wpa=2\n");
      if (nvram_match (akm, "psk psk2") || nvram_match (akm, "wpa wpa2"))
	fprintf (fp, "wpa=3\n");

      if (nvram_match (akm, "psk") ||
	  nvram_match (akm, "psk2") || nvram_match (akm, "psk psk2"))
	{
	  fprintf (fp, "wpa_passphrase=%s\n",
		   nvram_nget ("%s_wpa_psk", prefix));
	  fprintf (fp, "wpa_key_mgmt=WPA-PSK\n");
	}
      else
	{
	  // if (nvram_invmatch (akm, "radius"))
	  fprintf (fp, "wpa_key_mgmt=WPA-EAP\n");
	  // else
	  // fprintf (fp, "macaddr_acl=2\n");
	  fprintf (fp, "ieee8021x=1\n");
	  // fprintf (fp, "accept_mac_file=/tmp/hostapd.accept\n");
	  // fprintf (fp, "deny_mac_file=/tmp/hostapd.deny\n");
	  fprintf (fp, "own_ip_addr=%s\n", nvram_safe_get ("lan_ipaddr"));
	  fprintf (fp, "eap_server=0\n");
	  fprintf (fp, "auth_algs=1\n");
	  fprintf (fp, "radius_retry_primary_interval=60\n");
	  fprintf (fp, "auth_server_addr=%s\n",
		   nvram_nget ("%s_radius_ipaddr", prefix));
	  fprintf (fp, "auth_server_port=%s\n",
		   nvram_nget ("%s_radius_port", prefix));
	  fprintf (fp, "auth_server_shared_secret=%s\n",
		   nvram_nget ("%s_radius_key", prefix));
	  if (nvram_nmatch ("1", "%s_acct", prefix))
	    {
	      fprintf (fp, "acct_server_addr=%s\n",
		       nvram_nget ("%s_acct_ipaddr", prefix));
	      fprintf (fp, "acct_server_port=%s\n",
		       nvram_nget ("%s_acct_port", prefix));
	      fprintf (fp, "acct_server_shared_secret=%s\n",
		       nvram_nget ("%s_acct_key", prefix));
	    }
	}
      if (nvram_invmatch (akm, "radius"))
	{
	  sprintf (psk, "%s_crypto", prefix);
	  if (nvram_match (psk, "aes"))
	    fprintf (fp, "wpa_pairwise=CCMP\n");
	  if (nvram_match (psk, "tkip"))
	    fprintf (fp, "wpa_pairwise=TKIP\n");
	  if (nvram_match (psk, "tkip+aes"))
	    fprintf (fp, "wpa_pairwise=TKIP CCMP\n");
	  fprintf (fp, "wpa_group_rekey=%s\n",
		   nvram_nget ("%s_wpa_gtk_rekey", prefix));
	}
      // fprintf (fp, "jumpstart_p1=1\n");
      fclose (fp);
      eval ("hostapd", "-B", fstr);
    }
  else if (nvram_match (akm, "radius"))
    {
      // wrt-radauth $IFNAME $server $port $share $override $mackey $maxun
      // &
      char *ifname = prefix;
      char *server = nvram_nget ("%s_radius_ipaddr", prefix);
      char *port = nvram_nget ("%s_radius_port", prefix);
      char *share = nvram_nget ("%s_radius_key", prefix);
      char exec[64];
      char type[32];

      sprintf (type, "%s_radmactype", prefix);
      char *pragma = "";

      if (nvram_default_match (type, "0", "0"))
	pragma = "-n1 ";
      if (nvram_match (type, "1"))
	pragma = "-n2 ";
      if (nvram_match (type, "2"))
	pragma = "-n3 ";
      if (nvram_match (type, "3"))
	pragma = "";
      sleep (1);		// some delay is usefull
      sysprintf ("wrt-radauth %s %s %s %s %s 1 1 0 &", pragma, prefix,
		 server, port, share);
    }
  else
    {
      eval ("iwconfig", prefix, "key", "off");
    }

}
void
start_hostapdwan (void)
{
  char ath[32];
  char *next;
  char var[80];
  int c = getdevicecount ();
  int i;

  for (i = 0; i < c; i++)
    {
      sprintf (ath, "ath%d", i);
      setupHostAP (ath, 1);
      char *vifs = nvram_nget ("ath%d_vifs", i);

      if (vifs != NULL)
	foreach (var, vifs, next)
	{
	  setupHostAP (var, 1);
	}
    }

}

#define SIOCSSCANLIST  		(SIOCDEVPRIVATE+6)
#ifdef MADWIFI_OLD
static void
set_scanlist (char *dev, char *wif)
{

  char *next;
  struct iwreq iwr;
  char scanlist[32];
  unsigned short list[1024];

  sprintf (scanlist, "%s_scanlist", dev);
  char *sl = nvram_default_get (scanlist, "default");
  memset (list, 0, 1024 * sizeof (unsigned short));
  int c = 0;

  if (strlen (sl) > 0 && strcmp (sl, "default"))
    {
      foreach (var, sl, next)
      {
	int ch = atoi (var);

	if (ch < 1000 || ch > 7000)
	  {
	    c = 1;
	    break;
	  }
	u_int16_t chan = ch;

	// fprintf(stderr,"scanlist %d\n",chan);
	list[c++] = chan;
      }
    }
  else
    c = 1;

  memset (&iwr, 0, sizeof (struct iwreq));
  strncpy (iwr.ifr_name, wif, IFNAMSIZ);
  {
    /*
     * Argument data too big for inline transfer; setup a
     * parameter block instead; the kernel will transfer
     * the data for the driver.
     */
    iwr.u.data.pointer = &list[0];
    iwr.u.data.length = 1024 * sizeof (unsigned short);
  }

  int r = ioctl (getsocket (), SIOCSSCANLIST, &iwr);

  if (r < 0)
    {
      fprintf (stderr, "error while setting scanlist on %s, %d\n", wif, r);
    }
}
#else
static void
set_scanlist (char *dev, char *wif)
{
  char var[32];
  char *next;
  struct iwreq iwr;
  char scanlist[32];
  char list[64];

  sprintf (scanlist, "%s_scanlist", dev);
  char *sl = nvram_default_get (scanlist, "default");
  int c = 0;

  eval ("iwpriv", dev, "setscanlist", "-ALL");
  if (strlen (sl) > 0 && strcmp (sl, "default"))
    {
      foreach (var, sl, next)
      {
	sprintf (list, "+%s", var);
	eval ("iwpriv", dev, "setscanlist", list);
      }
    }
  else
    {
      eval ("iwpriv", dev, "setscanlist", "+ALL");
    }
}
#endif

static void
set_rate (char *dev)
{
  char rate[32];
  char maxrate[32];
  char net[32];
  char bw[32];
  char xr[32];

  sprintf (bw, "%s_channelbw", dev);
  sprintf (net, "%s_net_mode", dev);
  sprintf (rate, "%s_minrate", dev);
  sprintf (maxrate, "%s_maxrate", dev);
  sprintf (xr, "%s_xr", dev);
  char *r = nvram_default_get (rate, "0");
  char *mr = nvram_default_get (maxrate, "0");

#ifdef HAVE_WHRAG108
  char *netmode;

  if (!strcmp (dev, "ath0"))
    netmode = nvram_default_get (net, "a-only");
  else
    netmode = nvram_default_get (net, "mixed");
#else
  char *netmode = nvram_default_get (net, "mixed");
#endif

  if (nvram_match (bw, "20") && nvram_match (xr, "0"))
    if (atof (r) == 27.0f || atof (r) == 1.5f || atof (r) == 2.0f
	|| atof (r) == 3.0f || atof (r) == 4.5f || atof (r) == 9.0f
	|| atof (r) == 13.5f)
      {
	nvram_set (rate, "0");
	r = "0";
      }
  if (nvram_match (bw, "40"))
    if (atof (r) == 27.0f || atof (r) == 1.5f || atof (r) == 2.0f
	|| atof (r) == 3.0f || atof (r) == 4.5f || atof (r) == 9.0f
	|| atof (r) == 13.5f)
      {
	nvram_set (rate, "0");
	r = "0";
      }
  if (nvram_match (bw, "10"))
    if (atof (r) > 27.0f || atof (r) == 1.5f || atof (r) == 2.0f
	|| atof (r) == 13.5f)
      {
	nvram_set (rate, "0");
	r = "0";
      }
  if (nvram_match (bw, "5"))
    if (atof (r) > 13.5)
      {
	nvram_set (rate, "0");
	r = "0";
      }
  if (!strcmp (netmode, "b-only"))
    eval ("iwconfig", dev, "rate", "11M", "auto");
  else
    {
      /*
       * if (nvram_match (bw, "5")) eval ("iwconfig", dev, "rate", "13500", 
       * "auto"); else if (nvram_match (bw, "10")) eval ("iwconfig", dev,
       * "rate", "27000", "auto"); else
       */
      eval ("iwconfig", dev, "rate", "54M", "auto");
    }
  if (atol (mr) > 0)
    eval ("iwpriv", dev, "maxrate", mr);
  if (atoi (mr) > 0)
    eval ("iwpriv", dev, "minrate", r);
}
static void
set_netmode (char *wif, char *dev, char *use)
{
  char net[16];
  char mode[16];
  char xr[16];
  char comp[32];
  char ff[16];
  char bw[16];
  sprintf (mode, "%s_mode", dev);
  sprintf (net, "%s_net_mode", dev);
  sprintf (bw, "%s_channelbw", dev);
  sprintf (xr, "%s_xr", dev);
  sprintf (comp, "%s_compression", dev);
  sprintf (ff, "%s_ff", dev);
#ifdef HAVE_WHRAG108
  char *netmode;

  if (!strcmp (dev, "ath0"))
    netmode = nvram_default_get (net, "a-only");
  else
    netmode = nvram_default_get (net, "mixed");
#else
  char *netmode = nvram_default_get (net, "mixed");
#endif
  // fprintf (stderr, "set netmode of %s to %s\n", net, netmode);
  cprintf ("configure net mode %s\n", netmode);

  // eval ("iwconfig", use, "channel", "0");
  // else
  {
#ifdef HAVE_WHRAG108
    if (!strncmp (use, "ath0", 4))
      {
	eval ("iwpriv", use, "mode", "1");
      }
    else
#endif
#ifdef HAVE_TW6600
    if (!strncmp (use, "ath0", 4))
      {
	eval ("iwpriv", use, "mode", "1");
      }
    else
#endif
      {
	eval ("iwpriv", use, "turbo", "0");
	eval ("iwpriv", use, "xr", "0");
	if (!strcmp (netmode, "mixed"))
	  eval ("iwpriv", use, "mode", "0");
	if (!strcmp (netmode, "b-only"))
	  eval ("iwpriv", use, "mode", "2");
	if (!strcmp (netmode, "g-only"))
	  {
	    eval ("iwpriv", use, "mode", "3");
	    eval ("iwpriv", use, "pureg", "1");
	  }
	if (!strcmp (netmode, "ng-only"))
	  {
	    eval ("iwpriv", use, "mode", "7");
	  }
	if (!strcmp (netmode, "na-only"))
	  {
	    eval ("iwpriv", use, "mode", "6");
	  }
	if (!strcmp (netmode, "bg-mixed"))
	  {
	    eval ("iwpriv", use, "mode", "3");
	  }

	if (!strcmp (netmode, "a-only"))
	  eval ("iwpriv", use, "mode", "1");
      }
  }
  if (nvram_default_match (bw, "40", "20"))
    {
      {
	if (!strcmp (netmode, "g-only"))
	  {
	    eval ("iwpriv", use, "mode", "6");
	  }
	if (!strcmp (netmode, "a-only"))
	  {
	    eval ("iwpriv", use, "mode", "5");
	  }
	eval ("iwpriv", use, "turbo", "1");
      }
    }
  else
    {
      char *ext = nvram_get (xr);

      if (ext)
	{
	  if (strcmp (ext, "1") == 0)
	    {
	      eval ("iwpriv", use, "xr", "1");
	    }
	  else
	    {
	      eval ("iwpriv", use, "xr", "0");
	    }
	}
    }
  if (nvram_default_match (comp, "1", "0"))
    eval ("iwpriv", use, "compression", "1");
  else
    eval ("iwpriv", use, "compression", "0");

  if (nvram_default_match (ff, "1", "0"))
    eval ("iwpriv", use, "ff", "1");
  else
    eval ("iwpriv", use, "ff", "0");

}

static void
setRTS (char *use)
{
  char rts[32];

  sprintf (rts, "%s_protmode", use);
  nvram_default_get (rts, "None");

  sprintf (rts, "%s_rts", use);
  nvram_default_get (rts, "0");

  sprintf (rts, "%s_rtsvalue", use);
  nvram_default_get (rts, "2346");

  if (nvram_nmatch ("1", "%s_rts", use))
    {
      eval ("iwconfig", use, "rts", nvram_nget ("%s_rtsvalue", use));
    }
  else
    {
      eval ("iwconfig", use, "rts", "off");
    }

  if (nvram_nmatch ("None", "%s_protmode", use))
    eval ("iwpriv", use, "protmode", "0");	// avoid throughput problems
  // (CTS disabled for now)
  if (nvram_nmatch ("CTS", "%s_protmode", use))
    eval ("iwpriv", use, "protmode", "1");
  if (nvram_nmatch ("RTS/CTS", "%s_protmode", use))
    eval ("iwpriv", use, "protmode", "2");
}

static void
set_compression (int count)
{
  char comp[32];
  char wif[32];

  sprintf (wif, "wifi%d", count);
  sprintf (comp, "ath%d_compression", count);
  if (nvram_default_match (comp, "1", "0"))
    setsysctrl (wif, "compression", 1);
  else
    setsysctrl (wif, "compression", 0);
}

void
setMacFilter (char *iface)
{
  char *next;
  char var[32];

  eval ("ifconfig", iface, "down");
  eval ("iwpriv", iface, "maccmd", "3");
  // set80211param (iface, IEEE80211_PARAM_MACCMD, IEEE80211_MACCMD_FLUSH);

  char nvvar[32];

  sprintf (nvvar, "%s_macmode", iface);
  if (nvram_match (nvvar, "deny"))
    {
      eval ("iwpriv", iface, "maccmd", "2");
      // set80211param (iface, IEEE80211_PARAM_MACCMD,
      // IEEE80211_MACCMD_POLICY_DENY);
      eval ("ifconfig", iface, "up");
      char nvlist[32];

      sprintf (nvlist, "%s_maclist", iface);

      foreach (var, nvram_safe_get (nvlist), next)
      {
	eval ("iwpriv", iface, "addmac", var);
	/*
	 * char ea[ETHER_ADDR_LEN]; if (ether_atoe (var, ea)) { struct
	 * sockaddr sa; memcpy (sa.sa_data, ea, IEEE80211_ADDR_LEN);
	 * do80211priv (iface, IEEE80211_IOCTL_ADDMAC, &sa, sizeof
	 * (struct sockaddr)); }
	 */
      }
    }
  if (nvram_match (nvvar, "allow"))
    {
      eval ("iwpriv", iface, "maccmd", "1");
      // set80211param (iface, IEEE80211_PARAM_MACCMD,
      // IEEE80211_MACCMD_POLICY_ALLOW);
      eval ("ifconfig", iface, "up");
      char nvlist[32];

      sprintf (nvlist, "%s_maclist", iface);

      foreach (var, nvram_safe_get (nvlist), next)
      {
	eval ("iwpriv", iface, "addmac", var);
	/*
	 * char ea[ETHER_ADDR_LEN]; if (ether_atoe (var, ea)) { struct
	 * sockaddr sa; memcpy (sa.sa_data, ea, IEEE80211_ADDR_LEN);
	 * do80211priv (iface, IEEE80211_IOCTL_ADDMAC, &sa, sizeof
	 * (struct sockaddr)); }
	 */
      }
    }

}

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

static void
configure_single (int count)
{
  char *next;
  char var[80];
  char mode[80];
  int cnt = 0;
  char dev[10];
  char wif[10];
  char wl[16];
  char channel[16];
  char ssid[16];
  char net[16];
  char wifivifs[16];
  char broadcast[16];
  char power[32];
  char sens[32];
  char basedev[16];
  char diversity[32];
  char rxantenna[32];
  char txantenna[32];
  char athmac[16];
  sprintf (wif, "wifi%d", count);
  sprintf (dev, "ath%d", count);
  sprintf (wifivifs, "ath%d_vifs", count);
  sprintf (wl, "ath%d_mode", count);
  sprintf (channel, "ath%d_channel", count);
  sprintf (power, "ath%d_txpwrdbm", count);
  sprintf (sens, "ath%d_distance", count);
  sprintf (diversity, "ath%d_diversity", count);
  sprintf (txantenna, "ath%d_txantenna", count);
  sprintf (rxantenna, "ath%d_rxantenna", count);
  sprintf (athmac, "ath%d_hwaddr", count);
  // create base device
  cprintf ("configure base interface %d\n", count);
  eval ("ifconfig", wif, "up");
  sprintf (net, "%s_net_mode", dev);
  if (nvram_match (net, "disabled"))
    return;
  if (!count)
    strcpy (iflist, dev);
  set_compression (count);
  // create wds interface(s)
  int s;

  char *m;
  int vif = 0;

  char *vifs = nvram_safe_get (wifivifs);

  if (vifs != NULL)
    foreach (var, vifs, next)
    {
      sprintf (mode, "%s_mode", var);
      m = nvram_default_get (mode, "ap");
      // create device
      if (strlen (mode) > 0)
	{
	  if (!strcmp (m, "wet") || !strcmp (m, "sta")
	      || !strcmp (m, "wdssta"))
	    eval ("wlanconfig", var, "create", "wlandev", wif, "wlanmode",
		  "sta", "nosbeacon");
	  else if (!strcmp (m, "ap") || !strcmp (m, "wdsap"))
	    eval ("wlanconfig", var, "create", "wlandev", wif, "wlanmode",
		  "ap");
	  else
	    eval ("wlanconfig", var, "create", "wlandev", wif, "wlanmode",
		  "adhoc");
	  vif = 1;
	  strcat (iflist, " ");
	  strcat (iflist, var);
	  char vathmac[16];

	  sprintf (vathmac, "%s_hwaddr", var);
	  char vmacaddr[32];

	  getMacAddr (var, vmacaddr);
	  nvram_set (vathmac, vmacaddr);

	}
    }

  // create original primary interface
  m = nvram_default_get (wl, "ap");

  if (!strcmp (m, "wet") || !strcmp (m, "wdssta") || !strcmp (m, "sta"))
    {
      if (vif)
	eval ("wlanconfig", dev, "create", "wlandev", wif, "wlanmode",
	      "sta", "nosbeacon");
      else
	eval ("wlanconfig", dev, "create", "wlandev", wif, "wlanmode", "sta");

    }
  else if (!strcmp (m, "ap") || !strcmp (m, "wdsap"))
    eval ("wlanconfig", dev, "create", "wlandev", wif, "wlanmode", "ap");
  else
    eval ("wlanconfig", dev, "create", "wlandev", wif, "wlanmode", "adhoc");

  for (s = 1; s <= 10; s++)
    {
      char wdsvarname[32] = { 0 };
      char wdsdevname[32] = { 0 };
      char wdsmacname[32] = { 0 };
      char *wdsdev;
      char *hwaddr;

      sprintf (wdsvarname, "%s_wds%d_enable", dev, s);
      sprintf (wdsdevname, "%s_wds%d_if", dev, s);
      sprintf (wdsmacname, "%s_wds%d_hwaddr", dev, s);
      wdsdev = nvram_safe_get (wdsdevname);
      if (strlen (wdsdev) == 0)
	continue;
      if (nvram_match (wdsvarname, "0"))
	continue;
      hwaddr = nvram_get (wdsmacname);
      if (hwaddr != NULL)
	{
#ifdef OLD_MADWIFI
	  eval ("wlanconfig", wdsdev, "create", "wlandev", wif, "wlanmode",
		"wds", "nobssid");
	  eval ("ifconfig", wdsdev, "0.0.0.0", "up");
	  eval ("iwpriv", wdsdev, "wds_add", hwaddr);
	  eval ("iwpriv", wdsdev, "wds", "1");
#else
	  eval ("iwpriv", dev, "wds_add", hwaddr);
	  // eval ("iwpriv", dev, "wds", "1");
#endif
	}
    }

  cprintf ("detect maxpower\n");
  m = nvram_default_get (wl, "ap");
  char maxp[16];

  vifs = nvram_safe_get (wifivifs);
  // fprintf(stderr,"vifs %s\n",vifs);
  char *useif = NULL;
  char copyvap[64];

  if (vifs != NULL)
    foreach (var, vifs, next)
    {
      // fprintf(stderr,"vifs %s, %s\n",vifs, var);
      if (!useif)
	{
	  strcpy (copyvap, var);
	  useif = copyvap;
	}
    }

  // config net mode
  if (useif)
    set_netmode (wif, dev, useif);
  set_netmode (wif, dev, dev);

  char wmm[32];

  sprintf (wmm, "%s_wmm", dev);
  eval ("iwpriv", dev, "wmm", nvram_default_get (wmm, "0"));
  char doth[32];

  sprintf (doth, "%s_doth", dev);
  eval ("iwpriv", dev, "doth", nvram_default_get (doth, "0"));
  // eval ("iwpriv", dev, "uapsd","0");
  eval ("iwpriv", dev, "scandisable", "0");
  int disablescan = 0;

#ifdef MADWIFI_OLD
  if (strcmp (m, "sta") && strcmp (m, "wdssta") && strcmp (m, "wet"))
    {
      cprintf ("set channel\n");
      char *ch = nvram_default_get (channel, "0");

      if (strcmp (ch, "0") == 0)
	{
	  eval ("iwpriv", dev, "scandisable", "0");
	  eval ("iwconfig", dev, "channel", "0");
	}
      else
	{
	  char freq[64];

	  sprintf (freq, "%sM", ch);
	  eval ("iwpriv", dev, "scandisable", "1");
	  disablescan = 1;
	  eval ("iwconfig", dev, "freq", freq);
	}
    }
  else
    {
      set_scanlist (dev, wif);
    }
#else
  set_scanlist (dev, wif);
  if (strcmp (m, "sta") && strcmp (m, "wdssta") && strcmp (m, "wet"))
    {
      char *ch = nvram_default_get (channel, "0");

      if (strcmp (ch, "0") == 0)
	{
	  eval ("iwconfig", dev, "channel", "0");
	}
      else
	{
	  char freq[64];

	  sprintf (freq, "%sM", ch);
	  eval ("iwconfig", dev, "freq", freq);
	}
    }
#endif

  if (useif)
    set_netmode (wif, dev, useif);
  set_netmode (wif, dev, dev);
  setRTS (dev);

  char macaddr[32];

  getMacAddr (dev, macaddr);
  nvram_set (athmac, macaddr);

  cprintf ("adjust sensitivity\n");

  int distance = atoi (nvram_default_get (sens, "2000"));	// to meter

  if (distance > 0)
    {
      setsysctrl (wif, "dynack_count", 0);
      char *chanbw = nvram_nget ("%s_channelbw", dev);

      setdistance (wif, distance, atoi (chanbw));	// sets the receiver
      // sensitivity
    }
  else
    setsysctrl (wif, "dynack_count", 20);

  char wl_intmit[32];
  char wl_noise_immunity[32];
  char wl_ofdm_weak_det[32];

  sprintf (wl_intmit, "%s_intmit", dev);
  sprintf (wl_noise_immunity, "%s_noise_immunity", dev);
  sprintf (wl_ofdm_weak_det, "%s_ofdm_weak_det", dev);
  setsysctrl (wif, "intmit", atoi (nvram_default_get (wl_intmit, "-1")));
  setsysctrl (wif, "noise_immunity",
	      atoi (nvram_default_get (wl_noise_immunity, "-1")));
  setsysctrl (wif, "ofdm_weak_det",
	      atoi (nvram_default_get (wl_ofdm_weak_det, "1")));

#ifdef HAVE_NS5
  char *gpio = "1";
#endif
#ifdef HAVE_NS2
  char *gpio = "7";
#endif

#if defined(HAVE_NS2) || defined(HAVE_NS5)
  int tx = atoi (nvram_default_get (txantenna, "0"));

  setsysctrl (wif, "diversity", 0);
  switch (tx)
    {
    case 0:			// vertical
      setsysctrl (wif, "rxantenna", 2);
      setsysctrl (wif, "txantenna", 2);
      eval ("gpio", "enable", gpio);
      break;
    case 1:			// horizontal
      setsysctrl (wif, "rxantenna", 1);
      setsysctrl (wif, "txantenna", 1);
      eval ("gpio", "enable", gpio);
      break;
    case 2:			// external
      setsysctrl (wif, "rxantenna", 1);
      setsysctrl (wif, "txantenna", 1);
      eval ("gpio", "disable", gpio);
      break;
    case 3:			// adaptive
      setsysctrl (wif, "diversity", 1);
      setsysctrl (wif, "rxantenna", 0);
      setsysctrl (wif, "txantenna", 0);
      eval ("gpio", "enable", gpio);
      break;
    }
#else

  int rx = atoi (nvram_default_get (rxantenna, "1"));
  int tx = atoi (nvram_default_get (txantenna, "1"));
  int diva = atoi (nvram_default_get (diversity, "0"));

  setsysctrl (wif, "diversity", diva);
  setsysctrl (wif, "rxantenna", rx);
  setsysctrl (wif, "txantenna", tx);
#endif
  // setup vif interfaces first
  char chanshift_s[32];

  sprintf (chanshift_s, "%s_chanshift", dev);
  char *chanshift = nvram_default_get (chanshift_s, "0");
  switch (atoi (chanshift))
    {
    case 15:
      eval ("iwpriv", dev, "channelshift", "-3");
      break;
    case 10:
      eval ("iwpriv", dev, "channelshift", "-2");
      break;
    case 5:
      eval ("iwpriv", dev, "channelshift", "-1");
      break;
    case 0:
      eval ("iwpriv", dev, "channelshift", "0");
      break;
    case -5:
      eval ("iwpriv", dev, "channelshift", "1");
      break;
    case -10:
      eval ("iwpriv", dev, "channelshift", "2");
      break;
    case -15:
      eval ("iwpriv", dev, "channelshift", "3");
      break;
    default:
      eval ("iwpriv", dev, "channelshift", "0");
      break;
    }
  vifs = nvram_safe_get (wifivifs);
  if (vifs != NULL)
    foreach (var, vifs, next)
    {
      sprintf (net, "%s_net_mode", var);
      if (nvram_match (net, "disabled"))
	continue;
      sprintf (ssid, "%s_ssid", var);
      sprintf (mode, "%s_mode", var);
      switch (atoi (chanshift))
	{
	case 15:
	  eval ("iwpriv", var, "channelshift", "-3");
	  break;
	case 10:
	  eval ("iwpriv", var, "channelshift", "-2");
	  break;
	case 5:
	  eval ("iwpriv", var, "channelshift", "-1");
	  break;
	case 0:
	  eval ("iwpriv", var, "channelshift", "0");
	  break;
	case -5:
	  eval ("iwpriv", var, "channelshift", "1");
	  break;
	case -10:
	  eval ("iwpriv", var, "channelshift", "2");
	  break;
	case -15:
	  eval ("iwpriv", var, "channelshift", "3");
	  break;
	default:
	  eval ("iwpriv", var, "channelshift", "0");
	  break;
	}
//      sprintf( chanshift, "%s_chanshift", dev );
//      eval( "iwpriv", var, "channelshift",
//            nvram_default_get( chanshift, "0" ) );
      m = nvram_default_get (mode, "ap");
#ifndef OLD_MADWIFI
      set_scanlist (dev, wif);
#endif
      setRTS (var);

      if (strcmp (m, "sta") && strcmp (m, "wdssta") && strcmp (m, "wet"))
	{
	  cprintf ("set channel\n");
	  char *ch = nvram_default_get (channel, "0");

	  if (strcmp (ch, "0") == 0)
	    {
#ifdef OLD_MADWIFI
	      eval ("iwpriv", var, "scandisable", "0");
#endif
	      eval ("iwconfig", var, "channel", "0");
	    }
	  else
	    {
	      char freq[64];

	      sprintf (freq, "%sM", ch);
#ifdef OLD_MADWIFI
	      eval ("iwpriv", var, "scandisable", "1");
	      disablescan = 1;
#endif
	      eval ("iwconfig", var, "freq", freq);
	    }
	}
      else
	{
#ifdef OLD_MADWIFI
	  set_scanlist (dev, wif);
#endif
	}

      eval ("iwpriv", var, "bgscan", "0");
#ifdef HAVE_MAKSAT
      eval ("iwconfig", var, "essid", nvram_default_get (ssid, "maksat_vap"));
#elif defined(HAVE_TRIMAX)
      eval ("iwconfig", var, "essid", nvram_default_get (ssid, "trimax_vap"));
#else
if (!isregistered())
  eval ("iwconfig", dev, "essid", "need_activation");
else
  eval ("iwconfig", var, "essid", nvram_default_get (ssid, "dd-wrt_vap"));
#endif
      cprintf ("set broadcast flag vif %s\n", var);	// hide ssid
      sprintf (broadcast, "%s_closed", var);
      eval ("iwpriv", var, "hide_ssid", nvram_default_get (broadcast, "0"));
      sprintf (wmm, "%s_wmm", var);
      eval ("iwpriv", var, "wmm", nvram_default_get (wmm, "0"));
      // eval ("iwpriv", var, "uapsd", "0");
      char isolate[32];

      sprintf (isolate, "%s_ap_isolate", var);
      if (nvram_default_match (isolate, "1", "0"))
	eval ("iwpriv", var, "ap_bridge", "0");
      if (!strcmp (m, "wdssta") || !strcmp (m, "wdsap"))
	eval ("iwpriv", var, "wds", "1");
      if (!strcmp (m, "wdsap"))
	eval ("iwpriv", var, "wdssep", "1");
      else
	eval ("iwpriv", var, "wdssep", "0");
#ifdef OLD_MADWIFI
      if (disablescan)
	eval ("iwpriv", var, "scandisable", "1");
#endif
      eval ("iwpriv", var, "hostroaming", "0");
      cnt++;
    }

  if (!strcmp (m, "wdssta") || !strcmp (m, "wdsap"))
    eval ("iwpriv", dev, "wds", "1");

  if (!strcmp (m, "wdsap"))
    eval ("iwpriv", dev, "wdssep", "1");
  else
    eval ("iwpriv", dev, "wdssep", "0");

  char isolate[32];

  sprintf (isolate, "%s_ap_isolate", dev);
  if (nvram_default_match (isolate, "1", "0"))
    eval ("iwpriv", dev, "ap_bridge", "0");
  eval ("iwpriv", dev, "hostroaming", "0");

  sprintf (ssid, "ath%d_ssid", count);
  sprintf (broadcast, "ath%d_closed", count);

  memset (var, 0, 80);

  cprintf ("set ssid\n");
#ifdef HAVE_MAKSAT
  eval ("iwconfig", dev, "essid", nvram_default_get (ssid, "maksat"));
#elif defined(HAVE_TRIMAX)
  eval ("iwconfig", dev, "essid", nvram_default_get (ssid, "trimax"));
#else
if (!isregistered())
  eval ("iwconfig", dev, "essid", "need_activation");
else
  eval ("iwconfig", dev, "essid", nvram_default_get (ssid, "dd-wrt"));
#endif
  cprintf ("set broadcast flag\n");	// hide ssid
  eval ("iwpriv", dev, "hide_ssid", nvram_default_get (broadcast, "0"));
  eval ("iwpriv", dev, "bgscan", "0");
  m = nvram_default_get (wl, "ap");

  char preamble[32];

  sprintf (preamble, "%s_preamble", dev);
  if (nvram_default_match (preamble, "1", "0"))
    {
      eval ("iwpriv", dev, "shpreamble", "1");
    }
  else
    eval ("iwpriv", dev, "shpreamble", "0");

  if (strcmp (m, "sta") == 0 || strcmp (m, "infra") == 0
      || strcmp (m, "wet") == 0 || strcmp (m, "wdssta") == 0)
    {
      cprintf ("set ssid\n");
#ifdef HAVE_MAKSAT
      eval ("iwconfig", dev, "essid", nvram_default_get (ssid, "maksat"));
#elif defined(HAVE_TRIMAX)
      eval ("iwconfig", dev, "essid", nvram_default_get (ssid, "trimax"));
#else
      eval ("iwconfig", dev, "essid", nvram_default_get (ssid, "dd-wrt"));
#endif
    }

  cprintf ("adjust power\n");

  int newpower = atoi (nvram_default_get (power, "16"));

  // fprintf (stderr, "new power limit %d\n", newpower);
  sprintf (var, "%ddBm", newpower);
  eval ("iwconfig", dev, "txpower", var);

  cprintf ("done()\n");

  cprintf ("setup encryption");
  // @todo ifup
  // netconfig

  setMacFilter (dev);

  set_rate (dev);

  set_netmode (wif, dev, dev);

  if (strcmp (m, "sta"))
    {
      char bridged[32];

      sprintf (bridged, "%s_bridged", dev);
      if (nvram_default_match (bridged, "1", "1"))
	{
	  ifconfig (dev, IFUP, NULL, NULL);
	  br_add_interface (getBridge (dev), dev);
	  eval ("ifconfig", dev, "0.0.0.0", "up");
	}
      else
	{
	  char ip[32];
	  char mask[32];

	  sprintf (ip, "%s_ipaddr", dev);
	  sprintf (mask, "%s_netmask", dev);
	  eval ("ifconfig", dev, "mtu", "1500");
	  eval ("ifconfig", dev, nvram_safe_get (ip), "netmask",
		nvram_safe_get (mask), "up");
	}
    }
  else
    {
      char bridged[32];

      sprintf (bridged, "%s_bridged", dev);
      if (nvram_default_match (bridged, "0", "1"))
	{
	  char ip[32];
	  char mask[32];

	  sprintf (ip, "%s_ipaddr", dev);
	  sprintf (mask, "%s_netmask", dev);
	  eval ("ifconfig", dev, "mtu", "1500");
	  eval ("ifconfig", dev, nvram_safe_get (ip), "netmask",
		nvram_safe_get (mask), "up");
	}
    }
  if (strcmp (m, "sta") && strcmp (m, "wdssta") && strcmp (m, "wet"))
    setupHostAP (dev, 0);
  else
    setupSupplicant (dev, NULL);

  // setup encryption

  vifs = nvram_safe_get (wifivifs);
  if (vifs != NULL)
    foreach (var, vifs, next)
    {
      sprintf (mode, "%s_mode", var);
      m = nvram_default_get (mode, "ap");
      if (strcmp (m, "sta") && strcmp (m, "wdssta") && strcmp (m, "wet"))
	setupHostAP (var, 0);
      else
	setupSupplicant (var, NULL);
    }
  /*
   * set_rate (dev);
   */

  // vif netconfig
  vifs = nvram_safe_get (wifivifs);
  if (vifs != NULL && strlen (vifs) > 0)
    {
      foreach (var, vifs, next)
      {
	setMacFilter (var);
	eval ("iwpriv", var, "scandisable", "1");

	sprintf (mode, "%s_mode", var);
	char *m2 = nvram_default_get (mode, "ap");

	if (strcmp (m2, "sta"))
	  {
	    char bridged[32];

	    sprintf (bridged, "%s_bridged", var);
	    if (nvram_default_match (bridged, "1", "1"))
	      {
		ifconfig (var, IFUP, NULL, NULL);
		br_add_interface (getBridge (var), var);
		if (!strcmp (m, "sta") || !strcmp (m, "wdssta")
		    || !strcmp (m, "wet"))
		  eval ("ifconfig", var, "0.0.0.0", "down");
		else
		  {
		    eval ("ifconfig", var, "0.0.0.0", "down");
		    sleep (1);
		    eval ("ifconfig", var, "0.0.0.0", "up");
		  }
	      }
	    else
	      {
		char ip[32];
		char mask[32];

		sprintf (ip, "%s_ipaddr", var);
		sprintf (mask, "%s_netmask", var);
		eval ("ifconfig", var, "mtu", "1500");
		ifconfig (var, IFUP, nvram_safe_get (ip),
			  nvram_safe_get (mask));
		if (!strcmp (m, "sta") || !strcmp (m, "wdssta")
		    || !strcmp (m, "wet"))
		  eval ("ifconfig", var, "down");
		else
		  {
		    eval ("ifconfig", var, "down");
		    sleep (1);
		    eval ("ifconfig", var, nvram_safe_get (ip),
			  "netmask", nvram_safe_get (mask), "up");
		  }
	      }
	  }
      }
    }

  m = nvram_default_get (wl, "ap");
  eval ("iwpriv", dev, "scandisable", "0");
  if (strcmp (m, "sta") && strcmp (m, "wdssta") && strcmp (m, "wet"))
    {
      cprintf ("set channel\n");
      char *ch = nvram_default_get (channel, "0");

      if (strcmp (ch, "0") == 0)
	{
	  eval ("iwconfig", dev, "channel", "0");
	}
      else
	{
	  char freq[64];

	  sprintf (freq, "%sM", ch);
	  eval ("iwpriv", dev, "scandisable", "1");
	  eval ("iwconfig", dev, "freq", freq);
	  sleep (1);
	  eval ("ifconfig", dev, "down");
	  sleep (1);
	  eval ("ifconfig", dev, "up");
	}
    }

  for (s = 1; s <= 10; s++)
    {
      char wdsvarname[32] = { 0 };
      char wdsdevname[32] = { 0 };
      char wdsmacname[32] = { 0 };
      char *wdsdev;
      char *hwaddr;

      sprintf (wdsvarname, "%s_wds%d_enable", dev, (11 - s));
      sprintf (wdsdevname, "%s_wds%d_if", dev, (11 - s));
      sprintf (wdsmacname, "%s_wds%d_hwaddr", dev, (11 - s));
      wdsdev = nvram_safe_get (wdsdevname);
      if (strlen (wdsdev) == 0)
	continue;
      if (nvram_match (wdsvarname, "0"))
	continue;
      hwaddr = nvram_get (wdsmacname);
      if (hwaddr != NULL)
	{
	  eval ("ifconfig", wdsdev, "0.0.0.0", "up");
	}
    }

}

void
start_vifs (void)
{
  char *next;
  char var[80];
  char *vifs;
  char mode[32];
  char *m;
  char wifivifs[32];
  int c = getdevicecount ();
  int count = 0;

  for (count = 0; count < c; count++)
    {
      sprintf (wifivifs, "ath%d_vifs", count);
      vifs = nvram_safe_get (wifivifs);
      if (vifs != NULL && strlen (vifs) > 0)
	{
	  foreach (var, vifs, next)
	  {
	    setMacFilter (var);

	    sprintf (mode, "%s_mode", var);
	    m = nvram_default_get (mode, "ap");

	    if (strcmp (m, "sta"))
	      {
		char bridged[32];

		sprintf (bridged, "%s_bridged", var);
		if (nvram_default_match (bridged, "1", "1"))
		  {
		    ifconfig (var, IFUP, NULL, NULL);
		    br_add_interface (getBridge (var), var);
		    eval ("ifconfig", var, "0.0.0.0", "up");
		  }
		else
		  {
		    char ip[32];
		    char mask[32];

		    sprintf (ip, "%s_ipaddr", var);
		    sprintf (mask, "%s_netmask", var);
		    eval ("ifconfig", var, "mtu", "1500");
		    ifconfig (var, IFUP, nvram_safe_get (ip),
			      nvram_safe_get (mask));
		  }
	      }
	  }
	}
    }

}

void
stop_vifs (void)
{
  char *next;
  char var[80];
  char *vifs;
  char mode[32];
  char *m;
  char wifivifs[32];
  int c = getdevicecount ();
  int count = 0;

  for (count = 0; count < c; count++)
    {
      sprintf (wifivifs, "ath%d_vifs", count);
      vifs = nvram_safe_get (wifivifs);
      if (vifs != NULL && strlen (vifs) > 0)
	{
	  foreach (var, vifs, next)
	  {
	    eval ("ifconfig", var, "down");

	  }
	}
    }

}
extern void adjust_regulatory (int count);

void
configure_wifi (void)		// madwifi implementation for atheros based
				// cards
{
  deconfigure_wifi ();
  /*
   * int s; int existed=0; for (s=0;s<10;s++) { char wif[32];
   * sprintf(wif,"wifi%d",s); if (ifexists(wif)) {
   * eval("ifconfig",wif,"down"); existed=1; } } #if defined(HAVE_FONERA)
   * || defined(HAVE_WHRAG108) eval("rmmod","ath_ahb"); insmod("ath_ahb",
   * "autocreate=none"); #else eval("rmmod","ath_pci"); insmod("ath_pci",
   * "autocreate=none"); #endif for (s=0;s<10;s++) { char wif[32];
   * sprintf(wif,"wifi%d",s); if (ifexists(wif)) eval("ifconfig",wif,"up");
   * } 
   */

  // bridge the virtual interfaces too
  memset (iflist, 0, 1024);
  /*
   * char countrycode[64]; char xchanmode[64]; char outdoor[64];
   * 
   * if (strlen (nvram_safe_get ("wl_countrycode")) > 0) sprintf
   * (countrycode, "countrycode=%s", nvram_safe_get ("wl_countrycode"));
   * else sprintf (countrycode, "countrycode=0");
   * 
   * if (strlen (nvram_safe_get ("wl_xchanmode")) > 0) sprintf (xchanmode,
   * "xchanmode=%s", nvram_safe_get ("wl_xchanmode")); else sprintf
   * (xchanmode, "xchanmode=0");
   * 
   * if (strlen (nvram_safe_get ("wl_outdoor")) > 0) sprintf (outdoor,
   * "outdoor=%s", nvram_safe_get ("wl_outdoor")); else sprintf (outdoor,
   * "outdoor=0"); 
   */

  int c = getdevicecount ();
  int i;
  int changed = 0;

  for (i = 0; i < c; i++)
    adjust_regulatory (i);

  for (i = 0; i < c; i++)
    {
#ifdef REGDOMAIN_OVERRIDE
      // SeG's dirty hack to make everything possible without any channel
      // restrictions. regdomain 0x60 seems to be the best way
      char regdomain[16];

      sprintf (regdomain, "ath%d_regdomain", i);

      // read current reg domain from atheros card
      // the base io 0x50010000 is hardcoded here and can be different on
      // non RB500 ports
      // @fixme: detect io by reading pci data

      cprintf ("get reg domain()\n");
      int reg_domain = get_regdomain ((0x50010000) + (0x10000 * i));

      if (reg_domain > -1)	// reg domain was successfully readed 
	{
	  if (nvram_get (regdomain) != NULL)	// reg domain is
	    // defined in nvram
	    {
	      int destination = atoi (nvram_safe_get (regdomain));	// read 

	      // new 
	      // target 
	      // regdomain
	      if (destination != reg_domain)	// check if changed
		{
		  if (set_regdomain ((0x50010000) + (0x10000 * i), destination) == 0)	// modifiy 
		    // eeprom 
		    // with 
		    // new 
		    // regdomain
		    changed = 1;
		}
	    }

	}
      cprintf ("configure next\n");
      if (!changed)		// if regdomain not changed, configure it
#endif
	{
	  configure_single (i);
	}
    }

  if (changed)			// if changed, deconfigure myself and
    // reconfigure me in the same way. 
    {
      deconfigure_wifi ();
      configure_wifi ();
    }
  if (need_commit)
    {
      nvram_commit ();
      need_commit = 0;
    }
  eval ("killall", "-9", "roaming_daemon");
  if (getSTA () || getWET ())
    eval ("roaming_daemon");
}

void
start_deconfigurewifi (void)
{
  deconfigure_wifi ();
}

void
start_configurewifi (void)
{
  configure_wifi ();
}
#endif
