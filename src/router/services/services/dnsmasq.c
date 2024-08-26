/*
 * dnsmasq.c
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
#ifdef HAVE_DNSMASQ
#include <netinet/in.h>
#include <arpa/inet.h>
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
extern void genHosts(void);

static void add_server(FILE *fp, char *server)
{
	fprintf(fp, "server=/%s/\n", server);
}

static void add_ubnt_telemetry(FILE *fp)
{
	static char *servers[] = {
		"trace.svc.ui.com",
		"trace.dev.svc.ui.com",
		"trace.stg.svc.ui.com",
		"crash-report-service.svc.ui.com",
		"crash-report-service.stg.svc.ui.com",
		"crash-report-service.dev.svc.ui.com",
	};
	int i;
	for (i = 0; i < sizeof(servers) / sizeof(char *); i++)
		add_server(fp, servers[i]);
}

static void add_ms_telemetry(FILE *fp)
{
	static char *servers[] = {
		"1oavsblobprodcus350.blob.core.windows.net",
		"37bvsblobprodcus311.blob.core.windows.net",
		"a.ads1.msn.com",
		"a.ads2.msads.net",
		"a.ads2.msn.com",
		"a.rad.msn.com",
		"ac3.msn.com",
		"adnexus.net",
		"adnxs.com",
		"ads.msn.com",
		"ads1.msads.net",
		"ads1.msn.com",
		"aidps.atdmt.com",
		"aka-cdn-ns.adtech.de",
		"alpha.telemetry.microsoft.com",
		"api.cortana.ai",
		"api.edgeoffer.microsoft.com",
		"asimov-win.settings.data.microsoft.com.akadns.net",
		"azwancan.trafficmanager.net",
		"b.ads1.msn.com",
		"b.ads2.msads.net",
		"b.rad.msn.com",
		"bingads.microsoft.com",
		"blobcollector.events.data.trafficmanager.net",
		"bn2-ris-ap-prod-atm.trafficmanager.net",
		"bn2-ris-prod-atm.trafficmanager.net",
		"bn2wns1.wns.windows.com",
		"bn3sch020010558.wns.windows.com",
		"bn3sch020010560.wns.windows.com",
		"bn3sch020010618.wns.windows.com",
		"bn3sch020010629.wns.windows.com",
		"bn3sch020010631.wns.windows.com",
		"bn3sch020010635.wns.windows.com",
		"bn3sch020010636.wns.windows.com",
		"bn3sch020010650.wns.windows.com",
		"bn3sch020011727.wns.windows.com",
		"bn3sch020012850.wns.windows.com",
		"bn3sch020020322.wns.windows.com",
		"bn3sch020020749.wns.windows.com",
		"bn3sch020022328.wns.windows.com",
		"bn3sch020022335.wns.windows.com",
		"bn3sch020022361.wns.windows.com",
		"bn4sch101120814.wns.windows.com",
		"bn4sch101120818.wns.windows.com",
		"bn4sch101120911.wns.windows.com",
		"bn4sch101120913.wns.windows.com",
		"bn4sch101121019.wns.windows.com",
		"bn4sch101121109.wns.windows.com",
		"bn4sch101121118.wns.windows.com",
		"bn4sch101121223.wns.windows.com",
		"bn4sch101121407.wns.windows.com",
		"bn4sch101121618.wns.windows.com",
		"bn4sch101121704.wns.windows.com",
		"bn4sch101121709.wns.windows.com",
		"bn4sch101121714.wns.windows.com",
		"bn4sch101121908.wns.windows.com",
		"bn4sch101122117.wns.windows.com",
		"bn4sch101122310.wns.windows.com",
		"bn4sch101122312.wns.windows.com",
		"bn4sch101122421.wns.windows.com",
		"bn4sch101123108.wns.windows.com",
		"bn4sch101123110.wns.windows.com",
		"bn4sch101123202.wns.windows.com",
		"bn4sch102110124.wns.windows.com",
		"browser.pipe.aria.microsoft.com",
		"bs.serving-sys.com",
		"c.atdmt.com",
		"c.msn.com",
		"ca.telemetry.microsoft.com",
		"cache.datamart.windows.com",
		"cdn.atdmt.com",
		"cds1.stn.llnw.net",
		"cds10.stn.llnw.net",
		"cds1203.lon.llnw.net",
		"cds1204.lon.llnw.net",
		"cds1209.lon.llnw.net",
		"cds1219.lon.llnw.net",
		"cds1228.lon.llnw.net",
		"cds1244.lon.llnw.net",
		"cds1257.lon.llnw.net",
		"cds1265.lon.llnw.net",
		"cds1269.lon.llnw.net",
		"cds1273.lon.llnw.net",
		"cds1285.lon.llnw.net",
		"cds1287.lon.llnw.net",
		"cds1289.lon.llnw.net",
		"cds1293.lon.llnw.net",
		"cds1307.lon.llnw.net",
		"cds1310.lon.llnw.net",
		"cds1325.lon.llnw.net",
		"cds1327.lon.llnw.net",
		"cds177.dus.llnw.net",
		"cds20005.stn.llnw.net",
		"cds20404.lcy.llnw.net",
		"cds20411.lcy.llnw.net",
		"cds20415.lcy.llnw.net",
		"cds20416.lcy.llnw.net",
		"cds20417.lcy.llnw.net",
		"cds20424.lcy.llnw.net",
		"cds20425.lcy.llnw.net",
		"cds20431.lcy.llnw.net",
		"cds20435.lcy.llnw.net",
		"cds20440.lcy.llnw.net",
		"cds20443.lcy.llnw.net",
		"cds20445.lcy.llnw.net",
		"cds20450.lcy.llnw.net",
		"cds20452.lcy.llnw.net",
		"cds20457.lcy.llnw.net",
		"cds20461.lcy.llnw.net",
		"cds20469.lcy.llnw.net",
		"cds20475.lcy.llnw.net",
		"cds20482.lcy.llnw.net",
		"cds20485.lcy.llnw.net",
		"cds20495.lcy.llnw.net",
		"cds21205.lon.llnw.net",
		"cds21207.lon.llnw.net",
		"cds21225.lon.llnw.net",
		"cds21229.lon.llnw.net",
		"cds21233.lon.llnw.net",
		"cds21238.lon.llnw.net",
		"cds21244.lon.llnw.net",
		"cds21249.lon.llnw.net",
		"cds21256.lon.llnw.net",
		"cds21257.lon.llnw.net",
		"cds21258.lon.llnw.net",
		"cds21261.lon.llnw.net",
		"cds21267.lon.llnw.net",
		"cds21278.lon.llnw.net",
		"cds21281.lon.llnw.net",
		"cds21293.lon.llnw.net",
		"cds21309.lon.llnw.net",
		"cds21313.lon.llnw.net",
		"cds21321.lon.llnw.net",
		"cds27.ory.llnw.net",
		"cds299.lcy.llnw.net",
		"cds30027.stn.llnw.net",
		"cds308.lcy.llnw.net",
		"cds310.lcy.llnw.net",
		"cds38.ory.llnw.net",
		"cds405.lcy.llnw.net",
		"cds406.lcy.llnw.net",
		"cds407.fra.llnw.net",
		"cds416.lcy.llnw.net",
		"cds421.lcy.llnw.net",
		"cds422.lcy.llnw.net",
		"cds425.lcy.llnw.net",
		"cds426.lcy.llnw.net",
		"cds447.lcy.llnw.net",
		"cds458.lcy.llnw.net",
		"cds459.lcy.llnw.net",
		"cds46.ory.llnw.net",
		"cds461.lcy.llnw.net",
		"cds468.lcy.llnw.net",
		"cds469.lcy.llnw.net",
		"cds471.lcy.llnw.net",
		"cds483.lcy.llnw.net",
		"cds484.lcy.llnw.net",
		"cds489.lcy.llnw.net",
		"cds493.lcy.llnw.net",
		"cds494.lcy.llnw.net",
		"cds54.ory.llnw.net",
		"cds812.lon.llnw.net",
		"cds815.lon.llnw.net",
		"cds818.lon.llnw.net",
		"cds832.lon.llnw.net",
		"cds836.lon.llnw.net",
		"cds840.lon.llnw.net",
		"cds843.lon.llnw.net",
		"cds857.lon.llnw.net",
		"cds868.lon.llnw.net",
		"cds869.lon.llnw.net",
		"ceuswatcab01.blob.core.windows.net",
		"ceuswatcab02.blob.core.windows.net",
		"compatexchange1.trafficmanager.net",
		"corp.sts.microsoft.com",
		"corpext.msitadfs.glbdns2.microsoft.com",
		"cs1.wpc.v0cdn.net",
		"cy2.vortex.data.microsoft.com.akadns.net",
		"db3aqu.atdmt.com",
		"db5-eap.settings-win.data.microsoft.com.akadns.net",
		"db5.settings-win.data.microsoft.com.akadns.net",
		"db5.settings.data.microsoft.com.akadns.net",
		"db5.vortex.data.microsoft.com.akadns.net",
		"df.telemetry.microsoft.com",
		"diagnostics.support.microsoft.com",
		"eaus2watcab01.blob.core.windows.net",
		"eaus2watcab02.blob.core.windows.net",
		"ec.atdmt.com",
		"flex.msn.com",
		"g.msn.com",
		"geo.settings-win.data.microsoft.com.akadns.net",
		"geo.settings.data.microsoft.com.akadns.net",
		"geo.vortex.data.microsoft.com.akadns.net",
		"h1.msn.com",
		"h2.msn.com",
		"hk2.settings.data.microsoft.com.akadns.net",
		"hk2.wns.windows.com",
		"hk2sch130020721.wns.windows.com",
		"hk2sch130020723.wns.windows.com",
		"hk2sch130020726.wns.windows.com",
		"hk2sch130020729.wns.windows.com",
		"hk2sch130020732.wns.windows.com",
		"hk2sch130020824.wns.windows.com",
		"hk2sch130020843.wns.windows.com",
		"hk2sch130020851.wns.windows.com",
		"hk2sch130020854.wns.windows.com",
		"hk2sch130020855.wns.windows.com",
		"hk2sch130020924.wns.windows.com",
		"hk2sch130020936.wns.windows.com",
		"hk2sch130020940.wns.windows.com",
		"hk2sch130020956.wns.windows.com",
		"hk2sch130020958.wns.windows.com",
		"hk2sch130020961.wns.windows.com",
		"hk2sch130021017.wns.windows.com",
		"hk2sch130021029.wns.windows.com",
		"hk2sch130021035.wns.windows.com",
		"hk2sch130021137.wns.windows.com",
		"hk2sch130021142.wns.windows.com",
		"hk2sch130021153.wns.windows.com",
		"hk2sch130021217.wns.windows.com",
		"hk2sch130021246.wns.windows.com",
		"hk2sch130021249.wns.windows.com",
		"hk2sch130021260.wns.windows.com",
		"hk2sch130021264.wns.windows.com",
		"hk2sch130021322.wns.windows.com",
		"hk2sch130021323.wns.windows.com",
		"hk2sch130021329.wns.windows.com",
		"hk2sch130021334.wns.windows.com",
		"hk2sch130021360.wns.windows.com",
		"hk2sch130021432.wns.windows.com",
		"hk2sch130021433.wns.windows.com",
		"hk2sch130021435.wns.windows.com",
		"hk2sch130021437.wns.windows.com",
		"hk2sch130021440.wns.windows.com",
		"hk2sch130021450.wns.windows.com",
		"hk2sch130021518.wns.windows.com",
		"hk2sch130021523.wns.windows.com",
		"hk2sch130021526.wns.windows.com",
		"hk2sch130021527.wns.windows.com",
		"hk2sch130021544.wns.windows.com",
		"hk2sch130021554.wns.windows.com",
		"hk2sch130021618.wns.windows.com",
		"hk2sch130021634.wns.windows.com",
		"hk2sch130021638.wns.windows.com",
		"hk2sch130021646.wns.windows.com",
		"hk2sch130021652.wns.windows.com",
		"hk2sch130021654.wns.windows.com",
		"hk2sch130021657.wns.windows.com",
		"hk2sch130021723.wns.windows.com",
		"hk2sch130021726.wns.windows.com",
		"hk2sch130021727.wns.windows.com",
		"hk2sch130021730.wns.windows.com",
		"hk2sch130021731.wns.windows.com",
		"hk2sch130021754.wns.windows.com",
		"hk2sch130021829.wns.windows.com",
		"hk2sch130021830.wns.windows.com",
		"hk2sch130021833.wns.windows.com",
		"hk2sch130021840.wns.windows.com",
		"hk2sch130021842.wns.windows.com",
		"hk2sch130021851.wns.windows.com",
		"hk2sch130021852.wns.windows.com",
		"hk2sch130021927.wns.windows.com",
		"hk2sch130021928.wns.windows.com",
		"hk2sch130021929.wns.windows.com",
		"hk2sch130021958.wns.windows.com",
		"hk2sch130022035.wns.windows.com",
		"hk2sch130022041.wns.windows.com",
		"hk2sch130022049.wns.windows.com",
		"hk2sch130022135.wns.windows.com",
		"hk2wns1.wns.windows.com",
		"hk2wns1b.wns.windows.com",
		"ieonlinews.microsoft.com",
		"ieonlinews.trafficmanager.net",
		"insideruser.trafficmanager.net",
		"kmwatson.events.data.microsoft.com",
		"kmwatsonc.events.data.microsoft.com",
		"lb1.www.ms.akadns.net",
		"live.rads.msn.com",
		"m.adnxs.com",
		"mobile.pipe.aria.microsoft.com",
		"modern.watson.data.microsoft.com.akadns.net",
		"msedge.net",
		"msntest.serving-sys.com",
		"nexus.officeapps.live.com",
		"nexusrules.officeapps.live.com",
		"nw-umwatson.events.data.microsoft.com",
		"oca.telemetry.microsoft.com",
		"oca.telemetry.microsoft.us",
		"onecollector.cloudapp.aria.akadns.net",
		"par02p.wns.windows.com",
		"pre.footprintpredict.com",
		"presence.teams.live.com",
		"preview.msn.com",
		"rad.live.com",
		"rad.msn.com",
		"redir.metaservices.microsoft.com",
		"reports.wes.df.telemetry.microsoft.com",
		"romeccs.microsoft.com",
		"schemas.microsoft.akadns.net",
		"secure.adnxs.com",
		"secure.flashtalking.com",
		"services.wes.df.telemetry.microsoft.com",
		"settings-sandbox.data.microsoft.com",
		"settings-win-ppe.data.microsoft.com",
		"settings.data.glbdns2.microsoft.com",
		"settingsfd-geo.trafficmanager.net",
		"sg2p.wns.windows.com",
		"spynet2.microsoft.com",
		"spynetalt.microsoft.com",
		"spyneteurope.microsoft.akadns.net",
		"sqm.df.telemetry.microsoft.com",
		"sqm.telemetry.microsoft.com",
		"sqm.telemetry.microsoft.com.nsatc.net",
		"ssw.live.com",
		"survey.watson.microsoft.com",
		"tele.trafficmanager.net",
		"telecommand.telemetry.microsoft.com",
		"telemetry.appex.bing.net",
		"telemetry.microsoft.com",
		"telemetry.remoteapp.windowsazure.com",
		"telemetry.urs.microsoft.com",
		"teredo.ipv6.microsoft.com",
		"test.activity.windows.com",
		"uks.b.prd.ags.trafficmanager.net",
		"umwatson.events.data.microsoft.com",
		"umwatsonc.events.data.microsoft.com",
		"umwatsonc.telemetry.microsoft.us",
		"v10-win.vortex.data.microsoft.com.akadns.net",
		"v10.vortex-win.data.microsoft.com",
		"v20.vortex-win.data.microsoft.com",
		"view.atdmt.com",
		"vortex-sandbox.data.microsoft.com",
		"vortex.data.glbdns2.microsoft.com",
		"vortex.data.microsoft.com",
		"watson.live.com",
		"watson.microsoft.com",
		"watson.ppe.telemetry.microsoft.com",
		"watson.telemetry.microsoft.com",
		"web.vortex.data.microsoft.com",
		"wes.df.telemetry.microsoft.com",
		"weus2watcab01.blob.core.windows.net",
		"weus2watcab02.blob.core.windows.net",
		"win10.ipv6.microsoft.com",
		"win1710.ipv6.microsoft.com",
		"win8.ipv6.microsoft.com",
		"xblgdvrassets3010.blob.core.windows.net",
		"ztd.dds.microsoft.com",
		"au-v10.events.data.microsoft.com",
		"au-v20.events.data.microsoft.com",
		"au.vortex-win.data.microsoft.com",
		"de-v20.events.data.microsoft.com",
		"de.vortex-win.data.microsoft.com",
		"eu-v10.events.data.microsoft.com",
		"eu-v20.events.data.microsoft.com",
		"eu.vortex-win.data.microsoft.com",
		"events-sandbox.data.microsoft.com",
		"events.data.microsoft.com",
		"jp-v10.events.data.microsoft.com",
		"jp-v20.events.data.microsoft.com",
		"settings-win.data.microsoft.com",
		"uk-v20.events.data.microsoft.com",
		"uk.vortex-win.data.microsoft.com",
		"us-v10.events.data.microsoft.com",
		"us-v20.events.data.microsoft.com",
		"us.vortex-win.data.microsoft.com",
		"us4-v20.events.data.microsoft.com",
		"us5-v20.events.data.microsoft.com",
		"v10.events.data.microsoft.com",
		"v20.events.data.microsoft.com",
		"vortex-win-sandbox.data.microsoft.com",
		"vortex-win.data.microsoft.com",

	};
	int i;
	for (i = 0; i < sizeof(servers) / sizeof(char *); i++)
		add_server(fp, servers[i]);
}

void stop_dnsmasq(void);
#define IDX_IFNAME 0
#define IDX_DHCPON 1
#define IDX_LEASESTART 2
#define IDX_LEASEMAX 3
#define IDX_LEASETIME 4

char *getmdhcp(int count, int index, char *buffer)
{
	int cnt = 0;
	char *next, *wordlist;
	char word[128];
	wordlist = nvram_safe_get("mdhcpd");
	foreach(word, wordlist, next)
	{
		if (cnt < index) {
			cnt++;
			continue;
		}
		GETENTRYBYIDX(interface, word, 0);
		GETENTRYBYIDX(dhcpon, word, 1);
		GETENTRYBYIDX(start, word, 2);
		GETENTRYBYIDX(max, word, 3);
		GETENTRYBYIDX(leasetime, word, 4);
		if (leasetime == NULL) {
			leasetime = "3660";
		}
		switch (count) {
		case IDX_IFNAME:
			strcpy(buffer, interface);
			return buffer;
		case IDX_DHCPON:
			strcpy(buffer, dhcpon);
			return buffer;
		case IDX_LEASESTART:
			strcpy(buffer, start);
			return buffer;
		case IDX_LEASEMAX:
			strcpy(buffer, max);
			return buffer;
		case IDX_LEASETIME:
			strcpy(buffer, leasetime);
			return buffer;
		}
	}
	return "";
}

static int landhcp(void)
{
	if (!getWET())
		if (nvram_match("lan_proto", "dhcp") && nvram_matchi("dhcpfwd_enable", 0))
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

static int canlan(void)
{
	if ((nvram_matchi("dhcpfwd_enable", 0) || nvram_matchi("dns_crypt", 1)))
		return 1;
	return 0;
}

static void makeentry(FILE *fp, char *ifname, int dhcpnum, int dhcpstart, char *ip, char *netmask, char *leasetime)
{
	unsigned int ip1 = get_single_ip(ip, 0);
	unsigned int ip2 = get_single_ip(ip, 1);
	unsigned int ip3 = get_single_ip(ip, 2);
	unsigned int ip4 = get_single_ip(ip, 3);
	unsigned int im1 = get_single_ip(netmask, 0);
	unsigned int im2 = get_single_ip(netmask, 1);
	unsigned int im3 = get_single_ip(netmask, 2);
	unsigned int im4 = get_single_ip(netmask, 3);
	unsigned int sip = ((ip1 & im1) << 24) + ((ip2 & im2) << 16) + ((ip3 & im3) << 8) + dhcpstart;
	unsigned int eip = sip + dhcpnum - 1;

	fprintf(fp, "dhcp-range=%s,", ifname);

	fprintf(fp, "%d.%d.%d.%d,", ip1 & im1, ip2 & im2, ip3 & im3, dhcpstart);
	fprintf(fp, "%d.%d.%d.%d,", (eip >> 24) & 0xff, (eip >> 16) & 0xff, (eip >> 8) & 0xff, eip & 0xff);
	fprintf(fp, "%s,", netmask);
	fprintf(fp, "%sm\n", leasetime);
}

static void makeentry_full(FILE *fp, char *ifname, int dhcpnum, char *ip, char *netmask, char *leasetime)
{
	unsigned int ip1 = get_single_ip(ip, 0);
	unsigned int ip2 = get_single_ip(ip, 1);
	unsigned int ip3 = get_single_ip(ip, 2);
	unsigned int ip4 = get_single_ip(ip, 3);
	unsigned int im1 = 255;
	unsigned int im2 = 255;
	unsigned int im3 = 255;
	unsigned int im4 = 255;
	unsigned int sip = ((ip1 & im1) << 24) + ((ip2 & im2) << 16) + ((ip3 & im3) << 8) + ip4;
	unsigned int eip = sip + dhcpnum - 1;

	fprintf(fp, "dhcp-range=%s,", ifname);

	fprintf(fp, "%d.%d.%d.%d,", ip1 & im1, ip2 & im2, ip3 & im3, ip4);
	fprintf(fp, "%d.%d.%d.%d,", (eip >> 24) & 0xff, (eip >> 16) & 0xff, (eip >> 8) & 0xff, eip & 0xff);
	fprintf(fp, "%s,", netmask);
	fprintf(fp, "%sm\n", leasetime);
}

void start_dnsmasq(void)
{
	char path[64];
	FILE *fp;
	char wan_if_buffer[33];
	//struct dns_lists *dns_list = NULL;
	struct dns_lists *dns_list6 = NULL;
	int i;

	if (nvram_match("lan_proto", "dhcp") && nvram_matchi("dnsmasq_enable", 0)) {
		nvram_seti("dnsmasq_enable", 1);
		nvram_async_commit();
	}

	if (!nvram_matchi("dnsmasq_enable", 1)) {
		stop_dnsmasq();
		return;
	}
#ifdef HAVE_SMARTDNS
	start_smartdns();
#endif
	int leasechange = nvram_state_change("static_leases");

	update_timezone();

#ifdef HAVE_DNSCRYPT
	if (nvram_matchi("dns_crypt", 1)) {
		stop_process("dnscrypt-proxy", "daemon");
		log_eval("dnscrypt-proxy", "-d", "-S", "-a", "127.0.0.1:30", "-R", nvram_safe_get("dns_crypt_resolver"), "-L",
			 "/etc/dnscrypt/dnscrypt-resolvers.csv");
	}
#endif
	usejffs = 0;

	if (jffs_mounted() && nvram_matchi("dhcpd_usejffs", 1)) {
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
	if (nvram_matchi("chilli_enable", 1) || nvram_matchi("hotss_enable", 1)) {
		char *chilliif;
#ifdef HAVE_HOTSPOT
		if (nvram_matchi("hotss_enable", 1))
			chilliif = nvram_safe_get("hotss_interface");
		else
#endif
			chilliif = nvram_safe_get("chilli_interface");

		fprintf(fp, "interface=%s", chilliif);
		if (!canlan()) {
			fprintf(fp, ",");
		} else {
			if (strcmp(chilliif, nvram_safe_get("lan_ifname"))) {
				fprintf(fp, ",%s", nvram_safe_get("lan_ifname"));
			}
		}
	} else if (nvram_matchi("pptpd_enable", 1)) {
		fprintf(fp, "listen-address=127.0.0.1");
		if (canlan()) {
			fprintf(fp, ",%s", nvram_safe_get("lan_ipaddr"));
#ifdef HAVE_IPV6
			char buf[INET6_ADDRSTRLEN];
			char *ip = getifaddr_any(buf, nvram_safe_get("lan_ifname"), AF_INET6) ?: NULL;
			if (ip && nvram_matchi("ipv6_enable", 1))
				fprintf(fp, ",%s", ip);
#endif
		}
		char vifs[256];
		getIfLists(vifs, 256);
		char var[256], *wordlist, *next;
		foreach(var, vifs, next)
		{
			if (strcmp(safe_get_wan_face(wan_if_buffer), var) && strcmp(nvram_safe_get("lan_ifname"), var)) {
				char *ipaddr = nvram_nget("%s_ipaddr", var);
				if (*ipaddr && strcmp(ipaddr, "0.0.0.0"))
					fprintf(fp, ",%s", ipaddr);
			}
		}

		if (nvram_exists("dnsmasq_addlisten")) {
			fprintf(fp, ",%s", nvram_safe_get("dnsmasq_addlisten"));
		}
	} else {
		fprintf(fp, "interface=");
		if (canlan()) {
			fprintf(fp, "%s", nvram_safe_get("lan_ifname"));
		}
		char vifs[256];
		getIfLists(vifs, 256);
		char var[256], *wordlist, *next;
		foreach(var, vifs, next)
		{
			if (strcmp(safe_get_wan_face(wan_if_buffer), var) && strcmp(nvram_safe_get("lan_ifname"), var)) {
				char *ipaddr = nvram_nget("%s_ipaddr", var);
				if (*ipaddr && strcmp(ipaddr, "0.0.0.0"))
					fprintf(fp, ",%s", var);
			}
		}
		if (nvram_exists("dnsmasq_addif")) {
			fprintf(fp, ",%s", nvram_safe_get("dnsmasq_addif"));
		}
		if (nvram_exists("dnsmasq_addifvpn")) {
			fprintf(fp, ",%s", nvram_safe_get("dnsmasq_addifvpn"));
		}
	}
	int mdhcpcount = 0;
	/* this is usally not required anymore since we add all interfaces with valid ip addresses anyway before */
	if (nvram_exists("mdhcpd_count")) {
		mdhcpcount = nvram_geti("mdhcpd_count");
		for (i = 0; i < mdhcpcount; i++) {
			char buffer[128];
			if (!strcmp(getmdhcp(IDX_DHCPON, i, buffer), "Off"))
				continue;
			char *ifname = getmdhcp(IDX_IFNAME, i, buffer);
			if (!*(nvram_nget("%s_ipaddr", ifname)) || !*(nvram_nget("%s_netmask", ifname)))
				continue;
			if (canlan() || i > 0) {
				fprintf(fp, ",");
			}
			if (nvram_matchi("pptpd_enable", 1)) {
				fprintf(fp, "%s", nvram_nget("%s_ipaddr", ifname));
#ifdef HAVE_IPV6
				char buf[INET6_ADDRSTRLEN];
				char *ip = getifaddr_any(buf, ifname, AF_INET6) ?: NULL;
				if (ip && nvram_matchi("ipv6_enable", 1))
					fprintf(fp, ",%s", ip);
#endif
			} else
				fprintf(fp, "%s", ifname);
		}
	}
	fprintf(fp, "\n");
	fprintf(fp, "resolv-file=/tmp/resolv.dnsmasq\n");
	//fprintf(fp, "all-servers\n");
	if (nvram_matchi("dnsmasq_strict", 1))
		fprintf(fp, "strict-order\n");

#ifdef HAVE_SMARTDNS
	if (nvram_matchi("smartdns", 1)) {
		nvram_seti("dns_crypt", 0);
		fprintf(fp, "server=127.0.0.1#6053\n");
		fprintf(fp, "no-resolv\n");
	}
#endif
#ifdef HAVE_DNSCRYPT
	if (nvram_matchi("dns_crypt", 1)) {
		nvram_seti("recursive_dns", 0); // disable unbound
		fprintf(fp, "server=127.0.0.1#30\n");
		fprintf(fp, "no-resolv\n");
	}
#endif
#ifdef HAVE_UNBOUND
	if (nvram_matchi("recursive_dns", 1) && !nvram_matchi("smartdns", 1)) {
		fprintf(fp, "server=127.0.0.1#7053\n");
		fprintf(fp, "no-resolv\n");
	}
#endif
#ifdef HAVE_TOR
	if (nvram_match("tor_enable", "1") && !nvram_matchi("smartdns", 1))
		fprintf(fp, "server=%s#5353\n", nvram_safe_get("lan_ipaddr"));
#endif
#ifdef HAVE_IPV6
	if (nvram_matchi("ipv6_enable", 1)) {
		if (nvram_matchi("dnsipv6_enable", 1)) {
			//leasetime
			char ipv6_leasetime[12] = { 0 };
			char dnsipv6_rastate[16] = { 0 };
			nvram_geti("dnsipv6_rastate") ? strlcpy(dnsipv6_rastate, "slaac", sizeof(dnsipv6_rastate)) :
							strlcpy(dnsipv6_rastate, "ra-stateless", sizeof(dnsipv6_rastate));

			if (!strcmp(nvram_safe_get("dnsipv6_leasetime"), "0")) {
				strlcpy(ipv6_leasetime, "infinite", sizeof(ipv6_leasetime));
			} else {
				snprintf(ipv6_leasetime, sizeof(ipv6_leasetime), "%sm", nvram_safe_get("dnsipv6_leasetime"));
			}
			//interfaces
			char ifname[32];
			char *next;
			char *wordlist = nvram_safe_get("dnsipv6_interfaces");
			//dd_loginfo("dnsipv6if", "dnsmasq dnsipv6if: %s", wordlist);
			foreach(ifname, wordlist, next)
			{
				fprintf(fp, "dhcp-range=::%s,::%s,constructor:%s,ra-names,%s,%s\n",
					nvram_safe_get("dnsipv6_range_start"), nvram_safe_get("dnsipv6_range_end"), ifname,
					dnsipv6_rastate, ipv6_leasetime);
				fprintf(fp, "ra-param=%s%s%s\n", ifname, ",10,", nvram_safe_get("dnsipv6_ralifetime"));
			}

			fprintf(fp, "enable-ra\n");
			// Suppress logging of the routine operation:
			fprintf(fp, "quiet-dhcp6\nquiet-ra\n");
		}
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
		if (nvram_matchi("dhcpd_usenvram", 1)) {
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
			dhcp_max += nvram_geti("dhcp_num") + nvram_geti("static_leasenum");
		for (i = 0; i < mdhcpcount; i++) {
			char buffer[128];
			if (strcmp(getmdhcp(IDX_DHCPON, i, buffer), "On"))
				continue;
			char *ifname = getmdhcp(IDX_IFNAME, i, buffer);
			if (!*(nvram_nget("%s_ipaddr", ifname)) || !*(nvram_nget("%s_netmask", ifname)))
				continue;
			dhcp_max += atoi(getmdhcp(IDX_LEASEMAX, i, buffer));
		}
		fprintf(fp, "dhcp-lease-max=%d\n", dhcp_max);
		if (landhcp())
			fprintf(fp, "dhcp-option=%s,3,%s\n", nvram_safe_get("lan_ifname"), nvram_safe_get("lan_ipaddr"));
		for (i = 0; i < mdhcpcount; i++) {
			char buffer[128];
			if (strcmp(getmdhcp(IDX_DHCPON, i, buffer), "On"))
				continue;
			char *ifname = getmdhcp(IDX_IFNAME, i, buffer);
			if (!*(nvram_nget("%s_ipaddr", ifname)) || !*(nvram_nget("%s_netmask", ifname)))
				continue;
			fprintf(fp, "dhcp-option=%s,3,", ifname);
			fprintf(fp, "%s\n", nvram_nget("%s_ipaddr", ifname));
		}
		if (nvram_invmatch("wan_wins", "") && nvram_invmatch("wan_wins", "0.0.0.0"))
			fprintf(fp, "dhcp-option=44,%s\n", nvram_safe_get("wan_wins"));
		if (nvram_matchi("dns_dnsmasq", 0)) {
			fprintf(fp, "port=0\n");
#ifdef HAVE_UNBOUND
			if (nvram_matchi("recursive_dns", 1)) {
				fprintf(fp, "dhcp-option=6,%s\n", nvram_safe_get("lan_ipaddr"));
			} else
#endif
#ifdef HAVE_SMARTDNS
				if (nvram_matchi("smartdns", 1)) {
				fprintf(fp, "dhcp-option=6,%s\n", nvram_safe_get("lan_ipaddr"));
			} else
#endif
			{
				dns_list6 = get_dns_list(1);
				char buffdns6[256] = { 0 };
				char buffdns[256] = { 0 };

				if (dns_list6 && dns_list6->num_servers > 0) {
					for (i = 0; i < dns_list6->num_servers; i++) {
						//dd_loginfo("dnsipv6", "dns_list6-%d: %s", i, dns_list6->dns_server[i].ip); 
						if (strchr(dns_list6->dns_server[i].ip, ':')) {
							strlcat(buffdns6, ",[", sizeof(buffdns6));
							strlcat(buffdns6, dns_list6->dns_server[i].ip, sizeof(buffdns6));
							strlcat(buffdns6, "]", sizeof(buffdns6));
						} else {
							strlcat(buffdns, ",", sizeof(buffdns));
							strlcat(buffdns, dns_list6->dns_server[i].ip, sizeof(buffdns));
						}
					}
					//dd_loginfo("dnsipv6", "buffdns6: %s", buffdns6); 
					//dd_loginfo("dnsipv6", "buffdns: %s", buffdns); 
					if (nvram_matchi("ipv6_enable", 1) && nvram_matchi("dnsipv6_enable", 1) && buffdns6) {
						fprintf(fp, "dhcp-option=option6:dns-server%s\n", buffdns6);
					}
					if (buffdns) {
						fprintf(fp, "dhcp-option=option:dns-server%s\n", buffdns);
					}
				}
				if (dns_list6)
					free_dns_list(dns_list6);
			}
		}

		if (nvram_matchi("auth_dnsmasq", 1))
			fprintf(fp, "dhcp-authoritative\n");
		if (landhcp()) {
			unsigned int dhcpnum = nvram_geti("dhcp_num");
			char *ip = nvram_safe_get("lan_ipaddr");
			char *netmask = nvram_safe_get("lan_netmask");
			char *leasetime = nvram_safe_get("dhcp_lease");
			makeentry_full(fp, nvram_safe_get("lan_ifname"), dhcpnum, nvram_safe_get("dhcp_start"), netmask, leasetime);
		}

		for (i = 0; i < mdhcpcount; i++) {
			char buffer[128];
			char buffer2[128];
			if (strcmp(getmdhcp(IDX_DHCPON, i, buffer), "On"))
				continue;
			char *ifname = getmdhcp(IDX_IFNAME, i, buffer);
			if (!*(nvram_nget("%s_ipaddr", ifname)) || !*(nvram_nget("%s_netmask", ifname)))
				continue;
			unsigned int dhcpnum = atoi(getmdhcp(IDX_LEASEMAX, i, buffer2));
			unsigned int dhcpstart = atoi(getmdhcp(IDX_LEASESTART, i, buffer2));
			char *ip = nvram_nget("%s_ipaddr", ifname);
			char *netmask = nvram_nget("%s_netmask", ifname);
			char *leasetime = getmdhcp(IDX_LEASETIME, i, buffer2);
			makeentry(fp, ifname, dhcpnum, dhcpstart, ip, netmask, leasetime);
		}

		int leasenum = nvram_geti("static_leasenum");

		if (leasenum > 0) {
			char *lease = nvram_safe_get("static_leases");
			char *leasebuf = (char *)malloc(strlen(lease) + 1);
			char *cp = leasebuf;
			int first = 0;
			strcpy(leasebuf, lease);
			for (i = 0; i < leasenum; i++) {
				char *mac = strsep(&leasebuf, "=");
				char *host = strsep(&leasebuf, "=");
				char *ip = strsep(&leasebuf, "=");
				char *time = strsep(&leasebuf, " ");

				if (mac == NULL || host == NULL || ip == NULL)
					continue;
				fprintf(fp, "dhcp-host=%s,%s,%s,", mac, host, ip);
				char nv[64];
				sprintf(nv, "dnsmasq_lease_%s", ip);
				if (leasechange && nvram_exists(nv)) {
					nvram_unset(nv);
				}
				if (!time || !*time)
					fprintf(fp, "infinite\n");
				else
					fprintf(fp, "%sm\n", time);

#ifdef HAVE_UNBOUND
				if (!nvram_matchi("recursive_dns", 1))
#endif
				{
					if (!first) {
						genHosts();
						first = 1;
					}
					addHost(host, ip, 1);
				}
			}
			free(cp);
		}
	}
	fprintf(fp, "bogus-priv\n");
	fprintf(fp, "conf-file=/etc/rfc6761.conf\n");
	fprintf(fp, "clear-on-reload\n");
#ifdef HAVE_DNSSEC
	if (nvram_matchi("dnssec", 1)) {
		fprintf(fp, "conf-file=/etc/trust-anchors.conf\n");
		fprintf(fp, "dnssec\n");
		if (!nvram_matchi("ntp_enable", 1)) {
			fprintf(fp, "dnssec-no-timecheck\n");
		}
		if (nvram_matchi("dnssec_cu", 1)) {
			fprintf(fp, "dnssec-check-unsigned\n");
		} else {
			fprintf(fp, "dnssec-check-unsigned=no\n");
		}
	}
#endif
	if (nvram_matchi("dnssec_proxy", 1)) {
		fprintf(fp, "proxy-dnssec\n");
	}
	if (nvram_matchi("dnsmasq_rc", 1)) {
		fprintf(fp, "dhcp-rapid-commit\n");
	}
	/* stop dns rebinding for private addresses */
	if (nvram_matchi("dnsmasq_no_dns_rebind", 1)) {
		fprintf(fp, "stop-dns-rebind\n");
	}
	if (nvram_matchi("dnsmasq_add_mac", 1)) {
		fprintf(fp, "add-mac\n");
	}
#ifdef HAVE_PRIVOXY
	if (nvram_matchi("privoxy_enable", 1)) {
		if (nvram_matchi("privoxy_transp_enable", 1)) {
			fprintf(fp, "dhcp-option=252,http://config.privoxy.org/wpad.dat\n");
		} else {
			fprintf(fp, "dhcp-option=252,http://%s/wpad.dat\n", nvram_safe_get("lan_ipaddr"));
		}
	} else {
		fprintf(fp, "dhcp-option=252,\"\\n\"\n");
	}
#else
	fprintf(fp, "dhcp-option=252,\"\\n\"\n");
#endif
	char *addoptions = nvram_safe_get("dnsmasq_options");
#ifdef HAVE_SMARTDNS
	if (nvram_matchi("smartdns", 1) && !nvram_matchi("dnssec", 1))
		fprintf(fp, "cache-size=0\n");
	else
#endif
		if (!strstr(addoptions, "cache-size="))
		fprintf(fp, "cache-size=%d\n", nvram_default_geti("dnsmasq_cachesize", 1500));
	if (!strstr(addoptions, "dns-forward-max="))
		fprintf(fp, "dns-forward-max=%d\n", nvram_default_geti("dnsmasq_forward_max", 150));
	if (nvram_matchi("dnsmasq_ms_telemetry", 1))
		add_ms_telemetry(fp);
	if (nvram_matchi("dnsmasq_ubnt_telemetry", 1))
		add_ubnt_telemetry(fp);
	// write canarydomain to stop browsers from using DoH when DoT redirection is active
	if (nvram_matchi("dns_redirectdot", 1)) {
		fprintf(fp, "address=/use-application-dns.net/mask.icloud.com/mask-h2.icloud.com/\n");
	}
	/*
	 * Additional options 
	 */
	fwritenvram("dnsmasq_options", fp);
	fclose(fp);

	dns_to_resolv();

	chmod("/etc/lease_update.sh", 0700);
	log_eval("dnsmasq", "-u", "root", "-g", "root", "-C", getdefaultconfig("dnsmasq", path, sizeof(path), "dnsmasq.conf"));

	cprintf("done\n");
	return;
}

void restart_dnsmasq(void)
{
	stop_dnsmasq();
	start_dnsmasq();
}

void stop_dnsmasq(void)
{
#ifdef HAVE_SMARTDNS
	stop_smartdns();
#endif
	if (stop_process("dnsmasq", "daemon")) {
		unlink("/tmp/resolv.dnsmasq");
	}
}
#endif

#ifdef TEST
int main(int argc, char *argv[])
{
	start_dnsmasq();
}
#endif
