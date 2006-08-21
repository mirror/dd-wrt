
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <linux/if_ether.h>
#include <sys/sysinfo.h>
#include <time.h>
#include <unistd.h>
//#ifdef CDEBUG
//#include <utils.h>
//#endif
#include <broadcom.h>

#define	STATUS_RETRY_COUNT	10
#define STATUS_REFRESH_TIME1	5
#define STATUS_REFRESH_TIME2	60

int retry_count = -1;
int refresh_time = STATUS_REFRESH_TIME2;

void
ej_show_status_setting (int eid, webs_t wp, int argc, char_t ** argv)
{

  do_ej ("Status_Router1.asp", wp);

  return;
}

/* Report time in RFC-822 format */
void
ej_localtime (int eid, webs_t wp, int argc, char_t ** argv)
{
  time_t tm;

  time (&tm);

  if (time (0) > (unsigned long) 31536000)	//60 * 60 * 24 * 365
    websWrite (wp, rfctime (&tm));
  else
    websWrite (wp, "<script type=\"text/javascript\">Capture(status_router.notavail)</script>\n");
}

void
ej_dhcp_remaining_time (int eid, webs_t wp, int argc, char_t ** argv)
{
  // tofu12

  if (nvram_invmatch ("wan_proto", "dhcp"))
    return;

  long exp;
  char buf[128];
  struct sysinfo si;
  long n;

  exp = 0;
  if (file_to_buf ("/tmp/udhcpc.expires", buf, sizeof (buf)))
    {
      n = atol (buf);
      if (n > 0)
	{
	  sysinfo (&si);
	  exp = n - si.uptime;
	}
    }
  websWrite (wp, dhcp_reltime (buf, exp));

  return;
}

void
ej_nvram_status_get (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *type;
  char *wan_ipaddr, *wan_netmask, *wan_gateway;
  char *status1 = "", *status2 = "", *hidden1, *hidden2, *button1 = "";
  char *wan_proto = nvram_safe_get ("wan_proto");
  struct dns_lists *dns_list = NULL;
  int wan_link = check_wan_link (0);

  if (ejArgs (argc, argv, "%s", &type) < 1)
    {
      websError (wp, 400, "Insufficient args\n");
      return;
    }
  cdebug ("nvram_status_get proto check");
  if (nvram_match ("wan_proto", "pptp"))
    {
      wan_ipaddr =
	wan_link ? nvram_safe_get ("pptp_get_ip") :
	nvram_safe_get ("wan_ipaddr");
      wan_netmask =
	wan_link ? nvram_safe_get ("wan_netmask") :
	nvram_safe_get ("wan_netmask");
      wan_gateway =
	wan_link ? nvram_safe_get ("wan_gateway") :
	nvram_safe_get ("pptp_server_ip");
    }
  else if (!strcmp (nvram_safe_get ("wan_proto"), "pppoe"))
    {
      wan_ipaddr = wan_link ? nvram_safe_get ("wan_ipaddr") : "0.0.0.0";
      wan_netmask = wan_link ? nvram_safe_get ("wan_netmask") : "0.0.0.0";
      wan_gateway = wan_link ? nvram_safe_get ("wan_gateway") : "0.0.0.0";
    }
  else if (nvram_match ("wan_proto", "l2tp"))
    {
      wan_ipaddr =
	wan_link ? nvram_safe_get ("l2tp_get_ip") :
	nvram_safe_get ("wan_ipaddr");
      wan_netmask =
	wan_link ? nvram_safe_get ("wan_netmask") :
	nvram_safe_get ("wan_netmask");
      wan_gateway =
	wan_link ? nvram_safe_get ("wan_gateway") :
	nvram_safe_get ("wan_gateway");
    }
  else
    {
      wan_ipaddr = nvram_safe_get ("wan_ipaddr");
      wan_gateway = nvram_safe_get ("wan_gateway");
      wan_netmask = nvram_safe_get ("wan_netmask");
    }

  cdebug ("nvram_status_get get dns list");

  dns_list = get_dns_list ();

  if (!strcmp (wan_proto, "pppoe") || !strcmp (wan_proto, "pptp")
      || !strcmp (wan_proto, "l2tp") || !strcmp (wan_proto, "heartbeat"))
    {
      hidden1 = "";
      hidden2 = "";
      if (wan_link == 0)
	{
	  // submit_button old format is "Connect", new format is "Connect_pppoe" or "Connect_pptp" or "Connect_heartbeat"
	  //if(submit_type && !strncmp(submit_type,"Connect",7) && retry_count != -1){
	  if (retry_count != -1)
	    {
	      status1 = "Status";
	      status2 = "Connecting";
	      button1 = "Disconnect";
	    }
	  else
	    {
	      status1 = "Status";
	      status2 = "Disconnected";
	      button1 = "Connect";
	    }
	}
      else
	{
	  retry_count = -1;
	  status1 = "Status";
	  status2 = "Connected";
	  button1 = "Disconnect";
	}
    }
  else
    {
      status1 = "Disable";	// only for nonbrand
      status2 = "&nbsp;";
      hidden1 = "<!--";
      hidden2 = "-->";
    }

  cdebug ("nvram_status_get wan ip addr");

  if (!strcmp (type, "wan_ipaddr"))
    {
      websWrite (wp, "%s", wan_ipaddr);
    }
  else if (!strcmp (type, "wan_netmask"))
    websWrite (wp, "%s", wan_netmask);
  else if (!strcmp (type, "wan_gateway"))
    websWrite (wp, "%s", wan_gateway);
  else if (!strcmp (type, "wan_dns0"))
    {
      if (dns_list)
	websWrite (wp, "%s", dns_list->dns_server[0]);
    }
  else if (!strcmp (type, "wan_dns1"))
    {
      if (dns_list)
	websWrite (wp, "%s", dns_list->dns_server[1]);
    }
  else if (!strcmp (type, "wan_dns2"))
    {
      if (dns_list)
	websWrite (wp, "%s", dns_list->dns_server[2]);
    }
  else if (!strcmp (type, "status1"))
    websWrite (wp, "%s", status1);
  else if (!strcmp (type, "status2"))
    websWrite (wp, "%s", status2);
  else if (!strcmp (type, "button1"))
    websWrite (wp, "%s", button1);
  else if (!strcmp (type, "hidden1"))
    websWrite (wp, "%s", hidden1);
  else if (!strcmp (type, "hidden2"))
    websWrite (wp, "%s", hidden2);
  if (dns_list)
    free (dns_list);
  cdebug ("nvram_status_get leave");

  return;
}

void
ej_show_status (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *type;
  char *wan_proto = nvram_safe_get ("wan_proto");
  char *submit_type = websGetVar (wp, "submit_type", NULL);
  int wan_link = 0;
  char buf[254];

  if (!strcmp (wan_proto, "pppoe") || !strcmp (wan_proto, "pptp")
      || !strcmp (wan_proto, "l2tp") || !strcmp (wan_proto, "heartbeat"))
    {

      /* get type  [ refresh | reload ] */
      if (ejArgs (argc, argv, "%s", &type) < 1)
	{
	  websError (wp, 400, "Insufficient args\n");
	  return;
	}
      /* get ppp status , if /tmp/ppp/link exist, the connection is enabled */
      wan_link = check_wan_link (0);


      if (!strcmp (type, "init"))
	{

	  /* press [ Connect | Disconnect ] button */
	  /* set retry count */
	  if (gozila_action)
	    retry_count = STATUS_RETRY_COUNT;	// retry 3 times

	  /* set refresh time */
	  // submit_type old format is "Disconnect", new format is "Disconnect_pppoe" or "Disconnect_pptp" or "Disconnect_heartbeat"
	  if (submit_type && !strncmp (submit_type, "Disconnect", 10))	// Disconnect always 60 seconds to refresh
	    retry_count = -1;

	  refresh_time =
	    (retry_count <= 0) ? STATUS_REFRESH_TIME2 : STATUS_REFRESH_TIME1;

	}
      else if (!strcmp (type, "refresh_time"))
	{

	  websWrite (wp, "%d", refresh_time * 1000);
	}
      else if (!strcmp (type, "onload"))
	{
	  if (retry_count == 0
	      || (!submit_type && !wan_link && gozila_action))
	    {			//After refresh 2 times, if the status is disconnect, show Alert message.
	      websWrite (wp, "ShowAlert(\"TIMEOUT\");");
	      retry_count = -1;
	    }
	  else if (file_to_buf ("/tmp/ppp/log", buf, sizeof (buf)))
	    {
	      websWrite (wp, "ShowAlert(\"%s\");", buf);
	      retry_count = -1;
	      unlink ("/tmp/ppp/log");
	    }
	  else
	    {
	      websWrite (wp, "Refresh();");
	    }

	  if (retry_count != -1)
	    retry_count--;
	}
    }
  return;
}


void
ej_show_wan_domain (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *wan_domain;

  if (nvram_invmatch ("wan_domain", ""))
    wan_domain = nvram_safe_get ("wan_domain");
  else
    wan_domain = nvram_safe_get ("wan_get_domain");

  websWrite (wp, "%s", wan_domain);
  return;
}

void
ej_show_wl_mac (int eid, webs_t wp, int argc, char_t ** argv)
{
  char *wl_mac;

  /* In client mode the WAN MAC is the Wireless MAC */
  if (nvram_match ("wl_mode", "sta"))
    wl_mac = nvram_safe_get ("wan_hwaddr");
  else
    wl_mac = nvram_safe_get ("wl0_hwaddr");

  websWrite (wp, "%s", wl_mac);
  return;
}

int
stop_ppp (webs_t wp)
{
  unlink ("/tmp/ppp/log");
  return unlink ("/tmp/ppp/link");
}

/* Return WAN link state */
/*
static int
ej_link(int eid, webs_t wp, int argc, char_t **argv)
{
        char *name;
        int s;
        struct ifreq ifr;
        struct ethtool_cmd ecmd;
        FILE *fp;

        if (ejArgs(argc, argv, "%s", &name) < 1) {
                websError(wp, 400, "Insufficient args\n");
                return -1;
        }

        // PPPoE connection status 
        if (nvram_match("wan_proto", "pppoe")) {
                if ((fp = fopen("/tmp/ppp/link", "r"))) {
                        fclose(fp);
                       return websWrite(wp, "Connected");
                } else
                        return websWrite(wp, "Disconnected");
        }

        // Open socket to kernel 
        if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
                websError(wp, 400, strerror(errno));
                return -1;
        }

        // Check for non-zero link speed 
        strncpy(ifr.ifr_name, nvram_safe_get(name), IFNAMSIZ);
        ifr.ifr_data = (void *) &ecmd;
        ecmd.cmd = ETHTOOL_GSET;
        if (ioctl(s, SIOCETHTOOL, &ifr) < 0) {
                close(s);
                websError(wp, 400, strerror(errno));
                return -1;
	 }

        // Cleanup 
        close(s);

        if (ecmd.speed)
                return websWrite(wp, "Connected");
        else
                return websWrite(wp, "Disconnected");
}

*/

void
ej_show_meminfo (int eid, webs_t wp, int argc, char_t ** argv)
{
  FILE *fp;
  char line[254];

  if ((fp = popen ("free -simple", "r")))
    {
      while (fgets (line, sizeof (line), fp) != NULL)
	{
	  websWrite (wp, "%s", line);
	}
      pclose (fp);
    }
  return;
}
