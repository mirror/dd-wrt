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

extern int usejffs;

extern void addHost(char *host, char *ip);

void stop_dnsmasq(void);

char *getmdhcp(int count, int index)
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
		if (count == 0)
			return interface;
		if (count == 1)
			return dhcpon;
		if (count == 2)
			return start;
		if (count == 3)
			return max;
		if (count == 4)
			return leasetime;
	}
	return "";
}

int landhcp(void)
{
	if (!getWET())
		if (nvram_match("dhcp_dnsmasq", "1")
		    && nvram_match("lan_proto", "dhcp")
		    && nvram_match("dhcpfwd_enable", "0"))
			return 1;
	return 0;
}

int hasdhcp(void)
{
	int count = 0;
	int ret = landhcp();

	return ret;
	// for now, keep it disabled
/*    if( nvram_get( "mdhcpd_count" ) != NULL )
	count = atoi( nvram_safe_get( "mdhcpd_count" ) );
    ret |= count;
    return ret > 0 ? 1 : 0;*/
}

int canlan(void)
{
	if (nvram_match("dhcpfwd_enable", "0"))
		return 1;
	return 0;
}

void start_dnsmasq(void)
{
	FILE *fp;
	struct dns_lists *dns_list = NULL;
	int ret;
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
	if (nvram_match("fon_enable", "1")
	    || (nvram_match("chilli_nowifibridge", "1")
		&& nvram_match("chilli_enable", "1"))) {
		if (canlan())
			fprintf(fp, "interface=%s,br0",
				nvram_safe_get("wl0_ifname"));
		else
			fprintf(fp, "interface=%s",
				nvram_safe_get("wl0_ifname"));
	} else {
		if (nvram_match("chilli_enable", "1")) {
			if (canlan())
				fprintf(fp, "interface=%s",
					nvram_safe_get("wl0_ifname"));
			else
				fprintf(fp, "interface=%s,",
					nvram_safe_get("wl0_ifname"));
		} else if (nvram_match("pptpd_enable", "1")) {
			if (canlan())
				fprintf(fp, "listen-address=%s,%s", "127.0.0.1",
					nvram_safe_get("lan_ipaddr"));
			else
				fprintf(fp, "listen-address=%s", "127.0.0.1");
		} else {
			if (canlan())
				fprintf(fp, "interface=%s",
					nvram_safe_get("lan_ifname"));
			else
				fprintf(fp, "interface=");
		}
	}
	int mdhcpcount = 0;

	if (nvram_get("mdhcpd_count") != NULL) {
		mdhcpcount = atoi(nvram_safe_get("mdhcpd_count"));
		for (i = 0; i < mdhcpcount; i++) {
			if (strlen(nvram_nget("%s_ipaddr", getmdhcp(0, i))) == 0
			    || strlen(nvram_nget("%s_netmask", getmdhcp(0, i)))
			    == 0)
				continue;
			if (canlan() || i > 0) {
				if (nvram_match("pptpd_enable", "1"))
					fprintf(fp, ",%s",
						nvram_nget("%s_ipaddr",
							   getmdhcp(0, i)));
				else
					fprintf(fp, ",%s", getmdhcp(0, i));
			} else {
				if (nvram_match("pptpd_enable", "1"))
					fprintf(fp, "%s",
						nvram_nget("%s_ipaddr",
							   getmdhcp(0, i)));
				else
					fprintf(fp, "%s", getmdhcp(0, i));

			}
		}
	}
	fprintf(fp, "\n");

	fprintf(fp, "resolv-file=/tmp/resolv.dnsmasq\n");

	/*
	 * Domain 
	 */
	if (nvram_match("dhcp_domain", "wan")) {
		if (nvram_invmatch("wan_domain", ""))
			fprintf(fp, "domain=%s\n",
				nvram_safe_get("wan_domain"));
		else if (nvram_invmatch("wan_get_domain", ""))
			fprintf(fp, "domain=%s\n",
				nvram_safe_get("wan_get_domain"));
	} else {
		if (nvram_invmatch("lan_domain", ""))
			fprintf(fp, "domain=%s\n",
				nvram_safe_get("lan_domain"));
	}

	/*
	 * DD-WRT use dnsmasq as DHCP replacement 
	 */

	//bs mod
	if (hasdhcp()) {
		/*
		 * DHCP leasefile 
		 */
		if (nvram_match("dhcpd_usenvram", "1")) {
			fprintf(fp, "leasefile-ro\n");
			fprintf(fp, "dhcp-script=%s\n", "/etc/lease_update.sh");
		} else {
			if (usejffs)
				fprintf(fp,
					"dhcp-leasefile=/jffs/dnsmasq.leases\n");
			else
				fprintf(fp,
					"dhcp-leasefile=/tmp/dnsmasq.leases\n");
		}

		int dhcp_max = 0;

		if (landhcp())
			dhcp_max +=
			    atoi(nvram_safe_get("dhcp_num")) +
			    atoi(nvram_safe_get("static_leasenum"));
		for (i = 0; i < mdhcpcount; i++) {
			if (strlen(nvram_nget("%s_ipaddr", getmdhcp(0, i))) == 0
			    || strlen(nvram_nget("%s_netmask", getmdhcp(0, i)))
			    == 0)
				continue;
			dhcp_max += atoi(getmdhcp(3, i));
		}
		fprintf(fp, "dhcp-lease-max=%d\n", dhcp_max);
		if (landhcp())
			fprintf(fp, "dhcp-option=lan,3,%s\n",
				nvram_safe_get("lan_ipaddr"));
		for (i = 0; i < mdhcpcount; i++) {
			if (strlen(nvram_nget("%s_ipaddr", getmdhcp(0, i))) == 0
			    || strlen(nvram_nget("%s_netmask", getmdhcp(0, i)))
			    == 0)
				continue;
			fprintf(fp, "dhcp-option=%s,3,", getmdhcp(0, i));
			fprintf(fp, "%s\n",
				nvram_nget("%s_ipaddr", getmdhcp(0, i)));
		}
		if (nvram_invmatch("wan_wins", "")
		    && nvram_invmatch("wan_wins", "0.0.0.0"))
			fprintf(fp, "dhcp-option=44,%s\n",
				nvram_safe_get("wan_wins"));

		if (nvram_match("dns_dnsmasq", "0")) {
			dns_list = get_dns_list();

			if (dns_list
			    && (strlen(dns_list->dns_server[0]) > 0
				|| strlen(dns_list->dns_server[1]) > 0
				|| strlen(dns_list->dns_server[2]) > 0)) {

				fprintf(fp, "dhcp-option=6");

				if (strlen(dns_list->dns_server[0]) > 0)
					fprintf(fp, ",%s",
						dns_list->dns_server[0]);

				if (strlen(dns_list->dns_server[1]) > 0)
					fprintf(fp, ",%s",
						dns_list->dns_server[1]);

				if (strlen(dns_list->dns_server[2]) > 0)
					fprintf(fp, ",%s",
						dns_list->dns_server[2]);

				fprintf(fp, "\n");
			}

			if (dns_list)
				free(dns_list);
		}

		if (nvram_match("auth_dnsmasq", "1"))
			fprintf(fp, "dhcp-authoritative\n");
		if (landhcp()) {
			fprintf(fp, "dhcp-range=lan,");
			fprintf(fp, "%d.%d.%d.%s,",
				get_single_ip(nvram_safe_get("lan_ipaddr"), 0),
				get_single_ip(nvram_safe_get("lan_ipaddr"), 1),
				get_single_ip(nvram_safe_get("lan_ipaddr"), 2),
				nvram_safe_get("dhcp_start"));
			if (nvram_match("dhcp_num", "0")) {
				fprintf(fp, "static,");
			} else {
				fprintf(fp, "%d.%d.%d.%d,",
					get_single_ip(nvram_safe_get
						      ("lan_ipaddr"), 0),
					get_single_ip(nvram_safe_get
						      ("lan_ipaddr"), 1),
					get_single_ip(nvram_safe_get
						      ("lan_ipaddr"), 2),
					atoi(nvram_safe_get("dhcp_start")) +
					atoi(nvram_safe_get("dhcp_num")) - 1);
			}
			fprintf(fp, "%s,", nvram_safe_get("lan_netmask"));
			fprintf(fp, "%sm\n", nvram_safe_get("dhcp_lease"));
		}

		for (i = 0; i < mdhcpcount; i++) {
			if (strcmp(getmdhcp(1, i), "On"))
				continue;
			if (strlen(nvram_nget("%s_ipaddr", getmdhcp(0, i))) == 0
			    || strlen(nvram_nget("%s_netmask", getmdhcp(0, i)))
			    == 0)
				continue;
			fprintf(fp, "dhcp-range=%s,", getmdhcp(0, i));
			fprintf(fp, "%d.%d.%d.",
				get_single_ip(nvram_nget
					      ("%s_ipaddr", getmdhcp(0, i)),
					      0),
				get_single_ip(nvram_nget
					      ("%s_ipaddr", getmdhcp(0, i)),
					      1),
				get_single_ip(nvram_nget
					      ("%s_ipaddr", getmdhcp(0, i)),
					      2));
			fprintf(fp, "%s,", getmdhcp(2, i));
			fprintf(fp, "%d.%d.%d.",
				get_single_ip(nvram_nget
					      ("%s_ipaddr", getmdhcp(0, i)),
					      0),
				get_single_ip(nvram_nget
					      ("%s_ipaddr", getmdhcp(0, i)),
					      1),
				get_single_ip(nvram_nget
					      ("%s_ipaddr", getmdhcp(0, i)),
					      2));
			int end = atoi(getmdhcp(2, i));

			end += atoi(getmdhcp(3, i));
			fprintf(fp, "%d,", end);
			fprintf(fp, "%s,",
				nvram_nget("%s_netmask", getmdhcp(0, i)));
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
				char *ip = strsep(&leasebuf, " ");

				if (mac == NULL || host == NULL || ip == NULL)
					continue;

				fprintf(fp, "dhcp-host=%s,%s,%s,infinite\n",
					mac, host, ip);
				addHost(host, ip);
			}
			free(cp);
		}
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
	ret = eval("dnsmasq", "--conf-file=/tmp/dnsmasq.conf");
	dd_syslog(LOG_INFO, "dnsmasq : dnsmasq daemon successfully started\n");

	cprintf("done\n");
	return;
}

void stop_dnsmasq(void)
{

	if (pidof("dnsmasq") > 0) {
		syslog(LOG_INFO,
		       "dnsmasq : dnsmasq daemon successfully stopped\n");
		softkill("dnsmasq");
		unlink("/tmp/resolv.dnsmasq");

		cprintf("done\n");
	}
	return;
}
#endif
