/*
 * chillispot.c
 *
 * Copyright (C) 2007 Sebastian Gottschall <s.gottschall@dd-wrt.com>
 *						2013 Sash
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
#include <sys/stat.h>
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

static int jffs = 0;

#ifdef HAVE_CHILLI
#ifdef HAVE_HOTSPOT
void hotspotsys_config(void);
#endif
void main_config(void);
void chilli_config(void);
void stop_chilli(void);

static char log_accept[15];
static char log_drop[15];
static char log_reject[64];
#define TARG_PASS "ACCEPT"
#define TARG_RST "REJECT --reject-with tcp-reset"

void start_chilli(void)
{
	int ret = 0;
#ifdef HAVE_HOTSPOT
	char ssid[128];

	if (nvram_matchi("chilli_enable", 1) &&
	    nvram_matchi("chilli_def_enable", 0) &&
	    !nvram_matchi("hotss_enable", 1)) {
		nvram_unset("chilli_def_enable");
		nvram_seti("chilli_enable", 0);
		stop_chilli();
		return;
	}

	if (!nvram_matchi("chilli_enable", 1) &&
	    !nvram_matchi("hotss_enable", 1)) {
		nvram_unset("chilli_def_enable");
		stop_chilli();
		return;
	}
#else
	if (!nvram_matchi("chilli_enable", 1)) {
		stop_chilli();
		return;
	}
#endif
	insmod("tun");
	if ((nvram_matchi("usb_enable", 1) && nvram_matchi("usb_storage", 1) &&
	     nvram_matchi("usb_automnt", 1) &&
	     nvram_match("usb_mntpoint", "jffs")) ||
	    nvram_matchi("jffs_mounted", 1))
		jffs = 1;

	if (!*(nvram_safe_get("chilli_interface")))
		nvram_set("chilli_interface", get_wdev());
	if (!*(nvram_safe_get("hotss_interface")))
		nvram_set("hotss_interface", get_wdev());
	main_config();

#ifdef HAVE_HOTSPOT

	if (nvram_matchi("hotss_enable", 1)) {
		stop_cron();
		if (!nvram_matchi("chilli_enable", 1)) {
			nvram_seti("chilli_enable",
				   1); // to get care of firewall, network, etc.
			nvram_seti("chilli_def_enable", 0);
		}
		if (!nvram_matchi("hotss_preconfig", 1)) {
			nvram_seti("hotss_preconfig", 1);
			sprintf(ssid, "HotSpotSystem.com-%s_%s",
				nvram_safe_get("hotss_operatorid"),
				nvram_safe_get("hotss_locationid"));
			nvram_set("wl0_ssid", ssid);
		}
		hotspotsys_config();
		start_cron();
	} else if (nvram_matchi("chilli_enable", 1)) {
		nvram_unset("chilli_def_enable");
		chilli_config();
	}
#else
	chilli_config();

#endif
	if (!reload_process("chilli")) {
	} else if (f_exists("/tmp/chilli/hotss.conf")) {
#ifdef HAVE_COOVA_CHILLI
		putenv("CHILLISTATEDIR=/var/run/chilli1");
		mkdir("/var/run/chilli1", 0700);
		dd_logstart("chillispot",
			    eval("chilli", "--statedir=/var/run/chilli1",
				 "--pidfile=/var/run/chilli1/chilli.pid", "-c",
				 "/tmp/chilli/hotss.conf"));
#else
		dd_logstart("chillispot",
			    eval("chilli", "-c", "/tmp/chilli/hotss.conf"));
#endif
		dd_logstart("hotspotsystem", ret);
	} else {
#ifdef HAVE_COOVA_CHILLI
		putenv("CHILLISTATEDIR=/var/run/chilli1");
		mkdir("/var/run/chilli1", 0700);
		dd_logstart("chillispot",
			    eval("chilli", "--statedir=/var/run/chilli1",
				 "--pidfile=/var/run/chilli1/chilli.pid", "-c",
				 "/tmp/chilli/chilli.conf"));
#else
		dd_logstart("chillispot",
			    eval("chilli", "-c", "/tmp/chilli/chilli.conf"));
#endif
	}
#ifdef HAVE_TIEXTRA1
	start_mchilli();
#endif

	cprintf("done\n");
	return;
}

void restart_chilli(void)
{
	start_chilli();
}

void stop_chilli(void)
{
	if (stop_process("chilli", "daemon")) {
		unlink("/tmp/chilli/chilli.conf");
		unlink("/tmp/chilli/hotss.conf");
		unlink("/tmp/chilli/ip-up.sh");
		unlink("/tmp/chilli/ip-down.sh");
		unlink("/var/run/chilli1");
	}
	cprintf("done\n");
	return;
}

void main_config(void)
{
	int log_level = 0;
	char wan_if_buffer[33];

	FILE *fp;
	log_level = nvram_matchi("log_enable", "1") ? nvram_geti("log_level") :
						      0;
	mkdir("/tmp/chilli", 0700);

	if (!(fp = fopen("/tmp/chilli/ip-up.sh", "w"))) {
		perror("/tmp/chilli/ip-up.sh");
		return;
	}

	if (log_level >= 1)
		sprintf(log_drop, "%s", "logdrop");
	else
		sprintf(log_drop, "%s", "DROP");
	if (log_level >= 2)
		sprintf(log_accept, "%s", "logaccept");
	else
		sprintf(log_accept, "%s", TARG_PASS);
	if (log_level >= 1)
		sprintf(log_reject, "%s", "logreject");
	else
		sprintf(log_reject, "%s", TARG_RST);
	int hss_enable = nvram_matchi("hotss_enable", 1);
	int chilli_enable = nvram_matchi("chilli_enable", 1);

	/*	if we have a gw traffic will go there.
	but if we dont have any gw we might use chilli on a local network only 
	also we need to allow traffic in/outgoing to chilli*/
	fprintf(fp, "#!/bin/sh\n");
	fprintf(fp, "iptables -D INPUT -i $DEV -j %s\n", log_accept);
	fprintf(fp, "iptables -D FORWARD -i $DEV -j %s\n", log_accept);
	fprintf(fp, "iptables -D FORWARD -o $DEV -j %s\n", log_accept);
	fprintf(fp, "iptables -I INPUT -i $DEV -j %s\n", log_accept);
	fprintf(fp, "iptables -I FORWARD -i $DEV -j %s\n", log_accept);
	fprintf(fp, "iptables -I FORWARD -o $DEV -j %s\n", log_accept);
	//      secure chilli interface, only usefull if ! br0
	if (chilli_enable && !hss_enable &&
	    nvram_invmatch("chilli_interface", "br0")) {
		fprintf(fp,
			"iptables -t filter -D INPUT -i %s ! -s $NET/$MASK -j %s\n",
			nvram_safe_get("chilli_interface"), log_drop);
		fprintf(fp,
			"iptables -t filter -I INPUT -i %s ! -s $NET/$MASK -j %s\n",
			nvram_safe_get("chilli_interface"), log_drop);
	}
	if (chilli_enable && hss_enable &&
	    nvram_invmatch("hotss_interface", "br0")) {
		fprintf(fp,
			"iptables -t filter -D INPUT -i %s ! -s $NET/$MASK -j %s\n",
			nvram_safe_get("hotss_interface"), log_drop);
		fprintf(fp,
			"iptables -t filter -I INPUT -i %s ! -s $NET/$MASK -j %s\n",
			nvram_safe_get("hotss_interface"), log_drop);
	}
	// MASQUERADE chilli/hotss
	if (nvram_match("wan_proto", "disabled")) {
		//              fprintf(fp, "iptables -D FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n");
		fprintf(fp,
			"iptables -t nat -D POSTROUTING -s $NET/$MASK -j MASQUERADE\n");
		//              fprintf(fp, "iptables -I FORWARD 1 -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n");    // clamp when fw clamping is off
		fprintf(fp,
			"iptables -t nat -I POSTROUTING -s $NET/$MASK -j MASQUERADE\n");
	} else {
		fprintf(fp,
			"iptables -t nat -D POSTROUTING -o %s -s $NET/$MASK -j SNAT --to-source=%s\n",
			safe_get_wan_face(wan_if_buffer), get_wan_ipaddr());
		fprintf(fp,
			"iptables -t nat -I POSTROUTING -o %s -s $NET/$MASK -j SNAT --to-source=%s\n",
			safe_get_wan_face(wan_if_buffer), get_wan_ipaddr());
	}
	// enable Reverse Path Filtering to prevent double outgoing packages
	if (chilli_enable && !hss_enable) {
		fprintf(fp, "echo 1 > /proc/sys/net/ipv4/conf/%s/rp_filter\n",
			nvram_safe_get("chilli_interface"));
	}
	if (chilli_enable && hss_enable) {
		fprintf(fp, "echo 1 > /proc/sys/net/ipv4/conf/%s/rp_filter\n",
			nvram_safe_get("hotss_interface"));
	}
	fclose(fp);

	if (!(fp = fopen("/tmp/chilli/ip-down.sh", "w"))) {
		perror("/tmp/chilli/ip-down.sh");
		return;
	}

	fprintf(fp, "#!/bin/sh\n");
	fprintf(fp, "iptables -D INPUT -i $DEV -j %s\n", log_accept);
	fprintf(fp, "iptables -D FORWARD -i $DEV -j %s\n", log_accept);
	fprintf(fp, "iptables -D FORWARD -o $DEV -j %s\n", log_accept);
	if (nvram_matchi("chilli_enable", 1) &&
	    nvram_matchi("hotss_enable", 0) &&
	    nvram_invmatch("chilli_interface", "br0"))
		fprintf(fp,
			"iptables -t filter -D INPUT -i %s ! -s $NET/$MASK -j %s\n",
			nvram_safe_get("chilli_interface"), log_drop);
	if (nvram_matchi("chilli_enable", 1) &&
	    nvram_matchi("hotss_enable", 1) &&
	    nvram_invmatch("hotss_interface", "br0"))
		fprintf(fp,
			"iptables -t filter -D INPUT -i %s ! -s $NET/$MASK -j %s\n",
			nvram_safe_get("hotss_interface"), log_drop);
	if (nvram_match("wan_proto", "disabled")) {
		//              fprintf(fp, "iptables -D FORWARD -p tcp --tcp-flags SYN,RST SYN -j TCPMSS --clamp-mss-to-pmtu\n");
		fprintf(fp,
			"iptables -t nat -D POSTROUTING -s $NET/$MASK -j MASQUERADE\n");
	} else
		fprintf(fp,
			"iptables -t nat -D POSTROUTING -o %s -s $NET/$MASK -j SNAT --to-source=%s\n",
			safe_get_wan_face(wan_if_buffer), get_wan_ipaddr());
	fclose(fp);

	chmod("/tmp/chilli/ip-up.sh", 0700);
	chmod("/tmp/chilli/ip-down.sh", 0700);

	//  use usb/jffs for connection scripts if available
	if (jffs == 1) {
		mkdir("/jffs/etc", 0700);
		mkdir("/jffs/etc/chilli", 0700);
		if (!(fp = fopen("/jffs/etc/chilli/con-up.sh",
				 "r"))) { // dont overwrite
			fp = fopen("/jffs/etc/chilli/con-up.sh", "w");
			if (fp == NULL)
				return;
			fprintf(fp, "#!/bin/sh\n");
		}
		fclose(fp);
		if (!(fp = fopen("/jffs/etc/chilli/con-down.sh", "r"))) {
			fp = fopen("/jffs/etc/chilli/con-down.sh", "w");
			if (fp == NULL)
				return;
			fprintf(fp, "#!/bin/sh\n");
		}
		fclose(fp);
		chmod("/jffs/etc/chilli/con-up.sh", 0700);
		chmod("/jffs/etc/chilli/con-down.sh", 0700);
	}
}

void chilli_config(void)
{
	FILE *fp;
	int i;
	char *dnslist;
	char *next;
	char var[64];

#ifdef HAVE_CHILLILOCAL
	if (!(fp = fopen("/tmp/chilli/fonusers.local", "w"))) {
		perror("/tmp/chilli/fonusers.local");
		return;
	}
	char *users = nvram_safe_get("fon_userlist");
	char word[128];
	foreach(word, users, next)
	{
		char *pass = word;
		char *user = strsep(&pass, "=");
		if (user && pass)
			fprintf(fp, "%s:%s\n", user, pass);
	}
	fclose(fp);
#endif

	if (!(fp = fopen("/tmp/chilli/chilli.conf", "w"))) {
		perror("/tmp/chilli/chilli.conf");
		return;
	}
	fprintf(fp, "ipup /tmp/chilli/ip-up.sh\n");
	fprintf(fp, "ipdown /tmp/chilli/ip-down.sh\n");
	fprintf(fp, "radiusserver1 %s\n", nvram_safe_get("chilli_radius"));
	fprintf(fp, "radiusserver2 %s\n", nvram_safe_get("chilli_backup"));
	fprintf(fp, "radiussecret %s\n", nvram_safe_get("chilli_pass"));
	fprintf(fp, "dhcpif %s\n", nvram_safe_get("chilli_interface"));
	fprintf(fp, "uamserver %s\n", nvram_safe_get("chilli_url"));
	if (jffs == 1) {
		fprintf(fp, "conup /jffs/etc/chilli/con-up.sh\n");
		fprintf(fp, "condown /jffs/etc/chilli/con-down.sh\n");
	}
	//      if (strlen(nvram_safe_get("chilli_localusers")) > 0)
	//              localusers /tmp/chilli/localusers.db
	if (*(nvram_safe_get(
		    "fon_userlist"))) //only reuse it for testing. will be changed for better integration
		fprintf(fp, "localusers /tmp/chilli/fonusers.local\n");
	if (nvram_invmatch("chilli_dns1", "0.0.0.0") &&
	    nvram_invmatch("chilli_dns1", "")) {
		fprintf(fp, "dns1 %s\n", nvram_safe_get("chilli_dns1"));
		if (nvram_invmatch("sv_localdns", "0.0.0.0") &&
		    nvram_invmatch("sv_localdns", ""))
			fprintf(fp, "dns2 %s\n", nvram_safe_get("sv_localdns"));
	} else if (nvram_invmatch("wan_get_dns", "0.0.0.0") &&
		   nvram_invmatch("wan_get_dns", "")) {
		dnslist = nvram_safe_get("wan_get_dns");
		i = 1;
		foreach(var, dnslist, next)
		{
			if (i > 2)
				break;
			fprintf(fp, "dns%d %s\n", i, var);
			i++;
		}
	} else if (nvram_invmatch("wan_dns", "0.0.0.0") &&
		   nvram_invmatch("wan_dns", "")) {
		dnslist = nvram_safe_get("wan_dns");
		i = 1;
		foreach(var, dnslist, next)
		{
			if (i > 2)
				break;
			fprintf(fp, "dns%d %s\n", i, var);
			i++;
		}
	} else {
		if (nvram_invmatch("sv_localdns", "0.0.0.0") &&
		    nvram_invmatch("sv_localdns", ""))
			fprintf(fp, "dns1 %s\n", nvram_safe_get("sv_localdns"));
		if (nvram_invmatch("altdns1", "0.0.0.0") &&
		    nvram_invmatch("altdns1", ""))
			fprintf(fp, "dns2 %s\n", nvram_safe_get("altdns1"));
	}
	if (nvram_invmatch("chilli_uamsecret", ""))
		fprintf(fp, "uamsecret %s\n",
			nvram_safe_get("chilli_uamsecret"));
	if (nvram_invmatchi("chilli_uamanydns", 0))
		fprintf(fp, "uamanydns\n");
	if (nvram_invmatch("chilli_uamallowed", ""))
		fprintf(fp, "uamallowed %s\n",
			nvram_safe_get("chilli_uamallowed"));
#ifdef HAVE_COOVA_CHILLI
	if (nvram_invmatch("chilli_uamdomain", "")) {
		dnslist = nvram_safe_get("hotss_uamdomain");
		foreach(var, dnslist, next)
		{
			fprintf(fp, "uamdomain %s\n", var);
		}
	}
#endif
	if (nvram_invmatch("chilli_net", ""))
		fprintf(fp, "net %s\n", nvram_safe_get("chilli_net"));
	if (nvram_matchi("chilli_macauth", 1)) {
		fprintf(fp, "macauth\n");
		if (*(nvram_safe_get("chilli_macpasswd")))
			fprintf(fp, "macpasswd %s\n",
				nvram_safe_get("chilli_macpasswd"));
		else
			fprintf(fp, "macpasswd password\n");
	}
	if (nvram_matchi("chilli_802.1Xauth", 1))
		fprintf(fp, "eapolenable\n");

	if (nvram_invmatch("chilli_radiusnasid", ""))
		fprintf(fp, "radiusnasid %s\n",
			nvram_safe_get("chilli_radiusnasid"));

	if (nvram_invmatch("chilli_additional", "")) {
		char *add = nvram_safe_get("chilli_additional");

		i = 0;
		do {
			if (add[i] != 0x0D)
				fprintf(fp, "%c", add[i]);
		} while (add[++i]);
		i = 0;
		int a = 0;
		char *filter = strdup(add);

		do {
			if (add[i] != 0x0D)
				filter[a++] = add[i];
		} while (add[++i]);

		filter[a] = 0;
		if (strcmp(filter, add)) {
			nvram_set("chilli_additional", filter);
			nvram_async_commit();
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
		char et0[18];
		getLANMac(et0);
		if (!*et0)
			strcpy(et0, nvram_safe_get("et0macaddr_safe"));

		dd_md5_begin(&MD);
		dd_md5_hash(et0, 17, &MD);
		dd_md5_end((unsigned char *)hash, &MD);
		char idkey[16];
		int i;

		for (i = 0; i < 6; i++)
			snprintf(&idkey[2 * i], sizeof(idkey) - (2 * i), "%02d",
				 (hash[i] + hash[i + 1]) % 100);
		idkey[12] = '\0';
		nvram_set("hotss_remotekey", idkey);
		nvram_async_commit();
		char sendid[256];
		sprintf(sendid,
			"/usr/bin/wget http://tech.hotspotsystem.com/up.php?mac=`nvram get wl0_hwaddr|sed s/:/-/g`\\&operator=%s\\&location=%s\\&remotekey=%s",
			nvram_safe_get("hotss_operatorid"),
			nvram_safe_get("hotss_locationid"),
			nvram_safe_get("hotss_remotekey"));
		system(sendid);
	}

	if (!(fp = fopen("/tmp/chilli/hotss.conf", "w"))) {
		perror("/tmp/chilli/hotss.conf");
		return;
	}
	if (jffs == 1) {
		fprintf(fp, "conup /jffs/etc/chilli/con-up.sh\n");
		fprintf(fp, "condown /jffs/etc/chilli/con-down.sh\n");
	}
	fprintf(fp, "ipup /tmp/chilli/ip-up.sh\n");
	fprintf(fp, "ipdown /tmp/chilli/ip-down.sh\n");
	fprintf(fp, "radiusserver1 radius.hotspotsystem.com\n");
	fprintf(fp, "radiusserver2 radius2.hotspotsystem.com\n");
	fprintf(fp, "radiussecret hotsys123\n");
	fprintf(fp, "dhcpif %s\n", nvram_safe_get("hotss_interface"));
	if (nvram_invmatch("hotss_net", ""))
		fprintf(fp, "net %s\n", nvram_safe_get("hotss_net"));

	char *uamdomain = "customer.hotspotsystem.com";
	if (!nvram_match("hotss_customuam", "")) {
		uamdomain = nvram_safe_get("hotss_customuam");
	}
	fprintf(fp, "uamserver %s://%s/customer/hotspotlogin.php\n",
		nvram_default_get("hotss_customuamproto", "https"), uamdomain);

	if (nvram_invmatch("wan_get_dns", "0.0.0.0") &&
	    nvram_invmatch("wan_get_dns", "")) {
		dnslist = nvram_safe_get("wan_get_dns");
		i = 1;
		foreach(var, dnslist, next)
		{
			if (i > 2)
				break;
			fprintf(fp, "dns%d %s\n", i, var);
			i++;
		}
	} else if (nvram_invmatch("wan_dns", "0.0.0.0") &&
		   nvram_invmatch("wan_dns", "")) {
		dnslist = nvram_safe_get("wan_dns");
		i = 1;
		foreach(var, dnslist, next)
		{
			if (i > 2)
				break;
			fprintf(fp, "dns%d %s\n", i, var);
			i++;
		}
	} else if (nvram_invmatch("sv_localdns", "0.0.0.0") &&
		   nvram_invmatch("sv_localdns", "")) {
		fprintf(fp, "dns1 %s\n", nvram_safe_get("sv_localdns"));
		if (nvram_invmatch("altdns1", "0.0.0.0") &&
		    nvram_invmatch("altdns1", ""))
			fprintf(fp, "dns2 %s\n", nvram_safe_get("altdns1"));
	}
	fprintf(fp, "uamsecret hotsys123\n");
	fprintf(fp, "uamanydns\n");
	fprintf(fp, "radiusnasid %s_%s\n", nvram_safe_get("hotss_operatorid"),
		nvram_safe_get("hotss_locationid"));
	if (!nvram_matchi("hotss_loginonsplash", 1)) {
		fprintf(fp,
			"uamhomepage %s://%s/customer/index.php?operator=%s&location=%s%s\n",
			nvram_safe_get("hotss_customuamproto"), uamdomain,
			nvram_safe_get("hotss_operatorid"),
			nvram_safe_get("hotss_locationid"),
			nvram_matchi("hotss_customsplash", 1) ? "&forward=1" :
								"");
	}
	fprintf(fp, "coaport 3799\n");
	fprintf(fp, "coanoipcheck\n");
	fprintf(fp, "domain key.chillispot.info\n");

	if (nvram_invmatch("hotss_uamallowed", "") &&
	    nvram_matchi("hotss_uamenable", 1))
		fprintf(fp, "uamallowed %s\n",
			nvram_safe_get("hotss_uamallowed"));
#ifdef HAVE_COOVA_CHILLI
	if (nvram_invmatch("hotss_uamdomain", "") &&
	    nvram_matchi("hotss_uamenable", 1)) {
		dnslist = nvram_safe_get("hotss_uamdomain");
		foreach(var, dnslist, next)
		{
			fprintf(fp, "uamdomain %s\n", var);
		}
	}
#endif
	fprintf(fp, "uamallowed live.adyen.com,%s\n", uamdomain);
	fprintf(fp, "uamallowed 66.211.128.0/17,216.113.128.0/17\n");
	fprintf(fp, "uamallowed 70.42.128.0/17,128.242.125.0/24\n");
	fprintf(fp,
		"uamallowed 62.249.232.74,155.136.68.77,155.136.66.34,66.4.128.0/17,66.211.128.0/17,66.235.128.0/17\n");
	fprintf(fp,
		"uamallowed 88.221.136.146,195.228.254.149,195.228.254.152,203.211.140.157,203.211.150.204\n");
	fprintf(fp, "uamallowed 82.199.90.0/24,91.212.42.0/24\n");
#ifdef HAVE_COOVA_CHILLI
	fprintf(fp,
		"uamdomain paypal.com,paypalobjects.com,paypal-metrics.com\n");
	fprintf(fp, "uamdomain worldpay.com,rbsworldpay.com\n");
	fprintf(fp, "uamdomain mediaplex.com,hotspotsystem.com\n");
#else
	fprintf(fp, "uamallowed www.paypal.com,www.paypalobjects.com\n");
	fprintf(fp,
		"uamallowed www.worldpay.com,select.worldpay.com,secure.ims.worldpay.com,www.rbsworldpay.com,secure.wp3.rbsworldpay.com\n");
	fprintf(fp,
		"uamallowed hotspotsystem.com,www.hotspotsystem.com,tech.hotspotsystem.com\n");
	fprintf(fp,
		"uamallowed a1.hotspotsystem.com,a2.hotspotsystem.com,a3.hotspotsystem.com,a4.hotspotsystem.com,a5.hotspotsystem.com,a6.hotspotsystem.com\n");
	fprintf(fp,
		"uamallowed a7.hotspotsystem.com,a8.hotspotsystem.com,a9.hotspotsystem.com,a10.hotspotsystem.com,a11.hotspotsystem.com,a12.hotspotsystem.com\n");
	fprintf(fp,
		"uamallowed a13.hotspotsystem.com,a14.hotspotsystem.com,a15.hotspotsystem.com,a16.hotspotsystem.com,a17.hotspotsystem.com,a18.hotspotsystem.com\n");
	fprintf(fp,
		"uamallowed a19.hotspotsystem.com,a20.hotspotsystem.com,a21.hotspotsystem.com,a22.hotspotsystem.com,a23.hotspotsystem.com,a24.hotspotsystem.com\n");
	fprintf(fp,
		"uamallowed a25.hotspotsystem.com,a26.hotspotsystem.com,a27.hotspotsystem.com,a28.hotspotsystem.com,a29.hotspotsystem.com,a30.hotspotsystem.com\n");
#endif
	fprintf(fp, "interval 300\n");

	fflush(fp);
	fclose(fp);

	return;
}
#endif /* HAVE_HOTSPOT */
#endif /* HAVE_CHILLI */
