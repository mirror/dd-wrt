
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>		/* AhMan March 18 2005 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <wait.h>
#include <net/route.h>		/* AhMan March 18 2005 */
#include <sys/types.h>
#include <signal.h>
#include <dlfcn.h>

#include <bcmnvram.h>
#include <bcmconfig.h>
#include <netconf.h>
#include <shutils.h>
#include <utils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <rc.h>
#include <wlutils.h>
#include <nvparse.h>

#define WL_IOCTL(name, cmd, buf, len) (wl_ioctl((name), (cmd), (buf), (len)))

#define TXPWR_MAX 1000
#define TXPWR_DEFAULT 28

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)

/* 
 * AhMan March 18 2005 
 */
#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

/* 
 * AhMan March 18 2005 
 */
void start_tmp_ppp(int num);

void del_routes(char *route)
{
	char word[80], *tmp;
	char *ipaddr, *netmask, *gateway, *metric, *ifname;

	foreach(word, route, tmp) {
		netmask = word;
		ipaddr = strsep(&netmask, ":");
		if (!ipaddr || !netmask)
			continue;
		gateway = netmask;
		netmask = strsep(&gateway, ":");
		if (!netmask || !gateway)
			continue;
		metric = gateway;
		gateway = strsep(&metric, ":");
		if (!gateway || !metric)
			continue;
		ifname = metric;
		metric = strsep(&ifname, ":");
		if (!metric || !ifname)
			continue;

		if (!strcmp(ipaddr, "0.0.0.0"))
			eval("route", "del", "default", "gw", gateway);

		eval("route", "del", "-net", ipaddr, "netmask", netmask, "gw", gateway);
		// route_del (ifname, atoi (metric) + 1, ipaddr, gateway, netmask);
	}
}

int start_services_main(int argc, char **argv)
{

	nvram_set("qos_done", "0");
#ifdef HAVE_GPSI
	start_service_f("gps");
#endif
#ifdef HAVE_P910ND
	start_service_f("printer");
#endif
#ifdef HAVE_CPUTEMP
	start_service_f("hwmon");
#endif
#ifdef HAVE_TELNET
	start_service_f("telnetd");
#endif
#ifdef HAVE_FTP
	start_service_f("ftpsrv");
#endif
#ifdef HAVE_SAMBA3
	start_service_f("samba3");
#endif
#ifdef HAVE_MINIDLNA
	start_service_f("dlna");
#endif
#ifdef HAVE_PRIVOXY
	start_service_f("privoxy");
#endif
#ifdef HAVE_TOR
	start_service_f("tor");
#endif
#ifdef HAVE_WEBSERVER
	start_service_f("lighttpd");
#endif
#ifdef HAVE_TRANSMISSION
	start_service_f("transmission");
#endif
#ifdef HAVE_SYSLOG
	start_service_f("syslog");
#endif
#ifdef HAVE_TFTP
	start_service_f("tftpd");
#endif
	start_service_f("httpd");
	start_service_f("udhcpd");
#ifdef HAVE_DNSMASQ
	start_service_f("dnsmasq");
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	start_service_f("zebra");
#endif
#ifdef HAVE_OLSRD
	start_service_f("olsrd");
#endif

	start_service_f("wshaper");
	start_service_f("wland");
	start_service_f("cron");

#ifdef HAVE_PPTPD
	start_service_f("pptpd");
#endif

#ifdef HAVE_SSHD
#ifdef HAVE_REGISTER
	if (isregistered_real())
#endif
		start_service_f("sshd");
#endif

#ifdef HAVE_RADVD
	start_service_f("radvd");
#endif

#ifdef HAVE_SNMP
	start_service_f("snmp");
#endif

#ifdef HAVE_LLTD
	start_service_f("lltd");
#endif

#ifdef HAVE_PPPOESERVER
	start_service_f("pppoeserver");
#endif

#ifdef HAVE_WOL
	start_service_f("wol");
#endif

#ifdef HAVE_NOCAT
	start_service_f("splashd");
#endif

#ifdef HAVE_UPNP
	start_service_f("upnp");
#endif

#ifdef HAVE_OPENVPN
	start_service_f("openvpnserversys");
	start_service_f("openvpn");
#endif
#ifdef HAVE_VNCREPEATER
	start_service_f("vncrepeater");
#endif
#ifdef HAVE_RSTATS
	start_service_f("rstats");
#endif
#ifdef HAVE_NSTX
	start_service_f("nstxd");
#endif
#ifdef HAVE_PPPOERELAY
	start_service_f("pppoerelay");
#endif
#ifdef HAVE_MILKFISH
	start_service_f("milkfish");
#endif
#ifdef HAVE_FREERADIUS
	start_service_f("freeradius");
#endif
#ifdef HAVE_AP_SERV
	start_service_f("apserv");
#endif
#ifdef HAVE_SPOTPASS
	start_service_f("spotpass");
#endif
#ifdef HAVE_IAS
	if (atoi(nvram_safe_get("ias_startup")) > 0) {
		nvram_set("ias_startup", "3");
		nvram_set("ias_dnsresp", "1");
		system("/usr/sbin/dns_responder 192.168.11.1 55300 &");
	}
#endif

	cprintf("done\n");
	return 0;
}

int stop_services_main(int argc, char **argv)
{
#ifdef HAVE_P910ND
	stop_service_f("printer");
#endif

	// stop_ses();
#ifdef HAVE_AP_SERV
	stop_service_f("apserv");
#endif
#ifdef HAVE_FREERADIUS
	stop_service_f("freeradius");
#endif
#ifdef HAVE_MULTICAST
	stop_service_f("igmp_proxy");
#endif
#ifdef HAVE_UDPXY
	stop_service_f("udpxy");
#endif
#ifdef HAVE_PPPOERELAY
	stop_service_f("pppoerelay");
#endif
#ifdef HAVE_RSTATS
	stop_service_f("rstats");
#endif
#ifdef HAVE_NSTX
	stop_service_f("nstxd");
#endif

#ifdef HAVE_UPNP
	stop_service_f("upnp");
#endif
	stop_service_f("udhcpd");
	startstop_f("dns_clear_resolv");
	stop_service_f("cron");

#ifdef HAVE_TFTP
	stop_service_f("tftpd");
#endif
#ifdef HAVE_SYSLOG
	stop_service_f("syslog");
#endif
#ifdef HAVE_OLSRD
	stop_service_f("olsrd");
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	stop_service_f("zebra");
#endif
	stop_service_f("wland");
#ifdef HAVE_TELNET
	stop_service_f("telnetd");
#endif
#ifdef HAVE_CPUTEMP
	stop_service_f("hwmon");
#endif
#ifdef HAVE_FTP
	stop_service_f("ftpsrv");
#endif
#ifdef HAVE_SAMBA3
	stop_service_f("samba3");
#endif
#ifdef HAVE_MINIDLNA
	stop_service_f("dlna");
#endif
#ifdef HAVE_TOR
	stop_service_f("tor");
#endif
#ifdef HAVE_WEBSERVER
	stop_service_f("lighttpd");
#endif
#ifdef HAVE_TRANSMISSION
	stop_service_f("transmission");
#endif
#ifdef HAVE_PRIVOXY
	stop_service_f("privoxy");
#endif
#ifdef HAVE_SSHD
#ifdef HAVE_REGISTER
	if (isregistered_real())
#endif
		stop_service_f("sshd");
#endif

#ifdef HAVE_RADVD
	stop_service_f("radvd");
#endif

#ifdef HAVE_WIFIDOG
	stop_service_f("wifidog");
#endif

#ifdef HAVE_CHILLI
	stop_service_f("chilli");
#endif

#ifdef HAVE_PPPOESERVER
	stop_service_f("pppoeserver");
#endif

#ifdef HAVE_SNMP
	stop_service_f("snmp");
#endif

#ifdef HAVE_LLTD
	stop_service_f("lltd");
#endif

#ifdef HAVE_WOL
	stop_service_f("wol");
#endif
	stop_service_f("wland");
	stop_service_f("wshaper");

#ifdef HAVE_PPTPD
	stop_service_f("pptpd");
#endif
#ifdef HAVE_NOCAT
	stop_service_f("splashd");
#endif
#ifdef HAVE_VNCREPEATER
	stop_service_f("vncrepeater");
#endif
#ifdef HAVE_OPENVPN
	stop_service_f("openvpnserversys");
	stop_service_f("openvpn");
#endif
#ifdef HAVE_GPSI
	stop_service_f("gps");
#endif
	stop_running_main(0, NULL);

	cprintf("done\n");
	return 0;
}

static void handle_dhcpd(void)
{
	startstop_f("udhcpd");
}

static void handle_index(void)
{
	unlink("/tmp/ppp/log");

	stop_service_force_f("wan");
	stop_service_f("radio_timer");	//
#ifdef HAVE_MULTICAST
	stop_service_f("igmp_proxy");	//
#endif
#ifdef HAVE_UDPXY
	stop_service_f("udpxy");
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	stop_service_f("nas");	//
#endif
#ifdef HAVE_MADWIFI
	stop_service_f("stabridge");
#endif
#ifdef HAVE_EMF
	stop_service_f("emf");	//
#endif
#ifdef HAVE_VLANTAGGING
	stop_service_f("bridgesif");	//
	stop_service_f("vlantagging");	//
#endif
#ifdef HAVE_BONDING
	stop_service_f("bonding");	//
#endif
	stop_service_f("lan");	//
#ifdef HAVE_VLANTAGGING
	stop_service_f("bridging");	//
#endif
	stop_service_f("ttraff");	//

	stop_running_main(0, NULL);

#ifdef HAVE_VLANTAGGING
	start_service_f("bridging");
#endif
	start_service_force_f("lan");
#ifdef HAVE_BONDING
	start_service_f("bonding");
#endif
	start_service_force("wan_boot");
	start_service_f("ttraff");
#ifdef HAVE_MADWIFI
	start_service_f("stabridge");
#endif
	startstop_f("udhcpd");
#ifdef HAVE_DNSMASQ
	startstop_f("dnsmasq");
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	startstop_f("zebra");
#endif
#ifdef HAVE_OLSRD
	startstop_f("olsrd");
#endif
#ifdef HAVE_VLANTAGGING
	start_service("vlantagging");
	start_service("bridgesif");
#endif
#ifdef HAVE_EMF
	start_service("emf");	//
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	start_service("nas");
	start_service("guest_nas");
#endif
	start_service_f("radio_timer");
	startstop("firewall");
	// httpd will not
	// accept connection
	// anymore on wan/lan 
	// ip changes changes
	startstop_fdelay("httpd", 2);
	startstop_f("cron");
//      start_service_f("anchorfreednat");
#ifdef HAVE_NOCAT
	startstop_f("splashd");
#endif

}

static void handle_router(void)
{

#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	startstop_f("zebra");
#endif
#ifdef HAVE_OLSRD
	startstop_f("olsrd");
#endif
}

/*static void handle_anchorfree(void)
{

	startstop_f("anchorfree");
	start_service_f("anchorfreednat");
}
*/
static void handle_hotspot(void)
{
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	stop_service_f("zebra");
#endif

	stop_service_f("radio_timer");
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	stop_service_f("nas");
	eval("wlconf", nvram_safe_get("wl0_ifname"), "down");
	eval("wlconf", nvram_safe_get("wl1_ifname"), "down");
#endif
#ifdef HAVE_MADWIFI
	stop_service_f("stabridge");
#endif
	stop_service_f("ttraff");
	stop_service_force_f("wan");
#ifdef HAVE_EMF
	stop_service_f("emf");	//
#endif
#ifdef HAVE_VLANTAGGING
	stop_service_f("bridgesif");
	stop_service_f("vlantagging");
#endif
#ifdef HAVE_BONDING
	stop_service_f("bonding");
#endif
	stop_service_f("lan");
#ifdef HAVE_VLANTAGGING
	stop_service_f("bridging");
#endif
	stop_running_main(0, NULL);

#ifdef HAVE_WIFIDOG
	startstop_f("wifidog");
#endif
#ifdef HAVE_NOCAT
	startstop_f("splashd");
#endif
#ifdef HAVE_SPUTNIK_APD
	startstop_f("sputnik");
#endif
#ifdef HAVE_CHILLI
	startstop_f("chilli");
#endif
#ifdef HAVE_VLANTAGGING
	start_service("bridging");
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	start_service("wlconf");
#endif
	start_service("lan");
#ifdef HAVE_BONDING
	start_service("bonding");
#endif
	start_service_force_f("wan");
	start_service_f("ttraff");
#ifdef HAVE_MADWIFI
	start_service_f("stabridge");
#endif
#ifdef HAVE_VLANTAGGING
	start_service("vlantagging");
	start_service("bridgesif");
#endif
#ifdef HAVE_EMF
	start_service("emf");	//
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	start_service("nas");
	start_service("guest_nas");
#endif
	start_service_f("radio_timer");
	//restart dhcp as well, to fix repeater bridge save issue (dhcp disables itself here)
	startstop_f("udhcpd");
#ifdef HAVE_DNSMASQ
	startstop_f("dnsmasq");
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	start_service("zebra");
#endif
	//since start/stop is faster now we need to sleep, otherwise httpd is stopped/started while response is sent to client
	startstop_fdelay("httpd", 2);	// httpd will not accept connection anymore

	FORK(eval("/etc/config/http-redirect.firewall"));
	FORK(eval("/etc/config/smtp-redirect.firewall"));
#ifdef HAVE_ZEROIP
	FORK(eval("/etc/config/shat.startup"));
#endif
}

static void handle_services(void)
{
#ifdef HAVE_GPSI
	startstop_f("gps");
#endif
#ifdef HAVE_P910ND
	startstop_f("printer");
#endif
#ifdef HAVE_AP_SERV
	startstop_f("apserv");
#endif
#ifdef HAVE_PPPOERELAY
	startstop_f("pppoerelay");
#endif
	startstop_f("udhcpd");
#ifdef HAVE_SYSLOG
	startstop_f("syslog");
#endif
#ifdef HAVE_RSTATS
	startstop_f("rstats");
#endif
	startstop_f("ttraff");
#ifdef HAVE_NSTX
	startstop_f("nstxd");
#endif
#ifdef HAVE_PPPOESERVER
	startstop("firewall");
	startstop_f("pppoeserver");
#endif
#ifdef HAVE_DNSMASQ
	startstop_f("dnsmasq");
#endif
	startstop_f("udhcpd");
#ifdef HAVE_CPUTEMP
	startstop_f("hwmon");
#endif
#ifdef HAVE_TELNET
	startstop_f("telnetd");
#endif
#ifdef HAVE_SNMP
	startstop_f("snmp");
#endif
#ifdef HAVE_LLTD
	startstop_f("lltd");
#endif
#ifdef HAVE_PPTPD
	startstop_f("pptpd");
#endif
#ifdef HAVE_PPTP
	FORK(eval("/etc/config/pptpd_client.startup"));
#endif
#ifdef HAVE_RFLOW
	FORK(eval("/etc/config/rflow.startup"));
#endif
#ifdef HAVE_KAID
	FORK(eval("/etc/config/kaid.startup"));
#endif
#ifdef HAVE_SSHD
#ifdef HAVE_REGISTER
	if (isregistered_real())
#endif
		startstop_f("sshd");
#endif
	startstop("firewall");
	startstop_f("wshaper");
#ifdef HAVE_SYSLOG
	startstop_f("syslog");
#endif
#ifdef HAVE_VNCREPEATER
	startstop_f("vncrepeater");
#endif
#ifdef HAVE_OPENVPN
	stop_service("openvpnserver");
	stop_service("openvpn");
	start_service_f("openvpnserver");
	start_service_f("openvpn");
#endif
#ifdef HAVE_NOCAT
	startstop_f("splashd");
#endif
#ifdef HAVE_ZABBIX
	startstop_f("zabbix");
#endif
//      start_service_f("anchorfreednat");

}

static void handle_nassrv(void)
{

#ifdef HAVE_SAMBA3
	stop_service_f("samba3");
#endif
#ifdef HAVE_FTP
	stop_service_f("ftpsrv");
#endif
#ifdef HAVE_MINIDLNA
	stop_service_f("dlna");
#endif
#ifdef HAVE_TRANSMISSION
	stop_service_f("transmission");
#endif

	stop_running_main(0, NULL);

#ifdef HAVE_SAMBA3
	start_service_f("samba3");
#endif
#ifdef HAVE_FTP
	start_service_f("ftpsrv");
#endif
#ifdef HAVE_MINIDLNA
	start_service_f("dlna");
#endif
#ifdef HAVE_TRANSMISSION
	start_service_f("transmission");
#endif


}

static void handle_management(void)
{

#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	stop_service_f("nas");
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	stop_service_f("zebra");
#endif
	stop_service_f("cron");
	stop_service_f("udhcpd");
#ifdef HAVE_IPV6
	stop_service_f("ipv6");
#endif

	stop_running_main(0, NULL);

	start_service_f("udhcpd");
	start_service_f("cron");
#ifdef HAVE_IPV6
	eval("/etc/config/ipv6.startup");
	if (nvram_match("need_reboot", "1")) {
		nvram_set("need_reboot", "0");
		nvram_set("need_commit", "0");
		nvram_commit();
		start_service_f("ipv6");
	}
#endif
#ifdef HAVE_RADVD
	startstop_f("radvd");
#endif
#ifdef HAVE_PPTPD
	startstop_f("pptpd");
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	start_service_f("zebra");
#endif
	startstop("firewall");
	stop_service("wland");
	startstop_f("wshaper");
	start_service_f("wland");
	startstop_fdelay("httpd", 2);
#ifdef HAVE_NOCAT
	startstop_f("splashd");
#endif
#ifdef HAVE_WOL
	startstop_f("wol");
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	start_service("nas");
	start_service("guest_nas");
#endif
//      start_service_f("anchorfreednat");


}

static void handle_pppoe(void)
{
	unlink("/tmp/ppp/log");

	stop_service_f("radio_timer");
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	stop_service_f("nas");
#endif
#ifdef HAVE_MADWIFI
	stop_service_f("stabridge");
#endif
#ifdef HAVE_EMF
	stop_service_f("emf");	//
#endif
#ifdef HAVE_VLANTAGGING
	stop_service_f("bridgesif");
	stop_service_f("vlantagging");
#endif
	stop_service_f("lan");
#ifdef HAVE_BONDING
	stop_service_f("bonding");
#endif
#ifdef HAVE_VLANTAGGING
	stop_service_f("bridging");
#endif
	stop_service_f("ttraff");
	stop_service_force_f("wan");

	stop_running_main(0, NULL);

#ifdef HAVE_VLANTAGGING
	start_service("bridging");
#endif
	start_service("lan");
#ifdef HAVE_BONDING
	start_service("bonding");
#endif
	start_service_force("wan_boot");
	start_service_f("ttraff");
#ifdef HAVE_MADWIFI
	start_service_f("stabridge");
#endif
#ifdef HAVE_VLANTAGGING
	start_service("vlantagging");
	start_service("bridgesif");
#endif
#ifdef HAVE_EMF
	start_service("emf");	//
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	start_service("nas");
	start_service("guest_nas");
#endif
	start_service_f("radio_timer");

}

static void handle_spppoe(void)
{
	stop_service_f("ttraff");
	stop_service_force_f("wan");
}

static void handle_filters(void)
{

	stop_service("cron");
	startstop("firewall");
#ifdef HAVE_SYSLOG
	startstop_f("syslog");
#endif
	stop_service("wland");
	startstop_f("wshaper");
	start_service_f("wland");
	start_service_f("cron");
#ifdef HAVE_NOCAT
	startstop_f("splashd");
#endif
#ifdef HAVE_MULTICAST
	startstop_f("igmp_proxy");
#endif
#ifdef HAVE_UDPXY
	startstop_f("udpxy");
#endif
//      start_service_f("anchorfreednat");
}

static void handle_routing(void)
{

#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	stop_service("zebra");
#endif
	startstop("firewall");
	start_service_force_f("set_routes");
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	start_service_f("zebra");
#endif
#ifdef HAVE_OLSRD
	startstop_f("olsrd");
#endif
#ifdef HAVE_NOCAT
	startstop_f("splashd");
#endif
//      start_service_f("anchorfreednat");

}

static void handle_alive(void)
{

	stop_service("cron");
#ifdef HAVE_REGISTER
	if (isregistered_real())
#endif
	{
		FORK(eval("/etc/config/wdswatchdog.startup"));
		FORK(eval("/etc/config/schedulerb.startup"));
		FORK(eval("/etc/config/proxywatchdog.startup"));
	}
	start_service_f("cron");
}

static void handle_forward(void)
{

	stop_service("wland");
	stop_service("wshaper");
#ifdef HAVE_UPNP
//    stop_service( "upnp");
#endif
	startstop("firewall");
#ifdef HAVE_UPNP
//    start_service( "upnp");
#endif
	start_service_f("wshaper");
	start_service_f("wland");
//      start_service_f("anchorfreednat");
#ifdef HAVE_NOCAT
	startstop_f("splashd");
#endif

}

static void handle_qos(void)
{

	startstop_f("wshaper");
	startstop_f("wland");
#ifdef HAVE_NOCAT
	startstop_f("splashd");
#endif
#ifdef HAVE_OPENVPN
	stop_service("openvpnserver");
	stop_service("openvpn");
	start_service_f("openvpnserver");
	start_service_f("openvpn");
#endif
}

static void handle_forwardupnp(void)
{

#ifdef HAVE_UPNP
	stop_service("upnp");
#endif
	startstop("firewall");
#ifdef HAVE_UPNP
	start_service_f("upnp");
#endif
	stop_service("wland");
	startstop_f("wshaper");
	start_service_f("wland");
//      start_service_f("anchorfreednat");
#ifdef HAVE_NOCAT
	startstop_f("splashd");
#endif

}

static void handle_routedel(void)
{
	del_routes(nvram_safe_get("action_service_arg1"));

}

#ifdef HAVE_SPOTPASS
static void handle_spotpass(void)
{
	startstop("spotpass");
}
#endif

struct SERVICES {
	char *servicename;
	void (*service) (void);
};

static void handle_ddns(void)
{
	startstop("ddns");
	nvram_set("ddns_change", "update");

}

#ifdef HAVE_FREERADIUS

static void handle_freeradius(void)
{
	startstop_f("freeradius");
}
#endif

/* 
 * static void handle_ping (void) { char *ip = nvram_safe_get ("ping_ip"); // 
 * use Ping.asp as a debugging console char cmd[256] = { 0 }; //snprintf
 * (cmd, sizeof (cmd), "%s > %s 2>&1 &", ip, PING_TMP); setenv ("PATH",
 * "/sbin:/bin:/usr/sbin:/usr/bin", 1); // snprintf (cmd, sizeof (cmd), "%s
 * 2>&1 &", ip); // system (cmd);
 * 
 * snprintf (cmd, sizeof (cmd), "alias ping=\'ping -c 3\'; eval \"%s\" > %s
 * 2>&1 &", ip, PING_TMP); system (cmd);
 * 
 * } 
 */

static void handle_upgrade(void)
{

	stop_service_f("ttraff");
	stop_service_force_f("wan");
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	stop_service_f("zebra");
#endif
#ifdef HAVE_OLSRD
	stop_service_f("olsrd");
#endif
#ifdef HAVE_UPNP
	stop_service_f("upnp");
#endif
	stop_service_f("cron");
	stop_running_main(0, NULL);

}

#ifdef HAVE_MILKFISH
static void handle_milkfish(void)
{
	startstop("milkfish");
}
#endif

static void handle_wireless(void)
{
	int wanchanged = wanChanged();
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	stop_service_f("zebra");
#endif

	stop_service_f("radio_timer");
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	stop_service_f("nas");
	eval("wlconf", nvram_safe_get("wl0_ifname"), "down");
	eval("wlconf", nvram_safe_get("wl1_ifname"), "down");
#endif
#ifdef HAVE_MADWIFI
	stop_service_f("stabridge");
#endif
	if (getSTA() || getWET() || wanchanged
#ifdef HAVE_MADWIFI
	    || getWDSSTA()
#endif
	    ) {
#ifdef HAVE_3G
		if (!nvram_match("wan_proto", "3g"))
#endif
		{
			stop_service_f("ttraff");
			stop_service_force_f("wan");
		}
	}
#ifdef HAVE_EMF
	stop_service_f("emf");	//
#endif
#ifdef HAVE_VLANTAGGING
	stop_service_f("bridgesif");
	stop_service_f("vlantagging");
#endif
#ifdef HAVE_BONDING
	stop_service_f("bonding");
#endif
#ifdef HAVE_VLANTAGGING
	stop_service_f("bridging");
#endif
	stop_running_main(0, NULL);
	stop_service("lan");
#ifdef HAVE_VLANTAGGING
	start_service("bridging");
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	start_service("wlconf");
#endif
	start_service("lan");
#ifdef HAVE_BONDING
	start_service("bonding");
#endif
#ifdef HAVE_MADWIFI
	start_service_f("stabridge");
#endif
#ifdef HAVE_VLANTAGGING
	start_service("vlantagging");
	start_service("bridgesif");
#endif
#ifdef HAVE_EMF
	start_service("emf");	//
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	start_service("nas");
	start_service("guest_nas");
#endif
	start_service_f("radio_timer");
	//restart dhcp as well, to fix repeater bridge save issue (dhcp disables itself here)
	startstop_f("udhcpd");
#ifdef HAVE_DNSMASQ
	startstop_f("dnsmasq");
#endif
	if (getSTA() || getWET() || wanchanged
#ifdef HAVE_MADWIFI
	    || getWDSSTA()
#endif
	    ) {
#ifdef HAVE_3G
		if (!nvram_match("wan_proto", "3g"))
#endif
		{
			start_service_force("wan_boot");
			start_service_f("ttraff");
		}
	}
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	start_service("zebra");
#endif
	//since start/stop is faster now we need to sleep, otherwise httpd is stopped/started while response is sent to client
	startstop_fdelay("httpd", 2);	// httpd will not accept connection anymore on wan/lan ip changes changes

}

static void handle_wireless_2(void)
{
	int wanchanged = wanChanged();

#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	stop_service_f("zebra");
#endif

	stop_service_f("radio_timer");
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	stop_service_f("nas");
	eval("wlconf", nvram_safe_get("wl0_ifname"), "down");
	eval("wlconf", nvram_safe_get("wl1_ifname"), "down");
#endif
#ifdef HAVE_MADWIFI
	stop_service_f("stabridge");
#endif
	if (getSTA() || getWET() || wanchanged
#ifdef HAVE_MADWIFI
	    || getWDSSTA()
#endif
	    ) {
#ifdef HAVE_3G
		if (!nvram_match("wan_proto", "3g"))
#endif
		{
			stop_service_f("ttraff");
			stop_service_force_f("wan");
		}
	}
#ifdef HAVE_EMF
	stop_service_f("emf");	//
#endif
#ifdef HAVE_VLANTAGGING
	stop_service_f("bridgesif");
	stop_service_f("vlantagging");
#endif
#ifdef HAVE_BONDING
	stop_service_f("bonding");
#endif
	stop_service_f("lan");
#ifdef HAVE_VLANTAGGING
	stop_service_f("bridging");
#endif
	stop_running_main(0, NULL);
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	start_service("wlconf");
#endif
#ifdef HAVE_VLANTAGGING
	start_service("bridging");
#endif
	start_service("lan");
#ifdef HAVE_BONDING
	start_service("bonding");
#endif
#ifdef HAVE_MADWIFI
	start_service_f("stabridge");
#endif
#ifdef HAVE_VLANTAGGING
	start_service("vlantagging");
	start_service("bridgesif");
#endif
#ifdef HAVE_EMF
	start_service("emf");	//
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	start_service("nas");
	start_service("guest_nas");
#endif
	start_service_f("radio_timer");
	if (getSTA() || getWET() || wanchanged
#ifdef HAVE_MADWIFI
	    || getWDSSTA()
#endif
	    ) {
#ifdef HAVE_3G
		if (!nvram_match("wan_proto", "3g"))
#endif
			startstop_fdelay("httpd", 2);	// httpd will not accept connection anymore
	}
	// on wan/lan ip changes changes
#ifdef HAVE_MADWIFI
	start_service_f("hostapdwan");
#endif
	if (getSTA() || getWET() || wanchanged
#ifdef HAVE_MADWIFI
	    || getWDSSTA()
#endif
	    ) {
#ifdef HAVE_3G
		if (!nvram_match("wan_proto", "3g"))
#endif
		{
			start_service_force("wan_boot");
			start_service_f("ttraff");
		}
	}
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	start_service_f("zebra");
#endif

}

static void handle_dhcp_release(void)
{
	killall("udhcpc", SIGUSR2);
	sleep(1);

}

#ifdef HAVE_USB
static void handle_usbdrivers(void)
{
	startstop("drivers");	//stop is not yet implemented but we dont care about yet
#ifdef HAVE_P910ND
	startstop("printer");
#endif
}
#endif
#ifdef HAVE_EOP_TUNNEL
static void handle_eop(void)
{
	FORK(eval("/etc/config/eop-tunnel.startup"));
	FORK(eval("/etc/config/eop-tunnel.firewall"));

}
#endif
static struct SERVICES services_def[] = {
	{"dhcp", handle_dhcpd},
	{"index", handle_index},
	{"router", handle_router},
	{"hotspot", handle_hotspot},
//      {"anchorfree", handle_anchorfree},
	{"services", handle_services},
#if defined(HAVE_FTP) || defined(HAVE_SAMBA3) || defined(HAVE_MINIDLNA)
	{"nassrv", handle_nassrv},
#endif
	{"management", handle_management},
#ifdef HAVE_3G
	{"start_3g", handle_pppoe},
	{"stop_3g", handle_spppoe},
#endif
#ifdef HAVE_PPPOATM
	{"start_pppoa", handle_pppoe},
	{"stop_pppoa", handle_spppoe},
#endif
	{"start_pppoe", handle_pppoe},
	{"stop_pppoe", handle_spppoe},
	{"start_pptp", handle_pppoe},
	{"stop_pptp", handle_spppoe},
#ifdef HAVE_L2TP
	{"start_l2tp", handle_pppoe},
#endif
#ifdef HAVE_FREERADIUS
	{"freeradius", handle_freeradius},
#endif
#ifdef HAVE_HEARTBEAT
	{"start_heartbeat", handle_pppoe},
#endif
	{"stop_pppoe", handle_spppoe},
	{"stop_pptp", handle_spppoe},
#ifdef HAVE_L2TP
	{"stop_l2tp", handle_spppoe},
#endif
#ifdef HAVE_HEARTBEAT
	{"stop_heartbeat", handle_spppoe},
#endif
	{"filters", handle_filters},
	{"routing", handle_routing},
	{"alive", handle_alive},
	{"forward", handle_forward},
	{"qos", handle_qos},
	{"forward_upnp", handle_forwardupnp},
	{"static_route_del", handle_routedel},
	{"ddns", handle_ddns},
	// {"start_ping", handle_ping},
	{"http_upgrade", handle_upgrade},
#ifdef HAVE_MILKFISH
	{"milkfish", handle_milkfish},
#endif
	{"wireless", handle_wireless},
	{"wireless_2", handle_wireless_2},
	{"dhcp_release", handle_dhcp_release},
#ifdef HAVE_USB
	{"usbdrivers", handle_usbdrivers},
#endif
#ifdef HAVE_EOP_TUNNEL
	{"eop", handle_eop},
#endif
#ifdef HAVE_SPOTPASS
	{"spotpass", handle_spotpass},
#endif
	{NULL, NULL}
};

int start_single_service_main(int argc, char **argv)
{
	start_service_force("overclocking");
	char *next;
	char service[80];
	char *services = nvram_safe_get("action_service");

	foreach(service, services, next) {
#ifdef HAVE_OLED
		char message[32];
		sprintf(message, "restart: %s", service);
		lcdmessage(message);
#endif
		cprintf("Restart service=[%s]\n", service);
		int servicecount = 0;

		while (services_def[servicecount].servicename != NULL) {
			if (!strcmp(services_def[servicecount].servicename, service))
				services_def[servicecount].service();
			servicecount++;
		}
	}
	lcdmessage("");

	nvram_unset("nowebaction");
	nvram_unset("action_service");
	nvram_unset("action_service_arg1");
	return 0;
}

int is_running(char *process_name)
{
	DIR *dir;
	struct dirent *next;
	int retval = 0;

	dir = opendir("/proc");
	if (!dir) {
		perror("Cannot open /proc");
		return 0;
	}

	while ((next = readdir(dir)) != NULL) {
		FILE *status;
		char filename[100];
		char buffer[100];
		char name[100];

		if (strcmp(next->d_name, "..") == 0)
			continue;
		if (!isdigit(*next->d_name))
			continue;

		sprintf(filename, "/proc/%s/status", next->d_name);

		if (!(status = fopen(filename, "r")))
			continue;

		if (fgets(buffer, 99, status) == NULL) {
			fclose(status);
			continue;
		}
		fclose(status);

		sscanf(buffer, "%*s %s", name);

		if (strcmp(name, process_name) == 0)
			retval++;
	}

	closedir(dir);
	return retval;
}
