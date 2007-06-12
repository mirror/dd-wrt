
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>
#include <syslog.h>
#include <wait.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <wlutils.h>
#include <wlioctl.h>
#include <rc.h>

#include <cy_conf.h>
#include <bcmutils.h>
#include <utils.h>
#include <nvparse.h>



#define WL_IOCTL(name, cmd, buf, len) (wl_ioctl((name), (cmd), (buf), (len)))

#define TXPWR_MAX 251
#define TXPWR_DEFAULT 28

#define WLAND_INTERVAL 15

#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)



static char *
get_wshaper_dev (void)
{
  if (nvram_match ("wshaper_dev", "WAN"))
    return get_wan_face ();
  else
    return "br0";
}



static int
do_ap_watchdog (void)
{

  /* AP Watchdog - experimental check and fix for hung AP */
  int val = 0;
  struct stat s;
  static time_t last;
  int interval =
    atoi (nvram_safe_get ("apwatchdog_interval")) >
    WLAND_INTERVAL ? atoi (nvram_safe_get ("apwatchdog_interval")) :
    WLAND_INTERVAL;

  system ("/usr/sbin/wl assoclist 2>&1 > /tmp/.assoclist");
  stat ("/tmp/.assoclist", &s);
  unlink ("/tmp/.assoclist");

  if (s.st_size <= 0 &&
      time (NULL) - last > interval &&
      nvram_match ("apwatchdog_enable", "1") &&
      nvram_invmatch ("wl_net_mode", "disabled"))
    {
      time (&last);
      cprintf ("resetting ap radio\n");
      eval ("/usr/sbin/wlconf", get_wdev (), "down");

      val = atoi (nvram_safe_get ("wl0_channel")) + 1;
      if (val <= 2 || val >= 14)
	val = 2;

      wl_ioctl (get_wdev (), WLC_SET_CHANNEL, &val, sizeof (val));
      wl_ioctl (get_wdev (), WLC_UP, NULL, 0);

      eval ("/usr/sbin/wlconf", get_wdev (), "down");
      wlconf_up (get_wdev ());

    }

  return 0;

}

#ifdef HAVE_AQOS
int
compareNet (char *ip, char *net, char *dest)
{
  unsigned int ip1 = atoi (strsep (&ip, "."));
  unsigned int ip2 = atoi (strsep (&ip, "."));
  unsigned int ip3 = atoi (strsep (&ip, "."));
  unsigned int ip4 = atoi (ip);

  unsigned int dip1 = atoi (strsep (&dest, "."));
  unsigned int dip2 = atoi (strsep (&dest, "."));
  unsigned int dip3 = atoi (strsep (&dest, "."));
  unsigned int dip4 = atoi (dest);

  unsigned int fullip = (ip1 << 24) + (ip2 << 16) + (ip3 << 8) + ip4;
  unsigned int dfullip = (dip1 << 24) + (dip2 << 16) + (dip3 << 8) + dip4;
  int n = 1 << atoi (net);	//convert net to full mask
  if ((dfullip & n) == (fullip & n))
    return 1;
  return 0;
}

int
containsIP (char *ip)
{
  FILE *in;
  char buf_ip[20];
  char *i, *net;
  in = fopen ("/tmp/aqos_ips", "rb");
  if (in == NULL)
    return 0;
//cprintf("scan for ip %s\n",ip);
  while (fscanf (in, "%s", buf_ip) != EOF)
    {
      i = (char *) &buf_ip[0];
      net = strsep (&i, "/");
      cprintf ("found %s/%s\n", net, i);
      if (compareNet (net, i, ip))
	{
	  printf ("%s/%s fits to %s\n", net, i, ip);
	  return 1;
	}
      printf ("%s/%s dosnt fit to %s\n", net, i, ip);
    }
  fclose (in);
//cprintf("no ip found\n");
  return 0;
}
static int qosidx=1000;

int
containsMAC (char *ip)
{
  FILE *in;
  char buf_ip[16];
  in = fopen ("/tmp/aqos_macs", "rb");
  if (in == NULL)
    return 0;
//cprintf("scan for mac %s \n",ip);
  while (fscanf (in, "%s", buf_ip) != EOF)
    {
      if (!strcmp (buf_ip, ip))
	return 1;
    }
  fclose (in);
//cprintf("no mac found\n");
  return 0;
}


static int
do_aqos_check (void)
{
  FILE *arp = fopen ("/proc/net/arp", "rb");
  char ip_buf[16];
  char hw_buf[16];
  char cmd[1024];
  char fl_buf[16];
  char mac_buf[18];
  char mask_buf[16];
  char dev_buf[16];
  char *wdev = get_wshaper_dev ();
  int cmac;
  int defaultlevel;
  char *defaulup;
  char *defauldown;
  int cip;
  if (arp == NULL)
    {
      cprintf ("/proc/net/arp missing, check kernel config\n");
      return;
    }
  defaulup = nvram_safe_get ("default_uplevel");
  defauldown = nvram_safe_get ("default_downlevel");
  if (defaulup == NULL || strlen (defaulup) == 0)
    return;
  if (defauldown == NULL || strlen (defauldown) == 0)
    return;
  while (fgetc (arp) != '\n');

//fscanf(arp,"%s %s %s %s %s %s",ip_buf,hw_buf,fl_buf,mac_buf,mask_buf,dev_buf); //skip first line
  while (fscanf
	 (arp, "%s %s %s %s %s %s", ip_buf, hw_buf, fl_buf, mac_buf, mask_buf,
	  dev_buf) != EOF)
    {
      cmac = containsMAC (mac_buf);
      cip = containsIP (ip_buf);


      if (cip || cmac)
	continue;
//cprintf("nothing found for %s %s\n",ip_buf,mac_buf);

      if (!cmac)
	{
	  char addition[64];
	  sprintf(addition,"echo %s>>/tmp/aqos_macs",cmac);
	  system(addition);
	  //create default rule for mac
	  add_usermac(mac_buf,qosidx,defaulup,defauldown);
	  qosidx+=2;

	}
      if (!cip)
	{
	  char addition[64];
	  sprintf(addition,"echo %s>>/tmp/aqos_ips",cip);
	  system(addition);
	  //create default rule for ip
	  add_userip(ip_buf,qosidx,defaulup,defauldown);
	  qosidx+=2;
	}
    }

  fclose (arp);

}
#endif
#ifndef HAVE_MADWIFI
static int
do_ap_check (void)
{

//  if (nvram_match ("apwatchdog_enable", "1"))
//    do_ap_watchdog ();
  start_service ("wds_check");
//  do_wds_check ();

  return 0;
}

int
checkbssid (void)
{
  struct ether_addr bssid;
  wl_bss_info_t *bi;
  char buf[WLC_IOCTL_MAXLEN];
  if ((WL_IOCTL (get_wdev (), WLC_GET_BSSID, &bssid, ETHER_ADDR_LEN)) == 0)
    {
      *(uint32 *) buf = WLC_IOCTL_MAXLEN;
      if ((WL_IOCTL (get_wdev (), WLC_GET_BSS_INFO, buf, WLC_IOCTL_MAXLEN)) <
	  0)
	return 0;
      bi = (wl_bss_info_t *) (buf + 4);
      int i;
      for (i = 0; i < 6; i++)
	if (bi->BSSID.octet[i] != 0)
	  return 1;
    }
  return 0;
}

/* for Client/Wet mode */
/* be nice to rewrite this to use sta_info_t if we had proper Broadcom API specs */
static int
do_client_check (void)
{
  FILE *fp = NULL;
  char buf[1024];
//  char mac[512];
  int len;

  system ("/usr/sbin/wl assoc 2>&1 > /tmp/.xassocx");
  if ((fp = fopen ("/tmp/.xassocx", "r")) == NULL)
    return -1;

  len = fread (buf, 1, 1023, fp);

  buf[len] = 0;

  if ((len > 0 && strstr (buf, "Not associated.")) || checkbssid () == 0)
    {
#ifdef HAVE_DDLAN

      nvram_unset ("cur_rssi");
      nvram_unset ("cur_noise");
      nvram_unset ("cur_bssid");
      nvram_unset ("cur_snr");
      nvram_set ("cur_state",
		 "<span style=\"background-color: rgb(255, 0, 0);\">Nicht Verbunden</span>");

#endif
      eval ("wl", "disassoc");
#ifndef HAVE_MSSID
      eval ("wl", "join", nvram_safe_get ("wl_ssid"));
#else
      eval ("wl", "join", nvram_safe_get ("wl0_ssid"));
#endif
//      join(nvram_get("wl_ssid"));
      fclose (fp);
    }
  else
    {
#ifdef HAVE_DDLAN
      nvram_set ("cur_state",
		 "<span style=\"background-color: rgb(135, 255, 51);\">Verbunden</span>");
      eval ("/sbin/check.sh");
#endif
    }
  fclose (fp);
  return 0;
}
#endif

#ifdef HAVE_MADWIFI
static int notstarted[32];
static char assoclist[24 * 1024];
static int lastchans[256];
static void
do_madwifi_check (void)
{
  int c = getdevicecount ();
  char dev[32];
  int i, s;
  for (i = 0; i < c; i++)
    {
      sprintf (dev, "ath%d", i);
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
	      int count = getassoclist (wdsdevname, &assoclist[0]);
	      if (count < 1)
		{
		  eval ("ifconfig", wdsdevname, "down");
		  sleep (1);
		  eval ("ifconfig", wdsdevname, "up");
		}
	    }
	}
      char mode[32];
      sprintf (mode, "%s_mode", dev);
      if (nvram_match (mode, "sta") || nvram_match (mode, "wdssta"))
	{
	  int chan = wifi_getchannel (dev);
//	  fprintf(stderr,"chan %d %d\n",chan,lastchans[i]);
	  if (lastchans[i] == 0 && chan<1000)
	    lastchans[i] = chan;
	  else
	    {
//	  fprintf(stderr,"chan2 %d %d\n",chan,lastchans[i]);
	      if (chan == lastchans[i])
		{
		  int count = getassoclist (dev, &assoclist[0]);
//	  fprintf(stderr,"count %d\n",count);
		  if (count == 0 || count==-1)
		    {
		      char *next;
		      char var[80];
		      char *vifs;
		      char mode[32];
		      char *m;
		      char wifivifs[32];
		      sprintf (wifivifs, "%s_vifs", dev);
		      vifs = nvram_safe_get (wifivifs);
		      if (vifs != NULL && strlen (vifs) > 0)
			{
			  foreach (var, vifs, next)
			  {
//			    fprintf(stderr,"shutting down %s\n",var);
			    eval ("ifconfig", var, "down");
			  }
			}

		      notstarted[i] = 0;
//			    fprintf(stderr,"restarting %s\n",dev);
		      eval ("ifconfig", dev, "down");
		      sleep (1);
		      eval ("ifconfig", dev, "up");

		    }
		  else if (!notstarted[i])
		    {
		      notstarted[i] = 1;
		      char *next;
		      char var[80];
		      char *vifs;
		      char mode[32];
		      char *m;
		      char wifivifs[32];

		      sprintf (wifivifs, "%s_vifs", dev);
		      vifs = nvram_safe_get (wifivifs);
		      if (vifs != NULL && strlen (vifs) > 0)
			{
			  foreach (var, vifs, next)
			  {
//		           fprintf(stderr,"restarting %s\n",var);
			    eval ("ifconfig", var, "up");
			  }
			}

		    }

		}
		lastchans[i] = chan;

	    }
	}

    }

}
#endif

#ifdef HAVE_MADWIFI
/*static HAL_MIB_STATS laststats[16];
void detectACK(void)
{
int count = getdevicecount();
int i;
int s = socket(AF_INET, SOCK_DGRAM, 0);
for (i=0;i<count;i++)
{
char wifi[16];
sprintf(wifi,"wifi%d",i);
struct ifreq ifr;
strcpy(ifr.ifr_name, wifi);
ifr.ifr_data = (caddr_t) &laststats[i];
if (ioctl(s, SIOCGATHMIBSTATS, &ifr) < 0)
    {
    fprintf(stderr,"Error while gettting mib stats\n");
    return;
    }
}

close(s);
}
*/
#endif


#ifndef HAVE_MSSID

static void
setACK (void)
{
  char *v;
  char *name = get_wdev ();
  if ((v = nvram_get ("wl0_distance")))
    {
      rw_reg_t reg;
      uint32 shm;

      int val = atoi (v);
      val = 9 + (val / 150) + ((val % 150) ? 1 : 0);

      shm = 0x10;
      shm |= (val << 16);
      WL_IOCTL (name, 197, &shm, sizeof (shm));

      reg.byteoff = 0x684;
      reg.val = val + 510;
      reg.size = 2;
      WL_IOCTL (name, 102, &reg, sizeof (reg));
    }

}
#endif

/*static void setShortSlot(void)
{
  char *shortslot = nvram_safe_get ("wl0_shortslot");

  else if (!strcmp (afterburner, "long"))
    eval ("wl", "shortslot_override", "0");
  else if (!strcmp (afterburner, "short"))
    eval ("wl", "shortslot_override", "1");

}*/

static void
do_wlan_check (void)
{
#ifdef HAVE_AQOS
  do_aqos_check ();
#endif
#ifndef HAVE_MADWIFI
  if (nvram_invmatch ("wl0_mode", "ap"))
    do_client_check ();
  else
    do_ap_check ();
#else

  do_madwifi_check ();
#endif
#ifndef HAVE_MSSID
  setACK ();
#endif

}


int
wland_main (int argc, char **argv)
{
  /* Run it in the background */
  switch (fork ())
    {
    case -1:
      // can't fork
      exit (0);
      break;
    case 0:
      /* child process */
      // fork ok
      (void) setsid ();
      break;
    default:
      /* parent process should just die */
      _exit (0);
    }
qosidx=1000;
  /* Most of time it goes to sleep */
#ifdef HAVE_MADWIFI
  memset (lastchans, 0, 256*4);
  memset (notstarted, 0, 32*4);
#endif
  while (1)
    {
      do_wlan_check ();

      sleep (WLAND_INTERVAL);
    }

  return 0;
}				// end main
