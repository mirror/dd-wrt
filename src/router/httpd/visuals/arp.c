/*
 * arp.c
 *
 * Copyright (C) 2005 - 2022 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#if defined(HAVE_MICRO) // || defined(HAVE_80211AC)

EJ_VISIBLE void ej_dumparptable(webs_t wp, int argc, char_t **argv)
{
	FILE *f;
	FILE *host;
	FILE *conn;
	char buf[256];
	char hostname[128];
	char ip[16];
	char ip2[20];
	char fullip[18];
	char mac[18];
	char landev[16];
	int count = 0;
	int i, len;
	int conn_count = 0;

	if ((f = fopen("/proc/net/arp", "r")) != NULL) {
		while (fgets(buf, sizeof(buf), f)) {
			if (sscanf(buf, "%15s %*s %*s %17s %*s %s", ip, mac,
				   landev) != 3)
				continue;
			if ((strlen(mac) != 17) ||
			    (strcmp(mac, "00:00:00:00:00:00") == 0))
				continue;
			//                      if (strcmp(landev, nvram_safe_get("wan_iface")) == 0)
			//                              continue;       // skip all but LAN arp entries
			strcpy(hostname, "*"); // set name to *

			/*
			 * count open connections per IP
			 */
			if ((conn = fopen("/proc/net/ip_conntrack", "r")) ||
			    (conn = fopen("/proc/net/nf_conntrack", "r"))) {
				strcpy(ip2, ip);
				strcat(ip2, " ");

				while (fgets(buf, sizeof(buf), conn)) {
					if (strstr(buf, ip2))
						conn_count++;
				}
				fclose(conn);
			}

			/*
			 * look into hosts file for hostnames (static leases)
			 */
			if ((host = fopen("/tmp/hosts", "r")) != NULL &&
			    !strcmp(hostname, "*")) {
				while (fgets(buf, sizeof(buf), host)) {
					sscanf(buf, "%15s %*s", fullip);

					if (!strcmp(ip, fullip)) {
						sscanf(buf, "%*15s %s",
						       hostname);
					}
				}
				fclose(host);
			}
			/*
			 * end hosts file lookup
			 */

			/*
			 * check for dnsmasq leases in /tmp/dnsmasq.leases and /jffs/ if
			 * hostname is still unknown
			 */

			if (!strcmp(hostname, "*") &&
			    nvram_matchi("dhcpd_usenvram", 0)) {
				if (!(host = fopen("/tmp/dnsmasq.leases", "r")))
					host = fopen("/jffs/dnsmasq.leases",
						     "r");

				if (host) {
					while (fgets(buf, sizeof(buf), host)) {
						sscanf(buf, "%*s %*s %15s %*s",
						       fullip);

						if (strcmp(ip, fullip) == 0) {
							sscanf(buf,
							       "%*s %*s %*s %s",
							       hostname);
						}
					}
					fclose(host);
				}
			}

			if (!strcmp(hostname, "*") &&
			    nvram_matchi("dhcpd_usenvram", 1)) {
				sscanf(nvram_nget("dnsmasq_lease_%s", ip),
				       "%*s %*s %*s %s", hostname);
			}
			/*
			 * end nvram check
			 */
			len = strlen(mac);
			for (i = 0; i < len; i++)
				mac[i] = toupper(mac[i]);
			websWrite(wp, "%c'%s','%s','%s','%d', '%s','0','0','0'",
				  (count ? ',' : ' '), hostname, ip, mac,
				  conn_count, landev);
			++count;
			conn_count = 0;
		}
		fclose(f);
	}
}
#else
struct arptable {
	char *hostname;
	char *mac;
	char *ip;
	int proto;
	unsigned int v4;
	struct in6_addr v6;
	char *ifname;
	int conncount;
	long long in;
	long long out;
	long long total;
	int mark;
};

static int addtable(struct arptable **tbl, char *mac, char *ip, char *ifname,
		    long long in, long long out, long long total, int *tablelen)
{
	struct arptable *table = *tbl;
	int len = *tablelen;
	int i;
	for (i = 0; i < len; i++) {
		if (!strcmp(mac, table[i].mac) &&
		    !strcmp(ifname, table[i].ifname)) {
			table[i].in += in;
			table[i].out += out;
			table[i].total += total;
			char *oldip = table[i].ip;
			asprintf(&table[i].ip, "%s<br>%s", oldip, ip);
			debug_free(oldip);
			if (strlen(ip) < 16) {
				table[i].proto |= 1;
				inet_aton(ip, (struct in_addr *)&table[i].v4);
			} else {
				inet_pton(AF_INET6, ip, &table[i].v6);
				table[i].proto |= 2;
			}
			return len;
		}
	}
	table = realloc(table, sizeof(*table) * (len + 1));
	table[i].mac = strdup(mac);
	table[i].ip = strdup(ip);
	table[i].hostname = NULL;
	table[i].ifname = strdup(ifname);
	table[i].in = in;
	table[i].out = out;
	table[i].total = total;
	table[i].conncount = 0;
	table[i].mark = 0;
	if (strlen(ip) < 16) {
		table[i].proto = 1;
		inet_aton(ip, (struct in_addr *)&table[i].v4);
	} else {
		inet_pton(AF_INET6, ip, &table[i].v6);
		table[i].proto = 2;
	}

	len++;
	*tablelen = len;
	*tbl = table;
	return len;
}

static void readhosts(struct arptable *tbl, int tablelen)
{
	FILE *host;

	char fullip[18];
	int i;
	char buf[256];
	char r_hostname[128];
	/*
	 * look into hosts file for hostnames (static leases)
	 */
	if ((host = fopen("/tmp/hosts", "r")) != NULL) {
		while (fgets(buf, sizeof(buf), host)) {
			sscanf(buf, "%15s %s", fullip, r_hostname);
			unsigned int v4;
			inet_aton(fullip, (struct in_addr *)&v4);
			for (i = 0; i < tablelen; i++) {
				if (tbl[i].v4 == v4) {
					tbl[i].hostname = strdup(r_hostname);
					break;
				}
			}
		}
		fclose(host);
	}
	if (nvram_matchi("dhcpd_usenvram", 0)) {
		if (!(host = fopen("/tmp/dnsmasq.leases", "r")))
			host = fopen("/jffs/dnsmasq.leases", "r");

		if (host) {
			while (fgets(buf, sizeof(buf), host)) {
				sscanf(buf, "%*s %*s %15s %s", fullip,
				       r_hostname);
				unsigned int v4;
				inet_aton(fullip, (struct in_addr *)&v4);
				for (i = 0; i < tablelen; i++) {
					if (!tbl[i].hostname) {
						if (tbl[i].v4 == v4) {
							tbl[i].hostname = strdup(
								r_hostname);
							break;
						}
					}
				}
			}
			fclose(host);
		}
	}
	if (nvram_matchi("dhcpd_usenvram", 1)) {
		for (i = 0; i < tablelen; i++) {
			if (!tbl[i].hostname) {
				char *nv = nvram_nget("dnsmasq_lease_%s",
						      tbl[i].ip);
				if (*nv) {
					sscanf(nv, "%*s %*s %*s %s",
					       r_hostname);
					tbl[i].hostname = strdup(r_hostname);
				}
			}
		}
	}
	for (i = 0; i < tablelen; i++) {
		if (!tbl[i].hostname)
			tbl[i].hostname = strdup("*");
	}
}

static void filterarp(struct arptable *table, int tablelen)
{
	int i;
	FILE *arp;
	char buf[256];
	char ip[64];
	char mac[18];
	char landev[16];
	for (i = 0; i < tablelen; i++) {
		table[i].mark = 1;
	}

	if ((arp = fopen("/proc/net/arp", "r")) != NULL) {
		while (fgets(buf, sizeof(buf), arp)) {
			if (sscanf(buf, "%15s %*s %*s %17s %*s %s", ip, mac,
				   landev) != 3)
				continue;
			if ((strlen(mac) != 17) ||
			    (strcmp(mac, "00:00:00:00:00:00") == 0))
				continue;
			for (i = 0; i < tablelen; i++) {
				if (!strcasecmp(mac, table[i].mac) &&
				    !strcmp(landev, table[i].ifname)) {
					table[i].mark = 0;
				}
			}
		}
		fclose(arp);
	}
}

static void readconntrack(struct arptable *tbl, int tablelen)
{
	int i;
	char buf[256];
	FILE *conn;
	int nf = 1;
	conn = fopen("/proc/net/nf_conntrack", "r");
	if (!conn) {
		nf = 0;
		conn = fopen("/proc/net/ip_conntrack", "r");
	}

	if (conn) {
		while (fgets(buf, sizeof(buf), conn)) {
			char l3proto[32];
			char proto[32];
			char state[32];
			char src[64];
			if (nf)
				sscanf(buf, "%s %*s %s %*s %*s %s %s", l3proto,
				       proto, state, src);
			else {
				strcpy(l3proto, "ipv4");
				sscanf(buf, "%s %*s %*s %s %s", proto, state,
				       src);
			}
			unsigned int v4;
			struct in6_addr v6;
			int connv6 = 0;
			if (!strcmp(l3proto, "ipv6")) {
				connv6 = 1;
				if (!strcmp(proto, "tcp")) {
					inet_pton(AF_INET6, &src[4], &v6);
				} else
					inet_pton(AF_INET6, &state[4], &v6);
			} else {
				if (!strcmp(proto, "tcp"))
					inet_aton(&src[4],
						  (struct in_addr *)&v4);
				else
					inet_aton(&state[4],
						  (struct in_addr *)&v4);
			}
			if (!strcmp(proto, "tcp") &&
			    strcmp(state, "ESTABLISHED"))
				continue;
			for (i = 0; i < tablelen; i++) {
				if (!connv6 && (tbl[i].proto & 1) &&
				    v4 == tbl[i].v4)
					tbl[i].conncount++;
				else if (connv6 && (tbl[i].proto & 2) &&
					 !memcmp(&v6, &tbl[i].v6, sizeof(v6))) {
					tbl[i].conncount++;
				}
			}
		}
		fclose(conn);
	}
}

EJ_VISIBLE void ej_dumparptable(webs_t wp, int argc, char_t **argv)
{
	FILE *f = fopen("/tmp/bw.db", "rb");
	if (!f) {
		eval("wrtbwmon", "setup", "/tmp/bw.db");
	} else
		fclose(f);
	/* todo, rewrite that mess in C, which will speed up everything */
	eval("wrtbwmon", "update", "/tmp/bw.db");
	eval("wrtbwmon", "publish", "/tmp/bw.db", "/tmp/report.tmp");

	char hostname[128];
	char ip[64];
	char buf[256];
	char ip2[70];
	char mac[18];
	char landev[16];
	char peakin[64];
	char peakout[64];
	char total[64];
	int count = 0;
	int i, len;
	int conn_count = 0;
	struct arptable *table = NULL;
	int tablelen = 0;
	FILE *arp;

	if ((f = fopen("/tmp/report.tmp", "r")) != NULL) {
		while (fgets(buf, sizeof(buf), f)) {
			if (sscanf(buf, "%s %s %s %s %s %s", mac, ip, landev,
				   peakin, peakout, total) != 6)
				continue;
			if ((strlen(mac) != 17) ||
			    (strcmp(mac, "00:00:00:00:00:00") == 0))
				continue;

			len = strlen(mac);
			for (i = 0; i < len; i++)
				mac[i] = toupper(mac[i]);
			tablelen = addtable(&table, mac, ip, landev,
					    atoll(peakin), atoll(peakout),
					    atoll(total), &tablelen);
			conn_count = 0;
		}
		fclose(f);
		readconntrack(table, tablelen);
		readhosts(table, tablelen);
		if (!nvram_match("arp_longdelay", "1"))
			filterarp(table, tablelen);

		for (i = 0; i < tablelen; i++) {
			if (!table[i].mark) {
				websWrite(
					wp,
					"%c'%s','%s','%s','%d', '%s','%lld','%lld','%lld'",
					(count ? ',' : ' '), table[i].hostname,
					table[i].ip, table[i].mac,
					table[i].conncount, table[i].ifname,
					table[i].in, table[i].out,
					table[i].total);
				++count;
			}
		}
		for (i = 0; i < tablelen; i++) {
			debug_free(table[i].hostname);
			debug_free(table[i].ip);
			debug_free(table[i].mac);
			debug_free(table[i].ifname);
		}

		debug_free(table);
	}
}
#endif
