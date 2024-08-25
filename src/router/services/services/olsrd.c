/*
 * olsrd.c
 *
 * Copyright (C) 2007 - 2024 Sebastian Gottschall <s.gottschall@dd-wrt.com>
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

#ifdef HAVE_OLSRD
#include <stdlib.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <utils.h>
#include <syslog.h>
#include <signal.h>
#include <services.h>

void stop_olsrd(void)
{
	stop_process("olsrd", "daemon");
}

void start_olsrd(void)
{
	if (!nvram_match("wk_mode", "olsr"))
		return;
	stop_olsrd();
	char net[64];
	insmod("ipip");

	strcpy(net, nvram_safe_get("lan_ipaddr"));
	int a, b, c, d;

	sscanf(net, "%d.%d.%d.%d", &a, &b, &c, &d);
	sprintf(net, "%d.%d.%d.0", a, b, c);
	FILE *fp = fopen("/tmp/olsrd.conf", "wb");

	if (*(nvram_safe_get("olsrd_conf"))) {
		fwritenvram("olsrd_conf", fp);
	} else {
		fprintf(fp, "DebugLevel\t0\n");
		fprintf(fp, "IpVersion\t4\n");
		fprintf(fp, "AllowNoInt\tyes\n");
		fprintf(fp, "Pollrate\t%s\n", nvram_safe_get("olsrd_pollsize"));
		fprintf(fp, "TcRedundancy\t%s\n", nvram_safe_get("olsrd_redundancy"));
		fprintf(fp, "MprCoverage\t%s\n", nvram_safe_get("olsrd_coverage"));
		fprintf(fp, "MainIp %s\n", nvram_safe_get("lan_ipaddr"));
#ifdef HAVE_IPV6
		if (nvram_matchi("olsrd_smartgw", 1)) {
			nvram_seti("ipv6_enable", 1);
			start_ipv6(); // load ipv6 drivers
			fprintf(fp, "RtTable auto\n");
			fprintf(fp, "RtTableDefault auto\n");
			fprintf(fp, "RtTableTunnel auto\n");

			fprintf(fp, "RtTablePriority auto\n");
			fprintf(fp, "RtTableDefaultOlsrPriority auto\n");
			fprintf(fp, "RtTableTunnelPriority auto\n");
			fprintf(fp, "RtTableDefaultPriority auto\n");

			fprintf(fp, "SmartGateway yes\n");
			fprintf(fp, "SmartGatewayAllowNAT yes\n");
			fprintf(fp, "SmartGatewayUplink \"both\"\n");
			fprintf(fp, "SmartGatewayUplinkNAT yes\n");
			fprintf(fp, "SmartGatewaySpeed 128 1024\n");
			//                    fprintf(fp, "SmartGatewayPrefix 0::/0\n");
		} else {
#endif
			fprintf(fp, "SmartGateway no\n");
#ifdef HAVE_IPV6
		}
#endif
		fprintf(fp, "LinkQualityFishEye\t%s\n", nvram_safe_get("olsrd_lqfisheye"));
		fprintf(fp, "LinkQualityAging\t%s\n", nvram_safe_get("olsrd_lqaging"));
		fprintf(fp, "LinkQualityAlgorithm    \"etx_ff\"\n");
		//            fprintf(fp, "LinkQualityDijkstraLimit\t%s %s\n",
		//                    nvram_safe_get("olsrd_lqdijkstramin"),
		//                    nvram_safe_get("olsrd_lqdijkstramax"));
		fprintf(fp, "UseHysteresis\t%s\n", nvram_matchi("olsrd_hysteresis", 1) ? "yes" : "no");
		if (nvram_matchi("olsrd_hysteresis", 0))
			fprintf(fp, "LinkQualityLevel\t%s\n", nvram_safe_get("olsrd_lqlevel"));
		else
			fprintf(fp, "LinkQualityLevel\t0\n");
		fprintf(fp, "LoadPlugin \"olsrd_dyn_gw.so\"\n");
		fprintf(fp, "{\n");
		fprintf(fp, "\tPlParam \"ping\"\t\"8.8.8.8\"");
		fprintf(fp, "\tPlParam \"ping\"\t\"8.8.4.4\"");
		fprintf(fp, "\tPlParam \"ping\"\t\"141.1.1.1\"");
		fprintf(fp, "}\n");
#ifndef HAVE_MICRO
		fprintf(fp, "LoadPlugin \"olsrd_httpinfo.so\"\n");
		fprintf(fp, "{\n");
		fprintf(fp, "\tPlParam \"port\"\t\"8080\"\n");
		fprintf(fp, "\tPlParam \"Host\"\t\"127.0.0.1\"\n");
		fprintf(fp, "\tPlParam \"Net\"\t\"%s 255.255.255.0\"\n", net);
		fprintf(fp, "}\n");
#endif
		fprintf(fp, "IpcConnect\n");
		fprintf(fp, "{\n");
		fprintf(fp, "\tMaxConnections\t1\n");
		fprintf(fp, "\tHost\t127.0.0.1\n");
		fprintf(fp, "\tNet\t%s 255.255.255.0\n", net);
		fprintf(fp, "}\n");

		char *wordlist = nvram_safe_get("olsrd_interfaces");
		char *next;
		char word[128];

		foreach(word, wordlist, next)
		{
			GETENTRYBYIDX(interface, word, 0);
			GETENTRYBYIDX(hellointerval, word, 1);
			GETENTRYBYIDX(hellovaliditytime, word, 2);
			GETENTRYBYIDX(tcinterval, word, 3);
			GETENTRYBYIDX(tcvaliditytime, word, 4);
			GETENTRYBYIDX(midinterval, word, 5);
			GETENTRYBYIDX(midvaliditytime, word, 6);
			GETENTRYBYIDX(hnainterval, word, 7);
			GETENTRYBYIDX(hnavaliditytime, word, 8);
			GETENTRYBYIDX(linkqualitymult, word, 9);
			fprintf(fp, "Interface \"%s\"\n", interface);
			fprintf(fp, "{\n");
			fprintf(fp, "\tHelloInterval\t%s\n", hellointerval);
			fprintf(fp, "\tHelloValidityTime\t%s\n", hellovaliditytime);
			fprintf(fp, "\tTcInterval\t%s\n", tcinterval);
			fprintf(fp, "\tTcValidityTime\t%s\n", tcvaliditytime);
			fprintf(fp, "\tMidInterval\t%s\n", midinterval);
			fprintf(fp, "\tMidValidityTime\t%s\n", midvaliditytime);
			fprintf(fp, "\tHnaInterval\t%s\n", hnainterval);
			fprintf(fp, "\tHnaValidityTime\t%s\n", hnavaliditytime);
			fprintf(fp, "\tLinkQualityMult\tdefault\t%s\n", linkqualitymult);
			fprintf(fp, "}\n");
		}
		if (*(nvram_safe_get("olsrd_hna"))) {
			fprintf(fp, "Hna4{\n");
			fprintf(fp, "%s\n", nvram_safe_get("olsrd_hna"));
			fprintf(fp, "}\n");
		}
	}
	fclose(fp);
	log_eval("olsrd");
}

#endif
