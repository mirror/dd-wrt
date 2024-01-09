
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
#include <sys/ioctl.h> /* AhMan March 18 2005 */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <wait.h>
//#include <net/route.h>                /* AhMan March 18 2005 */
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

static void del_routes(char *route)
{
	char word[80], *tmp;
	char *ipaddr, *netmask, *gateway, *metric, *ifname;

	foreach(word, route, tmp)
	{
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

		eval("route", "del", "-net", ipaddr, "netmask", netmask, "gw",
		     gateway);
		// route_del (ifname, atoi (metric) + 1, ipaddr, gateway, netmask);
	}
}

static int start_services_main(int argc, char **argv)
{
	update_timezone();
	nvram_seti("qos_done", 0);
#ifdef HAVE_SYSLOG
	start_service_force_f("syslog");
#endif
#ifdef HAVE_SMARTD
	start_service_force_f("smartd");
#endif
#ifdef HAVE_GPSI
	start_service_f("gps");
#endif
#ifdef HAVE_P910ND
	start_service_f("printer");
#endif
#ifdef HAVE_CPUTEMP
	start_service_f("hwmon");
#endif
#ifdef HAVE_SOFTETHER
	start_service_f("softether");
#endif
#ifdef HAVE_TELNET
	start_service_force_f("telnetd");
#endif
#ifdef HAVE_MACTELNET
	start_service_force_f("mactelnetd");
#endif
#ifdef HAVE_RAID
	start_service_f("raid");
#endif
#ifdef HAVE_CHRONY
	start_service_f("chronyd");
#endif
#ifdef HAVE_FTP
	start_service_f("ftpsrv");
#endif
#ifdef HAVE_SAMBA3
	start_service_f("samba3");
#endif
#ifdef HAVE_NFS
	start_service_f("nfs");
#endif
#ifdef HAVE_RSYNC
	start_service_f("rsync");
#endif
#ifdef HAVE_MINIDLNA
	start_service_force_f("dlna");
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
#ifdef HAVE_PLEX
	start_service_f("plex");
#endif
#ifdef HAVE_TFTP
	start_service_f("tftpd");
#endif
	restart_fdelay("httpd", 2);
#ifdef HAVE_UNBOUND
	start_service_f("unbound");
#endif
#ifdef HAVE_DNSMASQ
	start_service_f("dnsmasq");
#endif
#ifdef HAVE_SMARTDNS
	start_service_f("smartdns");
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	start_service_f("zebra");
#endif
#ifdef HAVE_OLSRD
	start_service_f("olsrd");
#endif

	start_service_f("wland");
#ifndef HAVE_MICRO
	start_service_f("cron");
#endif

#ifdef HAVE_PPTPD
	start_service_f("pptpd");
#endif

#ifdef HAVE_SSHD
#ifdef HAVE_REGISTER
	if (isregistered_real())
#endif
		start_service_force_f("sshd");
#endif

#ifdef HAVE_IPV6
#ifdef HAVE_RADVD
	start_service_f("radvd");
#endif
#endif

#ifdef HAVE_SNMP
	start_service_force_f("snmp");
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

#ifdef HAVE_NODOG
	start_service_f("splashd");
#endif

#ifdef HAVE_UPNP
	start_service_f("upnp");
#endif

#ifdef HAVE_OPENVPN
	start_service_f("openvpnserversys");
	start_service_f("openvpn");
#endif
#ifdef HAVE_ANTAIRA_AGENT
	start_service_f("antaira_agent");
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
#ifdef HAVE_CONNTRACK
	start_service_f("notifier");
#endif
#ifdef HAVE_IAS
	if (nvram_geti("ias_startup") > 0) {
		nvram_seti("ias_startup", 3);
		nvram_seti("ias_dnsresp", 1);
		system("/usr/sbin/dns_responder 192.168.11.1 55300 &");
	}
#endif
#ifdef HAVE_MDNS
	start_service_f("mdns");
#endif
	cprintf("done\n");
	return 0;
}

static int stop_services_main(int argc, char **argv)
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
	stop_service_f("igmprt");
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
#ifdef HAVE_UNBOUND
	stop_service_f("unbound");
#endif
#ifdef HAVE_SMARTDNS
	stop_service_f("smartdns");
#endif
#ifdef HAVE_DNSMASQ
	stop_service_f("dnsmasq");
#endif
	restart_f("dns_clear_resolv");
#ifndef HAVE_MICRO
	stop_service_f("cron");
#endif
#ifdef HAVE_TFTP
	stop_service_f("tftpd");
#endif
#ifdef HAVE_OLSRD
	stop_service_f("olsrd");
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	stop_service_f("zebra");
#endif
	stop_service_f("wland");
#ifdef HAVE_SOFTETHER
	stop_service_f("softether");
#endif
#ifdef HAVE_CPUTEMP
	stop_service_f("hwmon");
#endif
#ifdef HAVE_SAMBA3
	stop_service_f("samba3");
#endif
#ifdef HAVE_NFS
	stop_service_f("nfs");
#endif
#ifdef HAVE_RSYNC
	stop_service_f("rsync");
#endif
#ifdef HAVE_CHRONY
	stop_service_f("chronyd");
#endif
#ifdef HAVE_RAID
	stop_service_f("raid");
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
#ifdef HAVE_PLEX
	stop_service_f("plex");
#endif
#ifdef HAVE_PRIVOXY
	stop_service_f("privoxy");
#endif

#ifdef HAVE_IPV6
#ifdef HAVE_RADVD
	stop_service_f("radvd");
#endif
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

#ifdef HAVE_LLTD
	stop_service_f("lltd");
#endif

#ifdef HAVE_WOL
	stop_service_f("wol");
#endif
	stop_service_f("wland");

#ifdef HAVE_PPTPD
	stop_service_f("pptpd");
#endif
#ifdef HAVE_NODOG
	stop_service_f("splashd");
#endif
#ifdef HAVE_VNCREPEATER
	stop_service_f("vncrepeater");
#endif
#ifdef HAVE_OPENVPN
	stop_service_f("openvpnserversys");
	stop_service_f("openvpn");
#endif
#ifdef HAVE_ANTAIRA_AGENT
	stop_service_f("antaira_agent");
#endif
#ifdef HAVE_GPSI
	stop_service_f("gps");
#endif
#ifdef HAVE_CONNTRACK
	stop_service_f("notifier");
#endif
#ifdef HAVE_MDNS
	stop_service_f("mdns");
#endif

	stop_running_main(0, NULL);

	cprintf("done\n");
	return 0;
}

static void handle_dhcpd(void)
{
	restart_f("dnsmasq");
#ifdef HAVE_SMARTDNS
	restart_f("smartdns");
#endif
}

static void handle_index(void)
{
	unlink("/tmp/ppp/log");
	restart_f("hostname");

	stop_service_force_f("wan");
	stop_service_f("radio_timer"); //
#ifdef HAVE_MULTICAST
	stop_service_f("igmprt"); //
#endif
#ifdef HAVE_UDPXY
	stop_service_f("udpxy");
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	stop_service_f("nas"); //
#endif
#ifdef HAVE_MADWIFI
	stop_service_f("stabridge");
#endif
#ifdef HAVE_EMF
	stop_service_f("emf"); //
#endif
#ifdef HAVE_VLANTAGGING
	stop_service_f("bridgesif"); //
	stop_service_f("vlantagging"); //
#endif
#ifdef HAVE_BONDING
	stop_service_f("bonding"); //
#endif

	stop_service_f("usteer"); //
	stop_service_f("ubus"); //
	stop_service_f("lan"); //
#ifdef HAVE_IPVS
	stop_service_f("ipvs"); //
#endif
#ifdef HAVE_VLANTAGGING
	stop_service_f("bridging"); //
#endif
	stop_service_f("ttraff"); //

	stop_running_main(0, NULL);

#ifdef HAVE_VLANTAGGING
	start_service("bridging");
#endif
	start_service_force("ubus");
	start_service_force("lan");
	start_service_force("setup_vlans");
#ifdef HAVE_IPVS
	start_service("ipvs");
#endif
#ifdef HAVE_BONDING
	start_service("bonding");
#endif
	start_service_force("wan_boot");
	start_service_f("ttraff");
#ifdef HAVE_MADWIFI
	start_service_f("stabridge");
#endif
#ifdef HAVE_UNBOUND
	restart_f("unbound");
#endif
#ifdef HAVE_DNSMASQ
	restart_f("dnsmasq");
#endif
#ifdef HAVE_SMARTDNS
	restart_f("smartdns");
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	restart_f("zebra");
#endif
#ifdef HAVE_OLSRD
	restart_f("olsrd");
#endif
#ifdef HAVE_VLANTAGGING
	start_service("vlantagging");
	start_service("bridgesif");
#endif
	start_service_force("usteer");
#ifdef HAVE_EMF
	start_service("emf"); //
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	start_service_f("nas");
#endif
#if defined(HAVE_TMK) || defined(HAVE_BKM)
	start_service_f("nsmd");
#endif

	start_service_f("radio_timer");
	restart("firewall");
	// httpd will not
	// accept connection
	// anymore on wan/lan
	// ip changes changes
	restart_fdelay("httpd", 2);
#ifndef HAVE_MICRO
	restart_f("cron");
#endif
//      start_service_f("anchorfreednat");
#ifdef HAVE_NODOG
	restart_f("splashd");
#endif
#ifdef HAVE_SMBD
	restart_f("samba3");
#endif
}

static void handle_router(void)
{
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	restart_f("zebra");
#endif
#ifdef HAVE_OLSRD
	restart_f("olsrd");
#endif
}

/*static void handle_anchorfree(void)
{

	restart_f("anchorfree");
	start_service_f("anchorfreednat");
}
*/
static void handle_hotspot(void)
{
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	stop_service_f("zebra");
#endif

	stop_service_f("radio_timer");
#ifdef HAVE_EMF
	stop_service_f("emf"); //
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
	stop_service_f("nas");
	wlconf_down(nvram_safe_get("wl0_ifname"));
	wlconf_down(nvram_safe_get("wl1_ifname"));
	wlconf_down(nvram_safe_get("wl2_ifname"));
#endif
#ifdef HAVE_MADWIFI
	stop_service_f("stabridge");
#endif
	stop_service_f("ttraff");
	stop_service_force_f("wan");

#ifdef HAVE_VLANTAGGING
	stop_service_f("bridgesif");
	stop_service_f("vlantagging");
#endif
#ifdef HAVE_BONDING
	stop_service_f("bonding");
#endif
#ifdef HAVE_IPVS
	stop_service_f("ipvs");
#endif
	stop_service_f("lan");
	stop_service_f("usteer");
	stop_service_f("ubus");
#ifdef HAVE_VLANTAGGING
	stop_service_f("bridging");
#endif
	stop_running_main(0, NULL);

#ifdef HAVE_WIFIDOG
	restart_f("wifidog");
#endif
#ifdef HAVE_NODOG
	restart_f("splashd");
#endif
#ifdef HAVE_SPUTNIK_APD
	restart_f("sputnik");
#endif
#ifdef HAVE_CHILLI
	restart_f("chilli");
#endif
#ifdef HAVE_VLANTAGGING
	start_service("bridging");
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
//      start_service("wlconf");
#endif
	start_service("ubus");
	start_service("lan");
	start_service_force("setup_vlans");
#ifdef HAVE_IPVS
	start_service("ipvs");
#endif
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
	start_service("usteer");
#ifdef HAVE_EMF
	start_service("emf"); //
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	start_service_f("nas");
#endif
	start_service_f("radio_timer");
	//restart dhcp as well, to fix repeater bridge save issue (dhcp disables itself here)
#ifdef HAVE_UNBOUND
	restart_f("unbound");
#endif
#ifdef HAVE_DNSMASQ
	restart_f("dnsmasq");
#endif
#ifdef HAVE_SMARTDNS
	restart_f("smartdns");
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	start_service("zebra");
#endif
	//since start/stop is faster now we need to sleep, otherwise httpd is stopped/started while response is sent to client
	restart_fdelay("httpd", 2); // httpd will not accept connection anymore
#ifdef HAVE_SMBD
	restart_f("samba3");
#endif

	restart("firewall");
#ifdef HAVE_ZEROIP
	FORK(eval("/etc/config/shat.startup"));
#endif
}

static void handle_pptp(void)
{
#ifdef HAVE_SOFTETHER
	restart_f("softether");
#endif
#ifdef HAVE_PPTPD
	restart_f("pptpd");
#endif
#ifdef HAVE_OPENVPN
	restart_f("openvpnserver");
	restart_f("openvpn");
	restart("firewall");
#endif
#ifdef HAVE_ANTAIRA_AGENT
	restart_f("antaira_agent");
#endif
}

#ifdef HAVE_SPEEDCHECKER
static void handle_speedchecker(void)
{
	restart_f("speedchecker");
}
#endif
static void handle_services(void)
{
#ifdef HAVE_SYSLOG
	restart_f("syslog");
#endif
#ifdef HAVE_GPSI
	restart_f("gps");
#endif
#ifdef HAVE_P910ND
	restart_f("printer");
#endif
#ifdef HAVE_AP_SERV
	restart_f("apserv");
#endif
#ifdef HAVE_PPPOERELAY
	restart_f("pppoerelay");
#endif
#ifdef HAVE_RSTATS
	restart_f("rstats");
#endif
	restart_f("ttraff");
#ifdef HAVE_NSTX
	restart_f("nstxd");
#endif
#ifdef HAVE_PPPOESERVER
	restart("firewall");
	restart_f("pppoeserver");
#endif
#ifdef HAVE_UNBOUND
	restart_f("unbound");
#endif
#ifdef HAVE_DNSMASQ
	restart_f("dnsmasq");
#endif
#ifdef HAVE_SMARTDNS
	restart_f("smartdns");
#endif
#ifdef HAVE_CPUTEMP
	restart_f("hwmon");
#endif
#ifdef HAVE_TOR
	restart_f("tor");
#endif
#ifdef HAVE_SOFTETHER
	restart_f("softether");
#endif
#ifdef HAVE_TELNET
	start_service_force_f("telnetd");
#endif
#ifdef HAVE_MACTELNET
	start_service_force_f("mactelnetd");
#endif
#ifdef HAVE_SNMP
	restart_f("snmp");
#endif
#ifdef HAVE_LLTD
	restart_f("lltd");
#endif
#ifdef HAVE_PPTPD
	restart_f("pptpd");
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
		start_service_force_f("sshd");
#endif
	restart("firewall");
#ifdef HAVE_VNCREPEATER
	restart_f("vncrepeater");
#endif
#ifdef HAVE_OPENVPN
	stop_service("openvpnserver");
	stop_service("openvpn");
	start_service_f("openvpnserver");
	start_service_f("openvpn");
	restart("firewall");
#endif
#ifdef HAVE_ANTAIRA_AGENT
	stop_service("antaira_agent");
	start_service_f("antaira_agent");
#endif
#ifdef HAVE_NODOG
	restart_f("splashd");
#endif
#ifdef HAVE_ZABBIX
	restart_f("zabbix");
#endif
#ifdef HAVE_MDNS
	restart_f("mdns");
#endif
#ifdef HAVE_CHRONY
	stop_service("chronyd");
	start_service_f("chronyd");
#endif
	//      start_service_f("anchorfreednat");
}

static void handle_nassrv(void)
{
#ifdef HAVE_TRANSMISSION
	stop_service_f("transmission");
#endif
#ifdef HAVE_PLEX
	stop_service_f("plex");
#endif
#ifdef HAVE_MINIDLNA
	stop_service_f("dlna");
#endif
#ifdef HAVE_FTP
	stop_service_f("ftpsrv");
#endif
#ifdef HAVE_RSYNC
	stop_service_f("rsync");
#endif
#ifdef HAVE_NFS
	stop_service_f("nfs");
#endif
#ifdef HAVE_RAID
	stop_service_f("raid");
#endif

	stop_running_main(0, NULL);

#ifdef HAVE_RAID
	start_service_f("raid");
#endif
#ifdef HAVE_NFS
	start_service_f("nfs");
#endif
#ifdef HAVE_RSYNC
	start_service_f("rsync");
#endif
#ifdef HAVE_FTP
	start_service_f("ftpsrv");
#endif
#ifdef HAVE_MINIDLNA
	start_service_force_f("dlna");
#endif
#ifdef HAVE_TRANSMISSION
	start_service_f("transmission");
#endif
#ifdef HAVE_PLEX
	start_service_f("plex");
#endif
#ifdef HAVE_SAMBA3
	restart_f("samba3");
#endif
#ifdef HAVE_MDNS
	restart_f("mdns");
#endif
	restart("firewall");
}

#ifdef HAVE_IPV6
static void handle_ipv6(void)
{
	stop_service_f("radvd");
	stop_service_f("httpd");
	stop_service_f("dhcp6c");
	stop_service_f("dhcp6s");
	stop_service_f("ipv6");
	stop_running_main(0, NULL);
	start_service("ipv6");
	start_service_f("radvd");
	start_service_f("httpd");
	start_service_f("dhcp6c");
	start_service_f("dhcp6s");
	handle_index();
}
#endif

static void handle_management(void)
{
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	stop_service_f("nas");
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	stop_service_f("zebra");
#endif
#ifndef HAVE_MICRO
	stop_service_f("cron");
#endif
#ifdef HAVE_IPV6
	stop_service_f("ipv6");
#endif

	stop_running_main(0, NULL);

#ifndef HAVE_MICRO
	start_service_f("cron");
#endif
#ifdef HAVE_IPV6
	start_service_f("ipv6");
#ifdef HAVE_RADVD
	restart_f("radvd");
#endif
#endif
#ifdef HAVE_PPTPD
	restart_f("pptpd");
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	start_service_f("zebra");
#endif
	restart("firewall");
	stop_service("wland");
	start_service_f("wland");
	restart_fdelay("httpd", 2);
#ifdef HAVE_NODOG
	restart_f("splashd");
#endif
#ifdef HAVE_WOL
	restart_f("wol");
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	start_service_f("nas");
#endif
#ifdef HAVE_JFFS2
	restart_f("jffs2");
#endif
#ifdef HAVE_X86
	restart_f("bootconfig");
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
	stop_service_f("emf"); //
#endif
#ifdef HAVE_VLANTAGGING
	stop_service_f("bridgesif");
	stop_service_f("vlantagging");
#endif
#ifdef HAVE_IPVS
	stop_service_f("ipvs");
#endif
	stop_service_f("lan");
	stop_service_f("usteer");
	stop_service_f("ubus");
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
	start_service("ubus");
	start_service("lan");
	start_service_force("setup_vlans");
#ifdef HAVE_IPVS
	start_service("ipvs");
#endif
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
	start_service("usteer");
#ifdef HAVE_EMF
	start_service("emf"); //
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	start_service_f("nas");
#endif
	start_service_f("radio_timer");
#ifdef HAVE_SMBD
	restart_f("samba3");
#endif
}

static void handle_speedtest(void)
{
// use minimum of 2 threads
	cpucount = 4;
	char nr[32];
	sprintf(nr, "%d", cpucount);
	char *args[] = { "speedtest_cli", "1", nr, "1", nr, NULL };
	_evalpid(args, NULL, 0, NULL);
}

static void handle_spppoe(void)
{
	stop_service_f("ttraff");
	stop_service_force_f("wan");
}

static void handle_filters(void)
{
#ifdef HAVE_SYSLOG
	restart_f("syslog");
#endif
#ifndef HAVE_MICRO
	stop_service("cron");
#endif
	restart("firewall");
	stop_service("wland");
	start_service_f("wland");
#ifndef HAVE_MICRO
	start_service_f("cron");
#endif
#ifdef HAVE_NODOG
	restart_f("splashd");
#endif
#ifdef HAVE_MULTICAST
	restart_f("igmprt");
#endif
#ifdef HAVE_UDPXY
	restart_f("udpxy");
#endif
#ifdef HAVE_CONNTRACK
	restart_f("notifier");
#endif
	//      start_service_f("anchorfreednat");
}

static void handle_routing(void)
{
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	stop_service("zebra");
#endif
	restart("firewall");
	start_service_force_f("set_routes");
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	start_service_f("zebra");
#endif
#ifdef HAVE_OLSRD
	restart_f("olsrd");
#endif
#ifdef HAVE_NODOG
	restart_f("splashd");
#endif
	//      start_service_f("anchorfreednat");
}

static void handle_alive(void)
{
#ifndef HAVE_MICRO
	stop_service("cron");
#endif
#ifdef HAVE_REGISTER
	if (isregistered_real())
#endif
	{
		FORK(eval("/etc/config/wdswatchdog.startup"));
		FORK(eval("/etc/config/schedulerb.startup"));
		FORK(eval("/etc/config/proxywatchdog.startup"));
	}
#ifndef HAVE_MICRO
	start_service_f("cron");
#endif
}

static void handle_forward(void)
{
	stop_service("wland");
#ifdef HAVE_UPNP
//    stop_service( "upnp");
#endif
	restart("firewall");
#ifdef HAVE_UPNP
//    start_service( "upnp");
#endif
	start_service_f("wland");
//      start_service_f("anchorfreednat");
#ifdef HAVE_NODOG
	restart_f("splashd");
#endif
}

static void handle_qos(void)
{
	restart_f("wland");
#ifdef HAVE_NODOG
	restart_f("splashd");
#endif
#ifdef HAVE_OPENVPN
	stop_service("openvpnserver");
	stop_service("openvpn");
	start_service_f("openvpnserver");
	start_service_f("openvpn");
#endif
	restart("firewall");
}

static void handle_forwardupnp(void)
{
#ifdef HAVE_UPNP
	stop_service("upnp");
#endif
	restart("firewall");
#ifdef HAVE_UPNP
	start_service_f("upnp");
#endif
	stop_service("wland");
	start_service_f("wland");
//      start_service_f("anchorfreednat");
#ifdef HAVE_NODOG
	restart_f("splashd");
#endif
}

static void handle_routedel(void)
{
	del_routes(nvram_safe_get("action_service_arg1"));
}

#ifdef HAVE_SPOTPASS
static void handle_spotpass(void)
{
	restart("spotpass");
}
#endif

struct SERVICES {
	char *servicename;
	void (*service)(void);
};

static void handle_ddns(void)
{
	restart("ddns");
	nvram_set("ddns_change", "update");
}

#ifdef HAVE_SYSCTL_EDIT
static void handle_sysctl(void)
{
	restart("sysctl_config");
}
#endif

#ifdef HAVE_FREERADIUS

static void handle_freeradius(void)
{
	restart_f("freeradius");
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
#ifndef HAVE_MICRO
	stop_service_f("cron");
#endif
	stop_running_main(0, NULL);
}

#ifdef HAVE_MILKFISH
static void handle_milkfish(void)
{
	restart("milkfish");
}
#endif

static void handle_wireless(void)
{
	int wanchanged = wanChanged();
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	stop_service_f("zebra");
#endif

	stop_service_f("radio_timer");
#ifdef HAVE_EMF
	stop_service_f("emf"); //
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
	stop_service_f("nas");
	wlconf_down(nvram_safe_get("wl0_ifname"));
	wlconf_down(nvram_safe_get("wl1_ifname"));
	wlconf_down(nvram_safe_get("wl2_ifname"));
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
#ifdef HAVE_VLANTAGGING
	stop_service("bridgesif");
	stop_service("vlantagging");
#endif
#ifdef HAVE_BONDING
	stop_service_f("bonding");
#endif
#ifdef HAVE_VLANTAGGING
	stop_service("bridging");
#endif
#ifdef HAVE_IPVS
	stop_service_f("ipvs");
#endif
	stop_running_main(0, NULL);
	stop_service("lan");
	stop_service("usteer");
	stop_service("ubus");
#ifdef HAVE_VLANTAGGING
	start_service("bridging");
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
//      start_service("wlconf");
#endif
	start_service_force("ubus");
	start_service_force("lan");
	start_service_force("setup_vlans");
#ifdef HAVE_IPVS
	start_service("ipvs");
#endif
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
	start_service_force("usteer");
#ifdef HAVE_EMF
	start_service("emf"); //
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	start_service_f("nas");
#endif
	start_service_f("radio_timer");
	//restart dhcp as well, to fix repeater bridge save issue (dhcp disables itself here)
#ifdef HAVE_UNBOUND
	restart_f("unbound");
#endif
#ifdef HAVE_DNSMASQ
	restart_f("dnsmasq");
#endif
#ifdef HAVE_SMARTDNS
	restart_f("smartdns");
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
#ifdef HAVE_IPV6
	restart_f("dhcp6c");
#endif
	//since start/stop is faster now we need to sleep, otherwise httpd is stopped/started while response is sent to client
#ifdef HAVE_80211AC
	restart_fdelay(
		"httpd",
		2); // httpd will not accept connection anymore on wan/lan ip changes changes
#else
	restart_fdelay(
		"httpd",
		4); // httpd will not accept connection anymore on wan/lan ip changes changes
#endif
#ifdef HAVE_SMBD
	restart_f("samba3");
#endif
}

static void handle_wireless_2(void)
{
	int wanchanged = wanChanged();

#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	stop_service_f("zebra");
#endif

	stop_service_f("radio_timer");
#ifdef HAVE_EMF
	stop_service_f("emf"); //
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
	stop_service_f("nas");
	wlconf_down(nvram_safe_get("wl0_ifname"));
	wlconf_down(nvram_safe_get("wl1_ifname"));
	wlconf_down(nvram_safe_get("wl2_ifname"));
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
#ifdef HAVE_VLANTAGGING
	stop_service("bridgesif");
	stop_service("vlantagging");
#endif
#ifdef HAVE_BONDING
	stop_service_f("bonding");
#endif
#ifdef HAVE_IPVS
	stop_service_f("ipvs");
#endif
	stop_service("lan");
	stop_service("usteer");
	stop_service("ubus");
#ifdef HAVE_VLANTAGGING
	stop_service("bridging");
#endif
	stop_running_main(0, NULL);
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
//      start_service("wlconf");
#endif
#ifdef HAVE_VLANTAGGING
	start_service("bridging");
#endif
	start_service_force("ubus");
	start_service_force("lan");
	start_service_force("setup_vlans");
#ifdef HAVE_IPVS
	start_service("ipvs");
#endif
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
	start_service_force("usteer");
#ifdef HAVE_EMF
	start_service("emf"); //
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	start_service_f("nas");
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
			restart_fdelay(
				"httpd",
				2); // httpd will not accept connection anymore
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
#ifdef HAVE_IPV6
	restart_f("dhcp6c");
#endif
#ifdef HAVE_SMBD
	restart_f("samba3");
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
	restart("drivers_net"); //stop is not yet implemented but we dont care about yet
#ifdef HAVE_P910ND
	restart("printer");
#endif
}
#endif
#ifdef HAVE_EOP_TUNNEL
static void handle_eop(void)
{
	restart("firewall");
}
#endif
static struct SERVICES services_def[] = {
	{ "dhcp", handle_dhcpd },
	{ "index", handle_index },
	{ "router", handle_router },
	{ "hotspot", handle_hotspot },
	//      {"anchorfree", handle_anchorfree},
	{ "services", handle_services },
#if defined(HAVE_FTP) || defined(HAVE_SAMBA3) || defined(HAVE_MINIDLNA) || \
	defined(HAVE_RAID)
	{ "nassrv", handle_nassrv },
#endif
	{ "management", handle_management },
#ifdef HAVE_3G
	{ "start_3g", handle_pppoe },
	{ "stop_3g", handle_spppoe },
#endif
#ifdef HAVE_PPPOATM
	{ "start_pppoa", handle_pppoe },
	{ "stop_pppoa", handle_spppoe },
#endif
	{ "start_pppoe", handle_pppoe },
	{ "stop_pppoe", handle_spppoe },
	{ "start_pptp", handle_pppoe },
	{ "stop_pptp", handle_spppoe },
	{ "speedtest", handle_speedtest },
#ifdef HAVE_L2TP
	{ "start_l2tp", handle_pppoe },
#endif
#ifdef HAVE_FREERADIUS
	{ "freeradius", handle_freeradius },
#endif
#ifdef HAVE_HEARTBEAT
	{ "start_heartbeat", handle_pppoe },
#endif
	{ "stop_pppoe", handle_spppoe },
	{ "stop_pptp", handle_spppoe },
#ifdef HAVE_L2TP
	{ "stop_l2tp", handle_spppoe },
#endif
#ifdef HAVE_HEARTBEAT
	{ "stop_heartbeat", handle_spppoe },
#endif
#ifdef HAVE_SYSCTL_EDIT
	{ "sysctl", handle_sysctl },
#endif
	{ "filters", handle_filters },
	{ "routing", handle_routing },
	{ "alive", handle_alive },
	{ "forward", handle_forward },
	{ "qos", handle_qos },
	{ "forward_upnp", handle_forwardupnp },
	{ "static_route_del", handle_routedel },
	{ "ddns", handle_ddns },
	// {"start_ping", handle_ping},
	{ "http_upgrade", handle_upgrade },
#ifdef HAVE_MILKFISH
	{ "milkfish", handle_milkfish },
#endif
	{ "wireless", handle_wireless },
	{ "wireless_2", handle_wireless_2 },
	{ "dhcp_release", handle_dhcp_release },
#ifdef HAVE_USB
	{ "usbdrivers", handle_usbdrivers },
#endif
#ifdef HAVE_EOP_TUNNEL
	{ "eop", handle_eop },
#endif
#ifdef HAVE_SPOTPASS
	{ "spotpass", handle_spotpass },
#endif
#ifdef HAVE_IPV6
	{ "ipv6", handle_ipv6 },
#endif
	{ "pptp", handle_pptp },
#ifdef HAVE_SPEEDCHECKER
	{ "speedchecker", handle_speedchecker },
#endif
	{ NULL, NULL }
};

static int single_service_helper(void)
{
	int sr = atoi(nvram_safe_get("service_running"));
	if (sr) {
		sr++;
		nvram_seti("service_running", sr);
		dd_loginfo("services", "increase delay to %d seconds\n",
			   sr * 5);
		return 0;
	}
	nvram_seti("service_running", 1);
	sleep(5);
	while ((sr = atoi(nvram_safe_get("service_running"))) > 1) {
		sr--;
		nvram_seti("service_running", sr);
		sleep(5);
	}
	start_service_force("overclocking");
	char *next;
	char service[80];
	char *services = nvram_safe_get("action_service");
	update_timezone();
	foreach(service, services, next)
	{
#ifdef HAVE_OLED
		char message[32];
		sprintf(message, "restart: %s", service);
		lcdmessage(message);
#endif
		cprintf("Restart service=[%s]\n", service);
		int servicecount = 0;

		while (services_def[servicecount].servicename != NULL) {
			if (!strcmp(services_def[servicecount].servicename,
				    service))
				services_def[servicecount].service();
			servicecount++;
		}
	}
	lcdmessage("");

	nvram_unset("nowebaction");
	nvram_unset("action_service");
	nvram_unset("action_service_arg1");
	nvram_unset("action_service_arg2");
	nvram_seti("service_running", 0);
	return 0;
}

static int start_single_service_main(int argc, char **argv)
{
	FORK(single_service_helper());
	return 0;
}
