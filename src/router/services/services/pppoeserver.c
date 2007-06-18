#ifdef HAVE_PPPOESERVER
#include <stdio.h>
#include <signal.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <malloc.h>
#include <sys/stat.h>
#include <syslog.h>

void
add_pppoe_natrule (void)
{

  if (nvram_match ("wan_proto", "disabled"))
    {
      char mask[128];
      sprintf (mask, "%s/%s", nvram_safe_get ("pppoeserver_remotenet"),
	       nvram_safe_get ("pppoeserver_remotemask"));
      eval ("iptables", "-A", "INPUT", "-i", nvram_get ("lan_ipaddr"), "-s",
	    mask, "-j", "DROP");
      eval ("iptables", "-t", "nat", "-A", "POSTROUTING", "-s", mask, "-j",
	    "SNAT", "--to-source", nvram_get ("lan_ipaddr"));
    }
}

void
del_pppoe_natrule (void)
{
  if (nvram_match ("wan_proto", "disabled"))
    {
      char mask[128];
      sprintf (mask, "%s/%s", nvram_safe_get ("pppoeserver_remotenet"),
	       nvram_safe_get ("pppoeserver_remotemask"));
      eval ("iptables", "-D", "INPUT", "-i", nvram_get ("lan_ipaddr"), "-s",
	    mask, "-j", "DROP");
      eval ("iptables", "-t", "nat", "-D", "POSTROUTING", "-s", mask, "-j",
	    "SNAT", "--to-source", nvram_get ("lan_ipaddr"));
    }
}

void
start_pppoeserver (void)
{
  if (nvram_default_match ("pppoeserver_enabled", "1", "0"))
    {
      add_pppoe_natrule ();
      if (nvram_default_match ("pppoeradius_enabled", "0", "0"))
	{
	  FILE *fp;
	  mkdir ("/tmp/ppp", 0777);
	  fp = fopen ("/tmp/ppp/pppoe-server-options", "wb");
	  //  fprintf (fp, "crtscts\n");
	  if (nvram_default_match ("pppoeserver_bsdcomp", "0", "0"))
	    fprintf (fp, "nobsdcomp\n");
	  else
	    fprintf (fp, "bsdcomp 12\n");
	  if (nvram_default_match ("pppoeserver_deflate", "0", "0"))
	    fprintf (fp, "nodeflate\n");
	  else
	    fprintf (fp, "deflate 12\n");
	  if (nvram_default_match ("pppoeserver_lzs", "0", "0"))
	    fprintf (fp, "nolzs\n");
	  else
	    fprintf (fp, "lzs\n");
	  if (nvram_default_match ("pppoeserver_mppc", "0", "0"))
	    fprintf (fp, "nomppc\n");
	  else
	    fprintf (fp, "mppc\n");
	  fprintf (fp, "nopcomp\n");
	  fprintf (fp, "idle %s\n", nvram_safe_get ("pppoeserver_idle"));	//todo ...
	  if (nvram_default_match ("pppoeserver_encryption", "1", "0"))	// make it configureable
	    {
	      fprintf (fp, "mppe required,no56,no40,stateless\n");
	      fprintf (fp, "refuse-eap\n");
	      fprintf (fp, "refuse-pap\n");
	      fprintf (fp, "refuse-chap\n");
	      fprintf (fp, "refuse-mschap\n");
	      fprintf (fp, "require-mschap-v2\n");
	    }
	  else
	    fprintf (fp, "nomppe\n");
	  fprintf (fp, "auth\n");
	  fprintf (fp, "default-mru\n");
	  fprintf (fp, "default-asyncmap\n");
	  fprintf (fp, "lcp-echo-interval %s\n",
		   nvram_safe_get ("pppoeserver_lcpechoint"));
	  fprintf (fp, "lcp-echo-failure %s\n",
		   nvram_safe_get ("pppoeserver_lcpechofail"));
	  struct dns_lists *dns_list = get_dns_list ();
	  if (!dns_list || dns_list->num_servers == 0)
	    {
	      if (nvram_invmatch ("lan_ipaddr", ""))
		fprintf (fp, "ms-dns %s\n", nvram_safe_get ("lan_ipaddr"));
	    }
	  else if (nvram_match ("local_dns", "1"))
	    {
	      if (dns_list
		  && (nvram_invmatch ("lan_ipaddr", "")
		      || strlen (dns_list->dns_server[0]) > 0
		      || strlen (dns_list->dns_server[1]) > 0
		      || strlen (dns_list->dns_server[2]) > 0))
		{

		  if (nvram_invmatch ("lan_ipaddr", ""))
		    fprintf (fp, "ms-dns %s\n",
			     nvram_safe_get ("lan_ipaddr"));
		  if (strlen (dns_list->dns_server[0]) > 0)
		    fprintf (fp, "ms-dns %s\n", dns_list->dns_server[0]);
		  if (strlen (dns_list->dns_server[1]) > 0)
		    fprintf (fp, "ms-dns %s\n", dns_list->dns_server[1]);
		  if (strlen (dns_list->dns_server[2]) > 0)
		    fprintf (fp, "ms-dns %s\n", dns_list->dns_server[2]);
		}
	    }
	  else
	    {
	      if (dns_list
		  && (strlen (dns_list->dns_server[0]) > 0
		      || strlen (dns_list->dns_server[1]) > 0
		      || strlen (dns_list->dns_server[2]) > 0))
		{
		  if (strlen (dns_list->dns_server[0]) > 0)
		    fprintf (fp, "ms-dns  %s\n", dns_list->dns_server[0]);
		  if (strlen (dns_list->dns_server[1]) > 0)
		    fprintf (fp, "ms-dns  %s\n", dns_list->dns_server[1]);
		  if (strlen (dns_list->dns_server[2]) > 0)
		    fprintf (fp, "ms-dns  %s\n", dns_list->dns_server[2]);

		}
	    }

	  if (dns_list)
	    free (dns_list);
	  fprintf (fp, "noipdefault\n");
	  fprintf (fp, "nodefaultroute\n");
	  fprintf (fp, "noproxyarp\n");
	  fprintf (fp, "noktune\n");
	  fprintf (fp, "netmask 255.255.255.255\n");
	  fclose (fp);

	  //parse chaps from nvram to file
	  static char word[256];
	  char *next, *wordlist;
	  char *user, *pass, *ip, *enable;
	  wordlist = nvram_safe_get ("pppoeserver_chaps");

	  fp = fopen ("/tmp/ppp/chap-secrets", "wb");

	  foreach (word, wordlist, next)
	  {
	    pass = word;
	    user = strsep (&pass, ":");
	    if (!user || !pass)
	      continue;

	    ip = pass;
	    pass = strsep (&ip, ":");
	    if (!pass || !ip)
	      continue;

	    enable = ip;
	    ip = strsep (&enable, ":");
	    if (!ip || !enable)
	      continue;

	    if (!strcmp (enable, "on"))
	      fprintf (fp, "%s * %s %s\n", user, pass, ip);

	  }
	  fclose (fp);
//end parsing
	  eval ("pppoe-server", "-k", "-I", "br0", "-L", nvram_safe_get ("lan_ipaddr"), "-R", nvram_safe_get ("pppoeserver_remoteaddr"));	//todo, make interface and base address configurable, see networking page options
	}
      else
	{
	  FILE *fp;
	  mkdir ("/tmp/ppp", 0777);
	  fp = fopen ("/tmp/ppp/pppoe-server-options", "wb");
//        fprintf (fp, "crtscts\n");
	  if (nvram_default_match ("pppoeserver_bsdcomp", "0", "0"))
	    fprintf (fp, "nobsdcomp\n");
	  else
	    fprintf (fp, "bsdcomp 12\n");
	  if (nvram_default_match ("pppoeserver_deflate", "0", "0"))
	    fprintf (fp, "nodeflate\n");
	  else
	    fprintf (fp, "deflate 12\n");
	  if (nvram_default_match ("pppoeserver_lzs", "0", "0"))
	    fprintf (fp, "nolzs\n");
	  else
	    fprintf (fp, "lzs\n");
	  if (nvram_default_match ("pppoeserver_mppc", "0", "0"))
	    fprintf (fp, "nomppc\n");
	  else
	    fprintf (fp, "mppc\n");
	  fprintf (fp, "nopcomp\n");
	  fprintf (fp, "idle %s\n", nvram_safe_get ("pppoeserver_idle"));	//todo ...
	  if (nvram_default_match ("pppoeserver_encryption", "1", "0"))	// make it configureable
	    {
	      fprintf (fp, "mppe required,no56,no40,stateless\n");
	      fprintf (fp, "refuse-eap\n");
	      fprintf (fp, "refuse-pap\n");
	      fprintf (fp, "refuse-chap\n");
	      fprintf (fp, "refuse-mschap\n");
	      fprintf (fp, "require-mschap-v2\n");
	    }
	  else
	    fprintf (fp, "nomppe\n");

	  struct dns_lists *dns_list = get_dns_list ();
	  if (!dns_list || dns_list->num_servers == 0)
	    {
	      if (nvram_invmatch ("lan_ipaddr", ""))
		fprintf (fp, "ms-dns %s\n", nvram_safe_get ("lan_ipaddr"));
	    }
	  else if (nvram_match ("local_dns", "1"))
	    {
	      if (dns_list
		  && (nvram_invmatch ("lan_ipaddr", "")
		      || strlen (dns_list->dns_server[0]) > 0
		      || strlen (dns_list->dns_server[1]) > 0
		      || strlen (dns_list->dns_server[2]) > 0))
		{

		  if (nvram_invmatch ("lan_ipaddr", ""))
		    fprintf (fp, "ms-dns %s\n",
			     nvram_safe_get ("lan_ipaddr"));
		  if (strlen (dns_list->dns_server[0]) > 0)
		    fprintf (fp, "ms-dns %s\n", dns_list->dns_server[0]);
		  if (strlen (dns_list->dns_server[1]) > 0)
		    fprintf (fp, "ms-dns %s\n", dns_list->dns_server[1]);
		  if (strlen (dns_list->dns_server[2]) > 0)
		    fprintf (fp, "ms-dns %s\n", dns_list->dns_server[2]);
		}
	    }
	  else
	    {
	      if (dns_list
		  && (strlen (dns_list->dns_server[0]) > 0
		      || strlen (dns_list->dns_server[1]) > 0
		      || strlen (dns_list->dns_server[2]) > 0))
		{
		  if (strlen (dns_list->dns_server[0]) > 0)
		    fprintf (fp, "ms-dns  %s\n", dns_list->dns_server[0]);
		  if (strlen (dns_list->dns_server[1]) > 0)
		    fprintf (fp, "ms-dns  %s\n", dns_list->dns_server[1]);
		  if (strlen (dns_list->dns_server[2]) > 0)
		    fprintf (fp, "ms-dns  %s\n", dns_list->dns_server[2]);
		}
	    }

	  if (dns_list)
	    free (dns_list);
	  fprintf (fp, "login\n");
	  fprintf (fp, "require-mschap-v2\n");
	  fprintf (fp, "default-mru\n");
	  fprintf (fp, "default-asyncmap\n");
	  fprintf (fp, "lcp-echo-interval %s\n", nvram_safe_get ("pppoeserver_lcpechoint"));	//todo optionally configurable
	  fprintf (fp, "lcp-echo-failure %s\n", nvram_safe_get ("pppoeserver_lcpechofail"));	// todo optionally configureable
	  fprintf (fp, "noipdefault\n");
	  fprintf (fp, "nodefaultroute\n");
	  fprintf (fp, "noproxyarp\n");
	  fprintf (fp, "noktune\n");
	  fprintf (fp, "netmask 255.255.255.255\n");
	  fprintf (fp, "plugin radius.so\n");
	  fprintf (fp, "plugin radattr.so\n");
	  fclose (fp);
	  mkdir ("/tmp/ppp/radius", 0777);
	  fp = fopen ("/tmp/ppp/radius/radiusclient.conf", "wb");
	  fprintf (fp, "auth_order\tradius\n");
	  fprintf (fp, "login_tries\t4\n");
	  fprintf (fp, "login_timeout\t60\n");
	  fprintf (fp, "nologin\t/etc/nologin\n");
	  fprintf (fp, "issue\t/etc/issue\n");
	  fprintf (fp, "servers\t/tmp/ppp/radius/servers\n");
	  fprintf (fp, "dictionary\t/etc/dictionary\n");
	  fprintf (fp, "login_radius\t/usr/local/sbin/login.radius\n");
	  fprintf (fp, "seqfile\t/var/run/radius.seq\n");
	  fprintf (fp, "mapfile\t/etc/port-id-map\n");
	  fprintf (fp, "default_realm\n");
	  fprintf (fp, "radius_timeout\t10\n");
	  fprintf (fp, "radius_retries\t3\n");
	  fprintf (fp, "login_local\t/bin/login\n");
	  fprintf (fp, "authserver %s:%s\n",
		   nvram_safe_get ("pppoeserver_authserverip"),
		   nvram_safe_get ("pppoeserver_authserverport"));
	  fprintf (fp, "acctserver %s:%s\n",
		   nvram_safe_get ("pppoeserver_authserverip"),
		   nvram_safe_get ("pppoeserver_acctserverport"));
	  fclose (fp);
	  fp = fopen ("/tmp/ppp/radius/servers", "wb");
	  fprintf (fp, "%s %s\n", nvram_safe_get ("pppoeserver_authserverip"), nvram_safe_get ("pppoeserver_sharedkey"));	//todo, shared secret for radius server, see above for server name, must be identical
	  fclose (fp);
	  eval ("pppoe-server", "-k", "-I", "br0", "-L", nvram_safe_get ("lan_ipaddr"), "-R", nvram_safe_get ("pppoeserver_remoteaddr"));	//todo, make interface and base address configurable, remote addr as well, see networking page options
	}
      syslog (LOG_INFO, "rp-pppoe : pppoe server successfully started\n");
    }
}

void
stop_pppoeserver (void)
{
  if (pidof ("pppoe-server") > 0)
    {
      syslog (LOG_INFO, "rp-pppoe : pppoe server successfully stopped\n");
      killall ("pppoe-server", SIGTERM);
      del_pppoe_natrule ();
    }
}

#endif
