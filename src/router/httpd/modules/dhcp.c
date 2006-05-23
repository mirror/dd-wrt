
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>

#include <broadcom.h>

#define DHCP_MAX_COUNT 254

char *
dnsmasq_reltime (char *buf, time_t t)
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
  char mac[32];
  char ip[32];
  char hostname[256];
  char buf[512];
  int count;
  char *p;

  /* Dump leases from DNSMasq */
  eval ("killall", "-SIGUSR2", "dnsmasq");

  /* Parse leases file */
  if (!(fp = fopen ("/tmp/udhcpd.leases", "r")))
    fp = fopen ("/jffs/udhcpd.leases", "r");
  if (fp)
    {
      count = 0;
      while (fgets (buf, sizeof (buf), fp))
	{
	  if (sscanf (buf, "%lu %17s %15s %255s", &expires, mac, ip, hostname)
	      != 4)
	    continue;
	  p = mac;
	  while ((*p = toupper (*p)) != 0)
	    ++p;
	  if ((p = strrchr (ip, '.')) == NULL)
	    continue;
	  websWrite (wp, "%c'%s','%s','%s','%s','%s'",
		     (count ? ',' : ' '),
		     (hostname[0] ? hostname : "unknown"),
		     ip, mac,
		     ((expires == 0) ? "never" : dnsmasq_reltime (buf,
								  expires)),
		     p + 1);
	  ++count;
	}
      fclose (fp);
    }
  return;
}

// void
// ej_dumpleases (int eid, webs_t wp, int argc, char_t ** argv)
// {
//   FILE *fp;
//   struct lease_t lease;
//   int i;
//   struct in_addr addr;
//   unsigned long expires;
//   char sigusr1[] = "-XX";
//   int count = 0;
//   char *ipaddr, mac[20] = "", expires_time[50] = "";
//   int macmask;
//   if (ejArgs (argc, argv, "%d", &macmask) < 1)
//     {
//       websError (wp, 400, "Insufficient args\n");
//       return;
//     }
// 
// 
//   /* Write out leases file */
//   sprintf (sigusr1, "-%d", SIGUSR1);
//   eval ("killall", sigusr1, "udhcpd");
// 
// 
//   /* Parse leases file */
//   if (!(fp = fopen ("/tmp/udhcpd.leases", "r")))
//     fp = fopen ("/jffs/udhcpd.leases", "r");
//   if (fp)
//     {
//       while (fread (&lease, sizeof (lease), 1, fp))
//      {
//        strcpy (mac, "");
// 
//        for (i = 0; i < 6; i++)
//          {
//            sprintf (mac + strlen (mac), "%02X", lease.chaddr[i]);
//            if (i != 5)
//              sprintf (mac + strlen (mac), ":");
//          }
//        mac[17] = '\0';
//        if (!strcmp (mac, "00:00:00:00:00:00"))
//          continue;
//        if (nvram_match ("maskmac", "1") && macmask)
//          {
// 
//            mac[0] = 'x';
//            mac[1] = 'x';
//            mac[3] = 'x';
//            mac[4] = 'x';
//            mac[6] = 'x';
//            mac[7] = 'x';
//            mac[9] = 'x';
//            mac[10] = 'x';
//          }
//        addr.s_addr = lease.yiaddr;
// 
//        ipaddr = inet_ntoa (addr);
// 
//        expires = ntohl (lease.expires);
// 
//        strcpy (expires_time, "");
//        if (!expires)
//          {
//            continue;
//            strcpy (expires_time, "expired");
//          }
//        else if (expires == (long) EXPIRES_NEVER)
//          {
//            strcpy (expires_time, "never");
//          }
//        else
//          {
//            if (expires > 60 * 60 * 24)
//              {
//                sprintf (expires_time + strlen (expires_time), "%ld days, ",
//                         expires / (60 * 60 * 24));
//                expires %= 60 * 60 * 24;
//              }
//            if (expires > 60 * 60)
//              {
//                sprintf (expires_time + strlen (expires_time), "%02ld:", expires / (60 * 60));        // hours
//                expires %= 60 * 60;
//              }
//            else
//              {
//                sprintf (expires_time + strlen (expires_time), "00:");        // no hours
//              }
//            if (expires > 60)
//              {
//                sprintf (expires_time + strlen (expires_time), "%02ld:", expires / 60);       // minutes
//                expires %= 60;
//              }
//            else
//              {
//                sprintf (expires_time + strlen (expires_time), "00:");        // no minutes
//              }
// 
//            sprintf (expires_time + strlen (expires_time), "%02ld:", expires);        // seconds
// 
//            expires_time[strlen (expires_time) - 1] = '\0';
//          }
//        websWrite (wp, "%c\"%s\",\"%s\",\"%s\",\"%s\",\"%d\"", count ? ',' : ' ',
//                   !*lease.hostname ? "&nbsp;" : lease.hostname, ipaddr,
//                   mac, expires_time, get_single_ip (inet_ntoa (addr), 3));
//        count++;
//      }
//       fclose (fp);
//     }
// 
//   return;
// }

/* Delete leases */
int
delete_leases (webs_t wp)
{
  FILE *f;
  int i;
  char name[32];
  const char *value;

  if ((f = fopen ("/tmp/.delete_leases", "w")) == NULL)
    {
      websError (wp, 400, "Error opening delete lease file\n");
      return -1;
    }
  for (i = 0; i < DHCP_MAX_COUNT; ++i)
    {
      sprintf (name, "d_%d", i);
      value = websGetVar (wp, name, NULL);
      if (!value)
	continue;
      fprintf (f, "%d.%d.%d.%s\n",
	       get_single_ip (nvram_safe_get ("lan_ipaddr"), 0),
	       get_single_ip (nvram_safe_get ("lan_ipaddr"), 1),
	       get_single_ip (nvram_safe_get ("lan_ipaddr"), 2), value);
    }
  fclose (f);

  eval ("killall", "-SIGUSR2", "dnsmasq");
  return 0;

}

//   FILE *fp_w;
//   char sigusr1[] = "-XX";
//   int i;
//   char buff[8];
//   char **ipbuf;
// 
//   if (nvram_match ("lan_proto", "static"))
//     return;
// 
//   unlink ("/tmp/.delete_leases");
// 
//   if (!(fp_w = fopen ("/tmp/.delete_leases", "w")))
//     {
//       websError (wp, 400, "Write leases error\n");
//       return;
//     }
//   int ipcount = 0;
//   for (i = 0; i < DHCP_MAX_COUNT; i++)
//     {
//       char name[] = "d_XXX";
//       char *value;
//       snprintf (name, sizeof (name), "d_%d", i);
//       value = websGetVar (wp, name, NULL);
//       if (!value)
//      continue;
//       fprintf (fp_w, "%d.%d.%d.%s\n",
//             get_single_ip (nvram_safe_get ("lan_ipaddr"), 0),
//             get_single_ip (nvram_safe_get ("lan_ipaddr"), 1),
//             get_single_ip (nvram_safe_get ("lan_ipaddr"), 2), value);
//       ipcount++;
//     }
// 
//   fclose (fp_w);
// 
//   if (nvram_match ("dhcp_dnsmasq", "1"))
//     {
//       if (!(fp_w = fopen ("/var/run/dnsmasq.pid", "r")))
//      {
//        websError (wp, 400, "Write leases error\n");
//        return;
//      }
//       if (ipcount == 0)
//      return;
//       fgets (buff, sizeof (buff), fp_w);
//       sprintf (sigusr1, "-%d", SIGUSR2);
//       eval ("kill", sigusr1, buff);  // call udhcpd to delete ip from lease table
//       //delete leases
//       struct lease_t lease;
//       struct in_addr addr;
//       char *ipaddr;
//       FILE *fp;
//       int usejffs = 0;
//       if (!(fp = fopen ("/tmp/udhcpd.leases", "r")))
//      {
//        usejffs = 1;
//        if (!(fp = fopen ("/jffs/udhcpd.leases", "r")))
//          {
//            perror ("could not open input file");
//            return;
//          }
//      }
//       FILE *nfp = fopen ("/tmp/newdhcp.leases", "wb");
//       ipbuf = malloc (ipcount * sizeof (ipbuf));
//       ipcount = 0;
//       for (i = 0; i < DHCP_MAX_COUNT; i++)
//      {
//        char name[] = "d_XXX";
//        char *value;
//        snprintf (name, sizeof (name), "d_%d", i);
//        value = websGetVar (wp, name, NULL);
//        if (!value)
//          continue;
//        ipbuf[ipcount] = (char *) malloc (20);
//        sprintf (ipbuf[ipcount++], "%d.%d.%d.%s",
//                 get_single_ip (nvram_safe_get ("lan_ipaddr"), 0),
//                 get_single_ip (nvram_safe_get ("lan_ipaddr"), 1),
//                 get_single_ip (nvram_safe_get ("lan_ipaddr"), 2), value);
//        cprintf ("%s needs to be deleted\n", ipbuf[ipcount - 1]);
//      }
// 
//       while (fread (&lease, sizeof (lease), 1, fp))
//      {
//        addr.s_addr = lease.yiaddr;
//        ipaddr = inet_ntoa (addr);
//        for (i = 0; i < ipcount; i++)
//          {
//            cprintf ("comparing %s with %s\n", ipbuf[i], ipaddr);
//            if (strcmp (ipbuf[i], ipaddr))
//              fwrite (&lease, sizeof (lease), 1, nfp);
//          }
//      }
//       fclose (nfp);
//       fclose (fp);
//       eval ("dnsmasq", "--conf-file", "/tmp/dnsmasq.conf");
//       if (usejffs)
//      eval ("mv", "-f", "/tmp/newdhcp.leases", "/jffs/udhcpd.leases");
//       else
//      eval ("mv", "-f", "/tmp/newdhcp.leases", "/tmp/udhcpd.leases");
//       for (i = 0; i < ipcount; i++)
//      {
//        free (ipbuf[i]);
//      }
//       free (ipbuf);
//     }
//   else
//     {
//       if (!(fp_w = fopen ("/var/run/udhcpd.pid", "r")))
//      {
//        websError (wp, 400, "Write leases error\n");
//        return;
//      }
//       fgets (buff, sizeof (buff), fp_w);
// 
//       sprintf (sigusr1, "-%d", SIGUSR2);
//       eval ("kill", sigusr1, buff);  // call udhcpd to delete ip from lease table
// 
//     }
// 
//   return;
//}


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
