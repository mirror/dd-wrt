/*
 * madwifi.c
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
#include <fcntl.h>

#define AR5K_PCICFG 0x4010
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
	      fprintf (stderr, "eeprom write access failed!\n");
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
      fprintf (stderr, "Retrying eeprom write!\n");
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
      fprintf (stderr, "Open of /dev/mem failed!\n");
      return -2;
    }
  membase = mmap (0, ATHEROS_PCI_MEM_SIZE, PROT_READ | PROT_WRITE,
		  MAP_SHARED | MAP_FILE, fd, base_addr);
  if (membase == (void *) -1)
    {
      fprintf (stderr,
	       "Mmap of device at 0x%08X for 0x%X bytes failed!\n",
	       base_addr, ATHEROS_PCI_MEM_SIZE);
      return -3;
    }
  if (vt_ar5211_eeprom_read ((unsigned char *) membase, 0xBF, &sdata))
    {
      fprintf (stderr, "EEPROM read failed\n");
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
      fprintf (stderr, "Open of /dev/mem failed!\n");
      return -2;
    }
  membase = mmap (0, ATHEROS_PCI_MEM_SIZE, PROT_READ | PROT_WRITE,
		  MAP_SHARED | MAP_FILE, fd, base_addr);
  if (membase == (void *) -1)
    {
      fprintf (stderr,
	       "Mmap of device at 0x%08X for 0x%X bytes failed!\n",
	       base_addr, ATHEROS_PCI_MEM_SIZE);
      return -3;
    }

#if 0
  (void) vt_ar5211_eeprom_write ((unsigned char *) membase,
				 AR5K_EEPROM_PROTECT_OFFSET, 0);
#endif /* #if 0 */
  int errcode = 0;
  if (vt_ar5211_eeprom_read ((unsigned char *) membase, 0xBF, &sdata))
    {
      fprintf (stderr, "EEPROM read failed\n");
      errcode = -4;
      close (fd);
      return errcode;
    }
  printf ("Current value 0x%04X will change to 0x%04X\n", sdata, new_cc);

  if (vt_ar5211_eeprom_write ((unsigned char *) membase, 0xBF, new_cc))
    {
      fprintf (stderr, "EEPROM write failed\n");
      errcode = -4;
      close (fd);
      return errcode;
    }

  if (vt_ar5211_eeprom_read ((unsigned char *) membase, 0xBF, &sdata))
    {
      fprintf (stderr, "EEPROM read failed\n");
      errcode = -4;
      close (fd);
      return errcode;
    }

  if (sdata != new_cc)
    {
      fprintf (stderr, "Write & read dont match 0x%04X != 0x%04X\n",
	       new_cc, sdata);
      errcode = -4;
    }
  close (fd);
  return errcode;
}






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



static int
setsysctrl (const char *dev, const char *control, u_long value)
{
  char buffer[256];
  FILE *fd;

  snprintf (buffer, sizeof (buffer), "/proc/sys/dev/%s/%s", dev, control);
  fd = fopen (buffer, "w");
  if (fd != NULL)
    {
      fprintf (fd, "%li", value);
    }
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
int
getdevicecount (void)
{
  eval ("ifconfig", "wifi0", "up");
  eval ("ifconfig", "wifi1", "up");
  eval ("ifconfig", "wifi2", "up");
  eval ("ifconfig", "wifi3", "up");
  eval ("ifconfig", "wifi4", "up");
  eval ("ifconfig", "wifi5", "up");

  system ("/sbin/ifconfig|grep wifi|wc -l>/tmp/.counts");
  FILE *in = fopen ("/tmp/.counts", "rb");
  if (in == NULL)
    return 0;
  int count;
  fscanf (in, "%d", &count);
  fclose (in);
//  unlink ("/tmp/.counts");
  if (count < 7 && count > 0)
    return count;
  return 0;
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
  eval ("brctl", "delif", "br0", dev);
  eval ("ifconfig", dev, "down");
  eval ("wlanconfig", dev, "destroy");
  char *vifs = nvram_safe_get (wifivifs);
  if (vifs != NULL)
    foreach (var, vifs, next)
    {
      eval ("wlanconfig", var, "destroy");
    }
  char buf[16];
  sprintf (buf, "wifi%d", count);
  eval ("ifconfig", buf, "down");
}

void
deconfigure_wifi (void)
{
  int c = getdevicecount ();
  int i;
  for (i = 0; i < c; i++)
    deconfigure_single (i);
  eval ("rmmod", "wlan_scan_sta");
  eval ("rmmod", "wlan_scan_ap");
  eval ("rmmod", "wlan_xauth");
  eval ("rmmod", "wlan_wep");
  eval ("rmmod", "wlan_tkip");
  eval ("rmmod", "wlan_ccmp");
  eval ("rmmod", "wlan_acl");
  eval ("modprobe", "-r", "ath_pci");

}

struct wifi_channels
{
  int channel;
  int freq;
};

/*
 * Convert MHz frequency to IEEE channel number.
 */
static u_int
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

static int
getsocket (void)
{
  static int s = -1;

  if (s < 0)
    {
      s = socket (AF_INET, SOCK_DGRAM, 0);
      if (s < 0)
	err (1, "socket(SOCK_DGRAM)");
    }
  return s;
}

#define IOCTL_ERR(x) [x - SIOCIWFIRSTPRIV] "ioctl[" #x "]"
static int
do80211priv (struct iwreq *iwr, const char *ifname, int op, void *data,
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


static int
get80211priv (const char *ifname, int op, void *data, size_t len)
{
  struct iwreq iwr;

  if (do80211priv (&iwr, ifname, op, data, len) < 0)
    return -1;
  if (len < IFNAMSIZ)
    memcpy (data, iwr.u.name, len);
  return iwr.u.data.length;
}


static struct wifi_channels *
list_channelsext (const char *ifname, int allchans)
{
  struct ieee80211req_chaninfo chans;
  struct ieee80211req_chaninfo achans;
  const struct ieee80211_channel *c;
  int i, half;
  cprintf ("get priv\n");
  if (get80211priv
      (ifname, IEEE80211_IOCTL_GETCHANINFO, &chans, sizeof (chans)) < 0)
    errx (1, "unable to get channel information");
  if (!allchans)
    {
      uint8_t active[32];
      cprintf ("get priv 2\n");

      if (get80211priv
	  (ifname, IEEE80211_IOCTL_GETCHANLIST, &active, sizeof (active)) < 0)
	errx (1, "unable to get active channel list");
      cprintf ("clear achans\n");
      memset (&achans, 0, sizeof (achans));
      for (i = 0; i < chans.ic_nchans; i++)
	{
	  c = &chans.ic_chans[i];
	  if (isset (active, ieee80211_mhz2ieee (c->ic_freq)) || allchans)
	    achans.ic_chans[achans.ic_nchans++] = *c;
	}
    }
  else
    achans = chans;

  cprintf ("malloc list\n");

  struct wifi_channels *list =
    (struct wifi_channels *) malloc (sizeof (struct wifi_channels) *
				     (achans.ic_nchans + 1));

  char wl_mode[16];
  char wl_turbo[16];
  sprintf (wl_mode, "%s_net_mode", ifname);
  sprintf (wl_turbo, "%s_turbo", ifname);
  int l = 0;
  for (i = 0; i < achans.ic_nchans; i++)
    {

      cprintf ("filter a\n");
      //filter out A channels if mode isnt A-Only or mixed
      if (IEEE80211_IS_CHAN_A (&achans.ic_chans[i]))
	{
	  if (nvram_invmatch (wl_mode, "a-only")
	      && nvram_invmatch (wl_mode, "mixed"))
	    continue;
	}
      cprintf ("filter g\n");
      //filter out B/G channels if mode isnt g-only, b-only or mixed
      if (IEEE80211_IS_CHAN_G (&achans.ic_chans[i]))
	{
	  if (nvram_invmatch (wl_mode, "g-only")
	      && nvram_invmatch (wl_mode, "mixed")
	      && nvram_invmatch (wl_mode, "b-only"))
	    continue;
	}
      cprintf ("filter turbo\n");
      //filter out channels which are not supporting turbo mode if turbo is enabled
      if (!IEEE80211_IS_CHAN_STURBO (&achans.ic_chans[i])
	  && !IEEE80211_IS_CHAN_DTURBO (&achans.ic_chans[i]))
	{
	  if (nvram_match (wl_turbo, "1"))
	    continue;
	}
      cprintf ("filter noo turbo\n");
      //filter out turbo channels if turbo mode is disabled
      if (IEEE80211_IS_CHAN_STURBO (&achans.ic_chans[i])
	  || IEEE80211_IS_CHAN_DTURBO (&achans.ic_chans[i]))
	{
	  if (nvram_match (wl_turbo, "0"))
	    continue;
	}
      cprintf ("set list\n");

      list[l].channel = achans.ic_chans[i].ic_ieee;
      list[l].freq = achans.ic_chans[i].ic_freq;
      l++;
    }
  cprintf ("term\n");

  list[l].freq = -1;
  return list;
}




struct wifi_channels *
list_channels (char *devnr)
{
  return list_channelsext (devnr, 1);
/*
  char csign[64];
  char channel[64];
  char ppp[64];
  char freq[64];
  char dum1[64];
  char dum2[64];
  char dum3[64];
  char dum4[64];

  char cmd[64];
  sprintf (cmd, "iwlist %s chan>/tmp/.channels", devnr);
  system (cmd);
  FILE *in = fopen ("/tmp/.channels", "rb");
  if (in == NULL)
    return NULL;
  fscanf (in, "%s %s %s %s %s %s %s %s", csign, channel, ppp, freq, dum1,
	  dum2, dum3, dum4);
  int ch = atoi (channel);
  int i;
  struct wifi_channels *list =
    (struct wifi_channels *) malloc (sizeof (struct wifi_channels) * (ch+1) );
  for (i = 0; i < ch; i++)
    {
      fscanf (in, "%s %s %s %s %s", csign, channel, ppp, freq, dum1);
      if (!strcmp (csign, "Current"))
	break;
      list[i].channel = atoi (channel);
      list[i].freq = strdup (freq);
      channelcount++;
    }
  fclose (in);
  return list;
*/
}


char *
default_get (char *var, char *def)
{
  char *v = nvram_get (var);
  if (v == NULL || strlen (v) == 0)
    {
      nvram_set (var, def);
      
      nvram_commit ();
    }
  return nvram_safe_get (var);
}

char *
default_match (char *var, char *match, char *def)
{
  char *v = nvram_get (var);
  if (v == NULL || strlen (v) == 0)
    {
      nvram_set (var, def);
      nvram_commit ();
    }
  return nvram_match (var, match);
}

/*
MADWIFI Encryption Setup
*/
void
setupEncryption (char *prefix)
{
  char akm[16];
  sprintf (akm, "%s_akm", prefix);
//wep key support
  if (nvram_match(akm, "wep"))
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
     sprintf(key,"%s_key",prefix);
     sprintf(bul,"[%s]",nvram_safe_get(key));
     eval ("iwconfig",prefix,"key",bul);	
     eval ("iwpriv", prefix, "authmode", "2");
    }
  else
    {
      eval ("iwconfig", prefix, "key", "off");
      eval ("iwpriv", prefix,"authmode","0");
    }


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
  char turbo[16];
  char channel[16];
  char ssid[16];
  char net[16];
  char wifivifs[16];
  char broadcast[16];
  char power[16];
  char sens[16];
  char basedev[16];
  sprintf (wifivifs, "ath%d_vifs", count);
  sprintf (dev, "ath%d", count);
  sprintf (wif, "wifi%d", count);
  sprintf (wl, "ath%d_mode", count);
  sprintf (turbo, "ath%d_turbo", count);
  sprintf (channel, "ath%d_channel", count);
  sprintf (ssid, "ath%d_ssid", count);
  sprintf (net, "ath%d_net_mode", count);
  sprintf (broadcast, "ath%d_closed", count);
  sprintf (power, "ath%d_txpwr", count);
  sprintf (sens, "ath%d_sens", count);
  //create base device
  cprintf ("configure base interface %d\n", count);

  char *m = default_get (wl, "ap");
  cprintf ("mode %s\n", m);
  if (!strcmp (m, "wet"))
    eval ("wlanconfig", dev, "create", "wlandev", wif, "wlanmode", "sta");
  else
    eval ("wlanconfig", dev, "create", "wlandev", wif, "wlanmode", m);
  //confige net mode
  char *netmode = default_get (net, "mixed");


  cprintf ("configure net mode %s\n", netmode);

  if (!strcmp (netmode, "mixed"))
    eval ("iwpriv", dev, "mode", "0");
  if (!strcmp (netmode, "b-only"))
    eval ("iwpriv", dev, "mode", "2");
  if (!strcmp (netmode, "g-only"))
    eval ("iwpriv", dev, "mode", "3");
  if (!strcmp (netmode, "a-only"))
    eval ("iwpriv", dev, "mode", "1");

  cprintf ("configure turbo\n");
  if (default_match (turbo, "1", "0"))
    {
      eval ("iwpriv", dev, "mode", "5");
      eval ("iwpriv", dev, "turbo", "1");
    }
  else
    eval ("iwpriv", dev, "turbo", "0");

  if (strcmp (m, "sta"))
    {
      cprintf ("set channel\n");
      char *ch = default_get (channel, "0");
      if (strcmp (ch, "0") == 0)
	eval ("iwconfig", dev, "channel", "auto");
      else
	eval ("iwconfig", dev, "channel", ch);
    }
  cprintf ("set ssid\n");
  eval ("iwconfig", dev, "essid", default_get (ssid, "default"));

  cprintf ("set broadcast flag\n");	//hide ssid
  eval ("iwpriv", dev, "hide_ssid", default_get (broadcast, "0"));

  cprintf ("adjust power\n");
  sprintf (var, "%dmW", default_get (power, "28"));
  eval ("iwconfig", dev, "txpower", var);

  cprintf ("adjust sensitivity\n");

  int distance = atoi (default_get (sens, "20")) * 1000;	//to meter
  setdistance (wif, distance);	//sets the receiver sensitivity
  memset (var, 0, 80);

  cprintf ("setup encryption");
  setupEncryption (dev);
//@todo ifup
  eval ("ifconfig", dev, "0.0.0.0", "up");

  eval ("brctl", "addif", "br0", dev);

  char *vifs = nvram_safe_get (wifivifs);
  if (vifs != NULL)
    foreach (var, vifs, next)
    {
      //create device
      sprintf (mode, "%s_mode", var);
      m = default_get (mode, "ap");
      //create device
      if (strlen (mode) > 0)
	{
	  if (!strcmp (m, "wet") || !strcmp (m, "sta"))
	    eval ("wlanconfig", var, "create", "wlandev", wif, "wlanmode",
		  "sta", "nosbeacon");
	  else
	    eval ("wlanconfig", var, "create", "wlandev", wif, "wlanmode", m);
	}
      sprintf (ssid, "%s_ssid", var);
      sprintf (channel, "%s_channel", var);

      if (strcmp (m, "sta"))
	{
	  eval ("iwconfig", var, "channel", default_get (channel, "6"));
	}
      eval ("iwconfig", var, "essid", default_get (ssid, "default"));

      cprintf ("setup encryption");
      setupEncryption (var);

      eval ("ifconfig", var, "0.0.0.0", "up");
      //ifconfig (var, IFUP, "0.0.0.0", NULL);
      eval ("brctl", "addif", "br0", var);

      //add to bridge
//                  eval ("brctl", "addif", lan_ifname, var);
      cnt++;
    }
  cprintf ("done()\n");
}

void
configure_wifi (void)		//madwifi implementation for atheros based cards
{
  //bridge the virtual interfaces too
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

/*#ifdef HAVE_RB500
  eval ("setpci","-s","0:04.0","SUBORDINATE_BUS=0A");
  eval ("setpci","-s","0:05.0","SUBORDINATE_BUS=0A");
  eval ("setpci","-s","0:06.0","SUBORDINATE_BUS=0A");
  eval ("setpci","-s","0:07.0","SUBORDINATE_BUS=0A");
  eval ("setpci","-s","0:08.0","SUBORDINATE_BUS=0A");
  eval ("setpci","-s","0:09.0","SUBORDINATE_BUS=0A");
#endif*/
//  eval ("insmod", "wlan");
//  eval ("insmod", "ath_hal");
//  eval ("insmod", "ath_rate_onoe");
//  sleep(1);
  eval ("modprobe", "ath_pci", countrycode, xchanmode, outdoor,"autocreate=none");

//  sleep(1);
//  eval ("modprobe", "-r", "ath_pci");
//  sleep(1);
//  eval ("modprobe", "ath_pci",countrycode,xchanmode,outdoor);

  eval ("insmod", "wlan_acl");
  eval ("insmod", "wlan_ccmp");
  eval ("insmod", "wlan_tkip");
  eval ("insmod", "wlan_wep");
  eval ("insmod", "wlan_xauth");
  eval ("insmod", "wlan_scan_ap");
  eval ("insmod", "wlan_scan_sta");

  int c = getdevicecount ();
  int i;
  int changed = 0;

  for (i = 0; i < c; i++)
    {

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
	configure_single (i);
    }

  if (changed)			// if changed, deconfigure myself and reconfigure me in the same way. 
    {
      deconfigure_wifi ();
      configure_wifi ();
    }

}
#endif
