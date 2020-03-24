/*
 * dnsmasq.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 * $Id:
 */
#ifdef HAVE_DNSMASQ
#include <netinet/in.h>
#include <arpa/inet.h>
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
#include <wlutils.h>
#include <services.h>

extern int usejffs;

extern void addHost(char *host, char *ip, int withdomain);

void stop_dnsmasq(void);
#define IDX_IFNAME 0
#define IDX_DHCPON 1
#define IDX_LEASESTART 2
#define IDX_LEASEMAX 3
#define IDX_LEASETIME 4

static char *getmdhcp(int count, int index, char *word, char *buffer)
{
	int cnt = 0;
	char *next, *wordlist;

	wordlist = nvram_safe_get("mdhcpd");
	foreach(word, wordlist, next) {
		if (cnt < index) {
			cnt++;
			continue;
		}
		GETENTRYBYIDX(interface, word, 0);
		GETENTRYBYIDX(dhcpon, word, 1);
		GETENTRYBYIDX(start, word, 2);
		GETENTRYBYIDX(max, word, 3);
		GETENTRYBYIDX(leasetime, word, 4);
		if (leasetime == NULL) {
			leasetime = "3660";
		}
		switch (count) {
		case 0:
			strcpy(buffer, interface);
			return buffer;
		case 1:
			strcpy(buffer, dhcpon);
			return buffer;
		case 2:
			strcpy(buffer, start);
			return buffer;
		case 3:
			strcpy(buffer, max);
			return buffer;
		case 4:
			strcpy(buffer, leasetime);
			return buffer;
		}
	}
	return "";
}

static int landhcp(void)
{
	if (!getWET())
		if (nvram_match("lan_proto", "dhcp")
		    && nvram_matchi("dhcpfwd_enable", 0))
			return 1;
	return 0;
}

static int hasmdhcp(void)
{
	if (nvram_exists("mdhcpd_count")) {
		int mdhcpcount = nvram_geti("mdhcpd_count");
		return mdhcpcount > 0 ? 1 : 0;
	}
	return 0;
}

static int canlan(void)
{
	if ((nvram_matchi("dhcpfwd_enable", 0) || nvram_matchi("dns_crypt", 1)))
		return 1;
	return 0;
}

static void makeentry(FILE * fp, char *ifname, int dhcpnum, int dhcpstart, char *ip, char *netmask, char *leasetime)
{
	unsigned int ip1 = get_single_ip(ip, 0);
	unsigned int ip2 = get_single_ip(ip, 1);
	unsigned int ip3 = get_single_ip(ip, 2);
	unsigned int ip4 = get_single_ip(ip, 3);
	unsigned int im1 = get_single_ip(netmask, 0);
	unsigned int im2 = get_single_ip(netmask, 1);
	unsigned int im3 = get_single_ip(netmask, 2);
	unsigned int im4 = get_single_ip(netmask, 3);
	unsigned int sip = ((ip1 & im1) << 24) + ((ip2 & im2) << 16) + ((ip3 & im3) << 8) + dhcpstart;
	unsigned int eip = sip + dhcpnum - 1;

	fprintf(fp, "dhcp-range=%s,", ifname);

	fprintf(fp, "%d.%d.%d.%d,", ip1 & im1, ip2 & im2, ip3 & im3, dhcpstart);
	fprintf(fp, "%d.%d.%d.%d,", (eip >> 24) & 0xff, (eip >> 16) & 0xff, (eip >> 8) & 0xff, eip & 0xff);
	fprintf(fp, "%s,", netmask);
	fprintf(fp, "%sm\n", leasetime);
}

void start_dnsmasq(void)
{
	FILE *fp;
	struct dns_lists *dns_list = NULL;
	int i;

	if (nvram_match("lan_proto", "dhcp")
	    && nvram_matchi("dnsmasq_enable", 0)) {
		nvram_seti("dnsmasq_enable", 1);
		nvram_commit();
	}

	if (!nvram_invmatchi("dnsmasq_enable", 0)) {
		stop_dnsmasq();
		return;
	}
#ifdef HAVE_SMARTDNS
	start_smartdns();
#endif
	int leasechange = nvram_state_change("static_leases");

	update_timezone();

#ifdef HAVE_DNSCRYPT
	if (nvram_matchi("dns_crypt", 1)) {
		stop_process("dnscrypt-proxy", "daemon");
		eval("dnscrypt-proxy", "-d", "-S", "-a", "127.0.0.1:30", "-R", nvram_safe_get("dns_crypt_resolver"), "-L", "/etc/dnscrypt/dnscrypt-resolvers.csv");
	}
#endif
	usejffs = 0;

	if (nvram_matchi("dhcpd_usejffs", 1)) {
		if (!(fp = fopen("/jffs/dnsmasq.leases", "a"))) {
			usejffs = 0;
		} else {
			fclose(fp);
			usejffs = 1;
		}
	}

	/*
	 * Write configuration file based on current information 
	 */
	if (!(fp = fopen("/tmp/dnsmasq.conf", "w"))) {
		perror("/tmp/dnsmasq.conf");
		return;
	}
//    fprintf(fp, "bind-interfaces\n");
	if (nvram_matchi("chilli_enable", 1) || nvram_matchi("hotss_enable", 1)) {
		char *chilliif;
#ifdef HAVE_HOTSPOT
		if (nvram_matchi("hotss_enable", 1))
			chilliif = nvram_safe_get("hotss_interface");
		else
#endif
			chilliif = nvram_safe_get("chilli_interface");

		fprintf(fp, "interface=%s", chilliif);
		if (!canlan()) {
			fprintf(fp, ",");
		} else {
			if (strcmp(chilliif, nvram_safe_get("lan_ifname"))) {
				fprintf(fp, ",%s", nvram_safe_get("lan_ifname"));
			}
		}
	} else if (nvram_matchi("pptpd_enable", 1)) {
		fprintf(fp, "listen-address=127.0.0.1");
		if (canlan()) {
			fprintf(fp, ",%s", nvram_safe_get("lan_ipaddr"));
#ifdef HAVE_IPV6
			char *ip = getifaddr(nvram_safe_get("lan_ifname"), AF_INET6, GIF_LINKLOCAL) ? : NULL;
			if (ip && nvram_matchi("ipv6_enable", 1))
				fprintf(fp, ",%s", ip);
#endif

		}
		if (nvram_exists("dnsmasq_addlisten")) {
			fprintf(fp, ",%s", nvram_safe_get("dnsmasq_addlisten"));
		}
	} else {
		fprintf(fp, "interface=");
		if (canlan()) {
			fprintf(fp, "%s", nvram_safe_get("lan_ifname"));
		}
		if (nvram_exists("dnsmasq_addif")) {
			fprintf(fp, ",%s", nvram_safe_get("dnsmasq_addif"));
		}
	}
	int mdhcpcount = 0;
	if (nvram_exists("mdhcpd_count")) {
		char *word = calloc(128, 1);
		mdhcpcount = nvram_geti("mdhcpd_count");
		for (i = 0; i < mdhcpcount; i++) {
			char buffer[128];
			char *ifname = getmdhcp(IDX_IFNAME, i, word, buffer);
			if (!*(nvram_nget("%s_ipaddr", ifname)) || !*(nvram_nget("%s_netmask", ifname)))
				continue;
			if (canlan() || i > 0) {
				fprintf(fp, ",");
			}
			if (nvram_matchi("pptpd_enable", 1)) {
				fprintf(fp, "%s", nvram_nget("%s_ipaddr", ifname));
#ifdef HAVE_IPV6
				char *ip = getifaddr(ifname, AF_INET6, GIF_LINKLOCAL) ? : NULL;
				if (ip && nvram_matchi("ipv6_enable", 1))
					fprintf(fp, ",%s", ip);
#endif
			} else
				fprintf(fp, "%s", ifname);
		}
		free(word);
	}
	fprintf(fp, "\n");
	fprintf(fp, "resolv-file=/tmp/resolv.dnsmasq\n");
	//fprintf(fp, "all-servers\n");
	if (nvram_matchi("dnsmasq_strict", 1))
		fprintf(fp, "strict-order\n");

#ifdef HAVE_SMARTDNS
	if (nvram_matchi("smartdns", 1)) {
		nvram_seti("dns_crypt", 0);
		fprintf(fp, "server=127.0.0.1#6053\n");
		fprintf(fp, "no-resolv\n");
	}
#endif
#ifdef HAVE_DNSCRYPT
	if (nvram_matchi("dns_crypt", 1)) {
		nvram_seti("recursive_dns", 0);	// disable unbound
		fprintf(fp, "server=127.0.0.1#30\n");
		fprintf(fp, "no-resolv\n");
	}
#endif
#ifdef HAVE_UNBOUND
	if (nvram_matchi("recursive_dns", 1) && !nvram_matchi("smartdns", 1)) {
		fprintf(fp, "server=127.0.0.1#7053\n");
		fprintf(fp, "no-resolv\n");
	}
#endif
	/*
	 * Domain 
	 */
	if (nvram_match("dhcp_domain", "wan")) {
		if (nvram_invmatch("wan_domain", ""))
			fprintf(fp, "domain=%s\n", nvram_safe_get("wan_domain"));
		else if (nvram_invmatch("wan_get_domain", ""))
			fprintf(fp, "domain=%s\n", nvram_safe_get("wan_get_domain"));
	} else {
		if (nvram_invmatch("lan_domain", ""))
			fprintf(fp, "domain=%s\n", nvram_safe_get("lan_domain"));
	}

	/*
	 * DD-WRT use dnsmasq as DHCP replacement 
	 */

	//bs mod
	if (landhcp() || hasmdhcp()) {
		/*
		 * DHCP leasefile 
		 */
		if (nvram_matchi("dhcpd_usenvram", 1)) {
			fprintf(fp, "leasefile-ro\n");
			fprintf(fp, "dhcp-script=%s\n", "/etc/lease_update.sh");
		} else {
			if (usejffs)
				fprintf(fp, "dhcp-leasefile=/jffs/dnsmasq.leases\n");
			else
				fprintf(fp, "dhcp-leasefile=/tmp/dnsmasq.leases\n");
		}

		int dhcp_max = 0;

		char *word = calloc(128, 1);

		if (landhcp())
			dhcp_max += nvram_geti("dhcp_num") + nvram_geti("static_leasenum");
		for (i = 0; i < mdhcpcount; i++) {
			char buffer[128];
			char *ifname = getmdhcp(IDX_IFNAME, i, word, buffer);
			if (!*(nvram_nget("%s_ipaddr", ifname)) || !*(nvram_nget("%s_netmask", ifname)))
				continue;
			dhcp_max += atoi(getmdhcp(IDX_LEASEMAX, i, word, buffer));
		}
		fprintf(fp, "dhcp-lease-max=%d\n", dhcp_max);
		if (landhcp())
			fprintf(fp, "dhcp-option=%s,3,%s\n", nvram_safe_get("lan_ifname"), nvram_safe_get("lan_ipaddr"));
		for (i = 0; i < mdhcpcount; i++) {
			char buffer[128];
			char *ifname = getmdhcp(IDX_IFNAME, i, word, buffer);
			if (!*(nvram_nget("%s_ipaddr", ifname)) || !*(nvram_nget("%s_netmask", ifname)))
				continue;
			fprintf(fp, "dhcp-option=%s,3,", ifname);
			fprintf(fp, "%s\n", nvram_nget("%s_ipaddr", ifname));
		}
		if (nvram_invmatch("wan_wins", "")
		    && nvram_invmatch("wan_wins", "0.0.0.0"))
			fprintf(fp, "dhcp-option=44,%s\n", nvram_safe_get("wan_wins"));
		free(word);
		if (nvram_matchi("dns_dnsmasq", 0)) {
#ifdef HAVE_UNBOUND
			if (nvram_matchi("recursive_dns", 1)) {
				fprintf(fp, "dhcp-option=6,%s\n", nvram_safe_get("lan_ipaddr"));
			} else
#endif
#ifdef HAVE_SMARTDNS
			if (nvram_matchi("smartdns", 1)) {
				fprintf(fp, "dhcp-option=6,%s\n", nvram_safe_get("lan_ipaddr"));
			} else
#endif
			{
				dns_list = get_dns_list();

				if (dns_list && dns_list->num_servers > 0) {

					fprintf(fp, "dhcp-option=6");
					for (i = 0; i < dns_list->num_servers; i++)
						fprintf(fp, ",%s", dns_list->dns_server[i]);
					fprintf(fp, "\n");
				}
				if (dns_list)
					free_dns_list(dns_list);
			}
		}

		if (nvram_matchi("auth_dnsmasq", 1))
			fprintf(fp, "dhcp-authoritative\n");
		if (landhcp()) {
			unsigned int dhcpnum = nvram_geti("dhcp_num");
			unsigned int dhcpstart = nvram_geti("dhcp_start");
			char *ip = nvram_safe_get("lan_ipaddr");
			char *netmask = nvram_safe_get("lan_netmask");
			char *leasetime = nvram_safe_get("dhcp_lease");
			makeentry(fp, nvram_safe_get("lan_ifname"), dhcpnum, dhcpstart, ip, netmask, leasetime);
		}

		for (i = 0; i < mdhcpcount; i++) {
			char *word = calloc(128, 1);
			char buffer[128];
			char buffer2[128];
			if (strcmp(getmdhcp(IDX_DHCPON, i, word, buffer), "On"))
				continue;
			char *ifname = getmdhcp(IDX_IFNAME, i, word, buffer);
			if (!*(nvram_nget("%s_ipaddr", ifname)) || !*(nvram_nget("%s_netmask", ifname)))
				continue;
			unsigned int dhcpnum = atoi(getmdhcp(IDX_LEASEMAX, i, word, buffer2));
			unsigned int dhcpstart = atoi(getmdhcp(IDX_LEASESTART, i, word, buffer2));
			char *ip = nvram_nget("%s_ipaddr", ifname);
			char *netmask = nvram_nget("%s_netmask", ifname);
			char *leasetime = getmdhcp(IDX_LEASETIME, i, word, buffer2);
			makeentry(fp, ifname, dhcpnum, dhcpstart, ip, netmask, leasetime);
			free(word);
		}

		int leasenum = nvram_geti("static_leasenum");

		if (leasenum > 0) {
			char *lease = nvram_safe_get("static_leases");
			char *leasebuf = (char *)malloc(strlen(lease) + 1);
			char *cp = leasebuf;

			strcpy(leasebuf, lease);
			for (i = 0; i < leasenum; i++) {
				char *mac = strsep(&leasebuf, "=");
				char *host = strsep(&leasebuf, "=");
				char *ip = strsep(&leasebuf, "=");
				char *time = strsep(&leasebuf, " ");

				if (mac == NULL || host == NULL || ip == NULL)
					continue;
				fprintf(fp, "dhcp-host=%s,%s,%s,", mac, host, ip);
				char nv[64];
				sprintf(nv, "dnsmasq_lease_%s", ip);
				if (leasechange && nvram_exists(nv)) {
					nvram_unset(nv);
				}
				if (!time || !*time)
					fprintf(fp, "infinite\n");
				else
					fprintf(fp, "%sm\n", time);

#ifdef HAVE_UNBOUND
				if (!nvram_matchi("recursive_dns", 1))
#endif
					addHost(host, ip, 1);
			}
			free(cp);
		}
	}
	fprintf(fp, "bogus-priv\n");
	fprintf(fp, "conf-file=/etc/rfc6761.conf\n");
#ifdef HAVE_DNSSEC
	if (nvram_matchi("dnssec", 1)) {
		fprintf(fp, "conf-file=/etc/trust-anchors.conf\n");
		fprintf(fp, "dnssec\n");
		if (!nvram_matchi("ntp_enable", 1)) {
			fprintf(fp, "dnssec-no-timecheck\n");
		}
		if (nvram_matchi("dnssec_cu", 1)) {
			fprintf(fp, "dnssec-check-unsigned\n");
		} else {
			fprintf(fp, "dnssec-check-unsigned=no\n");
		}
	}
#endif
	if (nvram_matchi("dnssec_proxy", 1)) {
		fprintf(fp, "proxy-dnssec\n");
	}
	if (nvram_matchi("dnsmasq_rc", 1)) {
		fprintf(fp, "dhcp-rapid-commit\n");
	}
	/* stop dns rebinding for private addresses */
	if (nvram_matchi("dnsmasq_no_dns_rebind", 1)) {
		fprintf(fp, "stop-dns-rebind\n");
	}
	if (nvram_matchi("dnsmasq_add_mac", 1)) {
		fprintf(fp, "add-mac\n");
	}
#ifdef HAVE_PRIVOXY
	if (nvram_matchi("privoxy_enable", 1)) {
		if (nvram_matchi("privoxy_transp_enable", 1)) {
			fprintf(fp, "dhcp-option=252,http://config.privoxy.org/wpad.dat\n");
		} else {
			fprintf(fp, "dhcp-option=252,http://%s/wpad.dat\n", nvram_safe_get("lan_ipaddr"));
		}
	} else {
		fprintf(fp, "dhcp-option=252,\"\\n\"\n");
	}
#else
	fprintf(fp, "dhcp-option=252,\"\\n\"\n");
#endif
	char *addoptions = nvram_safe_get("dnsmasq_options");
#ifdef HAVE_SMARTDNS
	if (nvram_matchi("smartdns", 1))
		fprintf(fp, "cache-size=0\n");
	else
#endif
	if (!strstr(addoptions, "cache-size="))
		fprintf(fp, "cache-size=%d\n", nvram_default_geti("dnsmasq_cachesize", 1500));
	/*
	 * Additional options 
	 */
	fwritenvram("dnsmasq_options", fp);
	fclose(fp);

	dns_to_resolv();

	chmod("/etc/lease_update.sh", 0700);

	FILE *conf = NULL;
	conf = fopen("/jffs/etc/dnsmasq.conf", "r");	//test if custom config is available

	if (conf != NULL) {
		eval("dnsmasq", "-u", "root", "-g", "root", "--conf-file=/jffs/etc/dnsmasq.conf");
		fclose(conf);
	} else {
		eval("dnsmasq", "-u", "root", "-g", "root", "--conf-file=/tmp/dnsmasq.conf");
	}

	dd_loginfo("dnsmasq", "daemon successfully started\n");

	cprintf("done\n");
	return;
}

void stop_dnsmasq(void)
{
#ifdef HAVE_SMARTDNS
	stop_smartdns();
#endif
	if (stop_process("dnsmasq", "daemon")) {
		unlink("/tmp/resolv.dnsmasq");
	}
}
#endif

#ifdef TEST
int main(int argc, char *argv[])
{
	start_dnsmasq();

}
#endif
