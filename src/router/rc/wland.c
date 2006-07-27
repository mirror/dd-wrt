
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

char *
get_wdev ()
{
  if (check_hw_type () == BCM4702_CHIP)
    return "eth2";
  else
    return "eth1";
}

/*
static void join(char *ss)
{
if (ss==NULL)
    return;
wlc_ssid_t ssid;
ssid.SSID_len = strlen(ss);
if (ssid.SSID_len > sizeof(ssid.SSID))
ssid.SSID_len = sizeof(ssid.SSID);
strncpy(ssid.SSID, ss, ssid.SSID_len);
wl_ioctl(get_wdev(), WLC_SET_SSID, &ssid, sizeof(ssid));
}*/

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
  char *defaul;
  int cip;
  if (arp == NULL)
    {
      cprintf ("/proc/net/arp missing, check kernel config\n");
      return;
    }
  defaul = nvram_safe_get ("default_level");

  if (defaul == NULL || strlen (defaul) == 0)
    return;
  defaultlevel = atoi (defaul) * 1000 / 8;
  while (fgetc (arp) != '\n');

//fscanf(arp,"%s %s %s %s %s %s",ip_buf,hw_buf,fl_buf,mac_buf,mask_buf,dev_buf); //skip first line
  while (fscanf
	 (arp, "%s %s %s %s %s %s", ip_buf, hw_buf, fl_buf, mac_buf, mask_buf,
	  dev_buf) != EOF)
    {
      cmac = containsMAC (mac_buf);
      cip = containsIP (ip_buf);

      snprintf (cmd, 1023,
		"/usr/sbin/iptables -t mangle -D PREROUTING -i br0 -m mac --mac-source %s -m connrate --connrate ! 0:%d -j DROP > /dev/null",
		mac_buf, defaultlevel);
      system (cmd);
      snprintf (cmd, 1023,
		"/usr/sbin/iptables -t mangle -D PREROUTING -i br0 -s %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null",
		ip_buf, defaultlevel);
      system (cmd);
      snprintf (cmd, 1023,
		"/usr/sbin/iptables -t mangle -D PREROUTING -i br0 -d %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null",
		ip_buf, defaultlevel);
      system (cmd);
      snprintf (cmd, 1023,
		"/usr/sbin/iptables -t mangle -D POSTROUTING -s %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null",
		ip_buf, defaultlevel);
      system (cmd);
      snprintf (cmd, 1023,
		"/usr/sbin/iptables -t mangle -D POSTROUTING -d %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null",
		ip_buf, defaultlevel);
      system (cmd);
//snprintf(cmd, 1023, "/usr/sbin/iptables -t nat -D PREROUTING -s %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null", mac_buf, default_level);
//system(cmd);
//snprintf(cmd, 1023, "/usr/sbin/iptables -t nat -D POSTROUTING -d %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null", mac_buf, default_level);
//system(cmd);

      if (cip || cmac)
	continue;
//cprintf("nothing found for %s %s\n",ip_buf,mac_buf);

      if (!cmac)
	{
	  //create default rule for mac
	  snprintf (cmd, 1023,
		    "/usr/sbin/iptables -t mangle -A PREROUTING -i br0 -m mac --mac-source %s -m connrate --connrate ! 0:%d -j DROP > /dev/null",
		    mac_buf, defaultlevel);
	  system (cmd);

	  //   snprintf(cmd, 1023, "/usr/sbin/iptables -t nat -A PREROUTING -s %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null", data, ilevel);
	  //    system(cmd);


//    snprintf(cmd, 1023, "/usr/sbin/iptables -t nat -A POSTROUTING -d %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null", data, ilevel);
//    system(cmd);

	}
      if (!cip)
	{
	  //create default rule for ip
	  snprintf (cmd, 1023,
		    "/usr/sbin/iptables -t mangle -A PREROUTING -i br0 -s %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null",
		    ip_buf, defaultlevel);
	  system (cmd);

	  snprintf (cmd, 1023,
		    "/usr/sbin/iptables -t mangle -A PREROUTING -i br0 -d %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null",
		    ip_buf, defaultlevel);
	  system (cmd);

	  snprintf (cmd, 1023,
		    "/usr/sbin/iptables -t mangle -A POSTROUTING -s %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null",
		    ip_buf, defaultlevel);
	  system (cmd);

	  snprintf (cmd, 1023,
		    "/usr/sbin/iptables -t mangle -A POSTROUTING -d %s -m connrate --connrate ! 0:%d -j DROP 2>&1 > /dev/null",
		    ip_buf, defaultlevel);
	  system (cmd);

	}
    }

  fclose (arp);

}
#endif

static int
do_ap_check (void)
{

//  if (nvram_match ("apwatchdog_enable", "1"))
//    do_ap_watchdog ();
  start_service ("wds_check");
//  do_wds_check ();

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

  if (len > 0 && strstr (buf, "Not associated."))
    {
#ifdef HAVE_DDLAN

      nvram_unset ("cur_rssi");
      nvram_unset ("cur_noise");
      nvram_unset ("cur_bssid");
      nvram_unset ("cur_snr");
      nvram_set ("cur_state",
		 "<span style=\"background-color: rgb(255, 0, 0);\">Nicht Verbunden</span>");

//      nvram_set ("cur_state", "Nicht Verbunden");
#endif
      /*if (nvram_match("wl_mode", "wet"))
         {

         system("/bin/rm /tmp/bridged");
         if (wl_probe("eth2"))
         eval("brctl", "delif", "br0", "eth1"); //create bridge
         else
         eval("brctl", "delif", "br0", "eth2");
         eval("wl","wet","1");
         } */
      /* let wl do this for us (no use in reinventing the wheel) */
      //eval("/usr/sbin/wlconf", get_wdev(), "down");
      //eval("/usr/sbin/wlconf", get_wdev(), "up"); 
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
    }				/*else if (nvram_match("wl_mode","bridge"))
				   {
				   FILE *in;
				   in=fopen("/tmp/bridged","rb");

				   if (in==NULL)
				   {
				   if (nvram_match("security_mode","wep") || nvram_match("security_mode","disabled"))
				   {
				   if (wl_probe("eth2"))
				   eval("brctl", "addif", "br0", "eth1"); //create bridge
				   else
				   eval("brctl", "addif", "br0", "eth2");
				   system("/bin/echo bridged>/tmp/bridged");

				   if (fp!=NULL)fclose(fp);
				   return 0;
				   }

				   system("/usr/sbin/wl assoc|grep BSSID:>/tmp/.associnfo");
				   in=fopen("/tmp/.associnfo","rb");
				   fscanf(in,"%s",mac);
				   fscanf(in,"%s",mac);
				   fclose(in);
				   sprintf(buf,"/usr/sbin/wl sta_info %s>/tmp/.sta_info",mac);
				   system(buf);
				   fclose(fp);
				   fp = NULL;
				   if( (fp = fopen("/tmp/.sta_info", "r")) == NULL)
				   return -1;
				   len = fread(buf, 1, 1023, fp);
				   buf[len] = 0;
				   fclose(fp);
				   //eval("/usr/sbin/wl","sta_info",mac,">/tmp/.sta_info");
				   in=fopen("/tmp/bridged","rb");

				   if (len>0 && strstr(buf,"AUTHORIZED") && in==NULL)
				   {
				   if (wl_probe("eth2"))
				   eval("brctl", "addif", "br0", "eth1"); //create bridge
				   else
				   eval("brctl", "addif", "br0", "eth2");
				   }
				   system("/bin/echo bridged>/tmp/bridged");

				   }
				   if (in!=NULL)fclose(in);
				   if (fp!=NULL)fclose(fp);

				   }

				   //unlink("/tmp/.xassocx");
				 */
  fclose (fp);
  return 0;
}

static void
do_wlan_check (void)
{

  if (nvram_invmatch ("wl0_mode", "ap"))
    do_client_check ();
  else
    do_ap_check ();
#ifdef HAVE_AQOS
  do_aqos_check ();
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

  /* Most of time it goes to sleep */
  while (1)
    {
      do_wlan_check ();

      sleep (WLAND_INTERVAL);
    }

  return 0;
}				// end main
