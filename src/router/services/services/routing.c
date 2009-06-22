/*
 * routing.c
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

#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <errno.h>
#include <wlutils.h>
#include <services.h>

#ifdef HAVE_QUAGGA

int zebra_ospf_init(void);
int zebra_bgp_init(void);
int zebra_ripd_init(void);

int zebra_init(void)
{
	char *sub;
	char var[32], *next;

	sub = nvram_safe_get("wk_mode");
	foreach(var, sub, next) {
		if (!strcmp(var, "gateway")) {
			printf("zebra disabled.\n");
			return 0;
		} else if (!strcmp(var, "ospf")) {
			zebra_ospf_init();
			dd_syslog(LOG_INFO,
				  "zebra : zebra (ospf) successfully initiated\n");
		} else if (!strcmp(var, "bgp")) {
			zebra_bgp_init();
			dd_syslog(LOG_INFO,
				  "zebra : zebra (ospf) successfully initiated\n");
		} else if (!strcmp(var, "router")) {
			zebra_ripd_init();
			dd_syslog(LOG_INFO,
				  "zebra : zebra (router) successfully initiated\n");
		}
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
		nvram_set("zebra_copt", "1");
		nvram_set("zebra_conf", buf);
		free(buf);
	} else {
		nvram_set("zebra_copt", "0");
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
		nvram_set("ospfd_copt", "1");
		nvram_set("ospfd_conf", buf);
		free(buf);
	} else {
		nvram_set("ospfd_copt", "0");
		nvram_unset("ospfd_conf");
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
		nvram_set("bgpd_copt", "1");
		nvram_set("bgpd_conf", buf);
		free(buf);
	} else {
		nvram_set("bgpd_copt", "0");
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
		nvram_set("ripd_copt", "1");
		nvram_set("ripd_conf", buf);
		free(buf);
	} else {
		nvram_set("ripd_copt", "0");
		nvram_unset("ripd_conf");
	}

	nvram_commit();
}

int zebra_ospf_init(void)
{
	char *lf = nvram_safe_get("lan_ifname");
	char *wf = get_wan_face();

	FILE *fp;
	int ret1, ret2, s = 0, i = 0;

	/*
	 * Write configuration file based on current information 
	 */
	if (!(fp = fopen("/tmp/zebra.conf", "w"))) {
		perror("/tmp/zebra.conf");
		return errno;
	}

	if (nvram_match("zebra_copt", "1")) {
		if (nvram_match("zebra_log", "1")) {
			fprintf(fp, "log file /var/log/zebra.log\n");
		}
	}

	if (strlen(nvram_safe_get("zebra_conf")) > 0) {
		fwritenvram("zebra_conf", fp);
	}

	fclose(fp);

	if (!(fp = fopen("/tmp/ospfd.conf", "w"))) {
		perror("/tmp/ospfd.conf");
		return errno;
	}

	if (nvram_match("ospfd_copt", "1")
	    && strlen(nvram_safe_get("ospfd_conf"))) {
		fwritenvram("ospfd_conf", fp);
	} else {
		fprintf(fp, "!\n");
		// fprintf (fp, "password %s\n", nvram_safe_get ("http_passwd"));
		// fprintf (fp, "enable password %s\n", nvram_safe_get
		// ("http_passwd"));
		fprintf(fp, "!\n!\n!\n");

		fprintf(fp, "interface %s\n!\n", lf);
		if (wf && strlen(wf) > 0)
			fprintf(fp, "interface %s\n", wf);

		int cnt = get_wl_instances();
		int c;

		for (c = 0; c < cnt; c++) {
			if (nvram_nmatch("1", "wl%d_br1_enable", c)) {
				fprintf(fp, "!\n! 'Subnet' WDS bridge\n");
				fprintf(fp, "interface br1\n");
			}
			if (nvram_nmatch("ap", "wl%d_mode", c))
				for (s = 1; s <= MAX_WDS_DEVS; s++) {
					char wdsdevospf[32] = { 0 };
					char *dev;

					sprintf(wdsdevospf, "wl%d_wds%d_ospf",
						c, s);
					dev = nvram_nget("wl%d_wds%d_if", c, s);

					if (nvram_nmatch
					    ("1", "wl%d_wds%d_enable", c, s)) {
						fprintf(fp, "!\n! WDS: %s\n",
							nvram_nget
							("wl%d_wds%d_desc", c,
							 s));
						fprintf(fp, "interface %s\n",
							dev);

						if (atoi
						    (nvram_safe_get(wdsdevospf))
						    > 0)
							fprintf(fp,
								" ip ospf cost %s\n",
								nvram_safe_get
								(wdsdevospf));
					}
				}
			fprintf(fp, "!\n");
		}
		fprintf(fp, "router ospf\n");
		fprintf(fp, " passive-interface lo\n");
		fprintf(fp, " ospf router-id %s\n",
			nvram_safe_get("lan_ipaddr"));
		fprintf(fp, " redistribute kernel\n");
		fprintf(fp, " redistribute connected\n");
		fprintf(fp, " redistribute static\n");
		fprintf(fp, " network 0.0.0.0/0 area 0\n");	// handle all routing
		fprintf(fp, " default-information originate\n");

		for (s = 1; s <= MAX_WDS_DEVS; s++) {
			char wdsdevospf[32] = { 0 };
			sprintf(wdsdevospf, "wl_wds%d_ospf", s);

			if (atoi(nvram_safe_get(wdsdevospf)) < 0)
				fprintf(fp, " passive-interface %s\n",
					nvram_nget("wl_wds%d_if", s));
		}

		if (nvram_match("zebra_log", "1")) {
			fprintf(fp, "!\n");
			fprintf(fp, "log file /var/log/ospf.log\n");
		}

		fprintf(fp, "!\nline vty\n!\n");
	}

	fflush(fp);
	fclose(fp);

	if (nvram_match("dyn_default", "1"))
		while (!eval("ip", "route", "del", "default")) ;

	ret1 = eval("zebra", "-d", "-f", "/tmp/zebra.conf");
	ret2 = eval("ospfd", "-d", "-f", "/tmp/ospfd.conf");

	return ret1 + ret2;
}

int zebra_ripd_init(void)
{

	char *lt = nvram_safe_get("dr_lan_tx");
	char *lr = nvram_safe_get("dr_lan_rx");
	char *wt = nvram_safe_get("dr_wan_tx");
	char *wr = nvram_safe_get("dr_wan_rx");
	char *lf = nvram_safe_get("lan_ifname");
	char *wf = get_wan_face();

	FILE *fp;
	int ret1, ret2;

	// printf("Start zebra\n");
	if (!strcmp(lt, "0") && !strcmp(lr, "0") &&
	    !strcmp(wt, "0") && !strcmp(wr, "0")
	    && !nvram_match("zebra_copt", "1")) {
		fprintf(stderr, "zebra disabled.\n");
		return 0;
	}

	/*
	 * Write configuration file based on current information 
	 */
	if (!(fp = fopen("/tmp/zebra.conf", "w"))) {
		perror("/tmp/zebra.conf");
		return errno;
	}

	if (nvram_match("zebra_copt", "1")) {
		if (nvram_match("zebra_log", "1")) {
			fprintf(fp, "log file /var/log/zebra.log\n");
		}
	}

	if (strlen(nvram_safe_get("zebra_conf")) > 0) {
		fwritenvram("zebra_conf", fp);
	}

	fclose(fp);

	if (!(fp = fopen("/tmp/ripd.conf", "w"))) {
		perror("/tmp/ripd.conf");
		return errno;
	}

	if (nvram_match("ripd_copt", "1")) {
		fwritenvram("ripd_conf", fp);
	} else {

		fprintf(fp, "router rip\n");
		fprintf(fp, "  network %s\n", lf);
		if (wf && strlen(wf) > 0)
			fprintf(fp, "  network %s\n", wf);
		fprintf(fp, "redistribute connected\n");
		// fprintf(fp, "redistribute kernel\n");
		// fprintf(fp, "redistribute static\n");

		fprintf(fp, "interface %s\n", lf);
		if (strcmp(lt, "0") != 0)
			fprintf(fp, "  ip rip send version %s\n", lt);
		if (strcmp(lr, "0") != 0)
			fprintf(fp, "  ip rip receive version %s\n", lr);

		if (wf && strlen(wf) > 0)
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
		if (wf && strlen(wf) > 0) {
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

	ret1 = eval("zebra", "-d", "-f", "/tmp/zebra.conf");
	ret2 = eval("ripd", "-d", "-f", "/tmp/ripd.conf");

	return ret1 + ret2;
}

int zebra_bgp_init(void)
{

	char *lt = nvram_safe_get("dr_lan_tx");
	char *lr = nvram_safe_get("dr_lan_rx");
	char *wt = nvram_safe_get("dr_wan_tx");
	char *wr = nvram_safe_get("dr_wan_rx");
	char *lf = nvram_safe_get("lan_ifname");
	char *wf = get_wan_face();

	FILE *fp;
	int ret1, ret2;

	if (!strcmp(lt, "0") && !strcmp(lr, "0") &&
	    !strcmp(wt, "0") && !strcmp(wr, "0")
	    && !nvram_match("zebra_copt", "1")) {
		return 0;
	}

	/*
	 * Write configuration file based on current information 
	 */
	if (!(fp = fopen("/tmp/zebra.conf", "w"))) {
		perror("/tmp/zebra.conf");
		return errno;
	}

	if (nvram_match("zebra_copt", "1")) {
		if (nvram_match("zebra_log", "1")) {
			fprintf(fp, "log file /var/log/zebra.log\n");
		}
	}

	if (strlen(nvram_safe_get("zebra_conf")) > 0) {
		fwritenvram("zebra_conf", fp);
	}

	fclose(fp);

	if (!(fp = fopen("/tmp/bgpd.conf", "w"))) {
		perror("/tmp/bgpd.conf");
		return errno;
	}
	if (nvram_match("bgpd_copt", "1")) {
		fwritenvram("bgpd_conf", fp);
	} else {
		fprintf(fp, "router bgp %s\n",
			nvram_safe_get("routing_bgp_as"));
		fprintf(fp, "  network %s/%d\n", nvram_safe_get("lan_ipaddr"),
			get_net(nvram_safe_get("lan_netmask")));
		if (wf && strlen(wf) > 0
		    && !nvram_match("wan_ipaddr", "0.0.0.0"))
			fprintf(fp, "  network %s/%s\n",
				nvram_safe_get("wan_ipaddr"),
				nvram_safe_get("wan_netmask"));
		fprintf(fp, "neighbor %s local-as %s\n", lf,
			nvram_safe_get("routing_bgp_as"));
		if (wf && strlen(wf) > 0)
			fprintf(fp, "neighbor %s local-as %s\n", wf,
				nvram_safe_get("routing_bgp_as"));
		fprintf(fp, "neighbor %s remote-as %s\n",
			nvram_safe_get("routing_bgp_neighbor_ip"),
			nvram_safe_get("routing_bgp_neighbor_as"));
		fprintf(fp, "access-list all permit any\n");

		fflush(fp);
	}
	fclose(fp);

	ret1 = eval("zebra", "-d", "-f", "/tmp/zebra.conf");
	ret2 = eval("bgpd", "-d", "-f", "/tmp/bgpd.conf");

	return ret1 + ret2;
}

#endif

#ifdef HAVE_BIRD
int bird_init(void)
{
	FILE *fp;
	int ret1;

	/*
	 * compatibitly for old nvram style (site needs to be enhanced)
	 */
	if (nvram_match("wk_mode", "gateway"))
		return 0;
	nvram_set("routing_ospf", "off");
	nvram_set("routing_bgp", "off");
	nvram_set("routing_rip2", "off");

	if (nvram_match("wk_mode", "ospf"))
		nvram_set("routing_ospf", "on");
	if (nvram_match("wk_mode", "router"))
		nvram_set("routing_rip2", "on");
	if (nvram_match("wk_mode", "bgp"))
		nvram_set("routing_bgp", "on");

	if (nvram_match("dr_setting", "1")) {
		nvram_set("routing_wan", "on");
		nvram_set("routing_lan", "off");
	}
	if (nvram_match("dr_setting", "2")) {
		nvram_set("routing_wan", "off");
		nvram_set("routing_lan", "on");
	}
	if (nvram_match("dr_setting", "3")) {
		nvram_set("routing_wan", "on");
		nvram_set("routing_lan", "on");
	}
	if (nvram_match("dr_setting", "0")) {
		nvram_set("routing_wan", "off");
		nvram_set("routing_lan", "off");
	}
	// DD-WRT bird support 
	if (nvram_match("routing_rip2", "on") ||
	    nvram_match("routing_ospf", "on")
	    || nvram_match("routing_bgp", "on")) {
		mkdir("/tmp/bird", 0744);
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
		}		// if wk_mode = ospf

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
		fflush(fp);
		fclose(fp);

		ret1 = eval("bird", "-c", "/tmp/bird/bird.conf");
		dd_syslog(LOG_INFO,
			  "bird : bird daemon successfully started\n");
	}
	return 0;

}
#endif				/* HAVE_BIRD */
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
/*
 * Written by Sparq in 2002/07/16 
 */
void start_zebra(void)
{

	if (!nvram_invmatch("zebra_enable", "0"))
		return;

#ifdef HAVE_BIRD

	if (bird_init() != 0)
		return;

#elif defined(HAVE_QUAGGA)

	if (zebra_init() != 0)
		return;

#endif				/* HAVE_BIRD */
	return;
}

/*
 * Written by Sparq in 2002/07/16 
 */
void stop_zebra(void)
{

#ifdef HAVE_QUAGGA

	if (pidof("zebra") > 0 || pidof("ripd") > 0 || pidof("ospfd") > 0
	    || pidof("bgpd") > 0) {
		dd_syslog(LOG_INFO,
			  "zebra : zebra (ripd and ospfd) daemon successfully stopped\n");
		killall("zebra", SIGTERM);
		killall("ripd", SIGTERM);
		killall("ospfd", SIGTERM);
		killall("bgpd", SIGTERM);

		while (!(killall("zebra", SIGTERM) && killall("ripd", SIGTERM)
			 && killall("ospfd", SIGTERM)
			 && killall("bgpd", SIGTERM)))
			sleep(1);

		cprintf("done\n");
	}
	return;

#elif defined(HAVE_BIRD)
	if (pidof("bird") > 0) {
		dd_syslog(LOG_INFO,
			  "bird : bird daemon successfully stopped\n");
		killall("bird", SIGTERM);

		cprintf("done\n");
	}
	return;

#else
	return;
#endif
}

#endif
