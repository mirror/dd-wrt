  
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <ctype.h>

#include <broadcom.h>

#define DHCP_MAX_COUNT 254
#define EXPIRES_NEVER 0xFFFFFFFF	/* static lease */

char *
dhcp_reltime (char *buf, time_t t)
{
  int days;
  int min;
  if (t < 0)
    t = 0;
  days = t / 86400;
  min = t / 60;
  sprintf (buf, "%d day%s %02d:%02d:%02d", days, ((days == 1) ? "" : "s"),
	   ((min / 60) % 24), (min % 60), (int) (t % 60));
  return buf;
}

/*dump in array: hostname,mac,ip,expires
read leases from leasefile as: expires mac ip hostname */
void
ej_dumpleases (int eid, webs_t wp, int argc, char_t ** argv)
{
  FILE *fp;
  unsigned long expires;
  int i = 0;
  int count = 0;
  int macmask;

  if (ejArgs (argc, argv, "%d", &macmask) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }

  if (nvram_match ("dhcp_dnsmasq", "1"))
    {
      char mac[32];
      char ip[32];
      char hostname[256];
      char buf[512];
      char *p;
      char *buff;

  if (nvram_invmatch ("dhcpd_usenvram", "1"))
    {
      /* Parse leases file */
      if (!(fp = fopen ("/tmp/dnsmasq.leases", "r")))
	fp = fopen ("/jffs/dnsmasq.leases", "r");

      if (fp)
	{
	  while (fgets (buf, sizeof (buf), fp))
	    {
	      if (sscanf
		  (buf, "%lu %17s %15s %255s", &expires, mac, ip,
		   hostname) != 4)
		continue;
	      p = mac;
	      while ((*p = toupper (*p)) != 0)
		++p;
	      if ((p = strrchr (ip, '.')) == NULL)
		continue;
	      if (!strcmp (mac, "00:00:00:00:00:00"))
		continue;
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
	      websWrite (wp, "%c'%s','%s','%s','%s','%s'",
			 (count ? ',' : ' '),
			 (hostname[0] ? hostname : "unknown"),
			 ip, mac,
			 ((expires == 0) ? "never" : dhcp_reltime (buf,
								   expires)),
			 p + 1);
	      ++count;
	    }
	  fclose (fp);
	}
    }
 else
    {
      for (i = 0; i < DHCP_MAX_COUNT; ++i)
	{
	      sprintf (buf, "dnsmasq_lease_%d.%d.%d.%d",
	      get_single_ip (nvram_safe_get ("lan_ipaddr"), 0),
	      get_single_ip (nvram_safe_get ("lan_ipaddr"), 1),
	      get_single_ip (nvram_safe_get ("lan_ipaddr"), 2), i);

	      buff = nvram_safe_get(buf);
	      if (sscanf
		  (buff, "%lu %17s %15s %255s", &expires, mac, ip,
		   hostname) != 4)
		continue;
	      p = mac;
	      while ((*p = toupper (*p)) != 0)
		++p;
	      if ((p = strrchr (ip, '.')) == NULL)
		continue;
	      if (!strcmp (mac, "00:00:00:00:00:00"))
		continue;
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
	      websWrite (wp, "%c'%s','%s','%s','%s','%s'",
			 (count ? ',' : ' '),
			 (hostname[0] ? hostname : "unknown"),
			 ip, mac,
			 ((expires == 0) ? "never" : dhcp_reltime (buf,
								   expires)),
			 p + 1);
	      ++i;
              ++count;
	  }
    }
  }
  else
    {
      struct lease_t lease;
      struct in_addr addr;
      char sigusr1[] = "-XX";
      char *ipaddr, mac[20] = "", expires_time[50] = "";

      /* Write out leases file from udhcpd */
      sprintf (sigusr1, "-%d", SIGUSR1);
      eval ("killall", sigusr1, "udhcpd");

      /* Parse leases file */
      if (!(fp = fopen ("/tmp/udhcpd.leases", "r")))
	fp = fopen ("/jffs/udhcpd.leases", "r");

      if (fp)
	{
	  while (fread (&lease, sizeof (lease), 1, fp))
	    {
	      strcpy (mac, "");

	      for (i = 0; i < 6; i++)
		{
		  sprintf (mac + strlen (mac), "%02X", lease.chaddr[i]);
		  if (i != 5)
		    sprintf (mac + strlen (mac), ":");
		}
	      mac[17] = '\0';
	      if (!strcmp (mac, "00:00:00:00:00:00"))
		continue;
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
	      addr.s_addr = lease.yiaddr;

	      ipaddr = inet_ntoa (addr);

	      expires = ntohl (lease.expires);

	      strcpy (expires_time, "");
	      if (!expires)
		{
		  continue;
		  strcpy (expires_time, "expired");
		}
	      else if (expires == (long) EXPIRES_NEVER)
		{
		  strcpy (expires_time, "never");
		}
	      else
		{
		  if (expires > 86400)	//60 * 60 * 24
		    {
		      sprintf (expires_time + strlen (expires_time), "%ld days ", expires / 86400);	//60 * 60 * 24
		      expires %= 86400;	//60 * 60 * 24
		    }
		  if (expires > 3600)	//60 * 60 
		    {
		      sprintf (expires_time + strlen (expires_time), "%02ld:", expires / (60 * 60));	// hours
		      expires %= 3600;	//60 * 60
		    }
		  else
		    {
		      sprintf (expires_time + strlen (expires_time), "00:");	// no hours
		    }
		  if (expires > 60)
		    {
		      sprintf (expires_time + strlen (expires_time), "%02ld:", expires / 60);	// minutes
		      expires %= 60;
		    }
		  else
		    {
		      sprintf (expires_time + strlen (expires_time), "00:");	// no minutes
		    }

		  sprintf (expires_time + strlen (expires_time), "%02ld:", expires);	// seconds

		  expires_time[strlen (expires_time) - 1] = '\0';
		}
	      websWrite (wp, "%c\"%s\",\"%s\",\"%s\",\"%s\",\"%d\"",
			 count ? ',' : ' ',
			 !*lease.hostname ? "&nbsp;" : lease.hostname, ipaddr,
			 mac, expires_time, get_single_ip (inet_ntoa (addr),
							   3));
	      count++;
	    }
	  fclose (fp);
	}
    }
  return;
}

/* Delete lease */
int
delete_leases (webs_t wp)
{
  char buf[100];
  char *mac;
  char *ip;

  if (nvram_match ("lan_proto", "static"))
    return -1;

  mac = websGetVar (wp, "mac_del", NULL);
  ip = websGetVar (wp, "ip_del", NULL);

  sprintf (buf, "dhcp_release %s %s %s", "br0", ip, mac);
  system (buf);

  return 0;
}


void
dhcp_check (webs_t wp, char *value, struct variable *v)
{
  return;			// The udhcpd can valid lease table when re-load udhcpd.leases. by honor 2003-08-05
}

int
dhcp_renew (webs_t wp)
{
  int ret;
  char sigusr[] = "-XX";

  sprintf (sigusr, "-%d", SIGUSR1);
  ret = eval ("killall", sigusr, "udhcpc");

  return ret;
}

int
dhcp_release (webs_t wp)
{
  int ret = 0;

  nvram_set ("wan_ipaddr", "0.0.0.0");
  nvram_set ("wan_netmask", "0.0.0.0");
  nvram_set ("wan_gateway", "0.0.0.0");
  nvram_set ("wan_get_dns", "");
  nvram_set ("wan_lease", "0");

  unlink ("/tmp/get_lease_time");
  unlink ("/tmp/lease_time");

  return ret;
}
