/*
 * dhcp.c
 *
 * Copyright (C) 2005 - 2018 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
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
#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
#include <ctype.h>

#include <broadcom.h>

#define DHCP_MAX_COUNT 254
#define EXPIRES_NEVER 0xFFFFFFFF /* static lease */

static int landhcp(void)
{
	if (!getWET())
		if (nvram_match("lan_proto", "dhcp") &&
		    nvram_matchi("dhcpfwd_enable", 0))
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

EJ_VISIBLE void ej_dhcpenabled(webs_t wp, int argc, char_t **argv)
{
	if (landhcp() || hasmdhcp())
		websWrite(wp, argv[0]);
	else
		websWrite(wp, argv[1]);
}

static char *dhcp_reltime(char *buf, size_t len, time_t t, int sub)
{
	if (sub) {
		time_t now = time(NULL);
		t -= now;
	}
	if (t < 0)
		t = 0;
	int days = t / 86400;
	int min = t / 60;

	snprintf(buf, len, "%d day%s %02d:%02d:%02d", days,
		 ((days == 1) ? "" : "s"), ((min / 60) % 24), (min % 60),
		 (int)(t % 60));
	return buf;
}

/*
 * dump in array: hostname,mac,ip,expires read leases from leasefile as:
 * expires mac ip hostname 
 */
EJ_VISIBLE void ej_dumpleases(webs_t wp, int argc, char_t **argv)
{
	FILE *fp;
	unsigned long expires;
	int i;
	int count = 0;
	int macmask;
	char ip[32];
	char hostname[256];
	char clid[256];
	int ifidx = -1;

	char buf[512];
	char *p;
	char *buff;
	struct lease_t lease;
	struct in_addr addr;
	char *ipaddr, mac[32] = "", expires_time[256] = "";

	if (argc < 1) {
		return;
	}
	macmask = atoi(argv[0]);
	if (landhcp() || hasmdhcp()) {
		if (nvram_invmatchi("dhcpd_usenvram", 1)) {
			/*
			 * Parse leases file 
			 */
			if (nvram_matchi("dhcpd_usejffs", 1)) {
				fp = fopen("/jffs/dnsmasq.leases", "r");
				if (!fp) {
					fp = fopen("/tmp/dnsmasq.leases", "r");
				}
				if (!fp) {
					return;
				}
			} else {
				fp = fopen("/tmp/dnsmasq.leases", "r");
				if (!fp) {
					fp = fopen("/jffs/dnsmasq.leases", "r");
				}
			}
			if (!fp) {
				return;
			}

			if (fp) {
				while (fgets(buf, sizeof(buf), fp)) {
					ifidx = -1;
					if (sscanf(buf,
						   "%lu %17s %15s %255s %255s %d",
						   &expires, mac, ip, hostname,
						   clid, &ifidx) < 4)
						continue;
					p = mac;
					while ((*p = toupper(*p)) != 0)
						++p;
					if ((p = strrchr(ip, '.')) == NULL)
						continue;
					if (nvram_matchi("maskmac", 1) &&
					    macmask) {
						mac[0] = 'x';
						mac[1] = 'x';
						mac[3] = 'x';
						mac[4] = 'x';
						mac[6] = 'x';
						mac[7] = 'x';
						mac[9] = 'x';
						mac[10] = 'x';
					}
					char ifname[32] = { 0 };
					if (ifidx != -1) {
						getIfByIdx(ifname, ifidx);
					}
					websWrite(
						wp,
						"%c'%s','%s','%s','%s','%s','%s','%s'",
						(count ? ',' : ' '),
						(hostname[0] ?
							 hostname :
							 live_translate(
								 wp,
								 "share.unknown")),
						ip, mac,
						((expires == 0) ?
							 live_translate(
								 wp,
								 "share.sttic") :
							 dhcp_reltime(
								 buf,
								 sizeof(buf),
								 expires, 1)),
						p + 1, ifname,
						nvram_nget("%s_label", ifname));
					++count;
				}
				fclose(fp);
			}
		} else {
			for (i = 0; i < DHCP_MAX_COUNT; ++i) {
				sprintf(buf, "dnsmasq_lease_%d.%d.%d.%d",
					get_single_ip(
						nvram_safe_get("lan_ipaddr"),
						0),
					get_single_ip(
						nvram_safe_get("lan_ipaddr"),
						1),
					get_single_ip(
						nvram_safe_get("lan_ipaddr"),
						2),
					i);

				buff = nvram_safe_get(buf);
				if (sscanf(buff, "%lu %17s %15s %255s",
					   &expires, mac, ip, hostname) < 4)
					continue;
				p = mac;
				while ((*p = toupper(*p)) != 0)
					++p;
				if ((p = strrchr(ip, '.')) == NULL)
					continue;
				if (nvram_matchi("maskmac", 1) && macmask) {
					mac[0] = 'x';
					mac[1] = 'x';
					mac[3] = 'x';
					mac[4] = 'x';
					mac[6] = 'x';
					mac[7] = 'x';
					mac[9] = 'x';
					mac[10] = 'x';
				}
				websWrite(
					wp,
					"%c'%s','%s','%s','%s','%s','N/A',''",
					(count ? ',' : ' '),
					(hostname[0] ?
						 hostname :
						 live_translate(
							 wp, "share.unknown")),
					ip, mac,
					((expires == 0) ?
						 live_translate(wp,
								"share.sttic") :
						 dhcp_reltime(buf, sizeof(buf),
							      expires, 1)),
					p + 1);
				++count;
			}
		}
	}
	return;
}

EJ_VISIBLE void ej_dhcp_remaining_time(webs_t wp, int argc, char_t **argv)
{
	// tofu12
	if (nvram_invmatch("wan_proto", "dhcp") &&
	    nvram_invmatch("wan_proto", "dhcp_auth"))
		return;

	long exp;
	char buf[128];
	struct sysinfo si;
	long n;

	exp = 0;
	if (file_to_buf("/tmp/udhcpc.expires", buf, sizeof(buf))) {
		n = atol(buf);
		if (n > 0) {
			sysinfo(&si);
			exp = n - si.uptime;
		}
	}
	websWrite(wp, dhcp_reltime(buf, sizeof(buf), exp, 0));

	return;
}
