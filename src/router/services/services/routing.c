/*
 * routing.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#include <stdlib.h>
#include <sys/stat.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <wlutils.h>
#include <services.h>

#ifdef HAVE_QUAGGA

static int zebra_ospf_init(void);
static int zebra_ospf6_init(void);
static int zebra_bgp_init(void);
static int zebra_ripd_init(void);

static int zebra_init(void)
{
	char *sub;
	char var[32], *next;
	char daemons[64];
	int services = 0;
	int has_ospfd = 0, has_ospf6d = 0, has_bgpd = 0, has_ripd = 0;
#ifdef HAVE_FRR
	sprintf(daemons,
		"watchfrr -d -s '%%s -d' -k 'killall %%s' -r '%%s -d' zebra");
#else
	sprintf(daemons, "watchquagga -dz -r '%%s -d' zebra");
#endif
	sub = nvram_safe_get("wk_mode");
	foreach(var, sub, next)
	{
		if (!strcmp(var, "ospf")) {
			int res = zebra_ospf_init();
			if (!res) {
				services++;
				has_ospfd = 1;
				strcat(daemons, " ospfd");
			}
		} else if (!strcmp(var, "ospf6")) {
			int res = zebra_ospf6_init();
			if (!res) {
				services++;
				has_ospf6d = 1;
				strcat(daemons, " ospf6d");
			}
		} else if (!strcmp(var, "bgp")) {
			int res = zebra_bgp_init();
			if (!res) {
				services++;
				has_bgpd = 1;
				strcat(daemons, " bgpd");
			}
		} else if (!strcmp(var, "router")) {
			int res = zebra_ripd_init();
			if (!res) {
				services++;
				has_ripd = 1;
				strcat(daemons, " ripd");
			}
		}
	}
#ifdef HAVE_FRR
	if (services) {
		log_eval("zebra", "-d");
		if (has_ospfd) {
			log_eval("ospfd", "-d");
		}
		if (has_ospf6d) {
			log_eval("ospf6d", "-d");
		}
		if (has_bgpd) {
			log_eval("bgpd", "-d");
		}
		if (has_ripd) {
			log_eval("ripd", "-d");
		}
	}
#endif

	if (services) {
		dd_loginfo("zebra", "(%s) successfully initiated\n", daemons);
		system(daemons);
	}
	return 0;
}

void start_quagga_writememory(void)
{
	FILE *in = fopen("/tmp/zebra.conf", "rb");

	if (in != NULL) {
		fseek(in, 0, SEEK_END);
		int len = ftell(in);

		rewind(in);
		char *buf = malloc(len + 1);

		fread(buf, len, 1, in);
		buf[len] = 0;
		fclose(in);
		nvram_seti("zebra_copt", 1);
		nvram_set("zebra_conf", buf);
		free(buf);
	} else {
		nvram_seti("zebra_copt", 0);
		nvram_unset("zebra_conf");
	}
	in = fopen("/tmp/ospfd.conf", "rb");

	if (in != NULL) {
		fseek(in, 0, SEEK_END);
		int len = ftell(in);

		rewind(in);
		char *buf = malloc(len + 1);

		fread(buf, len, 1, in);
		buf[len] = 0;
		fclose(in);
		nvram_seti("ospfd_copt", 1);
		nvram_set("ospfd_conf", buf);
		free(buf);
	} else {
		nvram_seti("ospfd_copt", 0);
		nvram_unset("ospfd_conf");
	}

	in = fopen("/tmp/ospf6d.conf", "rb");

	if (in != NULL) {
		fseek(in, 0, SEEK_END);
		int len = ftell(in);

		rewind(in);
		char *buf = malloc(len + 1);

		fread(buf, len, 1, in);
		buf[len] = 0;
		fclose(in);
		nvram_seti("ospf6d_copt", 1);
		nvram_set("ospf6d_conf", buf);
		free(buf);
	} else {
		nvram_seti("ospf6d_copt", 0);
		nvram_unset("ospf6d_conf");
	}

	in = fopen("/tmp/bgpd.conf", "rb");

	if (in != NULL) {
		fseek(in, 0, SEEK_END);
		int len = ftell(in);

		rewind(in);
		char *buf = malloc(len + 1);

		fread(buf, len, 1, in);
		buf[len] = 0;
		fclose(in);
		nvram_seti("bgpd_copt", 1);
		nvram_set("bgpd_conf", buf);
		free(buf);
	} else {
		nvram_seti("bgpd_copt", 0);
		nvram_unset("bgpd_conf");
	}

	in = fopen("/tmp/ripd.conf", "rb");

	if (in != NULL) {
		fseek(in, 0, SEEK_END);
		int len = ftell(in);

		rewind(in);
		char *buf = malloc(len + 1);

		fread(buf, len, 1, in);
		buf[len] = 0;
		fclose(in);
		nvram_seti("ripd_copt", 1);
		nvram_set("ripd_conf", buf);
		free(buf);
	} else {
		nvram_seti("ripd_copt", 0);
		nvram_unset("ripd_conf");
	}

	nvram_async_commit();
}

static int zebra_ospf_init(void)
{
	FILE *fp;
	int s = 0;
	char wan_if_buffer[33];

	/*
	 * Write configuration file based on current information 
	 */
	if (!(fp = fopen("/tmp/zebra.conf", "w"))) {
		perror("/tmp/zebra.conf");
		return errno;
	}

	if (nvram_matchi("zebra_copt", 1)) {
		if (nvram_matchi("zebra_log", 1)) {
			fprintf(fp, "log file /var/log/zebra.log\n");
		}
	}

	fwritenvram("zebra_conf", fp);

	fclose(fp);

	if (!(fp = fopen("/tmp/ospfd.conf", "w"))) {
		perror("/tmp/ospfd.conf");
		return errno;
	}

	if (nvram_matchi("ospfd_copt", 1)) {
		fwritenvram("ospfd_conf", fp);
	} else {
		char *next;
		char var[80];
		char eths[256];
		char eths2[256];
		char bufferif[512];

		bzero(eths, sizeof(eths));
		getIfLists(eths, sizeof(eths));
		//add ppp interfacs
		bzero(eths2, sizeof(eths2));
		getIfList(eths2, "ppp");
		strcat(eths, " ");
		strcat(eths, eths2);
		//add tun interfaces
		bzero(eths2, sizeof(eths2));
		getIfList(eths2, "tun");
		strcat(eths, " ");
		strcat(eths, eths2);
		bzero(bufferif, 256);
		getIfListB(bufferif, NULL, 1, 1, 1);
		foreach(var, eths, next)
		{
			if (!strcmp("etherip0", var))
				continue;
			char *ipaddr = nvram_nget("%s_ipaddr", var);
			if (nvram_nmatch("0", "%s_bridged", var) && *ipaddr &&
			    strcmp(ipaddr, "0.0.0.0")) {
				fprintf(fp, "interface %s\n", var);
			} else {
				if (!strcmp(safe_get_wan_face(wan_if_buffer),
					    var)) {
					char *ipaddr =
						nvram_safe_get("wan_ipaddr");
					if (*ipaddr &&
					    strcmp(ipaddr, "0.0.0.0")) {
						fprintf(fp, "interface %s\n",
							var);
					}
				}
			}
		}
		foreach(var, bufferif, next)
		{
			if (!strcmp("br0", var)) {
				char *ipaddr = nvram_safe_get("lan_ipaddr");
				char *netmask = nvram_safe_get("lan_netmask");
				if (*ipaddr && strcmp(ipaddr, "0.0.0.0"))
					fprintf(fp, "interface %s\n", var);
				continue;
			}

			char *ipaddr = nvram_nget("%s_ipaddr", var);
			if (*ipaddr && strcmp(ipaddr, "0.0.0.0")) {
				fprintf(fp, "interface %s\n", var);
			} else {
				if (!strcmp(safe_get_wan_face(wan_if_buffer),
					    var)) {
					char *ipaddr =
						nvram_safe_get("wan_ipaddr");
					if (*ipaddr &&
					    strcmp(ipaddr, "0.0.0.0")) {
						fprintf(fp, "interface %s\n",
							var);
					}
				}
			}
		}

		fprintf(fp, "router ospf\n");
		fprintf(fp, " passive-interface lo\n");
		fprintf(fp, " passive-interface br0:0\n");
		fprintf(fp, " ospf router-id %s\n",
			nvram_safe_get("lan_ipaddr"));
		fprintf(fp, " redistribute kernel metric-type 1\n");
		fprintf(fp, " redistribute connected metric-type 1\n");
		fprintf(fp, " redistribute static metric-type 1\n");
		foreach(var, eths, next)
		{
			if (!strcmp(safe_get_wan_face(wan_if_buffer), var)) {
				char *ipaddr = nvram_safe_get("wan_ipaddr");
				char *netmask = nvram_safe_get("wan_netmask");
				if (*ipaddr && strcmp(ipaddr, "0.0.0.0"))
					fprintf(fp,
						" network %s/%d area 0.0.0.0\n",
						ipaddr, getmask(netmask));

				continue;
			}

			if (nvram_nmatch("0", "%s_bridged", var)) {
				char *ipaddr = nvram_nget("%s_ipaddr", var);
				char *netmask = nvram_nget("%s_netmask", var);
				if (*ipaddr && strcmp(ipaddr, "0.0.0.0"))
					fprintf(fp,
						" network %s/%d area 0.0.0.0\n",
						ipaddr, getmask(netmask));
			}
		}

		foreach(var, bufferif, next)
		{
			if (strcmp(safe_get_wan_face(wan_if_buffer), "br0") &&
			    !strcmp(safe_get_wan_face(wan_if_buffer), var)) {
				char *ipaddr = nvram_safe_get("wan_ipaddr");
				char *netmask = nvram_safe_get("wan_netmask");
				if (*ipaddr && strcmp(ipaddr, "0.0.0.0"))
					fprintf(fp,
						" network %s/%d area 0.0.0.0\n",
						ipaddr, getmask(netmask));

				continue;
			}
			if (!strcmp("br0", var)) {
				char *ipaddr = nvram_safe_get("lan_ipaddr");
				char *netmask = nvram_safe_get("lan_netmask");
				if (*ipaddr && strcmp(ipaddr, "0.0.0.0"))
					fprintf(fp,
						" network %s/%d area 0.0.0.0\n",
						ipaddr, getmask(netmask));

				continue;
			}

			char *ipaddr = nvram_nget("%s_ipaddr", var);
			char *netmask = nvram_nget("%s_netmask", var);
			if (*ipaddr && strcmp(ipaddr, "0.0.0.0"))
				fprintf(fp, " network %s/%d area 0.0.0.0\n",
					ipaddr, getmask(netmask));
		}
		if (nvram_match("ospfd_default_route", "1"))
			fprintf(fp, " default-information originate\n");
		else
			fprintf(fp, " no default-information originate\n");
		char *hostname = nvram_safe_get("router_name");
		if (*hostname)
			fprintf(fp, "hostname %s\n", hostname);

		if (nvram_matchi("zebra_log", 1)) {
			fprintf(fp, "!\n");
			fprintf(fp, "log file /var/log/ospf.log\n");
		}

		fprintf(fp, "!\nline vty\n!\n");
	}

	fclose(fp);
	return 0;
}

static int zebra_ospf6_init(void)
{
	char *lf = nvram_safe_get("lan_ifname");
	char wan_if_buffer[33];
	char *wf = safe_get_wan_face(wan_if_buffer);

	FILE *fp;
	int s = 0;

	/*
	 * Write configuration file based on current information 
	 */
	if (!(fp = fopen("/tmp/zebra.conf", "w"))) {
		perror("/tmp/zebra.conf");
		return errno;
	}

	if (nvram_matchi("zebra_copt", 1)) {
		if (nvram_matchi("zebra_log", 1)) {
			fprintf(fp, "log file /var/log/zebra.log\n");
		}
	}

	fwritenvram("zebra_conf", fp);

	fclose(fp);

	if (!(fp = fopen("/tmp/ospf6d.conf", "w"))) {
		perror("/tmp/ospf6d.conf");
		return errno;
	}

	if (nvram_matchi("ospf6d_copt", 1)) {
		fwritenvram("ospf6d_conf", fp);
	} else {
		fprintf(fp, "!\n");
		// fprintf (fp, "password %s\n", nvram_safe_get ("http_passwd"));
		// fprintf (fp, "enable password %s\n", nvram_safe_get
		// ("http_passwd"));
		fprintf(fp, "!\n!\n!\n");

		fprintf(fp, "interface %s\n!\n", lf);
		if (wf && *wf)
			fprintf(fp, "interface %s\n", wf);

		int cnt = getdevicecount();
		int c;

		for (c = 0; c < cnt; c++) {
			if (nvram_nmatch("1", WIFINAME "%d_br1_enable", c)) {
				fprintf(fp, "!\n! 'Subnet' WDS bridge\n");
				fprintf(fp, "interface br1\n");
			}
			if (nvram_nmatch("ap", WIFINAME "%d_mode", c))
				for (s = 1; s <= MAX_WDS_DEVS; s++) {
					char wdsdevospf[32] = { 0 };
					char *dev;

					sprintf(wdsdevospf,
						WIFINAME "%d_wds%d_ospf", c, s);
					dev = nvram_nget(WIFINAME "%d_wds%d_if",
							 c, s);

					if (nvram_nmatch("1",
							 WIFINAME
							 "%d_wds%d_enable",
							 c, s)) {
						fprintf(fp, "!\n! WDS: %s\n",
							nvram_nget(
								WIFINAME
								"%d_wds%d_desc",
								c, s));
						fprintf(fp, "interface %s\n",
							dev);

						if (nvram_geti(wdsdevospf) > 0)
							fprintf(fp,
								" ip ospf cost %s\n",
								nvram_safe_get(
									wdsdevospf));
					}
				}
			fprintf(fp, "!\n");
		}
		fprintf(fp, "router osp6f\n");
		// fprintf(fp, " passive-interface lo\n");
		// fprintf(fp, " ospf router-id %s\n",
		// nvram_safe_get("lan_ipaddr"));
		fprintf(fp, " redistribute kernel\n");
		fprintf(fp, " redistribute connected\n");
		fprintf(fp, " redistribute static\n");
		// fprintf(fp, " network 0.0.0.0/0 area 0\n");  // handle all routing
		// fprintf(fp, " default-information originate\n");

		for (s = 1; s <= MAX_WDS_DEVS; s++) {
			char wdsdevospf[32] = { 0 };
			sprintf(wdsdevospf, "wl_wds%d_ospf", s);

			if (nvram_geti(wdsdevospf) < 0)
				fprintf(fp, " passive-interface %s\n",
					nvram_nget("wl_wds%d_if", s));
		}

		if (nvram_matchi("zebra_log", 1)) {
			fprintf(fp, "!\n");
			fprintf(fp, "log file /var/log/ospf6.log\n");
		}

		fprintf(fp, "!\nline vty\n!\n");
	}

	fclose(fp);

	return 0;
}

static int zebra_ripd_init(void)
{
	char wan_if_buffer[33];
	char *lt = nvram_safe_get("dr_lan_tx");
	char *lr = nvram_safe_get("dr_lan_rx");
	char *wt = nvram_safe_get("dr_wan_tx");
	char *wr = nvram_safe_get("dr_wan_rx");
	char *lf = nvram_safe_get("lan_ifname");
	char *wf = safe_get_wan_face(wan_if_buffer);

	FILE *fp;

	// printf("Start zebra\n");
	if (!strcmp(lt, "0") && !strcmp(lr, "0") && !strcmp(wt, "0") &&
	    !strcmp(wr, "0") && !nvram_matchi("zebra_copt", 1)) {
		fprintf(stderr, "zebra disabled.\n");
		return -1;
	}

	/*
	 * Write configuration file based on current information 
	 */
	if (!(fp = fopen("/tmp/zebra.conf", "w"))) {
		perror("/tmp/zebra.conf");
		return errno;
	}

	if (nvram_matchi("zebra_copt", 1)) {
		if (nvram_matchi("zebra_log", 1)) {
			fprintf(fp, "log file /var/log/zebra.log\n");
		}
	}

	fwritenvram("zebra_conf", fp);

	fclose(fp);

	if (!(fp = fopen("/tmp/ripd.conf", "w"))) {
		perror("/tmp/ripd.conf");
		return errno;
	}

	if (nvram_matchi("ripd_copt", 1)) {
		fwritenvram("ripd_conf", fp);
	} else {
		fprintf(fp, "router rip\n");
		fprintf(fp, "  network %s\n", lf);
		if (wf && *wf)
			fprintf(fp, "  network %s\n", wf);
		fprintf(fp, "redistribute connected\n");
		// fprintf(fp, "redistribute kernel\n");
		// fprintf(fp, "redistribute static\n");

		fprintf(fp, "interface %s\n", lf);
		if (strcmp(lt, "0") != 0)
			fprintf(fp, "  ip rip send version %s\n", lt);
		if (strcmp(lr, "0") != 0)
			fprintf(fp, "  ip rip receive version %s\n", lr);

		if (wf && *wf)
			fprintf(fp, "interface %s\n", wf);
		if (strcmp(wt, "0") != 0)
			fprintf(fp, "  ip rip send version %s\n", wt);
		if (strcmp(wr, "0") != 0)
			fprintf(fp, "  ip rip receive version %s\n", wr);

		fprintf(fp, "router rip\n");
		if (strcmp(lt, "0") == 0)
			fprintf(fp, "  distribute-list private out %s\n", lf);
		if (strcmp(lr, "0") == 0)
			fprintf(fp, "  distribute-list private in  %s\n", lf);
		if (wf && *wf) {
			if (strcmp(wt, "0") == 0)
				fprintf(fp,
					"  distribute-list private out %s\n",
					wf);
			if (strcmp(wr, "0") == 0)
				fprintf(fp,
					"  distribute-list private in  %s\n",
					wf);
		}
		fprintf(fp, "access-list private deny any\n");

		// fprintf(fp, "debug rip events\n");
		// fprintf(fp, "log file /tmp/ripd.log\n");
		fflush(fp);
	}
	fclose(fp);
	return 0;
}

static int zebra_bgp_init(void)
{
	char wan_if_buffer[33];
	char *lt = nvram_safe_get("dr_lan_tx");
	char *lr = nvram_safe_get("dr_lan_rx");
	char *wt = nvram_safe_get("dr_wan_tx");
	char *wr = nvram_safe_get("dr_wan_rx");
	char *lf = nvram_safe_get("lan_ifname");
	char *wf = safe_get_wan_face(wan_if_buffer);

	FILE *fp;

	if (!strcmp(lt, "0") && !strcmp(lr, "0") && !strcmp(wt, "0") &&
	    !strcmp(wr, "0") && !nvram_matchi("zebra_copt", 1)) {
		return -1;
	}

	/*
	 * Write configuration file based on current information 
	 */
	if (!(fp = fopen("/tmp/zebra.conf", "w"))) {
		perror("/tmp/zebra.conf");
		return errno;
	}

	if (nvram_matchi("zebra_copt", 1)) {
		if (nvram_matchi("zebra_log", 1)) {
			fprintf(fp, "log file /var/log/zebra.log\n");
		}
	}

	fwritenvram("zebra_conf", fp);

	fclose(fp);

	if (!(fp = fopen("/tmp/bgpd.conf", "w"))) {
		perror("/tmp/bgpd.conf");
		return errno;
	}
	if (nvram_matchi("bgpd_copt", 1)) {
		fwritenvram("bgpd_conf", fp);
	} else {
		fprintf(fp, "router bgp %s\n",
			nvram_safe_get("routing_bgp_as"));
		fprintf(fp, "  network %s/%d\n", nvram_safe_get("lan_ipaddr"),
			getmask(nvram_safe_get("lan_netmask")));
		if (wf && *wf && strcmp(get_wan_ipaddr(), "0.0.0.0"))
			fprintf(fp, "  network %s/%s\n", get_wan_ipaddr(),
				nvram_safe_get("wan_netmask"));
		fprintf(fp, "neighbor %s local-as %s\n", lf,
			nvram_safe_get("routing_bgp_as"));
		if (wf && *wf)
			fprintf(fp, "neighbor %s local-as %s\n", wf,
				nvram_safe_get("routing_bgp_as"));
		fprintf(fp, "neighbor %s remote-as %s\n",
			nvram_safe_get("routing_bgp_neighbor_ip"),
			nvram_safe_get("routing_bgp_neighbor_as"));
		fprintf(fp, "access-list all permit any\n");

		fflush(fp);
	}
	fclose(fp);

	return 0;
}

#endif

#ifdef HAVE_BIRD
static int bird_init(void)
{
	FILE *fp;

	/*
	 * compatibitly for old nvram style (site needs to be enhanced)
	 */
	if (has_gateway())
		return -1;
	nvram_set("routing_ospf", "off");
	nvram_set("routing_bgp", "off");
	nvram_set("routing_rip2", "off");

	if (nvram_match("wk_mode", "ospf"))
		nvram_set("routing_ospf", "on");
	if (nvram_match("wk_mode", "router"))
		nvram_set("routing_rip2", "on");
	if (nvram_match("wk_mode", "bgp"))
		nvram_set("routing_bgp", "on");

	if (nvram_matchi("dr_setting", 1)) {
		nvram_set("routing_wan", "on");
		nvram_set("routing_lan", "off");
	}
	if (nvram_matchi("dr_setting", 2)) {
		nvram_set("routing_wan", "off");
		nvram_set("routing_lan", "on");
	}
	if (nvram_matchi("dr_setting", 3)) {
		nvram_set("routing_wan", "on");
		nvram_set("routing_lan", "on");
	}
	if (nvram_matchi("dr_setting", 0)) {
		nvram_set("routing_wan", "off");
		nvram_set("routing_lan", "off");
	}
	// DD-WRT bird support
	if (nvram_match("routing_rip2", "on") ||
	    nvram_match("routing_ospf", "on") ||
	    nvram_match("routing_bgp", "on")) {
		mkdir("/tmp/bird", 0744);
		mkdir("/tmp/bird/run", 0744);
		if (!(fp = fopen("/tmp/bird/bird.conf", "w"))) {
			perror("/tmp/bird/bird.conf");
			return errno;
		}
		fprintf(fp, "router id %s;\n", nvram_safe_get("lan_ipaddr"));
		fprintf(fp,
			"protocol kernel { learn; persist; scan time 10; import all; export all; }\n");
		fprintf(fp, "protocol device { scan time 10; } \n");
		fprintf(fp, "protocol direct { interface \"*\";}\n");

		if (nvram_match("routing_rip2", "on")) {
			fprintf(fp, "protocol rip WRT54G_rip {\n");
			if (nvram_match("routing_lan", "on"))
				fprintf(fp, "	interface \"%s\" { };\n",
					nvram_safe_get("lan_ifname"));
			if (nvram_match("routing_wan", "on")) {
				if (getSTA())
					fprintf(fp,
						"	interface \"%s\" { };\n",
						getSTA());
				else
					fprintf(fp,
						"	interface \"%s\" { };\n",
						nvram_safe_get("wan_ifname"));
			}
			fprintf(fp, "	honor always;\n");
			fprintf(fp,
				"	import filter { print \"importing\"; accept; };\n");
			fprintf(fp,
				"	export filter { print \"exporting\"; accept; };\n");
			fprintf(fp, "}\n");
		}
		if (nvram_match("routing_ospf", "on")) {
			fprintf(fp, "protocol ospf WRT54G_ospf {\n");
			fprintf(fp, "area 0 {\n");
			if (nvram_match("routing_wan", "on"))
				fprintf(fp,
					"interface \"%s\" { cost 1; authentication simple; password \"%s\"; };\n",
					nvram_safe_get("wan_ifname"),
					nvram_safe_get("http_passwd"));
			if (nvram_match("routing_lan", "on"))
				fprintf(fp,
					"interface \"%s\" { cost 1; authentication simple; password \"%s\"; };\n",
					nvram_safe_get("lan_ifname"),
					nvram_safe_get("http_passwd"));
			fprintf(fp, "};\n}\n");
		} // if wk_mode = ospf

		if (nvram_match("routing_bgp", "on")) {
			fprintf(fp, "protocol bgp {\n");
			fprintf(fp, "local as %s;\n",
				nvram_safe_get("routing_bgp_as"));
			fprintf(fp, "neighbor %s as %s;\n",
				nvram_safe_get("routing_bgp_neighbor_ip"),
				nvram_safe_get("routing_bgp_neighbor_as"));
			fprintf(fp, "export all;\n");
			fprintf(fp, "import all;\n");
			fprintf(fp, "}\n");
		}
		fclose(fp);

		log_eval("bird", "-c", "/tmp/bird/bird.conf");
	}
	return 0;
}
#endif /* HAVE_BIRD */
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
/*
 * Written by Sparq in 2002/07/16 
 */
void start_zebra(void)
{
#ifdef HAVE_BIRD

	if (bird_init() != 0)
		return;

#elif defined(HAVE_QUAGGA)

	if (zebra_init() != 0)
		return;

#endif /* HAVE_BIRD */
	return;
}

/*
 * Written by Sparq in 2002/07/16 
 */
void stop_zebra(void)
{
#ifdef HAVE_QUAGGA
#ifdef HAVE_FRR
	stop_process("watchfrr", "daemon");
#else
	stop_process("watchquagga", "daemon");
#endif
	stop_process("zebra", "daemon");
	stop_process("ripd", "daemon");
	stop_process("ospfd", "daemon");
	stop_process("ospf6d", "daemon");
	stop_process("bgpd", "daemon");
	return;

#elif defined(HAVE_BIRD)
	stop_process("bird", "daemon");
	return;

#else
	return;
#endif
}

#endif
