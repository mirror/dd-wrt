/*
 * ipvs.c
 *
 * Copyright (C) 2005 - 2016 Sebastian Gottschall <gottschall@dd-wrt.com>
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
#ifdef HAVE_IPVS
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

void stop_ipvs(void);

void start_ipvs(void)
{
	char word[256];
	char *next, *wordlist;
	int first = 0;

	char tword[256];
	char *tnext, *twordlist;
	char *ipvsname, *sourceip, *sourceport, *scheduler, *targetip, *targetport, *matchname, *sourceproto, *targetweight, *targetnat;
	char *ipvs = nvram_safe_get("ipvs");
	char *ipvstarget = nvram_safe_get("ipvstarget");
	if (!strlen(ipvs) || !strlen(ipvstarget))
		return;
	wordlist = ipvs;
	foreach(word, wordlist, next) {
		sourceip = word;
		ipvsname = strsep(&sourceip, ">");
		sourceport = sourceip;
		sourceip = strsep(&sourceport, ">");
		scheduler = sourceport;
		sourceport = strsep(&scheduler, ">");
		sourceproto = scheduler;
		scheduler = strsep(&sourceproto, ">");
		if (!ipvsname || !sourceport || !sourceip || !scheduler || !sourceproto)
			break;
		if (!first) {
			first = 1;
			insmod("ipv6 ip_vs ip_ftp ip_pe_sip");
		}
		char modname[32];
		sprintf(modname, "ip_vs_%s", scheduler);	//build module name for scheduler implementation
		insmod(modname);
		char source[64];
		if (!strcasecmp(sourceip, "wan")) {
			sourceip = get_wan_ipaddr();
			if (strcmp(sourceport, "0")) {
				char net[32];
				sprintf(net, "%s/32", sourceip);
				eval("iptables", "-I", "INPUT", "-p", sourceproto, "--dport", sourceport, "-d", net, "-j", "ACCEPT");
			}
		}
		if (!strcasecmp(sourceip, "lan"))
			sourceip = nvram_safe_get("lan_ipaddr");
		snprintf(source, sizeof(source), "%s:%s", sourceip, sourceport);
		if (!strcmp(sourceproto, "tcp")) {
			if (!strcmp(sourceport, "0"))
				eval("ipvsadm", "-A", "-t", source, "-s", scheduler, "-p");
			else
				eval("ipvsadm", "-A", "-t", source, "-s", scheduler);
		} else if (!strcmp(sourceproto, "udp")) {
			if (!strcmp(sourceport, "0"))
				eval("ipvsadm", "-A", "-u", source, "-s", scheduler, "-p");
			else
				eval("ipvsadm", "-A", "-u", source, "-s", scheduler);
		} else if (!strcmp(sourceproto, "sip")) {
			insmod("nf_conntrack_sip");
			insmod("ip_vs_pe_sip");
			eval("ipvsadm", "-A", "-u", source, "-p", "60", "-M", "0.0.0.0", "-o", "--pe", "sip", "-s", scheduler);
		}
	}

	wordlist = ipvstarget;
	foreach(word, wordlist, next) {
		targetip = word;
		ipvsname = strsep(&targetip, ">");
		targetport = targetip;
		targetip = strsep(&targetport, ">");
		targetweight = targetport;
		targetport = strsep(&targetweight, ">");
		targetnat = targetweight;
		targetweight = strsep(&targetnat, ">");

		if (!ipvsname || !targetport || !targetip || !targetnat)
			break;
		twordlist = ipvs;
		int found = 0;
		foreach(tword, twordlist, tnext) {

			sourceip = tword;
			matchname = strsep(&sourceip, ">");
			sourceport = sourceip;
			sourceip = strsep(&sourceport, ">");
			scheduler = sourceport;
			sourceport = strsep(&scheduler, ">");
			sourceproto = scheduler;
			scheduler = strsep(&sourceproto, ">");
			if (!ipvsname || !sourceport || !sourceip || !scheduler || !sourceproto)
				break;

			if (!strcmp(matchname, ipvsname)) {
				found = 1;
				if (!strcasecmp(sourceip, "wan"))
					sourceip = get_wan_ipaddr();
				if (!strcasecmp(sourceip, "lan"))
					sourceip = nvram_safe_get("lan_ipaddr");
				break;
			}

		}
		if (found) {
			char source[64];
			snprintf(source, sizeof(source), "%s:%s", sourceip, sourceport);
			char target[64];
			snprintf(target, sizeof(target), "%s:%s", targetip, targetport);
			if (targetweight) {
				if (!strcmp(targetnat, "1"))
					eval("ipvsadm", "-a", "-t", source, "-r", target, "-m", "-w", targetweight);
				else
					eval("ipvsadm", "-a", "-t", source, "-r", target, "-g", "-w", targetweight);
			} else {
				if (!strcmp(targetnat, "1"))
					eval("ipvsadm", "-a", "-t", source, "-r", target, "-m");
				else
					eval("ipvsadm", "-a", "-t", source, "-r", target, "-g");

			}
		}
	}
	if (first) {
		eval("ipvsadm", "--start-daemon", nvram_default_get("ipvs_role", "master"), "--mcast-interface", nvram_safe_get("lan_ifname"));
		dd_syslog(LOG_INFO, "ipvs : IP Virtual Server successfully started\n");
	}
	return;
}

void stop_ipvs(void)
{
	eval("ipvsadm", "-C");
	dd_syslog(LOG_INFO, "ipvs : IP Virtual Server successfully stopped\n");
	return;
}

#endif
