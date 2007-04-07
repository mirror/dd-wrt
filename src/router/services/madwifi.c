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


/*

regdomains:
0x10 (FCC) 
0x20 (DOC) 
0x30 (ETSI) 
0x31 (Spain) 
0x32 (France) 
0x40 (MKK-Japan) 
0xFF (debug)
i

0x61 outdoor1 

*/


#ifdef HAVE_MADWIFI
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>

#define AR5K_PCICFG 0x4010

#define AR5K_EEPROM_PROTECT_OFFSET 0x3F
#define AR5K_PCICFG_EEAE 0x00000001
#define AR5K_PCICFG_CLKRUNEN 0x00000004
#define AR5K_PCICFG_LED_PEND 0x00000020
#define AR5K_PCICFG_LED_ACT 0x00000040
#define AR5K_PCICFG_SL_INTEN 0x00000800
#define AR5K_PCICFG_BCTL		 0x00001000
#define AR5K_PCICFG_SPWR_DN 0x00010000

 /* EEPROM Registers in the MAC */
#define AR5211_EEPROM_ADDR 0x6000
#define AR5211_EEPROM_DATA 0x6004
#define AR5211_EEPROM_COMD 0x6008
#define AR5211_EEPROM_COMD_READ 0x0001
#define AR5211_EEPROM_COMD_WRITE 0x0002
#define AR5211_EEPROM_COMD_RESET 0x0003
#define AR5211_EEPROM_STATUS 0x600C
#define AR5211_EEPROM_STAT_RDERR 0x0001
#define AR5211_EEPROM_STAT_RDDONE 0x0002
#define AR5211_EEPROM_STAT_WRERR 0x0003
#define AR5211_EEPROM_STAT_WRDONE 0x0004
#define AR5211_EEPROM_CONF 0x6010

#define VT_WLAN_IN32(a)  (*((volatile unsigned long int *)(mem + (a))))
#define VT_WLAN_OUT32(v,a) (*((volatile unsigned long int *)(mem + (a))) = (v))

#ifdef REGDOMAIN_OVERRIDE
static int
vt_ar5211_eeprom_read (unsigned char *mem,
		       unsigned long int offset, unsigned short int *data)
{
  int timeout = 10000;
  unsigned long int status;

  VT_WLAN_OUT32 (0, AR5211_EEPROM_CONF), usleep (5);

	/** enable eeprom read access */
  VT_WLAN_OUT32 (VT_WLAN_IN32 (AR5211_EEPROM_COMD)
		 | AR5211_EEPROM_COMD_RESET, AR5211_EEPROM_COMD);
  usleep (5);

	/** set address */
  VT_WLAN_OUT32 ((unsigned char) offset, AR5211_EEPROM_ADDR);
  usleep (5);

  VT_WLAN_OUT32 (VT_WLAN_IN32 (AR5211_EEPROM_COMD)
		 | AR5211_EEPROM_COMD_READ, AR5211_EEPROM_COMD);

  while (timeout > 0)
    {
      usleep (1);
      status = VT_WLAN_IN32 (AR5211_EEPROM_STATUS);
      if (status & AR5211_EEPROM_STAT_RDDONE)
	{
	  if (status & AR5211_EEPROM_STAT_RDERR)
	    {
	      (void) fputs ("eeprom read access failed!\n", stderr);
	      return 1;
	    }
	  status = VT_WLAN_IN32 (AR5211_EEPROM_DATA);
	  *data = status & 0x0000ffff;
	  return 0;
	}
      timeout--;
    }

  (void) fputs ("eeprom read timeout!\n", stderr);

  return 1;
}

static int
vt_ar5211_eeprom_write (unsigned char *mem,
			unsigned int offset, unsigned short int new_data)
{
  int timeout = 10000;
  unsigned long int status;
  unsigned long int pcicfg;
  int i;
  unsigned short int sdata;

	/** enable eeprom access */
  pcicfg = VT_WLAN_IN32 (AR5K_PCICFG);
  VT_WLAN_OUT32 ((pcicfg & ~AR5K_PCICFG_SPWR_DN), AR5K_PCICFG);
  usleep (500);
  VT_WLAN_OUT32 (pcicfg | AR5K_PCICFG_EEAE /* | 0x2 */ , AR5K_PCICFG);
  usleep (50);

  VT_WLAN_OUT32 (0, AR5211_EEPROM_STATUS);
  usleep (50);

  /* VT_WLAN_OUT32( 0x1, AR5211_EEPROM_CONF ) ; */
  VT_WLAN_OUT32 (0x0, AR5211_EEPROM_CONF);
  usleep (50);

  i = 100;
retry:
	/** enable eeprom write access */
  VT_WLAN_OUT32 (AR5211_EEPROM_COMD_RESET, AR5211_EEPROM_COMD);
  usleep (500);

  /* Write data */
  VT_WLAN_OUT32 (new_data, AR5211_EEPROM_DATA);
  usleep (5);

	/** set address */
  VT_WLAN_OUT32 (offset, AR5211_EEPROM_ADDR);
  usleep (5);

  VT_WLAN_OUT32 (AR5211_EEPROM_COMD_WRITE, AR5211_EEPROM_COMD);
  usleep (5);

  for (timeout = 10000; timeout > 0; --timeout)
    {
      status = VT_WLAN_IN32 (AR5211_EEPROM_STATUS);
      if (status & 0xC)
	{
	  if (status & AR5211_EEPROM_STAT_WRERR)
	    {
	      // fprintf (stderr, "eeprom write access failed!\n");
	      return 1;
	    }
	  VT_WLAN_OUT32 (0, AR5211_EEPROM_STATUS);
	  usleep (10);
	  break;
	}
      usleep (10);
      timeout--;
    }
  (void) vt_ar5211_eeprom_read (mem, offset, &sdata);
  if ((sdata != new_data) && i)
    {
      --i;
      // fprintf (stderr, "Retrying eeprom write!\n");
      goto retry;
    }

  return !i;
}

#define ATHEROS_PCI_MEM_SIZE 0x10000

static int
get_regdomain (unsigned long int base_addr)
{
  int fd;
  void *membase;
  unsigned short int sdata;
  fd = open ("/dev/mem", O_RDWR);
  if (fd < 0)
    {
      // fprintf (stderr, "Open of /dev/mem failed!\n");
      return -2;
    }
  membase = mmap (0, ATHEROS_PCI_MEM_SIZE, PROT_READ | PROT_WRITE,
		  MAP_SHARED | MAP_FILE, fd, base_addr);
  if (membase == (void *) -1)
    {
      // fprintf (stderr,
//             "Mmap of device at 0x%08X for 0x%X bytes failed!\n",
//             base_addr, ATHEROS_PCI_MEM_SIZE);
      return -3;
    }
  if (vt_ar5211_eeprom_read ((unsigned char *) membase, 0xBF, &sdata))
    {
      //  fprintf (stderr, "EEPROM read failed\n");
      return -1;
    }
  close (fd);
  return sdata;


}

static int
set_regdomain (unsigned long int base_addr, int code)
{
  int fd;
  void *membase;
  unsigned short int sdata;
  unsigned short int new_cc;


  if (code > 0xFFFF)
    {
      (void) fputs ("Error: New domain code must be 16 bits or less\n",
		    stderr);
      return -2;
    }
  new_cc = (unsigned short int) code;
  fd = open ("/dev/mem", O_RDWR);
  if (fd < 0)
    {
      // fprintf (stderr, "Open of /dev/mem failed!\n");
      return -2;
    }
  membase = mmap (0, ATHEROS_PCI_MEM_SIZE, PROT_READ | PROT_WRITE,
		  MAP_SHARED | MAP_FILE, fd, base_addr);
  if (membase == (void *) -1)
    {
      // fprintf (stderr,
//             "Mmap of device at 0x%08X for 0x%X bytes failed!\n",
//             base_addr, ATHEROS_PCI_MEM_SIZE);
      return -3;
    }

//#if 0
  (void) vt_ar5211_eeprom_write ((unsigned char *) membase,
				 AR5K_EEPROM_PROTECT_OFFSET, 0);
//#endif /* #if 0 */
  int errcode = 0;
  if (vt_ar5211_eeprom_read ((unsigned char *) membase, 0xBF, &sdata))
    {
      //  fprintf (stderr, "EEPROM read failed\n");
      errcode = -4;
      close (fd);
      return errcode;
    }
  printf ("Current value 0x%04X will change to 0x%04X\n", sdata, new_cc);

  if (vt_ar5211_eeprom_write ((unsigned char *) membase, 0xBF, new_cc))
    {
      //  fprintf (stderr, "EEPROM write failed\n");
      errcode = -4;
      close (fd);
      return errcode;
    }

  if (vt_ar5211_eeprom_read ((unsigned char *) membase, 0xBF, &sdata))
    {
      //  fprintf (stderr, "EEPROM read failed\n");
      errcode = -4;
      close (fd);
      return errcode;
    }

  if (sdata != new_cc)
    {
      //  fprintf (stderr, "Write & read dont match 0x%04X != 0x%04X\n",
//             new_cc, sdata);
      errcode = -4;
    }
  close (fd);
  return errcode;
}

#endif




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
	IOCTL_ERR (IEEE80211_IOCTL_WDSDELMAC),
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
  FILE *fd;

  snprintf (buffer, sizeof (buffer), "echo %li > /proc/sys/dev/%s/%s", value,
	    dev, control);

  system2 (buffer);
  /* fd = fopen (buffer, "w");
     if (fd != NULL)
     {
     fprintf (fd, "%li", value);
     } */
  return 0;
}

static void
setdistance (char *device, int distance)
{
  if (distance >= 0)
    {
      int slottime = 9 + (distance / 300) + ((distance % 300) ? 1 : 0);
      int acktimeout = slottime * 2 + 3;
      int ctstimeout = slottime * 2 + 3;

//              printf("Setting distance on interface %s to %i meters\n", device, distance);
      setsysctrl (device, "slottime", slottime);
      setsysctrl (device, "acktimeout", acktimeout);
      setsysctrl (device, "ctstimeout", ctstimeout);
    }
}

//returns the number of installed atheros devices/cards

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
      sprintf (dev, "wdsath%s.%d", ifname, s);
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
  destroy_wds (dev);

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


/*
 * Convert MHz frequency to IEEE channel number.
 */
u_int
ieee80211_mhz2ieee (u_int freq)
{
  if (freq == 2484)
    return 14;
  if (freq < 2484)
    return (freq - 2407) / 5;
  if (freq < 5000)
    return 15 + ((freq - 2512) / 20);
  return (freq - 5000) / 5;
}





static int need_commit = 0;

char *
default_get (char *var, char *def)
{
  char *v = nvram_get (var);
  if (v == NULL || strlen (v) == 0)
    {
      nvram_set (var, def);
      need_commit = 1;
    }
  return nvram_safe_get (var);
}

int
default_match (char *var, char *match, char *def)
{
  char *v = nvram_get (var);
  if (v == NULL || strlen (v) == 0)
    {
      nvram_set (var, def);
      need_commit = 1;
    }
  return nvram_match (var, match);
}



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
MADWIFI Encryption Setup
*/
void
setupSupplicant (char *prefix)
{
  char akm[16];
  sprintf (akm, "%s_akm", prefix);
  char wmode[16];
  sprintf (wmode, "%s_mode", prefix);
  if (nvram_match (akm, "wep"))
    {
      char key[16];
      int cnt = 1;
      int i;
      char bul[8];
      for (i = 1; i < 5; i++)
	{
	  sprintf (key, "%s_key%d", prefix, i);
	  char *athkey = nvram_safe_get (key);
	  if (athkey != NULL && strlen (athkey) > 0)
	    {
	      sprintf (bul, "[%d]", cnt++);
	      eval ("iwconfig", prefix, "key", bul, athkey);	// setup wep encryption key
	    }
	}
      sprintf (key, "%s_key", prefix);
      sprintf (bul, "[%s]", nvram_safe_get (key));
      eval ("iwconfig", prefix, "key", bul);
//      eval ("iwpriv", prefix, "authmode", "2");
    }
  else
    if (nvram_match (akm, "psk") ||
	nvram_match (akm, "psk2") || nvram_match (akm, "psk psk2"))
    {
      char fstr[32];
      char psk[16];
      sprintf (fstr, "/tmp/%s_wpa_supplicant.conf", prefix);
      FILE *fp = fopen (fstr, "wb");
#ifdef HAVE_MAKSAT
      fprintf (fp, "ap_scan=1\n");
#elif HAVE_NEWMEDIA
      fprintf (fp, "ap_scan=1\n");
#else
      fprintf (fp, "ap_scan=2\n");
#endif
      fprintf (fp, "fast_reauth=1\n");
      fprintf (fp, "eapol_version=1\n");
      fprintf (fp, "ctrl_interface_group=0\n");
      fprintf (fp, "ctrl_interface=/var/run/wpa_supplicant\n");

      fprintf (fp, "network={\n");
      sprintf (psk, "%s_ssid", prefix);
      fprintf (fp, "\tssid=\"%s\"\n", nvram_safe_get (psk));
//      fprintf (fp, "\tmode=0\n");
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

      sprintf (psk, "%s_wpa_psk", prefix);
      fprintf (fp, "\tpsk=\"%s\"\n", nvram_safe_get (psk));
      fprintf (fp, "}\n");
      fclose (fp);
      sprintf (psk, "-i%s", prefix);
      if (nvram_match (wmode, "wdssta") || nvram_match (wmode, "wet"))
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
#ifdef HAVE_MAKSAT
      fprintf (fp, "ap_scan=1\n");
#else
      fprintf (fp, "ap_scan=2\n");
#endif
      fprintf (fp, "fast_reauth=1\n");
      fprintf (fp, "eapol_version=1\n");
      fprintf (fp, "ctrl_interface_group=0\n");
      fprintf (fp, "ctrl_interface=/var/run/wpa_supplicant\n");
      fprintf (fp, "network={\n");
      sprintf (psk, "%s_ssid", prefix);
      fprintf (fp, "\tssid=\"%s\"\n", nvram_safe_get (psk));
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
	  fprintf (fp, "\tca_cert=/tmp/%s/ca.pem\n", prefix);
	  fprintf (fp, "\tclient_cert=/tmp/%s/user.pem\n", prefix);
	  fprintf (fp, "\tprivate_key=/tmp/%s/user.prv\n", prefix);
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
	  fprintf (fp, "\tca_cert=/tmp/%s/ca.pem\n", prefix);
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
//        sprintf (psk, "/tmp/%s", prefix);
//        mkdir (psk);
//        sprintf (psk, "/tmp/%s/ca.pem", prefix);
//        sprintf (ath, "%s_peap8021xca", prefix);
//        write_nvram (psk, ath);
//        fprintf (fp, "\tca_cert=/tmp/%s/ca.pem\n", prefix);
	}
      fprintf (fp, "}\n");
      fclose (fp);
      sprintf (psk, "-i%s", prefix);
      char bvar[32];
      sprintf (bvar, "%s_bridged", prefix);
      if (nvram_match (bvar, "1")
	  && (nvram_match (wmode, "wdssta") || nvram_match (wmode, "wet")))
	eval ("wpa_supplicant", "-b", nvram_safe_get ("lan_ifname"), "-B",
	      "-Dmadwifi", psk, "-c", fstr);
      else
	eval ("wpa_supplicant", "-B", "-Dmadwifi", psk, "-c", fstr);
    }
  else
    {
      eval ("iwconfig", prefix, "key", "off");
//      eval ("iwpriv", prefix, "authmode", "0");
    }


}

void
setupHostAP (char *prefix, int iswan)
{
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
//wep key support
  if (nvram_match (akm, "wep"))
    {
      char key[16];
      int cnt = 1;
      int i;
      char bul[8];
      for (i = 1; i < 5; i++)
	{
	  sprintf (key, "%s_key%d", prefix, i);
	  char *athkey = nvram_safe_get (key);
	  if (athkey != NULL && strlen (athkey) > 0)
	    {
	      sprintf (bul, "[%d]", cnt++);
	      eval ("iwconfig", prefix, "key", bul, athkey);	// setup wep encryption key
	    }
	}
      sprintf (key, "%s_key", prefix);
      sprintf (bul, "[%s]", nvram_safe_get (key));
      eval ("iwconfig", prefix, "key", bul);
      //  eval ("iwpriv", prefix, "authmode", "2");
    }
  else
    if (nvram_match (akm, "psk") ||
	nvram_match (akm, "psk2") ||
	nvram_match (akm, "psk psk2") ||
	nvram_match (akm, "wpa") ||
	nvram_match (akm, "wpa2") || nvram_match (akm, "wpa wpa2"))
    {
      char fstr[32];
      sprintf (fstr, "/tmp/%s_hostap.conf", prefix);
      FILE *fp = fopen (fstr, "wb");
      fprintf (fp, "interface=%s\n", prefix);
      //sprintf(buf, "rsn_preauth_interfaces=%s\n", "br0");
      char bvar[32];
      sprintf (bvar, "%s_bridged", prefix);
      if (nvram_match (bvar, "1"))
	fprintf (fp, "bridge=%s\n", getBridge (prefix));

      fprintf (fp, "driver=madwifi\n");
      fprintf (fp, "logger_syslog=-1\n");
      fprintf (fp, "logger_syslog_level=2\n");
      fprintf (fp, "logger_stdout=-1\n");
      fprintf (fp, "logger_stdout_level=2\n");
      fprintf (fp, "debug=0\n");
      fprintf (fp, "dump_file=/tmp/hostapd.dump\n");
//      fprintf (fp, "eap_server=0\n");
//      fprintf (fp, "own_ip_addr=127.0.0.1\n");
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
	  sprintf (psk, "%s_wpa_psk", prefix);
	  fprintf (fp, "wpa_passphrase=%s\n", nvram_safe_get (psk));
	  fprintf (fp, "wpa_key_mgmt=WPA-PSK\n");
	}
      else
	{
//        if (nvram_invmatch (akm, "radius"))
	  fprintf (fp, "wpa_key_mgmt=WPA-EAP\n");
//        else
//          fprintf (fp, "macaddr_acl=2\n");
	  fprintf (fp, "ieee8021x=1\n");
//        fprintf (fp, "accept_mac_file=/tmp/hostapd.accept\n");
//        fprintf (fp, "deny_mac_file=/tmp/hostapd.deny\n");
	  fprintf (fp, "own_ip_addr=%s\n", nvram_safe_get ("lan_ipaddr"));
	  fprintf (fp, "eap_server=0\n");
	  fprintf (fp, "auth_algs=1\n");
	  fprintf (fp, "radius_retry_primary_interval=60\n");
	  sprintf (psk, "%s_radius_ipaddr", prefix);
	  fprintf (fp, "auth_server_addr=%s\n", nvram_safe_get (psk));

	  sprintf (psk, "%s_radius_port", prefix);
	  fprintf (fp, "auth_server_port=%s\n", nvram_safe_get (psk));

	  sprintf (psk, "%s_radius_key", prefix);
	  fprintf (fp, "auth_server_shared_secret=%s\n",
		   nvram_safe_get (psk));
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
	  sprintf (psk, "%s_wpa_gtk_rekey", prefix);
	  fprintf (fp, "wpa_group_rekey=%s\n", nvram_safe_get (psk));
	}
//      fprintf (fp, "jumpstart_p1=1\n");
      fclose (fp);
      eval ("hostapd", "-B", fstr);
    }
  else if (nvram_match (akm, "radius"))
    {
      //  wrt-radauth $IFNAME $server $port $share $override $mackey $maxun &
      char *ifname = prefix;
      sprintf (psk, "%s_radius_ipaddr", prefix);
      char *server = nvram_safe_get (psk);
      sprintf (psk, "%s_radius_port", prefix);
      char *port = nvram_safe_get (psk);
      sprintf (psk, "%s_radius_key", prefix);
      char *share = nvram_safe_get (psk);
      char exec[64];
      char type[32];
      sprintf (type, "%s_radmactype", prefix);
      char *pragma = "";
      if (default_match (type, "0", "0"))
	pragma = "-n1 ";
      if (nvram_match (type, "1"))
	pragma = "-n2 ";
      if (nvram_match (type, "2"))
	pragma = "-n3 ";
      if (nvram_match (type, "3"))
	pragma = "";
      sleep (1);		//some delay is usefull
      sprintf (exec, "wrt-radauth %s%s %s %s %s 1 1 0 &", pragma, prefix,
	       server, port, share);
      system2 (exec);

//    eval("wrt-radauth",prefix,server,port,share,"1","1","0");


    }
  else
    {
      eval ("iwconfig", prefix, "key", "off");
//      eval ("iwpriv", prefix, "authmode", "0");
    }


}
void
start_hostapdwan (void)
{
  char ath[32];
  char wifivifs[16];
  char *next;
  char var[80];
  int c = getdevicecount ();
  int i;
  for (i = 0; i < c; i++)
    {
      sprintf (ath, "ath%d", i);
      setupHostAP (ath, 1);
      sprintf (wifivifs, "ath%d_vifs", i);
      char *vifs = nvram_safe_get (wifivifs);
      if (vifs != NULL)
	foreach (var, vifs, next)
	{
	  setupHostAP (var, 1);
	}
    }

}

static void set_rate(char *dev)
{
char rate[16];
  char net[16];
char bw[16];
  sprintf (bw, "%s_channelbw", bw);
  sprintf (net, "%s_net_mode", dev);
  sprintf (rate, "%s_rate", dev);
  char *r = default_get (rate, "0");
  char *netmode = default_get (net, "mixed");

if (nvram_match(bw,"10"))
    if (atoi(r)>24)
	{
	nvram_set(rate,"0");
	r="0";
	}
if (nvram_match(bw,"5"))
    if (atoi(r)>12)
	{
	nvram_set(rate,"0");
	r="0";
	}
  if (!strcmp (r, "0"))
    {
      if (!strcmp (netmode,"b-only"))
	eval ("iwconfig", dev, "rate", "11", "auto");
      else
	eval ("iwconfig", dev, "rate", "54", "auto");
    }
  else if (!strcmp (r, "5.5"))
    eval ("iwconfig", dev, "rate", "5500", "fixed");
  else
    eval ("iwconfig", dev, "rate", r, "fixed");

}
static void
set_netmode (char *wif, char *dev)
{
  char net[16];
  char turbo[16];
  char mode[16];
  char xr[16];
  char comp[32];
  char ff[16];
  sprintf (mode, "%s_mode", dev);
  sprintf (net, "%s_net_mode", dev);
  sprintf (turbo, "%s_turbo", dev);
  sprintf (xr, "%s_xr", dev);
  sprintf (comp,"%s_compression",dev);
  sprintf (ff,"%s_ff",dev);
  char *netmode = default_get (net, "mixed");
//  fprintf (stderr, "set netmode of %s to %s\n", net, netmode);
  cprintf ("configure net mode %s\n", netmode);

  eval ("iwconfig", dev, "channel", "0");
//  else
  {
    eval ("iwpriv", dev, "turbo", "0");
    eval ("iwpriv", dev, "xr", "0");
    if (!strcmp (netmode, "mixed"))
      eval ("iwpriv", dev, "mode", "0");
    if (!strcmp (netmode, "b-only"))
      eval ("iwpriv", dev, "mode", "2");
    if (!strcmp (netmode, "g-only"))
      {
	eval ("iwpriv", dev, "mode", "3");
	eval ("iwpriv", dev, "protmode", "0");
      }
    if (!strcmp (netmode, "bg-only"))
      {
	eval ("iwpriv", dev, "mode", "3");
	eval ("iwpriv", dev, "protmode", "1");
      }
    if (!strcmp (netmode, "a-only"))
      eval ("iwpriv", dev, "mode", "1");
  }
  if (default_match (turbo, "1", "0"))
    {
      if (nvram_match (mode, "sta") || nvram_match (mode, "wet")
	  || nvram_match (mode, "wdssta"))
	{
	  eval ("iwpriv", dev, "mode", "5");
	}
    }
  else
    {
//    fprintf(stderr,"set xr %s = %s\n",turbo,nvram_safe_get(turbo));
      char *ext = nvram_get (xr);
      if (ext)
	{
	  if (strcmp (ext, "1") == 0)
	    {
	      eval ("iwpriv", dev, "xr", "1");
	    }
	  else
	    {
	      eval ("iwpriv", dev, "xr", "0");
	    }
	}
      eval ("iwpriv", dev, "wmm", "0");

    }
if (default_match(comp,"1","0"))
    eval("iwpriv",dev,"compression","1");
    else
    eval("iwpriv",dev,"compression","0");

if (default_match(ff,"1","0"))
    eval("iwpriv",dev,"ff","1");
    else
    eval("iwpriv",dev,"ff","0");
    

}

static set_compression(int count)
{
char comp[32];
char wif[32];
sprintf(wif,"wifi%d",count);
sprintf(comp,"ath%d_compression",count);
if (default_match(comp,"1","0"))
    setsysctrl(wif,"compression",1);
else
    setsysctrl(wif,"compression",0);
}

void
setMacFilter (char *iface)
{
  char *next;
  char var[32];
  set80211param (iface, IEEE80211_PARAM_MACCMD, IEEE80211_MACCMD_FLUSH);

  char nvvar[32];
  sprintf (nvvar, "%s_macmode", iface);
  if (nvram_match (nvvar, "deny"))
    {
      set80211param (iface, IEEE80211_PARAM_MACCMD,
		     IEEE80211_MACCMD_POLICY_ALLOW);
      char nvlist[32];
      sprintf (nvlist, "%s_maclist", iface);

      foreach (var, nvram_safe_get (nvlist), next)
      {
	char ea[ETHER_ADDR_LEN];
	if (ether_atoe (var, ea))
	  {
	    struct sockaddr sa;
	    memcpy (sa.sa_data, ea, IEEE80211_ADDR_LEN);
	    do80211priv (iface, IEEE80211_IOCTL_ADDMAC, &sa,
			 sizeof (struct sockaddr));
	  }
      }
    }
  if (nvram_match (nvvar, "allow"))
    {
      set80211param (iface, IEEE80211_PARAM_MACCMD,
		     IEEE80211_MACCMD_POLICY_DENY);
      char nvlist[32];
      sprintf (nvlist, "%s_maclist", iface);

      foreach (var, nvram_safe_get (nvlist), next)
      {
	char ea[ETHER_ADDR_LEN];
	if (ether_atoe (var, ea))
	  {
	    struct sockaddr sa;
	    memcpy (sa.sa_data, ea, IEEE80211_ADDR_LEN);
	    do80211priv (iface, IEEE80211_IOCTL_ADDMAC, &sa,
			 sizeof (struct sockaddr));
	  }
      }
    }

}

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

static void
adjust_regulatory (int count)
{
  char dev[10];
  char wif[10];
  char turbo[16];
  char gain[32];
  char country[64];
  char bw[16];
  sprintf (wif, "wifi%d", count);
  sprintf (dev, "ath%d", count);
  sprintf (turbo, "%s_turbo", dev);
  sprintf (gain, "%s_antgain", dev);
  sprintf (country, "%s_regdomain", dev);
  sprintf (bw, "%s_channelbw", dev);
  default_get (country, "UNITED_STATES");
  sprintf (country, "%s_outdoor", dev);
  default_get (country, "0");
  default_get (gain, "6");
//  if (count == 0)
  {
    long tb = atol (nvram_safe_get (turbo));
    setsysctrl (wif, "turbo", tb);
    long regulatory = atol (nvram_safe_get ("ath_regulatory"));
    {
      if (regulatory == 0)
	{
	  setsysctrl (wif, "regulatory", regulatory);
	  setsysctrl (wif, "setregdomain", 0);
	  setsysctrl (wif, "outdoor", 0);
	  setsysctrl (wif, "countrycode", 0);
	  setsysctrl (wif, "antennagain", 0);
	}
      else
	{
	  sprintf (country, "%s_regdomain", dev);
	  setsysctrl (wif, "regulatory", 1);
	  setsysctrl (wif, "setregdomain",
		      getRegDomain (default_get (country, "UNITED_STATES")));
	  setsysctrl (wif, "countrycode",
		      getCountry (default_get (country, "UNITED_STATES")));
	  sprintf (country, "%s_outdoor", dev);
	  setsysctrl (wif, "outdoor", atoi (default_get (country, "0")));
	  setsysctrl (wif, "antennagain", atoi (default_get (gain, "6")));
	}
    }
  }
#ifndef HAVE_FONERA
  char *wid = nvram_safe_get (bw);
  int width = 20;
  if (wid)
    width = atoi (wid);
  char buf[64];
  setsysctrl (wif, "channelbw", (long) width);
#endif


}

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
  char turbo[16];
  sprintf (wif, "wifi%d", count);
  sprintf (dev, "ath%d", count);
  sprintf (turbo, "%s_turbo", dev);
  sprintf (wifivifs, "ath%d_vifs", count);
  sprintf (wl, "ath%d_mode", count);
  sprintf (channel, "ath%d_channel", count);
  sprintf (power, "ath%d_txpwrdbm", count);
  sprintf (sens, "ath%d_distance", count);
  sprintf (diversity, "ath%d_diversity", count);
  sprintf (txantenna, "ath%d_txantenna", count);
  sprintf (rxantenna, "ath%d_rxantenna", count);
  sprintf (athmac, "ath%d_hwaddr", count);
  //create base device
  cprintf ("configure base interface %d\n", count);
  eval ("ifconfig", wif, "up");
  sprintf (net, "%s_net_mode", dev);
  if (nvram_match (net, "disabled"))
    return;
  if (!count)
    strcpy (iflist, dev);
  set_compression(count);
//create wds interface(s)
  int s;



  char *m;
  char *first = NULL;
  int vif = 0;
  char *vifs = nvram_safe_get (wifivifs);
  if (vifs != NULL)
    foreach (var, vifs, next)
    {
      sprintf (mode, "%s_mode", var);
      m = default_get (mode, "ap");
      //create device
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
	  if (!first)
	    first = var;
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


//create original primary interface
  m = default_get (wl, "ap");
  cprintf ("mode %s\n", m);
  if (!first)
    first = dev;
  if (!strcmp (m, "wet") || !strcmp (m, "wdssta") || !strcmp (m, "sta"))
    {
      if (vif)
	eval ("wlanconfig", dev, "create", "wlandev", wif, "wlanmode", "sta",
	      "nosbeacon");
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
	  eval ("wlanconfig", wdsdev, "create", "wlandev", wif, "wlanmode",
		"wds");
//        eval ("ifconfig",wdsdev,"0.0.0.0","up");
	  eval ("iwpriv", wdsdev, "wds_add", hwaddr);
	  eval ("iwpriv", wdsdev, "wds", "1");
	}
    }



  cprintf ("detect maxpower\n");
  m = default_get (wl, "ap");
  char maxp[16];

  //confige net mode
  set_netmode (wif, dev);


  if (strcmp (m, "sta") && strcmp (m, "wdssta") && strcmp (m, "wet"))
    {
      cprintf ("set channel\n");
      char *ch = default_get (channel, "0");
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

  
  
  char macaddr[32];
  getMacAddr (dev, macaddr);
  nvram_set (athmac, macaddr);

  cprintf ("adjust sensitivity\n");

  int distance = atoi (default_get (sens, "2000"));	//to meter
  if (distance > 0)
    setdistance (wif, distance);	//sets the receiver sensitivity
  int rx = atoi (default_get (rxantenna, "1"));
  int tx = atoi (default_get (txantenna, "1"));
  int diva = atoi (default_get (diversity, "0"));

  setsysctrl (wif, "diversity", diva);
  setsysctrl (wif, "rxantenna", rx);
  setsysctrl (wif, "txantenna", tx);

//setup vif interfaces first

  vifs = nvram_safe_get (wifivifs);
  if (vifs != NULL)
    foreach (var, vifs, next)
    {
      sprintf (net, "%s_net_mode", var);
      if (nvram_match (net, "disabled"))
	continue;
      sprintf (ssid, "%s_ssid", var);
      sprintf (mode, "%s_mode", var);
      m = default_get (mode, "ap");

      eval ("iwpriv", var, "bgscan", "0");
#ifdef HAVE_MAKSAT
      eval ("iwconfig", var, "essid", default_get (ssid, "maksat_vap"));
#else
      eval ("iwconfig", var, "essid", default_get (ssid, "dd-wrt_vap"));
#endif
      cprintf ("set broadcast flag vif %s\n", var);	//hide ssid
      sprintf (broadcast, "%s_closed", var);
      eval ("iwpriv", var, "hide_ssid", default_get (broadcast, "0"));
      char isolate[32];
      sprintf (isolate, "%s_ap_isolate", var);
      if (default_match (isolate, "1", "0"))
	eval ("iwpriv", var, "ap_bridge", "0");
      if (!strcmp (m, "wdssta") || !strcmp (m, "wdsap"))
	eval ("iwpriv", var, "wds", "1");
      cnt++;
    }



  if (!strcmp (m, "wdssta") || !strcmp (m, "wdsap"))
    eval ("iwpriv", dev, "wds", "1");
  char isolate[32];
  sprintf (isolate, "%s_ap_isolate", dev);
  if (default_match (isolate, "1", "0"))
    eval ("iwpriv", dev, "ap_bridge", "0");


  sprintf (ssid, "ath%d_ssid", count);
  sprintf (broadcast, "ath%d_closed", count);


  memset (var, 0, 80);

  cprintf ("set ssid\n");
#ifdef HAVE_MAKSAT
  eval ("iwconfig", dev, "essid", default_get (ssid, "maksat"));
#else
  eval ("iwconfig", dev, "essid", default_get (ssid, "dd-wrt"));
#endif
  cprintf ("set broadcast flag\n");	//hide ssid
  eval ("iwpriv", dev, "hide_ssid", default_get (broadcast, "0"));
  eval ("iwpriv", dev, "bgscan", "0");
  m = default_get (wl, "ap");


  char preamble[32];
  sprintf (preamble, "%s_preamble", dev);
  if (default_match (preamble, "1", "0"))
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
      eval ("iwconfig", dev, "essid", default_get (ssid, "maksat"));
#else
      eval ("iwconfig", dev, "essid", default_get (ssid, "dd-wrt"));
#endif
    }

  if (strcmp (m, "sta") && strcmp (m, "wdssta") && strcmp (m, "wet"))
    {
      cprintf ("set channel\n");
      char *ch = default_get (channel, "0");
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


//  int maxpower = getMaxPower (dev);
//  if (maxpower == -1)
//    maxpower = 28;
//  sprintf (maxp, "%d", maxpower);     //set maximum power 
//  char max_power[32];
//  sprintf (max_power, "%s_maxpower", dev);
//  cprintf ("maxpower configured to %s\n", maxp);
//  nvram_set (max_power, maxp);

  cprintf ("adjust power\n");

  int newpower = atoi (default_get (power, "16"));
  // fprintf (stderr, "new power limit %d\n", newpower);
  sprintf (var, "%ddBm", newpower);
  eval ("iwconfig", dev, "txpower", var);

  cprintf ("done()\n");

  cprintf ("setup encryption");
//@todo ifup
//netconfig





  if (strcmp (m, "sta") && strcmp (m, "infra"))
    {
      char bridged[32];
      sprintf (bridged, "%s_bridged", dev);
      if (default_match (bridged, "1", "1"))
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
	  ifconfig (dev, IFUP, nvram_safe_get (ip), nvram_safe_get (mask));
	}
    }

// vif netconfig
  vifs = nvram_safe_get (wifivifs);
  if (vifs != NULL)
    foreach (var, vifs, next)
    {
      sprintf (mode, "%s_mode", var);
      m = default_get (mode, "ap");

      if (strcmp (m, "sta") && strcmp (m, "infra"))
	{
	  char bridged[32];
	  sprintf (bridged, "%s_bridged", var);
	  if (default_match (bridged, "1", "1"))
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
	      ifconfig (var, IFUP, nvram_safe_get (ip),
			nvram_safe_get (mask));
	    }
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
  //setup encryption
  m = default_get (wl, "ap");

  if (strcmp (m, "sta") && strcmp (m, "wdssta") && strcmp (m, "wet"))
    setupHostAP (dev, 0);
  else
    setupSupplicant (dev);
  vifs = nvram_safe_get (wifivifs);
  if (vifs != NULL)
    foreach (var, vifs, next)
    {
      sprintf (mode, "%s_mode", var);
      m = default_get (mode, "ap");
      if (strcmp (m, "sta") && strcmp (m, "wdssta") && strcmp (m, "wet"))
	setupHostAP (var, 0);
      else
	setupSupplicant (var);
      setMacFilter (var);
    }
  setMacFilter (dev);
  set_rate(dev);
}


void
configure_wifi (void)		//madwifi implementation for atheros based cards
{
  deconfigure_wifi ();
/*int s;
int existed=0;
for (s=0;s<10;s++)
{
char wif[32];
sprintf(wif,"wifi%d",s);
if (ifexists(wif))
    {
    eval("ifconfig",wif,"down");
    existed=1;
    }
}
#if defined(HAVE_FONERA) || defined(HAVE_WHRAG108)
eval("rmmod","ath_ahb");
eval ("insmod", "ath_ahb", "autocreate=none");
#else
eval("rmmod","ath_pci");
eval ("insmod", "ath_pci", "autocreate=none");
#endif
for (s=0;s<10;s++)
{
char wif[32];
sprintf(wif,"wifi%d",s);
if (ifexists(wif))
    eval("ifconfig",wif,"up");
}
*/

  //bridge the virtual interfaces too
  memset (iflist, 0, 1024);
/*
  char countrycode[64];
  char xchanmode[64];
  char outdoor[64];

  if (strlen (nvram_safe_get ("wl_countrycode")) > 0)
    sprintf (countrycode, "countrycode=%s",
	     nvram_safe_get ("wl_countrycode"));
  else
    sprintf (countrycode, "countrycode=0");

  if (strlen (nvram_safe_get ("wl_xchanmode")) > 0)
    sprintf (xchanmode, "xchanmode=%s", nvram_safe_get ("wl_xchanmode"));
  else
    sprintf (xchanmode, "xchanmode=0");

  if (strlen (nvram_safe_get ("wl_outdoor")) > 0)
    sprintf (outdoor, "outdoor=%s", nvram_safe_get ("wl_outdoor"));
  else
    sprintf (outdoor, "outdoor=0");
*/


  int c = getdevicecount ();
  int i;
  int changed = 0;
  for (i = 0; i < c; i++)
    adjust_regulatory (i);

  for (i = 0; i < c; i++)
    {
#ifdef REGDOMAIN_OVERRIDE
      // SeG's dirty hack to make everything possible without any channel restrictions. regdomain 0x60 seems to be the best way
      char regdomain[16];
      sprintf (regdomain, "ath%d_regdomain", i);

      // read current reg domain from atheros card
      // the base io 0x50010000 is hardcoded here and can be different on non RB500 ports
      // @fixme: detect io by reading pci data

      cprintf ("get reg domain()\n");
      int reg_domain = get_regdomain ((0x50010000) + (0x10000 * i));
      if (reg_domain > -1)	//reg domain was successfully readed 
	{
	  if (nvram_get (regdomain) != NULL)	// reg domain is defined in nvram
	    {
	      int destination = atoi (nvram_safe_get (regdomain));	// read new target regdomain
	      if (destination != reg_domain)	//check if changed
		{
		  if (set_regdomain ((0x50010000) + (0x10000 * i), destination) == 0)	//modifiy eeprom with new regdomain
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

  if (changed)			// if changed, deconfigure myself and reconfigure me in the same way. 
    {
      deconfigure_wifi ();
      configure_wifi ();
    }
  if (need_commit)
    {
      nvram_commit ();
      need_commit = 0;
    }
}
#endif


//test functions only

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
