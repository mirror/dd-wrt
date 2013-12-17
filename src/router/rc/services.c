
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
	void *handle = NULL;

	nvram_set("qos_done", "0");
#ifdef HAVE_GPSI
	handle = start_service_nofree_f("gps", handle);
#endif
#ifdef HAVE_P910ND
	handle = start_service_nofree_f("printer", handle);
#endif
#ifdef HAVE_CPUTEMP
	handle = start_service_nofree_f("hwmon", handle);
#endif
#ifdef HAVE_TELNET
	handle = start_service_nofree_f("telnetd", handle);
#endif
#ifdef HAVE_FTP
	handle = start_service_nofree_f("ftpsrv", handle);
#endif
#ifdef HAVE_SAMBA3
	handle = start_service_nofree_f("samba3", handle);
#endif
#ifdef HAVE_MINIDLNA
	handle = start_service_nofree_f("dlna", handle);
#endif
#ifdef HAVE_PRIVOXY
	handle = start_service_nofree_f("privoxy", handle);
#endif
#ifdef HAVE_TOR
	handle = start_service_nofree_f("tor", handle);
#endif
#ifdef HAVE_TRANSMISSION
	handle = start_service_nofree_f("transmission", handle);
#endif
#ifdef HAVE_SYSLOG
	handle = start_service_nofree_f("syslog", handle);
#endif
#ifdef HAVE_TFTP
	handle = start_service_nofree_f("tftpd", handle);
#endif
	handle = start_service_nofree_f("httpd", handle);
	handle = start_service_nofree_f("udhcpd", handle);
#ifdef HAVE_DNSMASQ
	handle = start_service_nofree_f("dnsmasq", handle);
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	handle = start_service_nofree_f("zebra", handle);
#endif
#ifdef HAVE_OLSRD
	handle = start_service_nofree_f("olsrd", handle);
#endif

	handle = start_service_nofree_f("wshaper", handle);
	handle = start_service_nofree_f("wland", handle);
	handle = start_service_nofree_f("cron", handle);

#ifdef HAVE_PPTPD
	handle = start_service_nofree_f("pptpd", handle);
#endif

#ifdef HAVE_SSHD
#ifdef HAVE_REGISTER
	if (isregistered_real())
#endif
		handle = start_service_nofree_f("sshd", handle);
#endif

#ifdef HAVE_RADVD
	handle = start_service_nofree_f("radvd", handle);
#endif

#ifdef HAVE_SNMP
	handle = start_service_nofree_f("snmp", handle);
#endif

#ifdef HAVE_LLTD
	handle = start_service_nofree_f("lltd", handle);
#endif

#ifdef HAVE_PPPOESERVER
	handle = start_service_nofree_f("pppoeserver", handle);
#endif

#ifdef HAVE_WOL
	handle = start_service_nofree_f("wol", handle);
#endif

#ifdef HAVE_NOCAT
	handle = start_service_nofree_f("splashd", handle);
#endif

#ifdef HAVE_UPNP
	handle = start_service_nofree_f("upnp", handle);
#endif

#ifdef HAVE_OPENVPN
	handle = start_service_nofree_f("openvpnserversys", handle);
	handle = start_service_nofree_f("openvpn", handle);
#endif
#ifdef HAVE_VNCREPEATER
	handle = start_service_nofree_f("vncrepeater", handle);
#endif
#ifdef HAVE_RSTATS
	handle = start_service_nofree_f("rstats", handle);
#endif
#ifdef HAVE_NSTX
	handle = start_service_nofree_f("nstxd", handle);
#endif
#ifdef HAVE_PPPOERELAY
	handle = start_service_nofree_f("pppoerelay", handle);
#endif
#ifdef HAVE_MILKFISH
	handle = start_service_nofree_f("milkfish", handle);
#endif
#ifdef HAVE_FREERADIUS
	handle = start_service_nofree_f("freeradius", handle);
#endif
#ifdef HAVE_AP_SERV
	handle = start_service_nofree_f("apserv", handle);
#endif
#ifdef HAVE_SPOTPASS
	handle = start_service_nofree_f("spotpass", handle);
#endif
#ifdef HAVE_IAS
	if (atoi(nvram_safe_get("ias_startup")) > 0) {
		nvram_set("ias_startup", "3");
		nvram_set("ias_dnsresp", "1");
		system("/usr/sbin/dns_responder 192.168.11.1 55300 &");
	}
#endif
//    if( handle )
//      dlclose( handle );

	cprintf("done\n");
	return 0;
}

int stop_services_main(int argc, char **argv)
{
	void *handle = NULL;
#ifdef HAVE_P910ND
	handle = stop_service_nofree("printer", handle);
#endif

	// stop_ses();
#ifdef HAVE_AP_SERV
	handle = stop_service_nofree("apserv", handle);
#endif
#ifdef HAVE_FREERADIUS
	handle = stop_service_nofree("freeradius", handle);
#endif
#ifdef HAVE_MULTICAST
	handle = stop_service_nofree("igmp_proxy", handle);
#endif
#ifdef HAVE_UDPXY
	handle = stop_service_nofree("udpxy", handle);
#endif
#ifdef HAVE_PPPOERELAY
	handle = stop_service_nofree("pppoerelay", handle);
#endif
#ifdef HAVE_RSTATS
	handle = stop_service_nofree("rstats", handle);
#endif
#ifdef HAVE_NSTX
	handle = stop_service_nofree("nstxd", handle);
#endif

#ifdef HAVE_UPNP
	handle = stop_service_nofree("upnp", handle);
#endif
	handle = stop_service_nofree("udhcpd", handle);
	handle = stop_service_nofree("dns_clear_resolv", handle);
	handle = stop_service_nofree("cron", handle);

#ifdef HAVE_TFTP
	handle = stop_service_nofree("tftpd", handle);
#endif
#ifdef HAVE_SYSLOG
	handle = stop_service_nofree("syslog", handle);
#endif
#ifdef HAVE_OLSRD
	handle = stop_service_nofree("olsrd", handle);
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	handle = stop_service_nofree("zebra", handle);
#endif
	handle = stop_service_nofree("wland", handle);
#ifdef HAVE_TELNET
	handle = stop_service_nofree("telnetd", handle);
#endif
#ifdef HAVE_FTP
	handle = stop_service_nofree("ftpsrv", handle);
#endif
#ifdef HAVE_SAMBA3
	handle = stop_service_nofree("samba3", handle);
#endif
#ifdef HAVE_MINIDLNA
	handle = stop_service_nofree("dlna", handle);
#endif
#ifdef HAVE_TOR
	handle = stop_service_nofree("tor", handle);
#endif
#ifdef HAVE_TRANSMISSION
	handle = stop_service_nofree("transmission", handle);
#endif
#ifdef HAVE_PRIVOXY
	handle = stop_service_nofree("privoxy", handle);
#endif
#ifdef HAVE_SSHD
#ifdef HAVE_REGISTER
	if (isregistered_real())
#endif
		handle = stop_service_nofree("sshd", handle);
#endif

#ifdef HAVE_RADVD
	handle = stop_service_nofree("radvd", handle);
#endif

#ifdef HAVE_WIFIDOG
	handle = stop_service_nofree("wifidog", handle);
#endif

#ifdef HAVE_CHILLI
	handle = stop_service_nofree("chilli", handle);
#endif

#ifdef HAVE_PPPOESERVER
	handle = stop_service_nofree("pppoeserver", handle);
#endif

#ifdef HAVE_SNMP
	handle = stop_service_nofree("snmp", handle);
#endif

#ifdef HAVE_LLTD
	handle = stop_service_nofree("lltd", handle);
#endif

#ifdef HAVE_WOL
	handle = stop_service_nofree("wol", handle);
#endif
	handle = stop_service_nofree("wland", handle);
	handle = stop_service_nofree("wshaper", handle);

#ifdef HAVE_PPTPD
	handle = stop_service_nofree("pptpd", handle);
#endif
#ifdef HAVE_NOCAT
	handle = stop_service_nofree("splashd", handle);
#endif
#ifdef HAVE_VNCREPEATER
	handle = stop_service_nofree("vncrepeater", handle);
#endif
#ifdef HAVE_OPENVPN
	handle = stop_service_nofree("openvpnserversys", handle);
	handle = stop_service_nofree("openvpn", handle);
#endif
#ifdef HAVE_GPSI
	handle = stop_service_nofree("gps", handle);
#endif
//    if( handle )
//      dlclose( handle );

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
	void *handle = NULL;

	handle = stop_service_nofree("wan", handle);
	handle = stop_service_nofree("radio_timer", handle);
#ifdef HAVE_MULTICAST
	handle = stop_service_nofree("igmp_proxy", handle);
#endif
#ifdef HAVE_UDPXY
	handle = stop_service_nofree("udpxy", handle);
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	handle = stop_service_nofree("nas", handle);
#endif
#ifdef HAVE_MADWIFI
	handle = stop_service_nofree("stabridge", handle);
#endif
#ifdef HAVE_VLANTAGGING
	handle = stop_service_nofree("bridgesif", handle);
	handle = stop_service_nofree("vlantagging", handle);
#endif
#ifdef HAVE_BONDING
	handle = stop_service_nofree("bonding", handle);
#endif
	handle = stop_service_nofree("lan", handle);
#ifdef HAVE_VLANTAGGING
	handle = stop_service_nofree("bridging", handle);
#endif
	handle = stop_service_nofree("ttraff", handle);
	handle = stop_service_nofree("wan", handle);
#ifdef HAVE_VLANTAGGING
	handle = start_service_nofree("bridging", handle);
#endif
	handle = start_service_nofree("lan", handle);
#ifdef HAVE_BONDING
	handle = start_service_nofree("bonding", handle);
#endif
	handle = start_service_nofree("wan_boot", handle);
	handle = start_service_nofree_f("ttraff", handle);
#ifdef HAVE_MADWIFI
	handle = start_service_nofree_f("stabridge", handle);
#endif
	handle = startstop_nofree_f("udhcpd", handle);
#ifdef HAVE_DNSMASQ
	handle = startstop_nofree_f("dnsmasq", handle);
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	handle = startstop_nofree_f("zebra", handle);
#endif
#ifdef HAVE_OLSRD
	handle = startstop_nofree_f("olsrd", handle);
#endif
#ifdef HAVE_VLANTAGGING
	handle = start_service_nofree("vlantagging", handle);
	handle = start_service_nofree("bridgesif", handle);
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	handle = start_service_nofree("nas", handle);
	handle = start_service_nofree("guest_nas", handle);
#endif
	handle = start_service_nofree_f("radio_timer", handle);
	handle = startstop_nofree("firewall", handle);
	handle = startstop_nofree_f("httpd", handle);	// httpd will not
	// accept connection
	// anymore on wan/lan 
	// ip changes changes
	handle = startstop_nofree_f("cron", handle);	// httpd will not
	// accept connection
	// anymore on wan/lan 
	// ip changes changes
//      handle = start_service_nofree_f("anchorfreednat", handle);
	handle = start_service_nofree("wan_boot", handle);
#ifdef HAVE_NOCAT
	handle = startstop_nofree_f("splashd", handle);
#endif
//    if( handle )
//      dlclose( handle );
}

static void handle_router(void)
{
	void *handle = NULL;

#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	handle = startstop_nofree_f("zebra", handle);
#endif
#ifdef HAVE_OLSRD
	handle = startstop_nofree_f("olsrd", handle);
#endif
//    if( handle )
//      dlclose( handle );
}

/*static void handle_anchorfree(void)
{
	void *handle = NULL;

	handle = startstop_nofree_f("anchorfree", handle);
	handle = start_service_nofree_f("anchorfreednat", handle);
//    if( handle )
//      dlclose( handle );
}
*/
static void handle_hotspot(void)
{
	void *handle = NULL;
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	handle = stop_service_nofree("zebra", handle);
#endif

	handle = stop_service_nofree("radio_timer", handle);
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	handle = stop_service_nofree("nas", handle);
	eval("wlconf", nvram_safe_get("wl0_ifname"), "down");
	eval("wlconf", nvram_safe_get("wl1_ifname"), "down");
#endif
#ifdef HAVE_MADWIFI
	handle = stop_service_nofree("stabridge", handle);
#endif
	handle = stop_service_nofree("ttraff", handle);
	handle = stop_service_nofree("wan", handle);
#ifdef HAVE_VLANTAGGING
	handle = stop_service_nofree("bridgesif", handle);
	handle = stop_service_nofree("vlantagging", handle);
#endif
#ifdef HAVE_BONDING
	handle = stop_service_nofree("bonding", handle);
#endif
	handle = stop_service_nofree("lan", handle);
#ifdef HAVE_VLANTAGGING
	handle = stop_service_nofree("bridging", handle);
#endif
#ifdef HAVE_WIFIDOG
	handle = startstop_nofree_f("wifidog", handle);
#endif
#ifdef HAVE_NOCAT
	handle = startstop_nofree_f("splashd", handle);
#endif
#ifdef HAVE_SPUTNIK_APD
	handle = startstop_nofree_f("sputnik", handle);
#endif
#ifdef HAVE_CHILLI
	handle = startstop_nofree_f("chilli", handle);
#endif
#ifdef HAVE_VLANTAGGING
	handle = start_service_nofree("bridging", handle);
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	handle = start_service_nofree("wlconf", handle);
#endif
	handle = start_service_nofree("lan", handle);
#ifdef HAVE_BONDING
	handle = start_service_nofree("bonding", handle);
#endif
	handle = start_service_nofree_f("wan", handle);
	handle = start_service_nofree_f("ttraff", handle);
#ifdef HAVE_MADWIFI
	handle = start_service_nofree_f("stabridge", handle);
#endif
#ifdef HAVE_VLANTAGGING
	handle = start_service_nofree("vlantagging", handle);
	handle = start_service_nofree("bridgesif", handle);
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	handle = start_service_nofree("nas", handle);
	handle = start_service_nofree("guest_nas", handle);
#endif
	handle = start_service_nofree_f("radio_timer", handle);
	//restart dhcp as well, to fix repeater bridge save issue (dhcp disables itself here)
	handle = startstop_nofree_f("udhcpd", handle);
#ifdef HAVE_DNSMASQ
	handle = startstop_nofree_f("dnsmasq", handle);
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	handle = start_service_nofree("zebra", handle);
#endif
	startstop_f("httpd");	// httpd will not accept connection anymore

	FORK(eval("/etc/config/http-redirect.firewall"));
	FORK(eval("/etc/config/smtp-redirect.firewall"));
#ifdef HAVE_ZEROIP
	FORK(eval("/etc/config/shat.startup"));
#endif
}

static void handle_services(void)
{
	void *handle = NULL;
#ifdef HAVE_GPSI
	handle = startstop_nofree_f("gps", handle);
#endif
#ifdef HAVE_P910ND
	handle = startstop_nofree_f("printer", handle);
#endif
#ifdef HAVE_AP_SERV
	handle = startstop_nofree_f("apserv", handle);
#endif
#ifdef HAVE_PPPOERELAY
	handle = startstop_nofree_f("pppoerelay", handle);
#endif
	handle = startstop_nofree_f("udhcpd", handle);
#ifdef HAVE_SYSLOG
	handle = startstop_nofree_f("syslog", handle);
#endif
#ifdef HAVE_RSTATS
	handle = startstop_nofree_f("rstats", handle);
#endif
	handle = startstop_nofree_f("ttraff", handle);
#ifdef HAVE_NSTX
	handle = startstop_nofree_f("nstxd", handle);
#endif
#ifdef HAVE_PPPOESERVER
	handle = startstop_nofree("firewall", handle);
	handle = startstop_nofree_f("pppoeserver", handle);
#endif
#ifdef HAVE_DNSMASQ
	handle = startstop_nofree_f("dnsmasq", handle);
#endif
	handle = startstop_nofree_f("udhcpd", handle);
#ifdef HAVE_CPUTEMP
	handle = start_service_nofree_f("hwmon", handle);
#endif
#ifdef HAVE_TELNET
	handle = startstop_nofree_f("telnetd", handle);
#endif
#ifdef HAVE_SNMP
	handle = startstop_nofree_f("snmp", handle);
#endif
#ifdef HAVE_LLTD
	handle = startstop_nofree_f("lltd", handle);
#endif
#ifdef HAVE_PPTPD
	handle = startstop_nofree_f("pptpd", handle);
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
		handle = startstop_nofree_f("sshd", handle);
#endif
	handle = startstop_nofree("firewall", handle);
	handle = startstop_nofree_f("wshaper", handle);
#ifdef HAVE_SYSLOG
	handle = startstop_nofree_f("syslog", handle);
#endif
#ifdef HAVE_VNCREPEATER
	handle = startstop_nofree_f("vncrepeater", handle);
#endif
#ifdef HAVE_OPENVPN
	handle = stop_service_nofree("openvpnserver", handle);
	handle = stop_service_nofree("openvpn", handle);
	handle = start_service_nofree_f("openvpnserver", handle);
	handle = start_service_nofree_f("openvpn", handle);
#endif
#ifdef HAVE_NOCAT
	handle = startstop_nofree_f("splashd", handle);
#endif
//      handle = start_service_nofree_f("anchorfreednat", handle);
//    if( handle )
//      dlclose( handle );

}

static void handle_nassrv(void)
{
	void *handle = NULL;

#ifdef HAVE_MINIDLNA
	handle = stop_service_nofree("dlna", handle);
#endif
#ifdef HAVE_FTP
	handle = stop_service_nofree("ftpsrv", handle);
#endif
#ifdef HAVE_SAMBA3
	handle = stop_service_nofree("samba3", handle);
	handle = start_service_nofree_f("samba3", handle);
#endif
#ifdef HAVE_FTP
	handle = start_service_nofree_f("ftpsrv", handle);
#endif
#ifdef HAVE_MINIDLNA
	handle = start_service_nofree_f("dlna", handle);
#endif
#ifdef HAVE_TRANSMISSION
	handle = start_service_nofree_f("transmission", handle);
#endif
//    if( handle )
//      dlclose( handle );

}

static void handle_management(void)
{
	void *handle = NULL;

#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	handle = stop_service_nofree("nas", handle);
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	handle = stop_service_nofree("zebra", handle);
#endif
	handle = stop_service_nofree("cron", handle);
	handle = stop_service_nofree("udhcpd", handle);
	handle = start_service_nofree_f("udhcpd", handle);
	handle = start_service_nofree_f("cron", handle);
#ifdef HAVE_IPV6
	eval("/etc/config/ipv6.startup");
	if (nvram_match("need_reboot", "1")) {
		nvram_set("need_reboot", "0");
		nvram_set("need_commit", "0");
		nvram_commit();
		handle = start_service_nofree_f("ipv6", handle);
	}
#endif
#ifdef HAVE_RADVD
	handle = startstop_nofree_f("radvd", handle);
#endif
#ifdef HAVE_PPTPD
	handle = startstop_nofree_f("pptpd", handle);
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	handle = start_service_nofree_f("zebra", handle);
#endif
	handle = startstop_nofree("firewall", handle);
	handle = stop_service_nofree("wland", handle);
	handle = startstop_nofree_f("wshaper", handle);
	handle = start_service_nofree_f("wland", handle);
	handle = startstop_nofree_f("httpd", handle);
#ifdef HAVE_NOCAT
	handle = startstop_nofree_f("splashd", handle);
#endif
#ifdef HAVE_WOL
	handle = startstop_nofree_f("wol", handle);
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	handle = start_service_nofree("nas", handle);
	handle = start_service_nofree("guest_nas", handle);
#endif
//      handle = start_service_nofree_f("anchorfreednat", handle);

//    if( handle )
//      dlclose( handle );

}

static void handle_pppoe(void)
{
	unlink("/tmp/ppp/log");
	void *handle = NULL;

	handle = stop_service_nofree("radio_timer", handle);
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	handle = stop_service_nofree("nas", handle);
#endif
#ifdef HAVE_MADWIFI
	handle = stop_service_nofree("stabridge", handle);
#endif
#ifdef HAVE_VLANTAGGING
	handle = stop_service_nofree("bridgesif", handle);
	handle = stop_service_nofree("vlantagging", handle);
#endif
	handle = stop_service_nofree("lan", handle);
#ifdef HAVE_BONDING
	handle = stop_service_nofree("bonding", handle);
#endif
#ifdef HAVE_VLANTAGGING
	handle = stop_service_nofree("bridging", handle);
#endif
	handle = stop_service_nofree("ttraff", handle);
	handle = stop_service_nofree("wan", handle);
#ifdef HAVE_VLANTAGGING
	handle = start_service_nofree("bridging", handle);
#endif
	handle = start_service_nofree("lan", handle);
#ifdef HAVE_BONDING
	handle = start_service_nofree("bonding", handle);
#endif
	handle = start_service_nofree("wan_boot", handle);
	handle = start_service_nofree_f("ttraff", handle);
#ifdef HAVE_MADWIFI
	handle = start_service_nofree_f("stabridge", handle);
#endif
#ifdef HAVE_VLANTAGGING
	handle = start_service_nofree("vlantagging", handle);
	handle = start_service_nofree("bridgesif", handle);
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	handle = start_service_nofree("nas", handle);
	handle = start_service_nofree("guest_nas", handle);
#endif
	handle = start_service_nofree_f("radio_timer", handle);
//    if( handle )
//      dlclose( handle );

}

static void handle_spppoe(void)
{
	stop_service_f("ttraff");
	stop_service_f("wan");
}

static void handle_filters(void)
{
	void *handle = NULL;

	handle = stop_service_nofree("cron", handle);
	handle = startstop_nofree("firewall", handle);
#ifdef HAVE_SYSLOG
	handle = startstop_nofree_f("syslog", handle);
#endif
	handle = stop_service_nofree("wland", handle);
	handle = startstop_nofree_f("wshaper", handle);
	handle = start_service_nofree_f("wland", handle);
	handle = start_service_nofree_f("cron", handle);
#ifdef HAVE_NOCAT
	handle = startstop_nofree_f("splashd", handle);
#endif
#ifdef HAVE_MULTICAST
	handle = startstop_nofree_f("igmp_proxy", handle);
#endif
#ifdef HAVE_UDPXY
	handle = startstop_nofree_f("udpxy", handle);
#endif
//      handle = start_service_nofree_f("anchorfreednat", handle);
//    if( handle )
//      dlclose( handle );
}

static void handle_routing(void)
{
	void *handle = NULL;

#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	handle = stop_service_nofree("zebra", handle);
#endif
	handle = startstop_nofree("firewall", handle);
	handle = start_service_nofree_f("set_routes", handle);
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	handle = start_service_nofree_f("zebra", handle);
#endif
#ifdef HAVE_OLSRD
	handle = startstop_nofree_f("olsrd", handle);
#endif
#ifdef HAVE_NOCAT
	handle = startstop_nofree_f("splashd", handle);
#endif
//      handle = start_service_nofree_f("anchorfreednat", handle);
//    if( handle )
//      dlclose( handle );

}

static void handle_alive(void)
{
	void *handle = NULL;

	handle = stop_service_nofree("cron", handle);
#ifdef HAVE_REGISTER
	if (isregistered_real())
#endif
	{
		FORK(eval("/etc/config/wdswatchdog.startup"));
		FORK(eval("/etc/config/schedulerb.startup"));
		FORK(eval("/etc/config/proxywatchdog.startup"));
	}
	handle = start_service_nofree_f("cron", handle);
//    if( handle )
//      dlclose( handle );
}

static void handle_forward(void)
{
	void *handle = NULL;

	handle = stop_service_nofree("wland", handle);
	handle = stop_service_nofree("wshaper", handle);
#ifdef HAVE_UPNP
//    handle = stop_service_nofree( "upnp", handle );
#endif
	handle = startstop_nofree("firewall", handle);
#ifdef HAVE_UPNP
//    handle = start_service_nofree( "upnp", handle );
#endif
	handle = start_service_nofree_f("wshaper", handle);
	handle = start_service_nofree_f("wland", handle);
//      handle = start_service_nofree_f("anchorfreednat", handle);
#ifdef HAVE_NOCAT
	handle = startstop_nofree_f("splashd", handle);
#endif
//    if( handle )
//      dlclose( handle );

}

static void handle_qos(void)
{
	void *handle = NULL;

	startstop_f("wshaper");
	startstop_f("wland");
#ifdef HAVE_NOCAT
	handle = startstop_nofree_f("splashd", handle);
#endif
#ifdef HAVE_OPENVPN
	handle = stop_service_nofree("openvpnserver", handle);
	handle = stop_service_nofree("openvpn", handle);
	handle = start_service_nofree_f("openvpnserver", handle);
	handle = start_service_nofree_f("openvpn", handle);
#endif
//    if( handle )
//      dlclose( handle );
}

static void handle_forwardupnp(void)
{
	void *handle = NULL;

#ifdef HAVE_UPNP
	handle = stop_service_nofree("upnp", handle);
#endif
	handle = startstop_nofree("firewall", handle);
#ifdef HAVE_UPNP
	handle = start_service_nofree_f("upnp", handle);
#endif
	handle = stop_service_nofree("wland", handle);
	handle = startstop_nofree_f("wshaper", handle);
	handle = start_service_nofree_f("wland", handle);
//      handle = start_service_nofree_f("anchorfreednat", handle);
#ifdef HAVE_NOCAT
	handle = startstop_nofree_f("splashd", handle);
#endif
//    if( handle )
//      dlclose( handle );

}

static void handle_routedel(void)
{
	del_routes(nvram_safe_get("action_service_arg1"));

}

#ifdef HAVE_SPOTPASS
static void handle_spotpass(void)
{
	void *handle = NULL;
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
	void *handle = NULL;

	handle = stop_service_nofree("ttraff", handle);
	handle = stop_service_nofree("wan", handle);
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	handle = stop_service_nofree("zebra", handle);
#endif
#ifdef HAVE_OLSRD
	handle = stop_service_nofree("olsrd", handle);
#endif
#ifdef HAVE_UPNP
	handle = stop_service_nofree("upnp", handle);
#endif
	handle = stop_service_nofree("cron", handle);
//    if( handle )
//      dlclose( handle );

}

#ifdef HAVE_MILKFISH
static void handle_milkfish(void)
{
	startstop("milkfish");
}
#endif

static void handle_wireless(void)
{
	void *handle = NULL;
	int wanchanged = wanChanged();
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	handle = stop_service_nofree("zebra", handle);
#endif

	handle = stop_service_nofree("radio_timer", handle);
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	handle = stop_service_nofree("nas", handle);
	eval("wlconf", nvram_safe_get("wl0_ifname"), "down");
	eval("wlconf", nvram_safe_get("wl1_ifname"), "down");
#endif
#ifdef HAVE_MADWIFI
	handle = stop_service_nofree("stabridge", handle);
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
			handle = stop_service_nofree("ttraff", handle);
			handle = stop_service_nofree("wan", handle);
		}
	}
#ifdef HAVE_VLANTAGGING
	handle = stop_service_nofree("bridgesif", handle);
	handle = stop_service_nofree("vlantagging", handle);
#endif
#ifdef HAVE_BONDING
	handle = stop_service_nofree("bonding", handle);
#endif
	handle = stop_service_nofree("lan", handle);
#ifdef HAVE_VLANTAGGING
	handle = stop_service_nofree("bridging", handle);
#endif
#ifdef HAVE_VLANTAGGING
	handle = start_service_nofree("bridging", handle);
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	handle = start_service_nofree("wlconf", handle);
#endif
	handle = start_service_nofree("lan", handle);
#ifdef HAVE_BONDING
	handle = start_service_nofree("bonding", handle);
#endif
#ifdef HAVE_MADWIFI
	handle = start_service_nofree_f("stabridge", handle);
#endif
#ifdef HAVE_VLANTAGGING
	handle = start_service_nofree("vlantagging", handle);
	handle = start_service_nofree("bridgesif", handle);
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	handle = start_service_nofree("nas", handle);
	handle = start_service_nofree("guest_nas", handle);
#endif
	handle = start_service_nofree_f("radio_timer", handle);
	//restart dhcp as well, to fix repeater bridge save issue (dhcp disables itself here)
	handle = startstop_nofree_f("udhcpd", handle);
#ifdef HAVE_DNSMASQ
	handle = startstop_nofree_f("dnsmasq", handle);
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
			handle = start_service_nofree("wan_boot", handle);
			handle = start_service_nofree_f("ttraff", handle);
		}
	}
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	handle = start_service_nofree("zebra", handle);
#endif
	startstop_f("httpd");	// httpd will not accept connection anymore
	// on wan/lan ip changes changes
//    if( handle )
//      dlclose( handle );

}

static void handle_wireless_2(void)
{
	void *handle = NULL;
	int wanchanged = wanChanged();

#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	handle = stop_service_nofree("zebra", handle);
#endif

	handle = stop_service_nofree("radio_timer", handle);
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	handle = stop_service_nofree("nas", handle);
	eval("wlconf", nvram_safe_get("wl0_ifname"), "down");
	eval("wlconf", nvram_safe_get("wl1_ifname"), "down");
#endif
#ifdef HAVE_MADWIFI
	handle = stop_service_nofree("stabridge", handle);
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
			handle = stop_service_nofree("ttraff", handle);
			handle = stop_service_nofree("wan", handle);
		}
	}
#ifdef HAVE_VLANTAGGING
	handle = stop_service_nofree("bridgesif", handle);
	handle = stop_service_nofree("vlantagging", handle);
#endif
#ifdef HAVE_BONDING
	handle = stop_service_nofree("bonding", handle);
#endif
	handle = stop_service_nofree("lan", handle);
#ifdef HAVE_VLANTAGGING
	handle = stop_service_nofree("bridging", handle);
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	handle = start_service_nofree("wlconf", handle);
#endif
#ifdef HAVE_VLANTAGGING
	handle = start_service_nofree("bridging", handle);
#endif
	handle = start_service_nofree("lan", handle);
#ifdef HAVE_BONDING
	handle = start_service_nofree("bonding", handle);
#endif
#ifdef HAVE_MADWIFI
	handle = start_service_nofree_f("stabridge", handle);
#endif
#ifdef HAVE_VLANTAGGING
	handle = start_service_nofree("vlantagging", handle);
	handle = start_service_nofree("bridgesif", handle);
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	handle = start_service_nofree("nas", handle);
	handle = start_service_nofree("guest_nas", handle);
#endif
	handle = start_service_nofree_f("radio_timer", handle);
	if (getSTA() || getWET() || wanchanged
#ifdef HAVE_MADWIFI
	    || getWDSSTA()
#endif
	    ) {
#ifdef HAVE_3G
		if (!nvram_match("wan_proto", "3g"))
#endif
			startstop_f("httpd");	// httpd will not accept connection anymore
	}
	// on wan/lan ip changes changes
#ifdef HAVE_MADWIFI
	handle = start_service_nofree_f("hostapdwan", handle);
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
			handle = start_service_nofree("wan_boot", handle);
			handle = start_service_nofree_f("ttraff", handle);
		}
	}
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	handle = start_service_nofree_f("zebra", handle);
#endif
//    if( handle )
//      dlclose( handle );

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

	sleep(3);
	start_service("overclocking");
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
