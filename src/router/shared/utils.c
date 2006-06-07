
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <syslog.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#include <sys/sysinfo.h>

#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <bcmdevs.h>
#include <net/route.h>
#include <cy_conf.h>
#include <bcmdevs.h>
#include <linux/if_ether.h>
//#include <linux/mii.h>
#include <linux/sockios.h>

#define SIOCGMIIREG	0x8948	/* Read MII PHY register.       */
#define SIOCSMIIREG	0x8949	/* Write MII PHY register.      */
void show_hw_type (int type);

struct mii_ioctl_data
{
  unsigned short phy_id;
  unsigned short reg_num;
  unsigned short val_in;
  unsigned short val_out;
};


int
startswith (char *source, char *cmp)
{
  int i;
  if (cmp == NULL)
    return 0;
  if (source == NULL)
    return 0;
  if (strlen (cmp) > strlen (source))
    return 0;
  for (i = 0; i < strlen (cmp); i++)
    if (source[i] != cmp[i])
      return 0;
  return 1;
}


void
setRouter (char *name)
{
  if (name)
    nvram_set (NVROUTER, name);
}

char *
getRouter ()
{
  char *n = nvram_get (NVROUTER);
  return n != NULL ? n : "Unknown Model";
}



int
internal_getRouterBrand ()
{

#ifdef HAVE_RB500
  setRouter ("Mikrotik RB500");
  return ROUTER_BOARD_500;
#else
  char *et0, *et1;
  if (nvram_match ("boardnum", "42") &&
      nvram_match ("boardtype", "bcm94710ap"))
    {
      cprintf ("router is buffalo\n");
      setRouter ("Buffalo WBR-G54 / WLA-G54");
      return ROUTER_BUFFALO_WBR54G;
    }

  if (nvram_match ("boardnum", "mn700") &&
      nvram_match ("boardtype", "bcm94710ap"))
    {
      cprintf ("router is Microsoft mn700\n");
      setRouter ("Microsoft MN-700");
      return ROUTER_MICROSOFT_MN700;
    }

  if (nvram_match ("boardnum", "asusX") &&
      nvram_match ("boardtype", "bcm94710dev"))
    {
      cprintf ("router is Asus WL300g\n");
      setRouter ("Asus WL300g");
      return ROUTER_RT210W;
    }  
    
  if (nvram_match ("boardnum", "100") &&	//added by Eko - experimental
      nvram_match ("boardtype", "bcm94710dev"))	//detect WLA-G54C 
    {
      cprintf ("router is buffalo\n");
      setRouter ("Buffalo WLA-G54C");	
      return ROUTER_BUFFALO_WLAG54C;	
    }

  if (nvram_match ("product_name", "Product_name") &&
      nvram_match ("boardrev", "0x10") &&
      nvram_match ("boardtype", "0x0101") && nvram_match ("boardnum", "00"))
    {
      cprintf ("router is buffalo WLA2");
      setRouter ("Buffalo WLA2-G54L");
      return ROUTER_BUFFALO_WLA2G54L;
    }


  if (nvram_match ("boardtype", "bcm95365r") &&
      nvram_match ("boardnum", "45"))
    {
      cprintf ("router is asus\n");
      setRouter ("Asus WL500G-Deluxe");
      return ROUTER_ASUS;
    }

  if (nvram_match ("boardnum", "00") &&
      nvram_match ("boardtype", "0x0101") && nvram_match ("boardrev", "0x10"))
    {
      cprintf ("router is buffalo wbr2\n");
      setRouter ("Buffalo WBR2-G54 / WBR2-G54S");
      return ROUTER_BUFFALO_WBR2G54S;
    }
  if (nvram_match ("boardnum", "00") &&
      nvram_match ("boardrev", "0x13") &&
      nvram_match ("boardtype", "0x467") &&
      nvram_match ("boardflags", "0x2758"))
    {
      cprintf ("router is buffalo WHR-G54S\n");
      setRouter ("Buffalo WHR-G54S");
      return ROUTER_BUFFALO_WHRG54S;
    }
  if (nvram_match ("boardnum", "00") &&
      nvram_match ("boardrev", "0x13") &&
      nvram_match ("boardtype", "0x467") &&
      nvram_match ("boardflags", "0x1758"))
    {
      cprintf ("router is buffalo WHR-HP-G54\n");
      setRouter ("Buffalo WHR-HP-G54");
      return ROUTER_BUFFALO_HP_WHRG54S;
    }
  if (nvram_match ("boardnum", "42") &&
      nvram_match ("boardtype", "0x042f") &&
      (nvram_match ("product_name", "Product_name")
       || nvram_match ("product_name", "WZR-RS-G54")
       || nvram_match ("product_name", "WZR-HP-G54")))
    {
      cprintf ("router is buffalo WZR-RS\n");
      setRouter ("Buffalo WZR-RS-G54");
      return ROUTER_BUFFALO_WZRRSG54;
    }
  if (nvram_match ("boardnum", "45") &&
      nvram_match ("boardtype", "0x042f") &&
      nvram_match ("boardrev", "0x10"))
    {
      cprintf ("router is Asus WL-500g premium\n");
      setRouter ("Asus WL-500g Premium");
      return ROUTER_ASUS_WL500G_PRE;
    }


  {
    et0 = nvram_safe_get ("et0macaddr");
    et1 = nvram_safe_get ("et1macaddr");
    if (et0 != NULL && et1 != NULL)
      {
	if (nvram_match ("boardnum", "100") &&
	    nvram_match ("boardtype", "bcm94710r4"))
	  {
	    if (startswith (et0, "00:11:50"))
	      {
		cprintf ("router is Belkin F5D7130 / F5D7330\n");
		setRouter ("Belkin F5D7130 / F5D7330");
		return ROUTER_WRT54G;
	      }
	    if (startswith (et0, "00:30:BD") || startswith (et0, "00:30:bd"))
	      {
		cprintf ("router is Belkin F5D7230 v1000\n");
		setRouter ("Belkin F5D7230-4 v1000");
		return ROUTER_RT210W;
	      }
	    if ((startswith (et0, "00:01:E3") 
	    	&& startswith (et1, "00:01:E3"))
		|| (startswith (et0, "00:01:e3")
		    && startswith (et1, "00:01:e3"))
		|| (startswith (et0, "00:90:96")
		    && startswith (et1, "00:90:96")))
	      {
		cprintf ("router is Siemens\n");
		setRouter ("Siemens SE505 v1");
		return ROUTER_RT210W;
	      }
	  	else
	  	{
		cprintf ("router is Askey generic\n");
		setRouter ("RT210W generic");
		return ROUTER_RT210W;
	  	}
	 }
	
	if (nvram_match ("boardtype", "0x0101"))
	  {
	    if ((startswith (et0, "00:11:50") && startswith (et1, "00:11:50"))
	    || (startswith (et0, "00:30:BD")
		    && startswith (et1, "00:30:BD"))
		|| (startswith (et0, "00:30:bd")
		    && startswith (et1, "00:30:bd")))
	      {
		cprintf ("router is Belkin F5D7230-4 v1444\n");
		setRouter ("Belkin F5D7230-4 v1444");
		return ROUTER_BELKIN_F5D7230;
	      }
	    if ((startswith (et0, "00:01:E3") && startswith (et1, "00:01:E3"))
		|| (startswith (et0, "00:01:e3")
		    && startswith (et1, "00:01:e3"))
		|| (startswith (et0, "00:90:96")
		    && startswith (et1, "00:90:96")))
	      {
		cprintf ("router is Siemens / Askey\n");
		setRouter ("Siemens SE505 v2");
		return ROUTER_SIEMENS;
	      }
	  }

	if (nvram_match ("boardnum", "2") &&
	    nvram_match ("boardtype", "bcm94710dev"))
	  {
	    if (nvram_match ("GemtekPmonVer", "9") &&
		((startswith (et0, "00:0C:E5")
		  && startswith (et1, "00:0C:E5"))
		 || (startswith (et0, "00:0c:e5")
		     && startswith (et1, "00:0c:e5"))
		 || (startswith (et0, "00:0C:10")
		     && startswith (et1, "00:0C:10"))
		 || (startswith (et0, "00:11:22")
		     && startswith (et1, "00:11:22"))))
	      {
		cprintf ("router Motorola WR850G v1\n");
		setRouter ("Motorola WR850G v1");
		return ROUTER_MOTOROLA_V1;
	      }
	    else
	      {
		cprintf ("router is linksys WRT55AG\n");
		setRouter ("Linksys WRT55AG v1");
		return ROUTER_LINKSYS_WRT55AG;
	      }
	  }

      }
  }
  if (nvram_match ("boardnum", "42") &&
      nvram_match ("boardtype", "bcm94710dev"))
    {
      setRouter ("Linksys WRT54G 1.x");
      return ROUTER_WRT54G1X;
    }
  if (nvram_invmatch ("CFEver", ""))
    {
      char *cfe = nvram_safe_get ("CFEver");
      if (!strncmp (cfe, "MotoWR", 6))
	{
	  cprintf ("router is motorola\n");
	  setRouter ("Motorola WR850G");
	  return ROUTER_MOTOROLA;
	}
    }

  setRouter ("Linksys WRT54G/GS");
  cprintf ("router is wrt54g\n");
  return ROUTER_WRT54G;
#endif

}
static int router_type = -1;
int
getRouterBrand ()
{
  if (router_type == -1)
    router_type = internal_getRouterBrand ();
  return router_type;
}


int
diag_led_4702 (int type, int act)
{
#ifdef HAVE_RB500
  return 0;
#else
  if (act == START_LED)
    {
      switch (type)
	{
	case DMZ:
	  system ("echo 1 > /proc/sys/diag");
	  break;
	}
    }
  else
    {
      switch (type)
	{
	case DMZ:
	  system ("echo 0 > /proc/sys/diag");
	  break;
	}
    }
  return 0;
#endif
}

int
C_led_4702 (int i)
{
#ifdef HAVE_RB500
  return 0;
#else
  FILE *fp;
  char string[10];
  int flg;

  memset (string, 0, 10);
  /* get diag before set */
  if ((fp = fopen ("/proc/sys/diag", "r")))
    {
      fgets (string, sizeof (string), fp);
      fclose (fp);
    }
  else
    perror ("/proc/sys/diag");

  if (i)
    flg = atoi (string) | 0x10;
  else
    flg = atoi (string) & 0xef;

  memset (string, 0, 10);
  sprintf (string, "%d", flg);
  if ((fp = fopen ("/proc/sys/diag", "w")))
    {
      fputs (string, fp);
      fclose (fp);
    }
  else
    perror ("/proc/sys/diag");

  return 0;
#endif
}

unsigned int
read_gpio (char *device)
{
  FILE *fp;
  unsigned int val;

  if ((fp = fopen (device, "r")))
    {
      fread (&val, 4, 1, fp);
      fclose (fp);
      //fprintf(stderr, "----- gpio %s = [%X]\n",device,val); 
      return val;
    }
  else
    {
      perror (device);
      return 0;
    }
}

unsigned int
write_gpio (char *device, unsigned int val)
{
  FILE *fp;

  if ((fp = fopen (device, "w")))
    {
      fwrite (&val, 4, 1, fp);
      fclose (fp);
      //fprintf(stderr, "----- set gpio %s = [%X]\n",device,val); 
      return 1;
    }
  else
    {
      perror (device);
      return 0;
    }
}

static char hw_error = 0;
int
diag_led_4704 (int type, int act)
{
#ifdef HAVE_RB500
  return 0;
#else
  unsigned int control, in, outen, out;

#ifdef BCM94712AGR
  /* The router will crash, if we load the code into broadcom demo board. */
  return 1;
#endif

  control = read_gpio ("/dev/gpio/control");
  in = read_gpio ("/dev/gpio/in");
  out = read_gpio ("/dev/gpio/out");
  outen = read_gpio ("/dev/gpio/outen");

  write_gpio ("/dev/gpio/outen", (outen & 0x7c) | 0x83);
  switch (type)
    {
    case DIAG:			// GPIO 1
      if (hw_error)
	{
	  write_gpio ("/dev/gpio/out", (out & 0x7c) | 0x00);
	  return 1;
	}

      if (act == STOP_LED)
	{			// stop blinking
	  write_gpio ("/dev/gpio/out", (out & 0x7c) | 0x83);
	  //cprintf("tallest:=====( DIAG STOP_LED !!)=====\n");
	}
      else if (act == START_LED)
	{			// start blinking
	  write_gpio ("/dev/gpio/out", (out & 0x7c) | 0x81);
	  //cprintf("tallest:=====( DIAG START_LED !!)=====\n");
	}
      else if (act == MALFUNCTION_LED)
	{			// start blinking
	  write_gpio ("/dev/gpio/out", (out & 0x7c) | 0x00);
	  hw_error = 1;
	  //cprintf("tallest:=====( DIAG MALFUNCTION_LED !!)=====\n");
	}
      break;
#if 0
    case DMZ:			// GPIO 7
      if (act == STOP_LED)
	{
	  write_gpio ("/dev/gpio/out", out | 0x80);
	  cprintf ("tallest:=====( DMZ STOP_LED !!)=====\n");
	}
      else if (act == START_LED)
	{
	  write_gpio ("/dev/gpio/out", out & 0x7f);
	  cprintf ("tallest:=====( DMZ START_LED !!)=====\n");
	}
      break;
    case WL:			// GPIO 0
      write_gpio ("/dev/gpio/control", control & 0xfe);
      write_gpio ("/dev/gpio/outen", outen | 0x01);
      if (act == STOP_LED)
	{
	  write_gpio ("/dev/gpio/out", out | 0x01);
	  //cprintf("tallest:=====( WL STOP_LED !!)=====\n");
	}
      else if (act == START_LED)
	{
	  write_gpio ("/dev/gpio/out", out & 0xfe);
	  //cprintf("tallest:=====( WL START_LED !!)=====\n");
	}
      break;
#endif
    }
  return 1;
#endif
}

int
diag_led_4712 (int type, int act)
{
  unsigned int control, in, outen, out, ctr_mask, out_mask;
#ifdef HAVE_RB500
  return 0;
#else


#ifdef BCM94712AGR
  /* The router will crash, if we load the code into broadcom demo board. */
  return 1;
#endif
  control = read_gpio ("/dev/gpio/control");
  in = read_gpio ("/dev/gpio/in");
  out = read_gpio ("/dev/gpio/out");
  outen = read_gpio ("/dev/gpio/outen");

  ctr_mask = ~(1 << type);
  out_mask = (1 << type);

  write_gpio ("/dev/gpio/control", control & ctr_mask);
  write_gpio ("/dev/gpio/outen", outen | out_mask);

  if (act == STOP_LED)
    {				// stop blinking
      //cprintf("%s: Stop GPIO %d\n", __FUNCTION__, type);
      write_gpio ("/dev/gpio/out", out | out_mask);
    }
  else if (act == START_LED)
    {				// start blinking
      //cprintf("%s: Start GPIO %d\n", __FUNCTION__, type);
      write_gpio ("/dev/gpio/out", out & ctr_mask);
    }

  return 1;
#endif
}

int
C_led_4712 (int i)
{
  if (i == 1)
    return diag_led (DIAG, START_LED);
  else
    return diag_led (DIAG, STOP_LED);
}

int
C_led (int i)
{
//show_hw_type(check_hw_type());

  if (check_hw_type () == BCM4702_CHIP)
    return C_led_4702 (i);
  else
    return C_led_4712 (i);
}

int
diag_led (int type, int act)
{
//show_hw_type(check_hw_type());

	if (getRouterBrand () == ROUTER_BELKIN_F5D7230)  //fix for belkin DMZ=enable reboot problem
		return 0;
	else
		{
		if (check_hw_type () == BCM4702_CHIP)
    		return diag_led_4702 (type, act);
		else if (check_hw_type () == BCM4704_BCM5325F_CHIP)
			return diag_led_4704 (type, act);
		else
			return diag_led_4712 (type, act);
		}
}

char *
get_mac_from_ip (char *ip)
{
  FILE *fp;
  char line[100];
  char ipa[50];			// ip address
  char hwa[50];			// HW address / MAC
  char mask[50];		// ntemask   
  char dev[50];			// interface
  int type;			// HW type
  int flags;			// flags
  static char mac[20];


  if ((fp = fopen ("/proc/net/arp", "r")) == NULL)
    return NULL;

  // Bypass header -- read until newline 
  if (fgets (line, sizeof (line), fp) != (char *) NULL)
    {
      // Read the ARP cache entries.
      // IP address       HW type     Flags       HW address            Mask     Device
      // 192.168.1.1      0x1         0x2         00:90:4C:21:00:2A     *        eth0
      for (; fgets (line, sizeof (line), fp);)
	{
	  if (sscanf
	      (line, "%s 0x%x 0x%x %100s %100s %100s\n", ipa, &type, &flags,
	       hwa, mask, dev) != 6)
	    continue;
	  //cprintf("ip1=[%s] ip2=[%s] mac=[%s] (flags & ATF_COM)=%d\n", ip, ipa, hwa, (flags & ATF_COM));
	  if (strcmp (ip, ipa))
	    continue;
	  //if (!(flags & ATF_COM)) {       //ATF_COM = 0x02   completed entry (ha valid)
	  strcpy (mac, hwa);
	  fclose (fp);
	  return mac;
	  //}
	}
    }

  fclose (fp);
  return "";
}

struct dns_lists *
get_dns_list (void)
{
  char list[254];
  char *next, word[254];
  struct dns_lists *dns_list = NULL;
  int i, match = 0, altdns_index = 1;

  dns_list = (struct dns_lists *) malloc (sizeof (struct dns_lists));
  memset (dns_list, 0, sizeof (struct dns_lists));

  dns_list->num_servers = 0;

  // nvram_safe_get("wan_dns") ==> Set by user
  // nvram_safe_get("wan_get_dns") ==> Get from DHCP, PPPoE or PPTP
  // The nvram_safe_get("wan_dns") priority is higher than nvram_safe_get("wan_get_dns")
  snprintf (list, sizeof (list), "%s %s %s", nvram_safe_get ("sv_localdns"),
	    nvram_safe_get ("wan_dns"), nvram_safe_get ("wan_get_dns"));
  foreach (word, list, next)
  {
    if (strcmp (word, "0.0.0.0") && strcmp (word, ""))
      {
	match = 0;
	for (i = 0; i < dns_list->num_servers; i++)
	  {			// Skip same DNS
	    if (!strcmp (dns_list->dns_server[i], word))
	      match = 1;
	  }
	if (!match)
	  {
	    snprintf (dns_list->dns_server[dns_list->num_servers],
		      sizeof (dns_list->dns_server[dns_list->num_servers]),
		      "%s", word);
	    dns_list->num_servers++;
	  }
      }
    if (dns_list->num_servers == 3)
      break;			// We only need 3 DNS entries
  }

  /* if < 3 DNS servers found, try to insert alternates */
  while (dns_list->num_servers < 3 && altdns_index <= 3)
    {
      char altdnsvar[32] = { 0 };

      snprintf (altdnsvar, 31, "altdns%d", altdns_index);

      if (strlen (nvram_safe_get (altdnsvar)) > 0)
	{
	  snprintf (dns_list->dns_server[dns_list->num_servers],
		    sizeof (dns_list->dns_server[dns_list->num_servers]),
		    "%s", nvram_safe_get (altdnsvar));
	  dns_list->num_servers++;
	}
      altdns_index++;
    }
  return dns_list;
}

int
dns_to_resolv (void)
{
  FILE *fp_w;
  struct dns_lists *dns_list = NULL;
  int i = 0;

  /* Save DNS to resolv.conf */
  if (!(fp_w = fopen (RESOLV_FILE, "w")))
    {
      perror (RESOLV_FILE);
      return errno;
    }
  if (nvram_invmatch ("wan_get_domain", ""))
    {
      fprintf (fp_w, "search %s\n", nvram_safe_get ("wan_get_domain"));
    }
  else if (nvram_invmatch ("wan_domain", ""))
    {
      fprintf (fp_w, "search %s\n", nvram_safe_get ("wan_domain"));
    }
  if (nvram_invmatch ("lan_domain", ""))
    {
      fprintf (fp_w, "search %s\n", nvram_safe_get ("lan_domain"));
    }
  if (nvram_match ("dnsmasq_enable", "1"))
    {
      fprintf (fp_w, "nameserver %s\n", nvram_get ("lan_ipaddr"));
      fclose (fp_w);
      if (!(fp_w = fopen (RESOLV_FORW, "w")))
	{
	  perror (RESOLV_FORW);
	  return errno;
	}
    }

  dns_list = get_dns_list ();

  for (i = 0; i < dns_list->num_servers; i++)
    fprintf (fp_w, "nameserver %s\n", dns_list->dns_server[i]);

  /* Put a pseudo DNS IP to trigger Connect On Demand */
  if (dns_list->num_servers == 0 &&
      (nvram_match ("wan_proto", "pppoe") || nvram_match ("wan_proto", "pptp")
       || nvram_match ("wan_proto", "l2tp"))
      && nvram_match ("ppp_demand", "1"))
    fprintf (fp_w, "nameserver 1.1.1.1\n");

  fclose (fp_w);
  if (dns_list)
    free (dns_list);

  eval ("touch", "/tmp/hosts");

  return 1;
}

/* Example:
 * lan_ipaddr = 192.168.1.1
 * get_dns_ip("lan_ipaddr", 1); produces "168"
 */
int
get_single_ip (char *ipaddr, int which)
{
  int ip[4] = { 0, 0, 0, 0 };
  int ret;

  ret = sscanf (ipaddr, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);

  return ip[which];
}

char *
get_complete_lan_ip (char *ip)
{
  static char ipaddr[20];

  int i[4];

  if (sscanf
      (nvram_safe_get ("lan_ipaddr"), "%d.%d.%d.%d", &i[0], &i[1], &i[2],
       &i[3]) != 4)
    return "0.0.0.0";

  snprintf (ipaddr, sizeof (ipaddr), "%d.%d.%d.%s", i[0], i[1], i[2], ip);

  return ipaddr;
}

char *
get_wan_face (void)
{
  static char localwanface[IFNAMSIZ];
/*  if (nvram_match ("pptpd_client_enable", "1"))
    {
	strncpy (localwanface, "ppp0", IFNAMSIZ);
	return localwanface;
    }*/
  if (nvram_match ("wan_proto", "pptp") || nvram_match ("wan_proto", "l2tp")
      || nvram_match ("wan_proto", "pppoe"))
    {
      if (nvram_match ("pppd_pppifname", ""))
	strncpy (localwanface, "ppp0", IFNAMSIZ);
      else
	strncpy (localwanface, nvram_safe_get ("pppd_pppifname"), IFNAMSIZ);
    }
#ifndef HAVE_MADWIFI
  else if (nvram_invmatch ("wl_mode", "ap"))
    {

      if (check_hw_type () == BCM4702_CHIP
	  || check_hw_type () == BCM4704_BCM5325F_CHIP)
	strcpy (localwanface, "eth2");
      else
	strcpy (localwanface, "eth1");

    }
#else
  else if (nvram_match ("ath0_mode", "sta"))
    {
      strcpy (localwanface, "ath0");
    }
#endif
  else
    strncpy (localwanface, nvram_safe_get ("wan_ifname"), IFNAMSIZ);

  return localwanface;
}

int
get_ppp_pid (char *file)
{
  char buf[80];
  int pid = -1;
  if (file_to_buf (file, buf, sizeof (buf)))
    {
      char tmp[80], tmp1[80];
      snprintf (tmp, sizeof (tmp), "/var/run/%s.pid", buf);
      file_to_buf (tmp, tmp1, sizeof (tmp1));
      pid = atoi (tmp1);
    }
  return pid;
}

/*
 =====================================================================================
				by tallest 
 =====================================================================================
 */


int
osl_ifflags (const char *ifname)
{
  int sockfd;
  struct ifreq ifreq;
  int flags = 0;

  if ((sockfd = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    {
      perror ("socket");
      return flags;
    }

  strncpy (ifreq.ifr_name, ifname, IFNAMSIZ);
  if (ioctl (sockfd, SIOCGIFFLAGS, &ifreq) < 0)
    {
      flags = 0;
    }
  else
    {
      flags = ifreq.ifr_flags;
    }
  close (sockfd);
  return flags;
}

int
check_wan_link (int num)
{
  int wan_link = 0;

  if (nvram_match ("wan_proto", "pptp")
      || nvram_match ("wan_proto", "l2tp")
      || nvram_match ("wan_proto", "pppoe")
      || nvram_match ("wan_proto", "heartbeat"))
    {
      FILE *fp;
      char filename[80];
      char *name;

      if (num == 0)
	strcpy (filename, "/tmp/ppp/link");
      if ((fp = fopen (filename, "r")))
	{
	  int pid = -1;
	  fclose (fp);
	  if (nvram_match ("wan_proto", "heartbeat"))
	    {
	      char buf[20];
	      file_to_buf ("/tmp/ppp/link", buf, sizeof (buf));
	      pid = atoi (buf);
	    }
	  else
	    pid = get_ppp_pid (filename);

	  name = find_name_by_proc (pid);
	  if (!strncmp (name, "pppoecd", 7) ||	// for PPPoE
	      !strncmp (name, "pppd", 4) ||	// for PPTP
	      !strncmp (name, "bpalogin", 8))	// for HeartBeat
	    wan_link = 1;	//connect
	  else
	    {
	      printf ("The %s had been died, remove %s\n",
		      nvram_safe_get ("wan_proto"), filename);
	      wan_link = 0;	// For some reason, the pppoed had been died, by link file still exist.
	      unlink (filename);
	    }
	}
    }
  else
    {
      if (nvram_invmatch ("wan_ipaddr", "0.0.0.0"))
	wan_link = 1;
    }

  return wan_link;
}

int
get_int_len (int num)
{
  char buf[80];

  snprintf (buf, sizeof (buf), "%d", num);

  return strlen (buf);
}

int
file_to_buf (char *path, char *buf, int len)
{
  FILE *fp;

  memset (buf, 0, len);

  if ((fp = fopen (path, "r")))
    {
      fgets (buf, len, fp);
      fclose (fp);
      return 1;
    }

  return 0;
}

int
buf_to_file (char *path, char *buf)
{
  FILE *fp;

  if ((fp = fopen (path, "w")))
    {
      fprintf (fp, "%s", buf);
      fclose (fp);
      return 1;
    }

  return 0;
}


#define READ_BUF_SIZE 254
/* from busybox find_pid_by_name.c */
pid_t *
find_pid_by_name (char *pidName)
{
  DIR *dir;
  struct dirent *next;
  pid_t *pidList = NULL;
  int i = 0;

  dir = opendir ("/proc");

  while ((next = readdir (dir)) != NULL)
    {
      FILE *status;
      char filename[READ_BUF_SIZE];
      char buffer[READ_BUF_SIZE];
      char name[READ_BUF_SIZE];

      /* Must skip ".." since that is outside /proc */
      if (strcmp (next->d_name, "..") == 0)
	continue;

      /* If it isn't a number, we don't want it */
      if (!isdigit (*next->d_name))
	continue;

      sprintf (filename, "/proc/%s/status", next->d_name);
      if (!(status = fopen (filename, "r")))
	{
	  continue;
	}
      if (fgets (buffer, READ_BUF_SIZE - 1, status) == NULL)
	{
	  fclose (status);
	  continue;
	}
      fclose (status);

      /* Buffer should contain a string like "Name:   binary_name" */
      sscanf (buffer, "%*s %s", name);
      //printf("buffer=[%s] name=[%s]\n",buffer,name);
      if (strcmp (name, pidName) == 0)
	{
	  pidList = realloc (pidList, sizeof (pid_t) * (i + 2));
	  pidList[i++] = strtol (next->d_name, NULL, 0);
	}
    }

  if (pidList)
    pidList[i] = 0;
  else
    {
      pidList = realloc (pidList, sizeof (pid_t));
      pidList[0] = -1;
    }
  return pidList;

}

/* Find first process pid with same name from ps command */
int
find_pid_by_ps (char *pidName)
{
  FILE *fp;
  int pid = -1;
  char line[254];

  if ((fp = popen ("ps -ax", "r")))
    {
      while (fgets (line, sizeof (line), fp) != NULL)
	{
	  if (strstr (line, pidName))
	    {
	      sscanf (line, "%d", &pid);
	      printf ("%s pid is %d\n", pidName, pid);
	      break;
	    }
	}
      pclose (fp);
    }

  return pid;
}

/* Find process name by pid from /proc directory */
char *
find_name_by_proc (int pid)
{
  FILE *fp;
  char line[254];
  char filename[80];
  static char name[80];

  snprintf (filename, sizeof (filename), "/proc/%d/status", pid);

  if ((fp = fopen (filename, "r")))
    {
      fgets (line, sizeof (line), fp);
      /* Buffer should contain a string like "Name:   binary_name" */
      sscanf (line, "%*s %s", name);
      fclose (fp);
      return name;
    }

  return "";
}

/* Find all process pid with same name from ps command */
int *
find_all_pid_by_ps (char *pidName)
{
  FILE *fp;
  int pid = -1;
  char line[254];
  int *pidList = NULL;
  int i = 0;
  printf ("Search for %s\n", pidName);
  if ((fp = popen ("ps", "r")))
    {
      while (fgets (line, sizeof (line), fp) != NULL)
	{
	  if (strstr (line, pidName))
	    {
	      sscanf (line, "%d", &pid);
	      printf ("%s pid is %d\n", pidName, pid);
	      pidList = realloc (pidList, sizeof (int) * (i + 2));
	      pidList[i++] = pid;
	    }
	}
      pclose (fp);
    }
  if (pidList)
    pidList[i] = 0;
  else
    {
      pidList = realloc (pidList, sizeof (int));
      pidList[0] = -1;
    }
  printf ("Search done...\n");

  return pidList;
}


int
count_processes (char *pidName)
{
  FILE *fp;
  char line[254];
  int i = 0;
  printf ("Search for %s\n", pidName);
  if ((fp = popen ("ps", "r")))
    {
      while (fgets (line, sizeof (line), fp) != NULL)
	{
	  if (strstr (line, pidName))
	    {
	      i++;
	    }
	}
      pclose (fp);
    }
  printf ("Search done... %d\n", i);

  return i;
}

void
encode (char *buf, int len)
{
  int i;
  char ch;

  for (i = 0; i < len; i++)
    {
      ch = (buf[i] & 0x03) << 6;
      buf[i] = (buf[i] >> 2);
      buf[i] &= 0x3f;
      buf[i] |= ch;
      buf[i] = ~buf[i];
    }
}

void
decode (char *buf, int len)
{
  int i;
  char ch;

  for (i = 0; i < len; i++)
    {
      ch = (buf[i] & 0xC0) >> 6;
      buf[i] = (buf[i] << 2) | ch;
      buf[i] = ~buf[i];
    }
}

/*	v1.41.7 => 014107
 *	v1.2	=> 0102
 */
long
convert_ver (char *ver)
{
  char buf[10];
  int v[3];
  int ret;

  ret = sscanf (ver, "v%d.%d.%d", &v[0], &v[1], &v[2]);

  if (ret == 2)
    {
      snprintf (buf, sizeof (buf), "%02d%02d", v[0], v[1]);
      return atol (buf);
    }
  else if (ret == 3)
    {
      snprintf (buf, sizeof (buf), "%02d%02d%02d", v[0], v[1], v[2]);
      return atol (buf);
    }
  else
    return -1;
}

/* To avoid user to download old image that is not support intel flash to new hardware with intel flash.
 */
int
check_flash (void)
{
  // The V2 image can support intel flash completely, so we don't want to check.
  if (check_hw_type () == BCM4712_CHIP)
    return FALSE;

  // The V1.X some images cann't support intel flash, so we want to avoid user to downgrade.
  if (nvram_match ("skip_amd_check", "1"))
    {
      if (strstr (nvram_safe_get ("flash_type"), "Intel")
	  && nvram_invmatch ("skip_intel_check", "1"))
	return TRUE;
      else
	return FALSE;
    }
  else				// Cann't downgrade to old firmware version, no matter AMD or Intel flash
    return TRUE;
}

int
check_action (void)
{
  char buf[80] = "";

  if (file_to_buf (ACTION_FILE, buf, sizeof (buf)))
    {
      if (!strcmp (buf, "ACT_TFTP_UPGRADE"))
	{
	  fprintf (stderr, "Upgrading from tftp now ...\n");
	  return ACT_TFTP_UPGRADE;
	}
#ifdef HAVE_HTTPS
      else if (!strcmp (buf, "ACT_WEBS_UPGRADE"))
	{
	  fprintf (stderr, "Upgrading from web (https) now ...\n");
	  return ACT_WEBS_UPGRADE;
	}
#endif
      else if (!strcmp (buf, "ACT_WEB_UPGRADE"))
	{
	  fprintf (stderr, "Upgrading from web (http) now ...\n");
	  return ACT_WEB_UPGRADE;
	}
      else if (!strcmp (buf, "ACT_SW_RESTORE"))
	{
	  fprintf (stderr, "Receiving restore command from web ...\n");
	  return ACT_SW_RESTORE;
	}
      else if (!strcmp (buf, "ACT_HW_RESTORE"))
	{
	  fprintf (stderr,
		   "Receiving restore commond from resetbutton ...\n");
	  return ACT_HW_RESTORE;
	}
      else if (!strcmp (buf, "ACT_NVRAM_COMMIT"))
	{
	  fprintf (stderr, "Committing nvram now ...\n");
	  return ACT_NVRAM_COMMIT;
	}
      else if (!strcmp (buf, "ACT_ERASE_NVRAM"))
	{
	  fprintf (stderr, "Erasing nvram now ...\n");
	  return ACT_ERASE_NVRAM;
	}
    }
  //fprintf(stderr, "Waiting for upgrading....\n");
  return ACT_IDLE;
}

int
check_now_boot (void)
{
  char *ver = nvram_safe_get ("pmon_ver");
  char *cfe = nvram_safe_get ("CFEver");
  // for 4712
  // The boot_ver value is lower v2.0 (no included)
  if (!strncmp (ver, "PMON", 4))
    {
      cprintf ("The boot is PMON\n");
      return PMON_BOOT;
    }
  // for 4712
  // The boot_ver value is higher v2.0 (included)
  else if (!strncmp (ver, "CFE", 3))
    {
      cprintf ("The boot is CFE\n");
      return CFE_BOOT;
    }
  else if (!strncmp (ver, "2", 1))
    {
      cprintf ("The boot is CFE %s\n", ver);
      return CFE_BOOT;
    }
  else if (!strncmp (cfe, "MotoWR", 6))
    {
      cprintf ("The boot is Motorola CFE\n");
      return CFE_BOOT;
    }
  else
    {
      cprintf ("The boot is UNKNOWN\n");
      return UNKNOWN_BOOT;
    }
}

void
show_hw_type (int type)
{
  if (type == BCM4702_CHIP)
    cprintf ("The chipset is BCM4702\n");
  else if (type == BCM5325E_CHIP)
    cprintf ("The chipset is BCM4712L + BCM5325E\n");
  else if (type == BCM4704_BCM5325F_CHIP)
    cprintf ("The chipset is BCM4704 + BCM5325F\n");
  else if (type == BCM5352E_CHIP)
    cprintf ("The chipset is BCM5352E\n");
  else if (type == BCM4712_CHIP)
    cprintf ("The chipset is BCM4712 + ADMtek\n");
  else
    cprintf ("The chipset is not defined\n");

}

int
check_hw_type (void)
{
  uint boardflags;

  boardflags = strtoul (nvram_safe_get ("boardflags"), NULL, 0);

  if (nvram_match ("boardtype", "bcm94710dev") || nvram_match ("boardtype", "bcm94710ap") || nvram_match ("boardtype", "bcm94710r4"))	//Eko 19.apr.06, 10.may.06
    return BCM4702_CHIP;
  else if (nvram_match ("boardtype", "0x0708") && !(boardflags & BFL_ENETADM))
    return BCM5325E_CHIP;
  else if (nvram_match ("boardtype", "0x042f") && !(boardflags & BFL_ENETADM))
    return BCM4704_BCM5325F_CHIP;
  else if (nvram_match ("boardtype", "0x0467"))
    return BCM5352E_CHIP;
  else if (nvram_match ("boardtype", "0x0101"))
    return BCM4712_CHIP;
  else
    return NO_DEFINE_CHIP;
}

int
is_exist (char *filename)
{
  FILE *fp;

  if ((fp = fopen (filename, "r")))
    {
      fclose (fp);
      return 1;
    }
  return 0;
}

int
ct_openlog (const char *ident, int option, int facility, char *log_name)
{
  int level = atoi (nvram_safe_get (log_name));

  switch (level)
    {
    case CONSOLE_ONLY:
      break;
    }
  return level;
}


void
ct_syslog (int level, int enable, const char *fmt, ...)
{
  char buf[1000];
  va_list args;

  va_start (args, fmt);
  vsnprintf (buf, sizeof (buf), fmt, args);
  va_end (args);

  switch (enable)
    {
    case CONSOLE_ONLY:
      cprintf ("[%d] %s\n", getpid (), buf);	// print to console
      break;
    }
}

void
ct_logger (int level, const char *fmt, ...)
{
}

void
set_ip_forward (char c)
{
  FILE *fp;

  if ((fp = fopen ("/proc/sys/net/ipv4/ip_forward", "r+")))
    {
      fputc (c, fp);
      fclose (fp);
    }
  else
    {
      perror ("/proc/sys/net/ipv4/ip_forward");
    }
}


static char *device_name[] = {
  "eth0",
  "qos0"
};

char *
get_device_name (void)
{
  int i;

  switch (check_hw_type ())
    {
    case BCM5325E_CHIP:
    case BCM4704_BCM5325F_CHIP:
    case BCM5352E_CHIP:
      i = 0;
      break;
    case BCM4702_CHIP:
    case BCM4712_CHIP:
    default:
      i = 1;
      break;
    }

  return device_name[i];
}

char *
strncpyz (char *dest, char const *src, size_t size)
{
  if (!size--)
    return dest;
  strncpy (dest, src, size);
  dest[size] = 0;		/* Make sure the string is null terminated */
  return dest;
}

static int
sockets_open (int domain, int type, int protocol)
{
  int fd = socket (domain, type, protocol);

  if (fd < 0)
    cprintf ("sockets_open: no usable address was found.\n");
  return fd;
}

int
sys_netdev_ioctl (int family, int socket, char *if_name, int cmd,
		  struct ifreq *ifr)
{
  int rc, s;

  if ((s = socket) < 0)
    {
      if ((s = sockets_open (family, family == AF_PACKET ? SOCK_PACKET :
			     SOCK_DGRAM,
			     family == AF_PACKET ? htons (ETH_P_ALL) : 0)) <
	  0)
	{
	  cprintf ("sys_netdev_ioctl: failed\n");
	  return -1;
	}
    }
  strncpyz (ifr->ifr_name, if_name, IFNAMSIZ);
  rc = ioctl (s, cmd, ifr);
  if (socket < 0)
    close (s);
  return rc;
}

int
set_register_value (unsigned short port_addr, unsigned short option_content)
{
  struct ifreq ifr;
  struct mii_ioctl_data stats;

  stats.phy_id = port_addr;
  stats.val_in = option_content;

  ifr.ifr_data = (void *) &stats;

  if (sys_netdev_ioctl (AF_INET, -1, get_device_name (), SIOCSMIIREG, &ifr) <
      0)
    return -1;

  return 0;
}

unsigned long
get_register_value (unsigned short id, unsigned short num)
{
  struct ifreq ifr;
  struct mii_ioctl_data stats;

  stats.phy_id = id;
  stats.reg_num = num;
  stats.val_in = 0;
  stats.val_out = 0;

  ifr.ifr_data = (void *) &stats;

  sys_netdev_ioctl (AF_INET, -1, get_device_name (), SIOCGMIIREG, &ifr);

  return ((stats.val_in << 16) | stats.val_out);
}


struct wl_assoc_mac *
get_wl_assoc_mac (int *c)
{
  FILE *fp;
  struct wl_assoc_mac *wlmac = NULL;
  int count;
  char line[80];
  char list[2][20];

  wlmac = NULL;
  count = *c = 0;

  if ((fp = popen ("wl assoclist", "r")))
    {
      while (fgets (line, sizeof (line), fp) != NULL)
	{
	  strcpy (list[0], "");
	  strcpy (list[1], "");

	  if (sscanf (line, "%s %s", list[0], list[1]) != 2)	// assoclist 00:11:22:33:44:55
	    continue;
	  if (strcmp (list[0], "assoclist"))
	    continue;

//not needed            if(count > 0)    // realloc(NULL,n) == malloc(n) -- tofu6
	  wlmac = realloc (wlmac, sizeof (struct wl_assoc_mac) * (count + 1));

//        if (count > 0)
//          wlmac =
//            realloc (wlmac, sizeof (struct wl_assoc_mac) * (count + 1));

	  memset (&wlmac[count], 0, sizeof (struct wl_assoc_mac));
	  strncpy (wlmac[count].mac, list[1], sizeof (wlmac[0].mac));
	  count++;
	}

      pclose (fp);
      //cprintf("Count of wl assoclist mac is %d\n", count);
      *c = count;
      return wlmac;
    }

  return NULL;
}

struct mtu_lists mtu_list[] = {
#if COUNTRY == JAPAN
  {"pppoe", "576", "1454"},
#else
  {"pppoe", "576", "1492"},
#endif

  {"pptp", "576", "1460"},

  {"l2tp", "576", "1460"},

  {"dhcp", "576", "1500"},
  {"static", "576", "1500"},
  {"heartbeat", "576", "1500"},
  {"default", "576", "1500"},	// The value must be at last
};

struct mtu_lists *
get_mtu (char *proto)
{
  struct mtu_lists *v = NULL;

  for (v = mtu_list; v < &mtu_list[STRUCT_LEN (mtu_list)]; v++)
    {
      if (!strcmp (proto, v->proto))
	return v;
    }
  return v;			// Use default settings
}

void
set_host_domain_name (void)
{
  char *hostname;
  char *domain;

  /* Allow you to use gethostname to get Host Name */
  if (strlen (nvram_safe_get ("wan_hostname")) > 0)
    hostname = nvram_safe_get ("wan_hostname");
  else if (strlen (nvram_safe_get ("router_name")) > 0)
    hostname = nvram_safe_get ("router_name");
  else
    hostname = "dd-wrt";
  sethostname (hostname, strlen (hostname));

  /* Allow you to use getdomainname to get Domain Name */
 if (strlen (nvram_safe_get ("wan_domain")) > 0 && strlen (nvram_safe_get ("wan_domain")) <= 64) //no more than 64
    domain = nvram_safe_get ("wan_domain");
  else
    domain = nvram_safe_get ("wan_get_domain");

  setdomainname (domain, strlen (domain));
}

int
first_time (void)
{
  struct sysinfo info;

  sysinfo (&info);
  if (info.uptime < 20L)
    return 1;
  return 0;
}

int
check_vlan_support (void)
{
#ifdef HAVE_RB500
  return 0;
#else

  uint boardflags;

  //if (getRouterBrand () == ROUTER_ASUS)
  //    return 1;
  boardflags = strtoul (nvram_safe_get ("boardflags"), NULL, 0);
  if (boardflags & BFL_ENETVLAN)
    return 1;

//  if ((nvram_match ("boardtype", "0x0101") || (boardflags & 0x0100))
//  && nvram_invmatch ("boardnum", "2"))
  if (getRouterBrand () == ROUTER_LINKSYS_WRT55AG
      || getRouterBrand () == ROUTER_MOTOROLA_V1)
    return 0;

  if (nvram_match ("boardtype", "bcm94710dev")
      || nvram_match ("boardtype", "0x0101") || (boardflags & 0x0100))
    return 1;
  else
    return 0;
#endif
}

#ifdef CDEBUG
int
coreleft (void)
{
  struct sysinfo info;
  sysinfo (&info);
  return info.freeram;
}

int
mcoreleft (void)
{
  struct mallinfo minfo;
  minfo = mallinfo ();
  return minfo.uordblks;
  //int uordblks; /* total allocated space */

}
#endif

#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

//#define WDS_DEBUG 1
#undef WDS_DEBUG
#ifdef WDS_DEBUG
FILE *fp;
#endif


int
wds_dev_config (int dev, int up)
{
  char wds_var[32] = "";
  char wds_enable_var[32] = "";
  char wds_dev[32] = "";
  char *wds = (void *) 0;
  char wds_gw_var[32] = "";
  char cmd[100] = "";
  char *gw = (void *) 0;
  int s = -1;
  struct ifreq ifr;

#ifdef WDS_DEBUG
  fp = fopen ("/tmp/.wds_debug.log", "a");
#endif

  memset (&ifr, 0, sizeof (struct ifreq));

  snprintf (wds_var, 31, "wl_wds%d", dev);
  snprintf (wds_enable_var, 31, "%s_enable", wds_var);

  if ((wds = nvram_safe_get (wds_enable_var)) == NULL ||
      strcmp (wds, "0") == 0)
    return -1;

  snprintf (wds_dev, 31, "wds0.491%d", 50 + dev + 1);

  snprintf (ifr.ifr_name, IFNAMSIZ, wds_dev);
#ifdef WDS_DEBUG
  fprintf (fp, "opening kernelsocket\n");
#endif
  if ((s = socket (AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
    return -1;

  if (up)
    {
      char wds_hwaddr_var[32] = "";
      char wds_ip_var[32] = "";
      char wds_netmask_var[32] = "";
      char *wds_list = (void *) 0;
      char *hwaddr = (void *) 0;
      char *ip = (void *) 0;
      char *netmask = (void *) 0;

#ifdef WDS_DEBUG
      fprintf (fp, "running up\n");
#endif

      wds_list = nvram_safe_get ("wl0_wds");
      if (wds_list == (void *) 0 || strlen (wds_list) <= 0)
	return 0;

      snprintf (wds_hwaddr_var, 31, "%s_hwaddr", wds_var);
      snprintf (wds_ip_var, 31, "%s_ipaddr", wds_var);
      snprintf (wds_netmask_var, 31, "%s_netmask", wds_var);

      hwaddr = nvram_safe_get (wds_hwaddr_var);
      ip = nvram_safe_get (wds_ip_var);
      netmask = nvram_safe_get (wds_netmask_var);

      if (!strstr (wds_list, hwaddr))
	return -1;

#ifdef WDS_DEBUG
      fprintf (fp, "checking validity\n");
#endif

      if (!sv_valid_hwaddr (hwaddr) || !sv_valid_ipaddr (ip)
	  || !sv_valid_ipaddr (netmask))
	return -1;

#ifdef WDS_DEBUG
      fprintf (fp, "valid mac %s ip %s nm %s\n", hwaddr, ip, netmask);
#endif

      snprintf (cmd, 99, "ifconfig %s down", wds_dev);
      system (cmd);

      snprintf (cmd, 99, "ifconfig %s %s netmask %s up", wds_dev, ip,
		netmask);
      system (cmd);

      snprintf (wds_gw_var, 31, "%s_gw", wds_var);
      gw = nvram_safe_get (wds_gw_var);
      if (strcmp (gw, "0.0.0.0") != 0)
	{
	  get_network (ip, netmask);
	  route_del (wds_dev, 0, ip, gw, netmask);
	  route_add (wds_dev, 0, ip, gw, netmask);
	}

    }
  else
    {
#ifdef WDS_DEBUG
      fprintf (fp, "running down\n");
#endif
      snprintf (cmd, 99, "ifconfig %s down", wds_dev);
      system (cmd);

    }

#ifdef WDS_DEBUG
  fprintf (fp, "running ioctl\n");
  fclose (fp);
#endif

  close (s);

  return 0;
}


int
ishexit (char c)
{

  if (strchr ("01234567890abcdefABCDEF", c) != (char *) 0)
    return 1;

  return 0;
}


/* Example:
 * legal_hwaddr("00:11:22:33:44:aB"); return true;
 * legal_hwaddr("00:11:22:33:44:5"); return false;
 * legal_hwaddr("00:11:22:33:44:HH"); return false;
 */
int
sv_valid_hwaddr (char *value)
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
      else if (ishexit (*(value + i)))	/* one of 0 1 2 3 4 5 6 7 8 9 a b c d e f A B C D E F */
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
      tag = FALSE;
    }
  else
    tag = TRUE;
#ifdef WDS_DEBUG
  if (tag == FALSE)
    fprintf (fp, "failed valid_hwaddr\n");
#endif

  return tag;
}


int
sv_valid_range (char *value, int low, int high)
{
  if (!isdigit (value[0]) || atoi (value) < low || atoi (value) > high)
    return FALSE;
  else
    return TRUE;

}

int
sv_valid_statics (char *value)
{
  char ip[16] = { 0 }, mac[18] =
  {
  0}, hostname[255] =
  {
  0}, *p = value;

  if (NULL == value)
    return FALSE;

  do
    {
      while (isspace (*p++) && p - value < strlen (value))
	;

      if (p - value >= strlen (value))
	return FALSE;

      if (sscanf (p, "%15s%17s%254s", ip, mac, hostname) < 3)
	return FALSE;

      if (!sv_valid_ipaddr (ip) || !sv_valid_hwaddr (mac)
	  || strlen (hostname) <= 0)
	return FALSE;

    }
  while ((p = strpbrk (p, "\n\r")) && p - value < strlen (value));

  return TRUE;
}

/* Example:
 * legal_ipaddr("192.168.1.1"); return true;
 * legal_ipaddr("192.168.1.1111"); return false;
 */
int
sv_valid_ipaddr (char *value)
{
  struct in_addr ipaddr;
  int ip[4], ret = 0;

  ret = sscanf (value, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);

  if (ret != 4 || !inet_aton (value, &ipaddr))
    return FALSE;
  else
    return TRUE;

}

// note - networl address returned in ipaddr
void
get_network (char *ipaddr, char *netmask)
{
  int ip[4], mask[4];

  if (!ipaddr || !netmask)
    return;

  sscanf (ipaddr, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
  sscanf (netmask, "%d.%d.%d.%d", &mask[0], &mask[1], &mask[2], &mask[3]);

  ip[0] &= mask[0];
  ip[1] &= mask[1];
  ip[2] &= mask[2];
  ip[3] &= mask[3];

  sprintf (ipaddr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
#ifdef WDS_DEBUG
  fprintf (fp, "get_network return %s\n", ipaddr);
#endif

}

// note - broadcast addr returned in ipaddr
void
get_broadcast (char *ipaddr, char *netmask)
{
  int ip[4], mask[4];

  if (!ipaddr || !netmask)
    return;

  sscanf (ipaddr, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
  sscanf (netmask, "%d.%d.%d.%d", &mask[0], &mask[1], &mask[2], &mask[3]);

  ip[0] = (ip[0] & mask[0]) | !mask[0];
  ip[1] = (ip[1] & mask[1]) | !mask[1];
  ip[2] = (ip[2] & mask[2]) | !mask[2];
  ip[3] = (ip[3] & mask[3]) | !mask[3];

  sprintf (ipaddr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
#ifdef WDS_DEBUG
  fprintf (fp, "get_broadcast return %s\n", value);
#endif

}

/* note: copied from Broadcom code and put in shared via this file */

int
route_manip (int cmd, char *name, int metric, char *dst, char *gateway,
	     char *genmask)
{
  int s;
  struct rtentry rt;

  //dprintf("cmd=[%d] name=[%s] ipaddr=[%s] netmask=[%s] gateway=[%s] metric=[%d]\n",cmd,name,dst,genmask,gateway,metric);

  /* Open a raw socket to the kernel */
  if ((s = socket (AF_INET, SOCK_DGRAM, 0)) < 0)
    goto err;

  /* Fill in rtentry */
  memset (&rt, 0, sizeof (rt));
  if (dst)
    inet_aton (dst, &sin_addr (&rt.rt_dst));
  if (gateway)
    inet_aton (gateway, &sin_addr (&rt.rt_gateway));
  if (genmask)
    inet_aton (genmask, &sin_addr (&rt.rt_genmask));
  rt.rt_metric = metric;
  rt.rt_flags = RTF_UP;
  if (sin_addr (&rt.rt_gateway).s_addr)
    rt.rt_flags |= RTF_GATEWAY;
  if (sin_addr (&rt.rt_genmask).s_addr == INADDR_BROADCAST)
    rt.rt_flags |= RTF_HOST;
  rt.rt_dev = name;

  /* Force address family to AF_INET */
  rt.rt_dst.sa_family = AF_INET;
  rt.rt_gateway.sa_family = AF_INET;
  rt.rt_genmask.sa_family = AF_INET;

  if (ioctl (s, cmd, &rt) < 0)
    goto err;

  close (s);
  return 0;

err:
  close (s);
  perror (name);
  return errno;
}

int
route_add (char *name, int metric, char *dst, char *gateway, char *genmask)
{
  return route_manip (SIOCADDRT, name, metric, dst, gateway, genmask);
}

int
route_del (char *name, int metric, char *dst, char *gateway, char *genmask)
{
  return route_manip (SIOCDELRT, name, metric, dst, gateway, genmask);
}
