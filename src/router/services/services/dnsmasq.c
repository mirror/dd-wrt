#ifdef HAVE_DNSMASQ
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <syslog.h>
#include <utils.h>
#include <bcmnvram.h>
#include <shutils.h>


extern int usejffs;


extern void addHost (char *host, char *ip);


int stop_dnsmasq (void);


int
start_dnsmasq (void)
{
  FILE *fp;
  struct dns_lists *dns_list = NULL;
  int ret;
  int i;

  if (nvram_match ("dhcp_dnsmasq", "1") && nvram_match ("lan_proto", "dhcp")
      && nvram_match ("dnsmasq_enable", "0"))
    {
      nvram_set ("dnsmasq_enable", "1");
      nvram_commit ();
    }

  // Sveasoft 2003-12-15 only start if enabled
  if (!nvram_invmatch ("dnsmasq_enable", "0"))
    {
      stop_dnsmasq ();
      return 0;
    }

  usejffs = 0;

  if (nvram_match ("dhcpd_usejffs", "1"))
    {
      if (!(fp = fopen ("/jffs/dnsmasq.leases", "a")))
	{
	  usejffs = 0;
	}
      else
	{
	  fclose (fp);
	  usejffs = 1;
	}
    }

  /* Write configuration file based on current information */
  if (!(fp = fopen ("/tmp/dnsmasq.conf", "w")))
    {
      perror ("/tmp/dnsmasq.conf");
      return errno;
    }

  if (nvram_match ("fon_enable", "1")
      || (nvram_match ("chilli_nowifibridge", "1")
	  && nvram_match ("chilli_enable", "1")))
    {
      fprintf (fp, "interface=%s,br0\n", nvram_safe_get ("wl0_ifname"));
    }
  else
    {
      if (nvram_match ("chilli_enable", "1"))
	fprintf (fp, "interface=%s\n", nvram_safe_get ("wl0_ifname"));
      else if (nvram_match ("pptpd_enable", "1"))
	fprintf (fp, "listen-address=%s,%s\n", "127.0.0.1",
		 nvram_safe_get ("lan_ipaddr"));
      else
	fprintf (fp, "interface=%s\n", nvram_safe_get ("lan_ifname"));
    }

  fprintf (fp, "resolv-file=/tmp/resolv.dnsmasq\n");

  /* Domain */
  if (nvram_match ("dhcp_domain", "wan"))
    {
      if (nvram_invmatch ("wan_domain", ""))
	fprintf (fp, "domain=%s\n", nvram_safe_get ("wan_domain"));
      else if (nvram_invmatch ("wan_get_domain", ""))
	fprintf (fp, "domain=%s\n", nvram_safe_get ("wan_get_domain"));
    }
  else
    {
      if (nvram_invmatch ("lan_domain", ""))
	fprintf (fp, "domain=%s\n", nvram_safe_get ("lan_domain"));
    }

  /* DD-WRT use dnsmasq as DHCP replacement */
  if (!nvram_match ("wl0_mode", "wet")
      && !nvram_match ("wl0_mode", "apstawet"))
    if (nvram_match ("dhcp_dnsmasq", "1") && nvram_match ("lan_proto", "dhcp")
	&& nvram_match ("dhcpfwd_enable", "0"))
      {
	/* DHCP leasefile */
	if (nvram_match ("dhcpd_usenvram", "1"))
	  {
	    fprintf (fp, "leasefile-ro\n");
	    fprintf (fp, "dhcp-script=%s\n", "/etc/lease_update.sh");
	  }
	else
	  {
	    if (usejffs)
	      fprintf (fp, "dhcp-leasefile=/jffs/dnsmasq.leases\n");
	    else
	      fprintf (fp, "dhcp-leasefile=/tmp/dnsmasq.leases\n");
	  }

	int dhcp_max =
	  atoi (nvram_safe_get ("dhcp_num")) +
	  atoi (nvram_safe_get ("static_leasenum"));
	fprintf (fp, "dhcp-lease-max=%d\n", dhcp_max);
	fprintf (fp, "dhcp-option=3,%s\n", nvram_safe_get ("lan_ipaddr"));
	if (nvram_invmatch ("wan_wins", "")
	    && nvram_invmatch ("wan_wins", "0.0.0.0"))
	  fprintf (fp, "dhcp-option=44,%s\n", nvram_safe_get ("wan_wins"));

	if (nvram_match ("dns_dnsmasq", "0"))
	  {
	    dns_list = get_dns_list ();

	    if (dns_list
		&& (strlen (dns_list->dns_server[0]) > 0
		    || strlen (dns_list->dns_server[1]) > 0
		    || strlen (dns_list->dns_server[2]) > 0))
	      {

		fprintf (fp, "dhcp-option=6");

		if (strlen (dns_list->dns_server[0]) > 0)
		  fprintf (fp, ",%s", dns_list->dns_server[0]);

		if (strlen (dns_list->dns_server[1]) > 0)
		  fprintf (fp, ",%s", dns_list->dns_server[1]);

		if (strlen (dns_list->dns_server[2]) > 0)
		  fprintf (fp, ",%s", dns_list->dns_server[2]);

		fprintf (fp, "\n");
	      }

	    if (dns_list)
	      free (dns_list);
	  }

	if (nvram_match ("auth_dnsmasq", "1"))
	  fprintf (fp, "dhcp-authoritative\n");
	fprintf (fp, "dhcp-range=");
	fprintf (fp, "%d.%d.%d.%s,",
		 get_single_ip (nvram_safe_get ("lan_ipaddr"), 0),
		 get_single_ip (nvram_safe_get ("lan_ipaddr"), 1),
		 get_single_ip (nvram_safe_get ("lan_ipaddr"), 2),
		 nvram_safe_get ("dhcp_start"));
	fprintf (fp, "%d.%d.%d.%d,",
		 get_single_ip (nvram_safe_get ("lan_ipaddr"), 0),
		 get_single_ip (nvram_safe_get ("lan_ipaddr"), 1),
		 get_single_ip (nvram_safe_get ("lan_ipaddr"), 2),
		 atoi (nvram_safe_get ("dhcp_start")) +
		 atoi (nvram_safe_get ("dhcp_num")) - 1);
	fprintf (fp, "%s,", nvram_safe_get ("lan_netmask"));
	fprintf (fp, "%sm\n", nvram_safe_get ("dhcp_lease"));
	int leasenum = atoi (nvram_safe_get ("static_leasenum"));
	if (leasenum > 0)
	  {
	    char *lease = nvram_safe_get ("static_leases");
	    char *leasebuf = (char *) malloc (strlen (lease) + 1);
	    char *cp = leasebuf;
	    strcpy (leasebuf, lease);
	    for (i = 0; i < leasenum; i++)
	      {
		char *mac = strsep (&leasebuf, "=");
		char *host = strsep (&leasebuf, "=");
		char *ip = strsep (&leasebuf, " ");
		if (mac == NULL || host == NULL || ip == NULL)
		  continue;

		fprintf (fp, "dhcp-host=%s,%s,%s,infinite\n", mac, host, ip);
		addHost (host, ip);
	      }
	    free (cp);
	  }
      }

  /* Additional options */
  if (nvram_invmatch ("dnsmasq_options", ""))
    {
      char *host_key = nvram_safe_get ("dnsmasq_options");
      i = 0;
      do
	{
	  if (host_key[i] != 0x0D)
	    fprintf (fp, "%c", host_key[i]);
	}
      while (host_key[++i]);
    }
  fclose (fp);

  dns_to_resolv ();

  chmod ("/etc/lease_update.sh", 0700);
  ret = eval ("dnsmasq", "--conf-file", "/tmp/dnsmasq.conf");
  syslog (LOG_INFO, "dnsmasq : dnsmasq daemon successfully started\n");

  cprintf ("done\n");
  return ret;
}

int
stop_dnsmasq (void)
{
  if (pidof ("dnsmasq") > 0)
    syslog (LOG_INFO, "dnsmasq : dnsmasq daemon successfully stopped\n");
  int ret = softkill ("dnsmasq");
  unlink ("/tmp/resolv.dnsmasq");

  cprintf ("done\n");
  return ret;
}
#endif
