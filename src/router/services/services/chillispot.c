/*
 * chillispot.c
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
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <nvparse.h>
#include "snmp.h"
#include <signal.h>
#include <utils.h>
#include <syslog.h>
#include <wlutils.h>
#include <errno.h>
#include <md5.h>
#include <services.h>

#ifdef HAVE_CHILLI

#ifdef HAVE_HOTSPOT
void hotspotsys_config(void);
#endif
void chilli_config(void);
void stop_chilli(void);

void start_chilli(void)
{
	int ret = 0;
	char ssid[128];

	stop_chilli();		//ensure that its stopped

	if (!strlen(nvram_safe_get("chilli_interface")))
		nvram_set("chilli_interface", get_wdev());
	if (!strlen(nvram_safe_get("hotss_interface")))
		nvram_set("hotss_interface", get_wdev());

#ifdef HAVE_HOTSPOT

	if (nvram_match("chilli_enable", "1")
	    && nvram_match("chilli_def_enable", "0")
	    && !nvram_match("hotss_enable", "1")) {
		nvram_unset("chilli_def_enable");
		nvram_set("chilli_enable", "0");
		return;
	}

	if (!nvram_match("chilli_enable", "1")
	    && !nvram_match("hotss_enable", "1")) {
		nvram_unset("chilli_def_enable");
		return;
	}

	if (nvram_match("hotss_enable", "1")) {
		stop_cron();
		if (!nvram_match("chilli_enable", "1")) {
			nvram_set("chilli_enable", "1");	// to get care of firewall, network, etc.
			nvram_set("chilli_def_enable", "0");
		}
		if (!nvram_match("hotss_preconfig", "1")) {
			nvram_set("hotss_preconfig", "1");
			sprintf(ssid, "HotSpotSystem.com-%s_%s",
				nvram_get("hotss_operatorid"),
				nvram_get("hotss_locationid"));
			nvram_set("wl0_ssid", ssid);
			nvram_set("time_zone", "+00");
			nvram_set("daylight_time", "1");
		}
		hotspotsys_config();
		start_cron();
	} else if (nvram_match("chilli_enable", "1")) {
		nvram_unset("chilli_def_enable");
		chilli_config();
	}
#else
	if (!nvram_match("chilli_enable", "1"))
		return;

	chilli_config();

#endif

	ret = killall("chilli", SIGTERM);
	ret = killall("chilli", SIGKILL);
	if (f_exists("/tmp/hotss.conf")) {
		ret = eval("chilli", "-c", "/tmp/hotss.conf");
		dd_syslog(LOG_INFO,
			  "hotspotsystem : chilli daemon successfully started\n");
	} else {
		ret = eval("chilli", "-c", "/tmp/chilli.conf");
		dd_syslog(LOG_INFO,
			  "chilli : chilli daemon successfully started\n");
	}

	cprintf("done\n");
	return;
}

void stop_chilli(void)
{
	if (stop_process("chilli", "chilli daemon")) {
		unlink("/tmp/chilli.conf");
		unlink("/tmp/hotss.conf");
	}
	cprintf("done\n");
	return;
}

void chilli_config(void)
{
	FILE *fp;
	int i;

#ifdef HAVE_CHILLILOCAL

	if (!(fp = fopen("/tmp/fonusers.local", "w"))) {
		perror("/tmp/fonusers.local");
		return;
	}
	char *users = nvram_safe_get("fon_userlist");
	char *u = (char *)malloc(strlen(users) + 1);
	char *o = u;

	strcpy(u, users);
	char *sep = strsep(&u, "=");

	while (sep != NULL) {
		fprintf(fp, "%s ", sep);
		char *pass = strsep(&u, " ");

		fprintf(fp, "%s \n", pass != NULL ? pass : "");
		sep = strsep(&u, "=");
	}
	free(o);
	fclose(fp);
#endif

	if (!(fp = fopen("/tmp/chilli.conf", "w"))) {
		perror("/tmp/chilli.conf");
		return;
	}

	fprintf(fp, "radiusserver1 %s\n", nvram_get("chilli_radius"));
	fprintf(fp, "radiusserver2 %s\n", nvram_get("chilli_backup"));
	fprintf(fp, "radiussecret %s\n", nvram_get("chilli_pass"));

	fprintf(fp, "dhcpif %s\n", nvram_safe_get("chilli_interface"));

	fprintf(fp, "uamserver %s\n", nvram_get("chilli_url"));
	if (nvram_invmatch("chilli_dns1", "0.0.0.0")
	    && nvram_invmatch("chilli_dns1", "")) {
		fprintf(fp, "dns1 %s\n", nvram_get("chilli_dns1"));
		if (nvram_invmatch("sv_localdns", "0.0.0.0")
		    && nvram_invmatch("sv_localdns", ""))
			fprintf(fp, "dns2 %s\n", nvram_get("sv_localdns"));
	} else if (nvram_invmatch("sv_localdns", "0.0.0.0")
		   && nvram_invmatch("sv_localdns", ""))
		fprintf(fp, "dns1 %s\n", nvram_get("sv_localdns"));

	if (nvram_invmatch("chilli_uamsecret", ""))
		fprintf(fp, "uamsecret %s\n", nvram_get("chilli_uamsecret"));
	if (nvram_invmatch("chilli_uamanydns", "0"))
		fprintf(fp, "uamanydns\n");
	if (nvram_invmatch("chilli_uamallowed", ""))
		fprintf(fp, "uamallowed %s\n", nvram_get("chilli_uamallowed"));
	if (nvram_invmatch("chilli_net", ""))
		fprintf(fp, "net %s\n", nvram_get("chilli_net"));
	if (nvram_match("chilli_macauth", "1"))
		fprintf(fp, "macauth\n");
#ifndef HAVE_FON
	if (nvram_match("fon_enable", "1")) {
#endif
		char hyp[32];

		strcpy(hyp, nvram_safe_get("wl0_hwaddr"));
		for (i = 0; i < strlen(hyp); i++)
			if (hyp[i] == ':')
				hyp[i] = '-';
		if (i > 0)
			fprintf(fp, "radiusnasid %s\n", hyp);
		nvram_set("chilli_radiusnasid", hyp);
		fprintf(fp, "interval 300\n");
#ifndef HAVE_FON
	} else {
		if (nvram_invmatch("chilli_radiusnasid", ""))
			fprintf(fp, "radiusnasid %s\n",
				nvram_get("chilli_radiusnasid"));
	}
#endif

	if (nvram_invmatch("chilli_additional", "")) {
		char *add = nvram_safe_get("chilli_additional");

		i = 0;
		do {
			if (add[i] != 0x0D)
				fprintf(fp, "%c", add[i]);
		}
		while (add[++i]);
		i = 0;
		int a = 0;
		char *filter = strdup(add);

		do {
			if (add[i] != 0x0D)
				filter[a++] = add[i];
		}
		while (add[++i]);

		filter[a] = 0;
		if (strcmp(filter, add)) {
			nvram_set("chilli_additional", filter);
			nvram_commit();
		}
		free(filter);
	}
	fflush(fp);
	fclose(fp);

	return;
}

#ifdef HAVE_HOTSPOT

void hotspotsys_config(void)
{
	FILE *fp;
	char *next;
	char var[64];
	char *dnslist;
	int i;
	
	md5_ctx_t MD;

	if (strlen(nvram_safe_get("hotss_remotekey")) != 12) {
		unsigned char hash[32];
		char *et0 = nvram_safe_get("et0macaddr");

		md5_begin(&MD);
		md5_hash(et0, 17, &MD);
		md5_end((unsigned char *)hash, &MD);
		char idkey[16];
		int i;

		for (i = 0; i < 6; i++)
			sprintf(&idkey[2 * i], "%02d", (hash[i] + hash[i+1]) % 100);
		idkey[12] = '\0';
		nvram_set("hotss_remotekey", idkey);
		nvram_commit();
		char sendid[256];
		sprintf(sendid, "/usr/bin/wget http://tech.hotspotsystem.com/up.php?mac=`nvram get wl0_hwaddr|sed s/:/-/g`\\&operator=%s\\&location=%s\\&remotekey=%s",
			nvram_get("hotss_operatorid"), nvram_get("hotss_locationid"), nvram_get("hotss_remotekey")); 
		system2(sendid);
	}

	if (!(fp = fopen("/tmp/hotss.conf", "w"))) {
		perror("/tmp/hotss.conf");
		return;
	}

	fprintf(fp, "radiusserver1 radius.hotspotsystem.com\n");
	fprintf(fp, "radiusserver2 radius2.hotspotsystem.com\n");
	fprintf(fp, "radiussecret hotsys123\n");

	fprintf(fp, "dhcpif %s\n", nvram_safe_get("hotss_interface"));
	if (nvram_invmatch("hotss_net", ""))
		fprintf(fp, "net %s\n", nvram_get("hotss_net"));

	char *uamdomain = "customer.hotspotsystem.com";
	if (!nvram_match("hotss_customuam", "")) {
		uamdomain = nvram_safe_get("hotss_customuam");
	}
	fprintf(fp,
		"uamserver %s://%s/customer/hotspotlogin.php\n",
		nvram_default_get("hotss_customuamproto", "https"), uamdomain);

	if (nvram_invmatch("wan_get_dns", "0.0.0.0")
	    && nvram_invmatch("wan_get_dns", "")) {
		dnslist = nvram_safe_get("wan_get_dns");
		i = 1;
		foreach(var, dnslist, next) {
			if (i > 2)
				break;
			fprintf(fp, "dns%d %s\n", i, var);
			i++;
		}
	} else if (nvram_invmatch("wan_dns", "0.0.0.0")
		   && nvram_invmatch("wan_dns", "")) {
		dnslist = nvram_safe_get("wan_dns");
		i = 1;
		foreach(var, dnslist, next) {
			if (i > 2)
				break;
			fprintf(fp, "dns%d %s\n", i, var);
			i++;
		}
	} else if (nvram_invmatch("sv_localdns", "0.0.0.0")
		   && nvram_invmatch("sv_localdns", "")) {
		fprintf(fp, "dns1 %s\n", nvram_get("sv_localdns"));
	}

	fprintf(fp, "uamsecret hotsys123\n");
	fprintf(fp, "uamanydns\n");

	fprintf(fp, "radiusnasid %s_%s\n", nvram_get("hotss_operatorid"),
		nvram_get("hotss_locationid"));
	if (!nvram_match("hotss_loginonsplash", "1")) {
		fprintf(fp,
			"uamhomepage %s://%s/customer/index.php?operator=%s&location=%s%s\n",
			nvram_safe_get("hotss_customuamproto"), uamdomain,
			nvram_get("hotss_operatorid"),
			nvram_get("hotss_locationid"),
			nvram_match("hotss_customsplash",
				    "1") ? "&forward=1" : "");
	}
	fprintf(fp, "coaport 3799\n");
	fprintf(fp, "coanoipcheck\n");
	fprintf(fp, "domain key.chillispot.info\n");

	if (nvram_invmatch("hotss_uamallowed", "")
	    && nvram_match("hotss_uamenable", "1"))
		fprintf(fp, "uamallowed %s\n", nvram_get("hotss_uamallowed"));

	fprintf(fp,
		"uamallowed hotspotsystem.com,%s,www.hotspotsystem.com,tech.hotspotsystem.com\n", uamdomain);
	fprintf(fp,
		"uamallowed 66.211.128.0/17,216.113.128.0/17\n");
	fprintf(fp,
		"uamallowed 70.42.128.0/17,128.242.125.0/24\n");
	fprintf(fp,
		"uamallowed 62.249.232.74,155.136.68.77,155.136.66.34,66.4.128.0/17,66.211.128.0/17,66.235.128.0/17\n");
	fprintf(fp,
		"uamallowed 88.221.136.146,195.228.254.149,195.228.254.152,203.211.140.157,203.211.150.204\n");
	fprintf(fp,
		"uamallowed 82.199.90.0/24,91.212.42.0/24\n");
	fprintf(fp,
		"uamallowed live.adyen.com\n");
	fprintf(fp,
		"uamallowed www.paypal.com,www.paypalobjects.com\n");
	fprintf(fp,
		"uamallowed www.worldpay.com,select.worldpay.com,secure.ims.worldpay.com,www.rbsworldpay.com,secure.wp3.rbsworldpay.com\n");
	fprintf(fp,
		"uamallowed a1.hotspotsystem.com,a2.hotspotsystem.com,a3.hotspotsystem.com,a4.hotspotsystem.com,a5.hotspotsystem.com,a6.hotspotsystem.com\n");
	fprintf(fp,
		"uamallowed a7.hotspotsystem.com,a8.hotspotsystem.com,a9.hotspotsystem.com,a10.hotspotsystem.com\n");

	fprintf(fp, "interval 300\n");

	fflush(fp);
	fclose(fp);

	return;
}

#endif				/* HAVE_HOTSPOT */
#endif				/* HAVE_CHILLI */
