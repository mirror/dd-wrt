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

static char *getmdhcp(int count, int index)
{
	int cnt = 0;
	static char word[256];
	char *next, *wordlist;

	wordlist = nvram_safe_get("mdhcpd");
	foreach(word, wordlist, next) {
		if (cnt < index) {
			cnt++;
			continue;
		}
		char *interface = word;
		char *dhcpon = interface;

		interface = strsep(&dhcpon, ">");
		char *start = dhcpon;

		dhcpon = strsep(&start, ">");
		char *max = start;

		start = strsep(&max, ">");
		char *leasetime = max;

		max = strsep(&leasetime, ">");
		if (max == NULL) {
			max = leasetime;
			leasetime = "3660";
		}
		switch (count) {
		case 0:
			return interface;
		case 1:
			return dhcpon;
		case 2:
			return start;
		case 3:
			return max;
		case 4:
			return leasetime;
		}
	}
	return "";
}

static int landhcp(void)
{
	if (!getWET())
		if (nvram_match("dhcp_dnsmasq", "1")
		    && nvram_match("lan_proto", "dhcp")
		    && nvram_match("dhcpfwd_enable", "0"))
			return 1;
	return 0;
}

static int hasmdhcp(void)
{
	if (nvram_get("mdhcpd_count") != NULL) {
		int mdhcpcount = atoi(nvram_safe_get("mdhcpd_count"));
		return mdhcpcount > 0 ? 1 : 0;
	}
	return 0;
}

static int canlan(void)
{
	if (nvram_match("dhcpfwd_enable", "0"))
		return 1;
	return 0;
}

void start_dnsmasq(void)
{
	FILE *fp;
	struct dns_lists *dns_list = NULL;
	int i;

	if (nvram_match("dhcp_dnsmasq", "1")
	    && nvram_match("lan_proto", "dhcp")
	    && nvram_match("dnsmasq_enable", "0")) {
		nvram_set("dnsmasq_enable", "1");
		nvram_commit();
	}

	if (!nvram_invmatch("dnsmasq_enable", "0")) {
		stop_dnsmasq();
		return;
	}

	usejffs = 0;

	if (nvram_match("dhcpd_usejffs", "1")) {
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
	if (nvram_match("chilli_enable", "1")) {
		if (canlan())
			fprintf(fp, "interface=%s", get_wdev());
		else
			fprintf(fp, "interface=%s,", get_wdev());
	} else if (nvram_match("pptpd_enable", "1")) {
		if (canlan())
			fprintf(fp, "listen-address=%s,%s", "127.0.0.1", nvram_safe_get("lan_ipaddr"));
		else
			fprintf(fp, "listen-address=%s", "127.0.0.1");
	} else {
		if (canlan())
			fprintf(fp, "interface=%s", nvram_safe_get("lan_ifname"));
		else
			fprintf(fp, "interface=");
	}
	int mdhcpcount = 0;

	if (nvram_get("mdhcpd_count") != NULL) {
		mdhcpcount = atoi(nvram_safe_get("mdhcpd_count"));
		for (i = 0; i < mdhcpcount; i++) {
			if (strlen(nvram_nget("%s_ipaddr", getmdhcp(0, i))) == 0 || strlen(nvram_nget("%s_netmask", getmdhcp(0, i)))
			    == 0)
				continue;
			if (canlan() || i > 0) {
				if (nvram_match("pptpd_enable", "1"))
					fprintf(fp, ",%s", nvram_nget("%s_ipaddr", getmdhcp(0, i)));
				else
					fprintf(fp, ",%s", getmdhcp(0, i));
			} else {
				if (nvram_match("pptpd_enable", "1"))
					fprintf(fp, "%s", nvram_nget("%s_ipaddr", getmdhcp(0, i)));
				else
					fprintf(fp, "%s", getmdhcp(0, i));

			}
		}
	}
	fprintf(fp, "\n");

	fprintf(fp, "resolv-file=/tmp/resolv.dnsmasq\n" "all-servers\n");
	if (nvram_match("dnsmasq_strict", "1"))
		fprintf(fp, "strict-order\n");

#ifdef HAVE_UNBOUND
	if (nvram_match("recursive_dns", "1")) {
		fprintf(fp, "port=0\n");
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
		if (nvram_match("dhcpd_usenvram", "1")) {
			fprintf(fp, "leasefile-ro\n");
			fprintf(fp, "dhcp-script=%s\n", "/etc/lease_update.sh");
		} else {
			if (usejffs)
				fprintf(fp, "dhcp-leasefile=/jffs/dnsmasq.leases\n");
			else
				fprintf(fp, "dhcp-leasefile=/tmp/dnsmasq.leases\n");
		}

		int dhcp_max = 0;

		if (landhcp())
			dhcp_max += atoi(nvram_safe_get("dhcp_num")) + atoi(nvram_safe_get("static_leasenum"));
		for (i = 0; i < mdhcpcount; i++) {
			if (strlen(nvram_nget("%s_ipaddr", getmdhcp(0, i))) == 0 || strlen(nvram_nget("%s_netmask", getmdhcp(0, i)))
			    == 0)
				continue;
			dhcp_max += atoi(getmdhcp(3, i));
		}
		fprintf(fp, "dhcp-lease-max=%d\n", dhcp_max);
		if (landhcp())
			fprintf(fp, "dhcp-option=%s,3,%s\n", nvram_safe_get("lan_ifname"), nvram_safe_get("lan_ipaddr"));
		for (i = 0; i < mdhcpcount; i++) {
			if (strlen(nvram_nget("%s_ipaddr", getmdhcp(0, i))) == 0 || strlen(nvram_nget("%s_netmask", getmdhcp(0, i)))
			    == 0)
				continue;
			fprintf(fp, "dhcp-option=%s,3,", getmdhcp(0, i));
			fprintf(fp, "%s\n", nvram_nget("%s_ipaddr", getmdhcp(0, i)));
		}
		if (nvram_invmatch("wan_wins", "")
		    && nvram_invmatch("wan_wins", "0.0.0.0"))
			fprintf(fp, "dhcp-option=44,%s\n", nvram_safe_get("wan_wins"));

		if (nvram_match("dns_dnsmasq", "0")) {
			dns_list = get_dns_list();

			if (dns_list && (strlen(dns_list->dns_server[0]) > 0 || strlen(dns_list->dns_server[1]) > 0 || strlen(dns_list->dns_server[2]) > 0)) {

				fprintf(fp, "dhcp-option=6");

				if (strlen(dns_list->dns_server[0]) > 0)
					fprintf(fp, ",%s", dns_list->dns_server[0]);

				if (strlen(dns_list->dns_server[1]) > 0)
					fprintf(fp, ",%s", dns_list->dns_server[1]);

				if (strlen(dns_list->dns_server[2]) > 0)
					fprintf(fp, ",%s", dns_list->dns_server[2]);

				fprintf(fp, "\n");
			}

			if (dns_list)
				free(dns_list);
		} else {
#ifdef HAVE_UNBOUND
			if (nvram_match("recursive_dns", "1")) {
				fprintf(fp, "dhcp-option=%s,6,%s\n", nvram_safe_get("lan_ifname"), nvram_safe_get("lan_ipaddr"));
				for (i = 0; i < mdhcpcount; i++) {
					if (strlen(nvram_nget("%s_ipaddr", getmdhcp(0, i))) == 0 || strlen(nvram_nget("%s_netmask", getmdhcp(0, i)))
					    == 0)
						continue;
					fprintf(fp, "dhcp-option=%s,6,", getmdhcp(0, i));
					fprintf(fp, "%s\n", nvram_nget("%s_ipaddr", getmdhcp(0, i)));
				}
			}
#endif
		}

		if (nvram_match("auth_dnsmasq", "1"))
			fprintf(fp, "dhcp-authoritative\n");
		if (landhcp()) {
			unsigned int dhcpnum = atoi(nvram_safe_get("dhcp_num"));
			unsigned int dhcpstart = atoi(nvram_safe_get("dhcp_start"));
			unsigned int ip1 = get_single_ip(nvram_safe_get("lan_ipaddr"), 0);
			unsigned int ip2 = get_single_ip(nvram_safe_get("lan_ipaddr"), 1);
			unsigned int ip3 = get_single_ip(nvram_safe_get("lan_ipaddr"), 2);
			unsigned int ip4 = get_single_ip(nvram_safe_get("lan_ipaddr"), 4);
			unsigned int im1 = get_single_ip(nvram_safe_get("lan_netmask"), 0);
			unsigned int im2 = get_single_ip(nvram_safe_get("lan_netmask"), 1);
			unsigned int im3 = get_single_ip(nvram_safe_get("lan_netmask"), 2);
			unsigned int im4 = get_single_ip(nvram_safe_get("lan_netmask"), 4);
			unsigned int sip = ((ip1 & im1) << 24) + ((ip2 & im2) << 16) + ((ip3 & im3) << 8) + dhcpstart;
			unsigned int eip = sip + dhcpnum - 1;
			// Do new code - multi-subnet range
			// Assumes that lan_netmask is set accordingly
			// Assumes that dhcp_num is a multiple of 256
			fprintf(fp, "dhcp-range=%s,", nvram_safe_get("lan_ifname"));
			fprintf(fp, "%d.%d.%d.%d,", ip1 & im1, ip2 & im2, ip3 & im3, dhcpstart);
			fprintf(fp, "%d.%d.%d.%d,", (eip >> 24) & 0xff, (eip >> 16) & 0xff, (eip >> 8) & 0xff, eip & 0xff);
			fprintf(fp, "%s,", nvram_safe_get("lan_netmask"));
			fprintf(fp, "%sm\n", nvram_safe_get("dhcp_lease"));
		}

		for (i = 0; i < mdhcpcount; i++) {
			if (strcmp(getmdhcp(1, i), "On"))
				continue;
			if (strlen(nvram_nget("%s_ipaddr", getmdhcp(0, i))) == 0 || strlen(nvram_nget("%s_netmask", getmdhcp(0, i)))
			    == 0)
				continue;
			char *ifname = getmdhcp(0, i);
			unsigned int dhcpstart = atoi(getmdhcp(2, i));
			unsigned int dhcpnum = atoi(getmdhcp(3, i));
			unsigned int ip1 = get_single_ip(nvram_nget("%s_ipaddr", ifname), 0);
			unsigned int ip2 = get_single_ip(nvram_nget("%s_ipaddr", ifname), 1);
			unsigned int ip3 = get_single_ip(nvram_nget("%s_ipaddr", ifname), 2);
			unsigned int ip4 = get_single_ip(nvram_nget("%s_ipaddr", ifname), 4);
			unsigned int im1 = get_single_ip(nvram_nget("%s_netmask", ifname), 0);
			unsigned int im2 = get_single_ip(nvram_nget("%s_netmask", ifname), 1);
			unsigned int im3 = get_single_ip(nvram_nget("%s_netmask", ifname), 2);
			unsigned int im4 = get_single_ip(nvram_nget("%s_netmask", ifname), 4);
			unsigned int sip = ((ip1 & im1) << 24) + ((ip2 & im2) << 16) + ((ip3 & im3) << 8) + dhcpstart;
			unsigned int eip = sip + dhcpnum - 1;

			fprintf(fp, "dhcp-range=%s,", ifname);

			fprintf(fp, "%d.%d.%d.%d,", ip1 & im1, ip2 & im2, ip3 & im3, dhcpstart);
			fprintf(fp, "%d.%d.%d.%d,", (eip >> 24) & 0xff, (eip >> 16) & 0xff, (eip >> 8) & 0xff, eip & 0xff);
			fprintf(fp, "%s,", nvram_nget("%s_netmask", ifname));
			fprintf(fp, "%sm\n", getmdhcp(4, i));
		}

		int leasenum = atoi(nvram_safe_get("static_leasenum"));

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
				if (!time || strlen(time) == 0)
					fprintf(fp, "dhcp-host=%s,%s,%s,infinite\n", mac, host, ip);
				else
					fprintf(fp, "dhcp-host=%s,%s,%s,%sm\n", mac, host, ip, time);

#ifdef HAVE_UNBOUND
				if (!nvram_match("recursive_dns", "1"))
#endif
					addHost(host, ip, 1);
			}
			free(cp);
		}
	}
	/* stop dns rebinding for private addresses */
	if (nvram_match("dnsmasq_no_dns_rebind", "1")) {
		fprintf(fp, "stop-dns-rebind\n");
	}
	if (nvram_match("dnsmasq_add_mac", "1")) {
		fprintf(fp, "add-mac\n");
	}
	/*
	 * Additional options 
	 */
	if (nvram_invmatch("dnsmasq_options", "")) {
		fwritenvram("dnsmasq_options", fp);
	}
	fclose(fp);

	dns_to_resolv();

	chmod("/etc/lease_update.sh", 0700);
	eval("dnsmasq", "-u", "root", "-g", "root", "--conf-file=/tmp/dnsmasq.conf", "--cache-size=1500");
	dd_syslog(LOG_INFO, "dnsmasq : dnsmasq daemon successfully started\n");

	cprintf("done\n");
	return;
}

void stop_dnsmasq(void)
{
	if (stop_process("dnsmasq", "dnsmasq daemon")) {
		unlink("/tmp/resolv.dnsmasq");
	}
}
#endif
