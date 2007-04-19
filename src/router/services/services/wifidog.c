#ifdef HAVE_WIFIDOG
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
//unfinished. do not use
void
start_wifidog (void)
{
  if (nvram_match ("wd_enable", "1"))
    {
      mkdir ("/tmp/etc/", 0744);
      FILE *fp = fopen ("/tmp/etc/wifidog.conf", "wb");
      if (!strlen (nvram_safe_get ("wd_gwid")))
	fprintf (fp, "GatewayID default\n");
      else
	fprintf (fp, "GatewayID %s\n", nvram_safe_get ("wd_gwid"));
      fprintf (fp, "ExternalInterface %s\n", get_wan_face ());
      fprintf (fp, "GatewayInterface %s\n", nvram_safe_get ("lan_ifname"));
      fprintf (fp, "Portal %s\n", nvram_safe_get ("wd_url"));
      fprintf (fp, "GatewayPort %s\n", nvram_safe_get ("wd_gwport"));
      fprintf (fp, "HTTPDMaxConn %s\n", nvram_safe_get ("wd_httpdcon"));
      fprintf (fp, "HTTPDName %s\n", nvram_safe_get ("wd_httpdname"));
      fprintf (fp, "CheckInterval %s\n", nvram_safe_get ("wd_interval"));
      fprintf (fp, "ClientTimeout %s\n", nvram_safe_get ("wd_timeout"));
      fprintf (fp, "TrustedMACList %s\n", nvram_safe_get ("wd_maclist"));
      fprintf (fp, "AuthServer {\n");
      fprintf (fp, "Hostname %s\n", nvram_safe_get ("wd_hostname"));
      fprintf (fp, "SSLAvailable %s\n",
	       nvram_match ("wd_sslavailable", "1") ? "yes" : "no");
      fprintf (fp, "SSLPort %s\n", nvram_safe_get ("wd_sslport"));
      fprintf (fp, "HTTPPort %s\n", nvram_safe_get ("wd_httpport"));
      fprintf (fp, "Path %s\n", nvram_safe_get ("wd_path"));
      fprintf (fp, "}\n");
      fprintf (fp, "FirewallRuleSet validating-users {\n");
      fprintf (fp, "FirewallRule allow to 0.0.0.0/0\n");
      fprintf (fp, "}\n");
      fprintf (fp, "FirewallRuleSet known-users {\n");
      fprintf (fp, "FirewallRule allow to 0.0.0.0/0\n");
      fprintf (fp, "}\n");
      fprintf (fp, "FirewallRuleSet unknown-users {\n");
      fprintf (fp, "FirewallRule allow udp port 53\n");
      fprintf (fp, "FirewallRule allow tcp port 53\n");
      fprintf (fp, "FirewallRule allow udp port 67\n");
      fprintf (fp, "FirewallRule allow tcp port 67\n");
      fprintf (fp, "}\n");
      fprintf (fp, "FirewallRuleSet locked-users {\n");
      fprintf (fp, "FirewallRule block to 0.0.0.0/0\n");
      fprintf (fp, "}\n");

      fclose (fp);
      eval ("/usr/sbin/wifidog");
      syslog (LOG_INFO, "wifidog successfully started\n");
    }
}

void
stop_wifidog (void)
{
  if (pidof ("wifidog") > 0)
    syslog (LOG_INFO, "wifidog successfully stopped\n");
  killall ("wifidog", SIGKILL);
}

#endif
