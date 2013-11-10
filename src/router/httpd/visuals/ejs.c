#define VISUALSOURCE 1

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <signal.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/statfs.h>
#include <sys/utsname.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <broadcom.h>
#include <cymac.h>
#include <wlutils.h>
#include <bcmparams.h>
#include <dirent.h>
#include <netdb.h>
#include <utils.h>
#include <bcmnvram.h>
#include <revision.h>
#include <shutils.h>
#ifdef HAVE_SAMBA_SERVER
#include <jansson.h>
#endif
#ifdef HAVE_ATH9K
#include <glob.h>
#endif

void (*do_ej_buffer) (char *buffer, webs_t stream) = NULL;
int (*httpd_filter_name) (char *old_name, char *new_name, size_t size, int type) = NULL;
char *(*websGetVar) (webs_t wp, char *var, char *d) = NULL;
int (*websWrite) (webs_t wp, char *fmt, ...) = NULL;
struct wl_client_mac *wl_client_macs = NULL;
void (*do_ej) (struct mime_handler * handler, char *path, webs_t stream, char *query) = NULL;	// jimmy, 
									// https, 
									// 8/4/2003
int (*ejArgs) (int argc, char_t ** argv, char_t * fmt, ...) = NULL;
FILE *(*getWebsFile) (char *path) = NULL;
int (*wfflush) (webs_t fp) = NULL;
int (*wfputc) (char c, webs_t fp) = NULL;
int (*wfputs) (char *buf, webs_t fp) = NULL;
char *(*live_translate) (char *tran) = NULL;
websRomPageIndexType *PwebsRomPageIndex = NULL;
char *(*GOZILA_GET) (webs_t wp, char *name) = NULL;
void (*validate_cgi) (webs_t fp) = NULL;

#ifdef HAVE_HTTPS
int do_ssl;
#endif

void initWeb(struct Webenvironment *env)
{

	websGetVar = env->PwebsGetVar;
	httpd_filter_name = env->Phttpd_filter_name;
	wl_client_macs = env->Pwl_client_macs;
	websWrite = env->PwebsWrite;
	do_ej_buffer = env->Pdo_ej_buffer;
	do_ej = env->Pdo_ej;
#ifdef HAVE_HTTPS
	do_ssl = env->Pdo_ssl;
#endif
	ejArgs = env->PejArgs;
	getWebsFile = env->PgetWebsFile;
	wfflush = env->Pwfflush;
	wfputc = env->Pwfputc;
	wfputs = env->Pwfputs;
	PwebsRomPageIndex = env->PwebsRomPageIndex;
	live_translate = env->Plive_translate;
	GOZILA_GET = env->PGOZILA_GET;
	validate_cgi = env->Pvalidate_cgi;
}

struct onload onloads[] = {
	// { "Filters", filter_onload },
	{"WL_ActiveTable-wl0", wl_active_onload},
	{"WL_ActiveTable-wl1", wl_active_onload},
	{"MACClone", macclone_onload},
	{"FilterSummary", filtersummary_onload},
	{"Ping", ping_onload},
	// {"Traceroute", traceroute_onload},
};

void ej_onload(webs_t wp, int argc, char_t ** argv)
{
	char *type, *arg;
	struct onload *v;

#ifndef FASTWEB
	if (argc < 2) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	type = argv[0];
	arg = argv[1];

	for (v = onloads; v < &onloads[STRUCT_LEN(onloads)]; v++) {
		if (!strcmp(v->name, type)) {
			v->go(wp, arg);
			return;
		}
	}

	return;
}

/*
 * Meta tag command that will no allow page cached by browsers. The will
 * force the page to be refreshed when visited. 
 */
void ej_no_cache(webs_t wp, int argc, char_t ** argv)
{
	websWrite(wp, "<meta http-equiv=\"expires\" content=\"0\">\n");
	websWrite(wp, "<meta http-equiv=\"cache-control\" content=\"no-cache\">\n");
	websWrite(wp, "<meta http-equiv=\"pragma\" content=\"no-cache\">\n");

	return;
}

void prefix_ip_get(char *name, char *buf, int type)
{
	if (type == 1)
		sprintf(buf, "%d.%d.%d.", get_single_ip(nvram_safe_get(name), 0), get_single_ip(nvram_safe_get(name), 1), get_single_ip(nvram_safe_get(name), 2));
	if (type == 2)
		sprintf(buf, "%d.%d.", get_single_ip(nvram_safe_get(name), 0), get_single_ip(nvram_safe_get(name), 1));
}

/*
 * Example:
 * lan_ipaddr=192.168.1.1
 * <% prefix_ip_get("lan_ipaddr",1); %> produces "192.168.1."
 */
void ej_prefix_ip_get(webs_t wp, int argc, char_t ** argv)
{
	char *name;
	int type;

#ifndef FASTWEB
	if (argc < 2) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	name = argv[0];
	type = atoi(argv[1]);

	if (type == 1)
		websWrite(wp, "%d.%d.%d.", get_single_ip(nvram_safe_get(name), 0), get_single_ip(nvram_safe_get(name), 1), get_single_ip(nvram_safe_get(name), 2));
	if (type == 2)
		websWrite(wp, "%d.%d.", get_single_ip(nvram_safe_get(name), 0), get_single_ip(nvram_safe_get(name), 1));

	return;
}

/*
 * Example:
 * lan_ipaddr = 192.168.1.1
 * <% nvram_get("lan_ipaddr"); %> produces "192.168.1.1"
 */
void ej_nvram_get(webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
	if (argc < 1) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif

#if COUNTRY == JAPAN
	websWrite(wp, "%s", nvram_safe_get(argv[0]));
#else

	tf_webWriteESCNV(wp, argv[0]);	// test: buffered version of above

	return;
#endif

	return;
}

void ej_nvram_real_get(webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
	if (argc < 1) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	websWrite(wp, "%s", nvram_safe_get(argv[0]));

	return;
}

/*
 * Example:
 * lan_ipaddr = 192.168.1.1, gozila_action = 0
 * <% nvram_selget("lan_ipaddr"); %> produces "192.168.1.1"
 * lan_ipaddr = 192.168.1.1, gozila_action = 1, websGetVar(wp, "lan_proto", NULL) = 192.168.1.2;
 * <% nvram_selget("lan_ipaddr"); %> produces "192.168.1.2"
 */
void ej_nvram_selget(webs_t wp, int argc, char_t ** argv)
{
	char *name;

#ifndef FASTWEB
	if (argc < 1) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	name = argv[0];
	if (nvram_match("gozila_action", "1")) {
		char *buf = websGetVar(wp, name, NULL);

		if (buf) {
			websWrite(wp, "%s", buf);
			return;
		}
	}
	tf_webWriteESCNV(wp, name);	// test: buffered version of above

	return;
}

/*
 * Example:
 * wan_mac = 00:11:22:33:44:55
 * <% nvram_mac_get("wan_mac"); %> produces "00-11-22-33-44-55"
 */
void ej_nvram_mac_get(webs_t wp, int argc, char_t ** argv)
{
	char *c;
	char *mac;
	int i;

#ifndef FASTWEB
	if (argc < 1) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	c = nvram_safe_get(argv[0]);

	if (c) {
		mac = strdup(c);
		for (i = 0; *(mac + i); i++) {
			if (*(mac + i) == ':')
				*(mac + i) = '-';
		}
		websWrite(wp, "%s", mac);
		free(mac);	// leak, thx tofu
	}

	return;

}

/*
 * Example:
 * wan_proto = dhcp; gozilla = 0;
 * <% nvram_gozila_get("wan_proto"); %> produces "dhcp"
 *
 * wan_proto = dhcp; gozilla = 1; websGetVar(wp, "wan_proto", NULL) = static;
 * <% nvram_gozila_get("wan_proto"); %> produces "static"
 */
void ej_nvram_gozila_get(webs_t wp, int argc, char_t ** argv)
{
	char *type;

#ifndef FASTWEB
	if (argc < 1) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	type = GOZILA_GET(wp, argv[0]);
	if (type == NULL)
		type = nvram_safe_get(argv[0]);

	websWrite(wp, "%s", type);
}

void ej_webs_get(webs_t wp, int argc, char_t ** argv)
{
	char *value;

#ifndef FASTWEB
	if (argc < 1) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif

	value = websGetVar(wp, argv[0], NULL);

	if (value)
		websWrite(wp, "%s", value);

	return;
}

/*
 * Example:
 * lan_ipaddr = 192.168.1.1
 * <% get_single_ip("lan_ipaddr","1"); %> produces "168"
 */
void ej_get_single_ip(webs_t wp, int argc, char_t ** argv)
{
	char *c;

#ifndef FASTWEB
	if (argc < 1) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif

	c = nvram_safe_get(argv[0]);
	if (c) {
		if (!strcmp(c, PPP_PSEUDO_IP) || !strcmp(c, PPP_PSEUDO_GW))
			c = "0.0.0.0";
		else if (!strcmp(c, PPP_PSEUDO_NM))
			c = "255.255.255.0";

		websWrite(wp, "%d", get_single_ip(c, atoi(argv[1])));
	} else
		websWrite(wp, "0");

	return;
}

void ej_get_single_nm(webs_t wp, int argc, char_t ** argv)
{
	char *c;

#ifndef FASTWEB
	if (argc < 1) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif

	c = nvram_safe_get(argv[0]);
	if (c) {
		websWrite(wp, "%d", get_single_ip(c, atoi(argv[1])));
	} else
		websWrite(wp, "0");

	return;
}

/*
 * Example: wan_mac = 00:11:22:33:44:55 get_single_mac("wan_mac", 1);
 * produces "11" 
 */
int get_single_mac(char *macaddr, int which)
{
	int mac[6] = { 0, 0, 0, 0, 0, 0 };
	int ret;

	ret = sscanf(macaddr, "%2X:%2X:%2X:%2X:%2X:%2X", &mac[0], &mac[1], &mac[2], &mac[3], &mac[4], &mac[5]);
	return mac[which];
}

/*
 * Example:
 * wan_mac = 00:11:22:33:44:55
 * <% get_single_mac("wan_mac","1"); %> produces "11"
 */
void ej_get_single_mac(webs_t wp, int argc, char_t ** argv)
{
	char *c;
	int mac;

#ifndef FASTWEB
	if (argc < 2) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif

	c = nvram_safe_get(argv[0]);
	if (c) {
		mac = get_single_mac(c, atoi(argv[1]));
		websWrite(wp, "%02X", mac);
	} else
		websWrite(wp, "00");

	return;
}

/*
 * Example:
 * wan_proto = dhcp; gozilla = 0;
 * <% nvram_selmatch("wan_proto", "dhcp", "selected"); %> produces "selected"
 *
 * wan_proto = dhcp; gozilla = 1; websGetVar(wp, "wan_proto", NULL) = static;
 * <% nvram_selmatch("wan_proto", "static", "selected"); %> produces "selected"
 */

int nvram_selmatch(webs_t wp, char *name, char *match)
{
	char *type = GOZILA_GET(wp, name);

	if (!type) {
		if (nvram_match(name, match)) {
			return 1;
		}
	} else {
		if (!strcmp(type, match)) {
			return 1;
		}
	}
	return 0;
}

void ej_nvram_selmatch(webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
	if (argv < 3) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	if (nvram_selmatch(wp, argv[0], argv[1])) {
		websWrite(wp, argv[2]);
	}
	return;
}

void ej_nvram_else_selmatch(webs_t wp, int argc, char_t ** argv)
{
	char *type;

#ifndef FASTWEB
	if (argc < 4) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif

	type = GOZILA_GET(wp, argv[0]);

	if (!type) {
		if (nvram_match(argv[0], argv[1])) {
			websWrite(wp, argv[2]);
		} else
			websWrite(wp, argv[3]);
	} else {
		if (!strcmp(type, argv[1])) {
			websWrite(wp, argv[2]);
		} else
			websWrite(wp, argv[3]);
	}

	return;
}

void ej_selchecked(webs_t wp, int argc, char_t ** argv)
{
	char *type;

#ifndef FASTWEB
	if (argc < 2) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif

	type = websGetVar(wp, argv[0], "0");

	if (type) {
		if (!strcmp(type, argv[1]))
			websWrite(wp, "checked=\"checked\"");
	}

	return;
}

void ej_else_selmatch(webs_t wp, int argc, char_t ** argv)
{
	char *type;

#ifndef FASTWEB
	if (argc < 4) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif

	type = websGetVar(wp, argv[0], NULL);

	if (type) {
		if (!strcmp(type, argv[1])) {
			websWrite(wp, argv[2]);
		} else
			websWrite(wp, argv[3]);
	} else {
		websWrite(wp, argv[3]);
	}

	return;
}

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_else_match("wan_proto", "dhcp", "0","1"); %> produces "0"
 * <% nvram_else_match("wan_proto", "static", "0","1"); %> produces "1"
 */
void ej_nvram_else_match(webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
	if (argc < 4) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif

	if (nvram_match(argv[0], argv[1]))
		websWrite(wp, argv[2]);
	else
		websWrite(wp, argv[3]);

	return;
}

void ej_startswith(webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
	if (argc < 3) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	if (startswith(nvram_safe_get(argv[0]), argv[1]))
		websWrite(wp, argv[2]);

	return;
}

static char *s_conditions[] = {
#ifdef HAVE_MICRO
	"MICRO",
#endif
#ifdef HAVE_EXTHELP
	"EXTHELP",
#endif
#ifdef HAVE_MULTICAST
	"MULTICAST",
#endif
#ifdef HAVE_SNMP
	"SNMP",
#endif
#ifdef HAVE_WIVIZ
	"WIVIZ",
#endif
#ifdef HAVE_RSTATS
	"RSTATS",
#endif
#ifdef HAVE_ACK
	"ACK",
#endif
#ifdef HAVE_SSHD
	"SSHD",
#endif
#ifdef HAVE_PPTPD
	"PPTPD",
#endif
#ifdef HAVE_QUAGGA
	"QUAGGA",
#endif
#ifdef HAVE_SAMBA
	"SAMBA",
#endif
#ifdef HAVE_SAMBA3
	"SAMBA3",
#endif
#ifdef HAVE_JFFS2
	"JFFS2",
#endif
#ifdef HAVE_GPSI
	"GPSI",
#endif
#ifdef HAVE_MMC
	"MMC",
#endif
#ifdef HAVE_SPUTNIK_APD
	"SPUTNIK_APD",
#endif
#ifdef HAVE_RFLOW
	"RFLOW",
#endif
#ifdef HAVE_USB
	"USB",
#endif
#ifdef HAVE_RADIUSPLUGIN
	"RADIUSPLUGIN",
#endif
#ifdef HAVE_PPPOESERVER
	"PPPOESERVER",
#endif
#ifdef HAVE_MILKFISH
	"MILKFISH",
#endif
#ifdef HAVE_LANGUAGE
	"LANGUAGE",
#endif
#ifdef HAVE_BUFFALO
	"HAVE_BUFFALO",
#endif
#ifdef HAVE_WPS
	"HAVE_WPS",
#endif
#ifdef HAVE_ATH9K
	"HAVE_ATH9K",
#endif
#ifdef HAVE_USBIP
	"USBIP",
#endif
	NULL
};

void ej_ifdef(webs_t wp, int argc, char_t ** argv)
{
	char *name, *output;

#ifndef FASTWEB
	if (argc < 2) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	name = argv[0];
	output = argv[1];
	int cnt = 0;
	while (s_conditions[cnt]) {
		if (!strcmp(name, s_conditions[cnt++])) {
			websWrite(wp, output);
			return;
		}
	}

	if (!strcmp(name, "MINI"))	// to include mini + mini-special
	{
		if (startswith(nvram_safe_get("dist_type"), "mini")) {
			websWrite(wp, output);
			return;
		}
	}
	if (!strcmp(name, "VPN"))	// to include vpn + vpn-special
	{
		if (startswith(nvram_safe_get("dist_type"), "vpn")) {
			websWrite(wp, output);
			return;
		}
	}
	if (!strcmp(name, "WET")) {
		if (getWET())
			websWrite(wp, output);
		return;
	}
	if (!strcmp(name, "STA")) {
		if (getSTA())
			websWrite(wp, output);
		return;
	}

	return;
}

void ej_ifndef(webs_t wp, int argc, char_t ** argv)
{
	char *name, *output;

#ifndef FASTWEB
	if (argc < 2) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	name = argv[0];
	output = argv[1];

	int cnt = 0;
	while (s_conditions[cnt]) {
		if (!strcmp(name, s_conditions[cnt++])) {
			return;
		}
	}

	// HAVE_AFTERBURNER
	if (!strncmp(name, "AFTERBURNER", 11)) {
#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880)
		return;
#else
		int afterburner = 0;
		char cap[WLC_IOCTL_SMLEN];
		char caps[WLC_IOCTL_SMLEN];
		char *ifname;
		name = name + 11;
		if (!strncmp(name, "_wl0", 4))
			ifname = nvram_safe_get("wl0_ifname");
		else		// "_wl1"
			ifname = nvram_safe_get("wl1_ifname");
		char *next;

		if (wl_iovar_get(ifname, "cap", (void *)caps, WLC_IOCTL_SMLEN)
		    == 0) {
			foreach(cap, caps, next) {
				if (!strcmp(cap, "afterburner"))
					afterburner = 1;
			}

			if (afterburner)
				return;
		}
#endif
	}
	// end HAVE_AFTERBURNER
	// HAVE_HASWIFI
	if (!strcmp(name, "HASWIFI")) {
		if (haswifi())
			return;
	}
	// end HAVE_HASWIFI
	if (!strcmp(name, "WET")) {
		if (getWET())
			return;
	}
	if (!strcmp(name, "STA")) {
		if (getSTA())
			return;
	}

	websWrite(wp, output);

	return;
}

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_match("wan_proto", "dhcp", "selected"); %> produces "selected"
 * <% nvram_match("wan_proto", "static", "selected"); %> does not produce
 */
void ej_nvram_match(webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
	if (argc < 3) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif

	if (nvram_match(argv[0], argv[1]))
		websWrite(wp, argv[2]);

	return;
}

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_invmatch("wan_proto", "dhcp", "disabled"); %> does not produce
 * <% nvram_invmatch("wan_proto", "static", "disabled"); %> produces "disabled"
 */
void ej_nvram_invmatch(webs_t wp, int argc, char_t ** argv)
{
	char *name, *invmatch, *output;

#ifdef FASTWEB
	ejArgs(argc, argv, "%s %s %s", &name, &invmatch, &output);
#else
	if (ejArgs(argc, argv, "%s %s %s", &name, &invmatch, &output) < 3) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif

	if (!nvram_match(name, invmatch))
		websWrite(wp, output);

	return;
}

/*
 * static void ej_scroll (webs_t wp, int argc, char_t ** argv) { char *type;
 * int y;
 * 
 * #ifdef FASTWEB ejArgs (argc, argv, "%s %d", &type, &y); #else if (ejArgs
 * (argc, argv, "%s %d", &type, &y) < 2) { websError (wp, 400, "Insufficient
 * args\n"); return; } #endif if (gozila_action) websWrite (wp, "%d", y);
 * else websWrite (wp, "0");
 * 
 * return; } 
 */
/*
 * Example:
 * filter_mac=00:12:34:56:78:00 00:87:65:43:21:00
 * <% nvram_list("filter_mac", 1); %> produces "00:87:65:43:21:00"
 * <% nvram_list("filter_mac", 100); %> produces ""
 */
void ej_nvram_list(webs_t wp, int argc, char_t ** argv)
{
	int which;
	char word[256], *next;

#ifndef FASTWEB
	if (argc < 2) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	which = atoi(argv[1]);
	char *list = nvram_safe_get(argv[0]);

	foreach(word, list, next) {
		if (which-- == 0)
			websWrite(wp, word);
	}

	return;
}

/*
 * Example: wan_dns = 1.2.3.4 10.20.30.40 15.25.35.45 get_dns_ip("wan_dns",
 * 1, 2); produces "20" 
 */
int get_dns_ip(char *name, int which, int count)
{
	static char word[256];
	char *next;
	int ip;
	char *list = nvram_safe_get(name);

	foreach(word, list, next) {
		if (which-- == 0) {
			ip = get_single_ip(word, count);
			return ip;
		}
	}
	return 0;
}

/*
 * Example: wan_dns = 168.95.1.1 210.66.161.125 168.95.192.1 <%
 * get_dns_ip("wan_dns", "1", "2"); %> produces "161" <%
 * get_dns_ip("wan_dns", "2", "3"); %> produces "1" 
 */
void ej_get_dns_ip(webs_t wp, int argc, char_t ** argv)
{
	int which;
	char word[256], *next;

#ifndef FASTWEB
	if (argc < 3) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	which = atoi(argv[1]);
	char *list = nvram_safe_get(argv[0]);

	foreach(word, list, next) {
		if (which-- == 0) {
			websWrite(wp, "%d", get_single_ip(word, atoi(argv[2])));
			return;
		}
	}

	websWrite(wp, "0");	// not find
}

void ej_get_http_prefix(webs_t wp, int argc, char_t ** argv)
{
	char http[10];
	char ipaddr[20];
	char port[10];

	char *http_enable = websGetVar(wp, "http_enable", NULL);

#ifdef HAVE_HTTPS
	char *https_enable = websGetVar(wp, "https_enable", NULL);

	if (do_ssl && http_enable == NULL && https_enable == NULL) {
		strcpy(http, "https");
	} else if (do_ssl && http_enable && https_enable) {
		if (atoi(https_enable) && atoi(http_enable))
			strcpy(http, "https");
		else if (atoi(https_enable) && !atoi(http_enable))
			strcpy(http, "https");
		else		// !atoi(https_enable) && atoi(http_enable)
			strcpy(http, "http");
	} else if (do_ssl && !http_enable && !https_enable) {
		strcpy(http, "http");
	} else if (!do_ssl && http_enable && https_enable) {
		if (atoi(https_enable) && atoi(http_enable))
			strcpy(http, "http");
		else if (atoi(https_enable) && !atoi(http_enable))
			strcpy(http, "https");
		else		// !atoi(https_enable) && atoi(http_enable)
			strcpy(http, "http");
	} else
#endif
		strcpy(http, "http");

	if (nvram_match("browser_method", "USE_LAN")) {	// Use LAN to browser
		if (nvram_match("restore_defaults", "1")
		    || nvram_match("sv_restore_defaults", "1")) {

			strcpy(http, "http");
		} else
			strcpy(ipaddr, nvram_safe_get("lan_ipaddr"));
		strcpy(port, "");
	} else {

		if (nvram_match("wan_proto", "pptp"))
			strcpy(ipaddr, nvram_safe_get("pptp_get_ip"));
		else if (nvram_match("wan_proto", "l2tp"))
			strcpy(ipaddr, nvram_safe_get("l2tp_get_ip"));
		else
			strcpy(ipaddr, nvram_safe_get("wan_ipaddr"));

		sprintf(port, ":%s", nvram_safe_get("http_wanport"));
	}

	websWrite(wp, "%s://%s%s/", http, ipaddr, port);

	return;
}

void ej_get_mtu(webs_t wp, int argc, char_t ** argv)
{
	struct mtu_lists *mtu_list;
	char *type;
	char *proto = GOZILA_GET(wp, "wan_proto");

	type = argv[1];
#ifndef FASTWEB
	if (argc < 1) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	if (proto == NULL)
		proto = nvram_safe_get("wan_proto");
	mtu_list = get_mtu(proto);

	if (!strcmp(type, "min"))
		websWrite(wp, "%s", mtu_list->min);
	else if (!strcmp(type, "max"))
		websWrite(wp, "%s", mtu_list->max);

	return;
}

void ej_show_forward(webs_t wp, int argc, char_t ** argv)
// ej_show_forward(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg,
// char_t *url, char_t *path, char_t *query)
{
	int i;
	char *count;
	int c = 0;

	count = nvram_safe_get("forward_entries");
	if (count == NULL || strlen(count) == 0 || (c = atoi(count)) <= 0) {
		// return -1; botho 07/03/06 add "- None -" if empty
		websWrite(wp, "<tr>\n");
		websWrite(wp, "<td colspan=\"6\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n");
		websWrite(wp, "</tr>\n");
	}
	for (i = 0; i < c; i++) {
		websWrite(wp, "<tr><td>\n");
		websWrite(wp, "<input maxlength=\"12\" size=\"12\" name=\"name%d\" onblur=\"valid_name(this,'Name')\" value=\"", i);
		port_forward_table(wp, "name", i);
		websWrite(wp, "\" /></td>\n");
		websWrite(wp, "<td>\n");
		websWrite(wp, "<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"from%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"", i);
		port_forward_table(wp, "from", i);
		websWrite(wp, "\" /></td>\n");
		websWrite(wp, "<td>\n");
		websWrite(wp, "<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"to%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"", i);
		port_forward_table(wp, "to", i);
		websWrite(wp, "\"/></td>\n");
		websWrite(wp, "<td><select size=\"1\" name=\"pro%d\">\n", i);
		websWrite(wp, "<option value=\"tcp\" ");
		port_forward_table(wp, "sel_tcp", i);
		websWrite(wp, ">TCP</option>\n");
		websWrite(wp, "<option value=\"udp\" ");
		port_forward_table(wp, "sel_udp", i);
		websWrite(wp, ">UDP</option>\n");
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n\
		 		document.write(\"<option value=\\\"both\\\" ");
		port_forward_table(wp, "sel_both", i);
		websWrite(wp, " >\" + share.both + \"</option>\");\n\
      	\n//]]>\n</script>\n");
		websWrite(wp, "</select></td>\n");
		websWrite(wp, "<td>\n");
		websWrite(wp, "<input class=\"num\" maxlength=\"15\" size=\"15\" name=\"ip%d\" value=\"", i);
		port_forward_table(wp, "ip", i);
		websWrite(wp, "\" /></td>\n");
		websWrite(wp, "<td>\n");
		websWrite(wp, "<input type=\"checkbox\" value=\"on\" name=\"enable%d\" ", i);
		port_forward_table(wp, "enable", i);
		websWrite(wp, " /></td>\n");
		websWrite(wp, "</tr>\n");
	}
	return;
}

void ej_show_forward_spec(webs_t wp, int argc, char_t ** argv)
// ej_show_forward(webs_t wp, char_t *urlPrefix, char_t *webDir, int arg,
// char_t *url, char_t *path, char_t *query)
{
	int i;
	char *count;
	int c = 0;

	count = nvram_safe_get("forwardspec_entries");
	if (count == NULL || strlen(count) == 0 || (c = atoi(count)) <= 0) {
		// return -1; botho 07/03/06 add "- None -" if empty
		// websWrite (wp, "<tr></tr><tr></tr>\n");
		websWrite(wp, "<tr>\n");
		websWrite(wp, "<td colspan=\"7\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n");
		websWrite(wp, "</tr>\n");
	}
	for (i = 0; i < c; i++) {
		//name
		websWrite(wp, "<tr><td>\n");
		websWrite(wp, "<input maxlength=\"12\" size=\"12\" name=\"name%d\" onblur=\"valid_name(this,'Name')\" value=\"", i);
		port_forward_spec(wp, "name", i);
		websWrite(wp, "\" /></td>\n");

		//proto
		websWrite(wp, "<td><select size=\"1\" name=\"pro%d\">\n", i);
		websWrite(wp, "<option value=\"tcp\" ");
		port_forward_spec(wp, "sel_tcp", i);
		websWrite(wp, ">TCP</option>\n");
		websWrite(wp, "<option value=\"udp\" ");
		port_forward_spec(wp, "sel_udp", i);
		websWrite(wp, ">UDP</option>\n");
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n\
		 		document.write(\"<option value=\\\"both\\\" ");
		port_forward_spec(wp, "sel_both", i);
		websWrite(wp, " >\" + share.both + \"</option>\");\n\
      		\n//]]>\n</script>\n");
		websWrite(wp, "</select></td>\n");

		//src net
		websWrite(wp, "<td>\n");
		websWrite(wp, "<input class=\"num\" maxlength=\"18\" size=\"18\" name=\"src%d\" value=\"", i);
		port_forward_spec(wp, "src", i);
		websWrite(wp, "\" /></td>\n");

		//from
		websWrite(wp, "<td>\n");
		websWrite(wp, "<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"from%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"", i);
		port_forward_spec(wp, "from", i);
		websWrite(wp, "\" /></td>\n");

		//dest ip
		websWrite(wp, "<td>\n");
		websWrite(wp, "<input class=\"num\" maxlength=\"15\" size=\"15\" name=\"ip%d\" value=\"", i);
		port_forward_spec(wp, "ip", i);
		websWrite(wp, "\" /></td>\n");

		//port to
		websWrite(wp, "<td>\n");
		websWrite(wp, "<input class=\"num\" maxlength=\"5\" size=\"5\" name=\"to%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"", i);
		port_forward_spec(wp, "to", i);
		websWrite(wp, "\" /></td>\n");

		//checkbox
		websWrite(wp, "<td>\n");
		websWrite(wp, "<input type=\"checkbox\" value=\"on\" name=\"enable%d\" ", i);
		port_forward_spec(wp, "enable", i);
		websWrite(wp, " /></td>\n");

		//end of table
		websWrite(wp, "</tr>\n");
	}
	return;
}

void ej_show_triggering(webs_t wp, int argc, char_t ** argv)
{
	int i;
	char *count;
	int c = 0;

	count = nvram_safe_get("trigger_entries");
	if (count == NULL || strlen(count) == 0 || (c = atoi(count)) <= 0) {
		websWrite(wp, "<tr>\n");
		websWrite(wp, "<td colspan=\"6\" align=\"center\" valign=\"middle\">- <script type=\"text/javascript\">Capture(share.none)</script> -</td>\n");
		websWrite(wp, "</tr>\n");
	}
	for (i = 0; i < c; i++) {
		websWrite(wp, "<tr>\n");
		websWrite(wp, "<td><input maxlength=\"12\" size=\"12\" name=\"name%d\" onblur=\"valid_name(this,'Name')\" value=\"", i);
		port_trigger_table(wp, "name", i);
		websWrite(wp, "\" /></td>\n");
		websWrite(wp, "<td><input class=\"num\" maxlength=\"5\" size=\"5\" name=\"i_from%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"", i);
		port_trigger_table(wp, "i_from", i);
		websWrite(wp, "\" /></td>\n");
		websWrite(wp, "<td><input class=\"num\" maxlength=\"5\" size=\"5\" name=\"i_to%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"", i);
		port_trigger_table(wp, "i_to", i);
		websWrite(wp, "\" /></td>\n");
		websWrite(wp, "<td><select size=\"1\" name=\"pro%d\">\n", i);
		websWrite(wp, "<option value=\"tcp\" ");
		port_trigger_table(wp, "sel_tcp", i);
		websWrite(wp, ">TCP</option>\n");
		websWrite(wp, "<option value=\"udp\" ");
		port_trigger_table(wp, "sel_udp", i);
		websWrite(wp, ">UDP</option>\n");
		websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n\
		 		document.write(\"<option value=\\\"both\\\" ");
		port_trigger_table(wp, "sel_both", i);
		websWrite(wp, " >\" + share.both + \"</option>\");\n\
      		\n//]]>\n</script>\n");
		websWrite(wp, "</select></td>\n");
		websWrite(wp, "<td><input class=\"num\" maxlength=\"5\" size=\"5\" name=\"o_from%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"", i);
		port_trigger_table(wp, "o_from", i);
		websWrite(wp, "\" /></td>\n");
		websWrite(wp, "<td><input class=\"num\" maxlength=\"5\" size=\"5\" name=\"o_to%d\" onblur=\"valid_range(this,1,65535,'Port')\" value=\"", i);
		port_trigger_table(wp, "o_to", i);
		websWrite(wp, "\" /></td>\n");
		websWrite(wp, "<td><input type=\"checkbox\" value=\"on\" name=\"enable%d\" ", i);
		port_trigger_table(wp, "enable", i);
		websWrite(wp, " /></td>\n");
		websWrite(wp, "</tr>\n");
	}
	return;
}

// SEG DD-WRT addition
void ej_show_styles(webs_t wp, int argc, char_t ** argv)
{
	// <option value="blue" <% nvram_selected("router_style", "blue");
	// %>>Blue</option>
	DIR *directory;
	char buf[256];

	directory = opendir("/www/style");
	struct dirent *entry;

	while ((entry = readdir(directory)) != NULL) {
		sprintf(buf, "style/%s/style.css", entry->d_name);
		FILE *web = getWebsFile(buf);

		if (web == NULL) {
			sprintf(buf, "/www/style/%s/style.css", entry->d_name);
			if (!f_exists(buf))
				continue;
		}
		fclose(web);

		websWrite(wp, "<option value=\"%s\" %s>%s</option>\n", entry->d_name, nvram_match("router_style", entry->d_name) ? "selected=\"selected\"" : "", entry->d_name);
	}
	closedir(directory);
	return;
}

#ifdef HAVE_LANGUAGE
// extern websRomPageIndexType websRomPageIndex[];
void ej_show_languages(webs_t wp, int argc, char_t ** argv)
{
	char buf[256];

	websWrite(wp, "<script type=\"text/javascript\">\n//<![CDATA[\n");
	int i = 0;

	while (PwebsRomPageIndex[i].path != NULL) {
		cprintf("checking %s\n", PwebsRomPageIndex[i].path);
		if (!strncmp(PwebsRomPageIndex[i].path, "lang_pack/", strlen("lang_pack/"))) {
			cprintf("found language\n");
			if (strlen(PwebsRomPageIndex[i].path) < 14)
				continue;
			strcpy(buf, PwebsRomPageIndex[i].path);
			char *mybuf = &buf[strlen("lang_pack/")];

			if (strchr(mybuf, (int)'-') == NULL) {
				mybuf[strlen(mybuf) - 3] = 0;	// strip .js
				websWrite(wp, "document.write(\"<option value=\\\"%s\\\" %s >\" + management.lang_%s + \"</option>\");\n", mybuf, nvram_match("language", mybuf) ? "selected=\\\"selected\\\"" : "", mybuf);
			}
		}
		i++;
	}
	websWrite(wp, "//]]>\n</script>\n");
	return;
}
#endif

static char *directories[] = {
	"/jffs/etc/config",
	"/mmc/etc/config",
	"/etc/config",
};

static int checkandadd(char *name)
{
	static char *list = NULL;
	if (!name) {
		if (list)
			free(list);
		list = NULL;
		return 0;
	}
	if (!list) {
		list = malloc(strlen(name) + 1);
		strcpy(list, name);
	} else {
		char cap[128];
		char *next;
		foreach(cap, list, next) {
			if (!strcmp(cap, name)) {
				return 1;
			}
		}
		list = realloc(list, strlen(list) + 1 + strlen(name) + 1);
		sprintf(list, "%s %s", list, name);
	}
	return 0;

}

void ej_show_modules(webs_t wp, int argc, char_t ** argv)
{
	char buf[256];
	struct dirent *entry;
	DIR *directory;
	int resultcount = 0;
	char *result[256];
	result[0] = NULL;

	// display modules
	int idx;

	for (idx = 0; idx < 3; idx++) {
		directory = opendir(directories[idx]);
		if (directory == NULL)
			continue;
		// list all files in this directory
		while ((entry = readdir(directory)) != NULL) {
			if (checkandadd(entry->d_name))
				continue;
			if (argc > 0) {
				if (endswith(entry->d_name, argv[0])) {
#if defined(HAVE_ERC)
					if (strcmp(entry->d_name, "base.webconfig") && !wp->userid)	//show only base.webconfig for this user and nothing else
					{
						continue;
					}
#endif
					sprintf(buf, "%s/%s", directories[idx], entry->d_name);
					result[resultcount] = malloc(strlen(entry->d_name) + 1);
					strcpy(result[resultcount], entry->d_name);
					resultcount++;
					result[resultcount] = NULL;
				}
			} else {
				if (endswith(entry->d_name, ".webconfig")) {
					sprintf(buf, "%s/%s", directories[idx], entry->d_name);
					result[resultcount] = malloc(strlen(entry->d_name) + 1);
					strcpy(result[resultcount], entry->d_name);
					resultcount++;
					result[resultcount] = NULL;
				}
			}

		}
		/* now sort entries to solve EXT2 unsorted problem */
		int i, a;
		for (a = 0; a < resultcount; a++) {
			int change = 0;
			for (i = 0; i < resultcount - 1; i++) {
				int step = 0;
			      again:;
				if (!result[i][step] || !result[i + 1][step])
					continue;
				if (result[i][step] == result[i + 1][step]) {
					step++;
					goto again;
				}
				if (result[i][step] > result[i + 1][step]) {
					char *temp = result[i + 1];
					result[i + 1] = result[i];
					result[i] = temp;
					step = 0;
					change++;
				}
			}
			if (!change)
				break;	//no more sortable entries found, so just break up here
		}
		for (i = 0; i < resultcount; i++) {
			sprintf(buf, "%s/%s", directories[idx], result[i]);
			do_ej(NULL, buf, wp, NULL);
		}
		for (i = 0; i < resultcount; i++) {
			free(result[i]);
		}
		resultcount = 0;
		result[0] = NULL;
		closedir(directory);
	}
	checkandadd(NULL);
	return;
}

void ej_get_sysmodel(webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_XIOCOM
	websWrite(wp, "XWR");
#elif HAVE_ONNET
#ifdef HAVE_ONNET_BLANK
	if (nvram_match("DD_BOARD", "Atheros Hornet")) {
		websWrite(wp, "9331");
	} else if (nvram_match("DD_BOARD", "Compex WPE72")) {
		websWrite(wp, "E72");
	} else if (nvram_match("DD_BOARD", "ACCTON AC622")) {
		if (iscpe()) {
			websWrite(wp, "7240-2");
		} else {
			websWrite(wp, "7240-2");
		}
	} else if (nvram_match("DD_BOARD", "ACCTON AC722")) {
		if (iscpe()) {
			websWrite(wp, "OTAi 7240-5");
		} else {
			websWrite(wp, "OTAi 7240-5");
		}
	} else if (nvram_match("DD_BOARD", "Compex WP546")) {
		websWrite(wp, "546");
	} else {
		websWrite(wp, "%s", nvram_get("DD_BOARD"));
	}
#else
	if (nvram_match("DD_BOARD", "Atheros Hornet")) {
		websWrite(wp, "OTAi 9331");
	} else if (nvram_match("DD_BOARD", "Compex WPE72")) {
		websWrite(wp, "OTAi 724");
	} else if (nvram_match("DD_BOARD", "ACCTON AC622")) {
		if (iscpe()) {
			websWrite(wp, "OTAi 724S");
		} else {
			websWrite(wp, "OTAi 724AP");
		}
	} else if (nvram_match("DD_BOARD", "ACCTON AC722")) {
		if (iscpe()) {
			websWrite(wp, "OTAi 724S");
		} else {
			websWrite(wp, "OTAi 724AP");
		}
	} else if (nvram_match("DD_BOARD", "Compex WP546")) {
		websWrite(wp, "OTAi 724S");
	} else {
		websWrite(wp, "OTAi %s", nvram_get("DD_BOARD"));
	}
#endif
#elif HAVE_SANSFIL
	websWrite(wp, "%s", "SANSFIL");
#elif HAVE_KORENRON
	websWrite(wp, "KORENRON %s", nvram_get("DD_BOARD"));
#else
	websWrite(wp, "%s", nvram_safe_get("DD_BOARD"));
#endif
} void ej_get_syskernel(webs_t wp, int argc, char_t ** argv)
{
	struct utsname name;
	uname(&name);
	websWrite(wp, "%s %s %s %s", name.sysname, name.release, name.version, name.machine);
} void ej_get_totaltraff(webs_t wp, int argc, char_t ** argv)
{
	char *type;
	static char wanface[32];
	char line[256];
	unsigned long rcvd, sent, megcounti, megcounto;
	FILE *in;
	int ifl;

#ifndef FASTWEB
	if (argc < 1) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	type = argv[0];

	if (!nvram_match("ttraff_enable", "1"))
		return;

	if (nvram_match("ttraff_iface", "") || !nvram_get("ttraff_iface"))
		strncpy(wanface, get_wan_face(), sizeof(wanface));
	else
		strncpy(wanface, nvram_safe_get("ttraff_iface"), sizeof(wanface));
	strcat(wanface, ":");

	in = fopen("/proc/net/dev", "rb");
	if (in == NULL)
		return;

	/* eat first two lines */
	fgets(line, sizeof(line), in);
	fgets(line, sizeof(line), in);
	while (fgets(line, sizeof(line), in) != NULL) {
		ifl = 0;

		if (strstr(line, wanface)) {
			while (line[ifl] != ':')
				ifl++;
			line[ifl] = 0;

			sscanf(line + ifl + 1, "%lu %*ld %*ld %*ld %*ld %*ld %*ld %*ld %lu %*ld %*ld %*ld %*ld %*ld %*ld %*ld", &rcvd, &sent);
		}
	}

	fclose(in);

	rcvd >>= 20;		// output in MBytes
	sent >>= 20;

	if ((in = fopen("/tmp/.megc", "r")) != NULL) {
		fgets(line, sizeof(line), in);
		sscanf(line, "%lu:%lu", &megcounti, &megcounto);
		rcvd += megcounti;
		sent += megcounto;
		fclose(in);
	}

	if (!strcmp(type, "in")) {
		websWrite(wp, "%lu", rcvd);	// output in MBytes
	} else if (!strcmp(type, "out")) {
		websWrite(wp, "%lu", sent);
	}
	return;
}

void show_bwif(webs_t wp, char *ifname, char *name)
{
	websWrite(wp, "<h2>%s - %s</h2>\n", live_translate("status_band.h2"), name);
	websWrite(wp, "<fieldset>\n");
	websWrite(wp, "<iframe src=\"/graph_if.svg?%s\" width=\"555\" height=\"275\" frameborder=\"0\" type=\"image/svg+xml\">\n", ifname);
	websWrite(wp, "</iframe>\n");
	websWrite(wp, "</fieldset>\n");
	websWrite(wp, "<br />\n");
} void ej_show_bandwidth(webs_t wp, int argc, char_t ** argv)
{
	char name[32];
	char *next, *bnext;
	char var[80];
	char eths[256];
	char eths2[256];
	static char bword[256];
	char bufferif[512];
#ifdef HAVE_ATH9K
	glob_t globbuf;
	char globstring[1024];
	int globresult;
#endif

	show_bwif(wp, nvram_safe_get("lan_ifname"), "LAN");
	memset(eths, 0, sizeof(eths));
	getIfLists(eths, sizeof(eths));
	//add ppp interfacs
	memset(eths2, 0, sizeof(eths2));
	getIfLists(eths2, "ppp");
	sprinf(eths,"%s %s",eths,eths2);
	//add tun interfaces
	memset(eths2, 0, sizeof(eths2));
	getIfLists(eths2, "tun");
	sprinf(eths,"%s %s",eths,eths2);
	
	memset(bufferif, 0, 256);
	getIfList(bufferif, "br");

#ifndef HAVE_MADWIFI
	int cnt = get_wl_instances();
	int c;
	for (c = 0; c < cnt; c++) {
		strcat(bufferif, " ");
		strcat(bufferif, get_wl_instance_name(c));
	}
#endif
	foreach(var, eths, next) {
		if (!strcmp("etherip0", var))
			continue;
		if (!strncmp("ath", var, 3))
			continue;
		if (strchr(var, '.') == NULL) {
			if (!strcmp(get_wan_face(), var))
				continue;
			if (!strcmp(nvram_safe_get("lan_ifname"), var))
				continue;
			foreach(bword, bufferif, bnext) {
				if (!strcmp(bword, var)) {
					goto skip;
				}
			}
			sprintf(name, "LAN (%s)", var);
			show_bwif(wp, var, name);
		}
	      skip:;
	}

	if (!nvram_match("wan_proto", "disabled")) {
#ifdef HAVE_MADWIFI
		if (getSTA()) {
			sprintf(name, "%s WAN", live_translate("share.wireless"));
			show_bwif(wp, get_wan_face(), name);
		} else
			show_bwif(wp, get_wan_face(), "WAN");
#else
		if (!getSTA())
			show_bwif(wp, get_wan_face(), "WAN");
#endif

		if (nvram_match("dtag_vlan8", "1")) {
			if (getRouterBrand() == ROUTER_WRT600N || getRouterBrand() == ROUTER_WRT610N)
				show_bwif(wp, "eth2.0008", "IPTV");
			else
				show_bwif(wp, "eth0.0008", "IPTV");
		}

	}
#ifdef HAVE_MADWIFI
	int c = getdevicecount();
	int i;

	for (i = 0; i < c; i++) {
		char dev[32];

		sprintf(dev, "ath%d", i);

		sprintf(name, "%s (%s)", live_translate("share.wireless"), dev);
		show_bwif(wp, dev, name);
		char *vifs = nvram_nget("%s_vifs", dev);

		if (vifs == NULL)
			continue;
		foreach(var, vifs, next) {
			sprintf(name, "%s (%s)", live_translate("share.wireless"), var);
			show_bwif(wp, var, name);
		}
		int s;

		for (s = 1; s <= 10; s++) {
			char *wdsdev;

			wdsdev = nvram_nget("%s_wds%d_if", dev, s);
			if (strlen(wdsdev) == 0)
				continue;
			if (nvram_nmatch("0", "%s_wds%d_enable", dev, s))
				continue;
			sprintf(name, "%s (%s)", live_translate("share.wireless"), wdsdev);
			show_bwif(wp, wdsdev, name);
		}
#ifdef HAVE_ATH9K
		if (is_ath9k(dev)) {
			sprintf(globstring, "/sys/class/ieee80211/phy*/device/net/%s.sta*", dev);
			globresult = glob(globstring, GLOB_NOSORT, NULL, &globbuf);
			int awdscount;
			for (awdscount = 0; awdscount < globbuf.gl_pathc; awdscount++) {
				char *ifname;
				ifname = strrchr(globbuf.gl_pathv[awdscount], '/');
				if (!ifname)
					continue;
				sprintf(name, "%s (%s)", live_translate("share.wireless"), ifname);
				show_bwif(wp, ifname, name);
			}
			globfree(&globbuf);
		}
#endif
	}

#else
	for (c = 0; c < cnt; c++) {
		sprintf(name, "%s (wl%d)", live_translate("share.wireless"), c);
		show_bwif(wp, get_wl_instance_name(c), name);
	}
#endif
#ifdef HAVE_WAVESAT

	sprintf(name, "%s", live_translate("wl_wimax.titl"));
	show_bwif(wp, "ofdm", name);
#endif
}

void ej_do_menu(webs_t wp, int argc, char_t ** argv)
{
	char *mainmenu, *submenu;

#ifndef FASTWEB
	if (argc < 2) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	mainmenu = argv[0];
	submenu = argv[1];

	int vlan_supp = check_vlan_support();

#ifdef HAVE_SPUTNIK_APD
	int sputnik = nvram_match("apd_enable", "1");
#else
	int sputnik = 0;
#endif
	int openvpn = nvram_match("openvpn_enable",
				  "1") | nvram_match("openvpncl_enable", "1");
	int auth = nvram_match("status_auth", "1");
	int registered = 1;
#ifdef HAVE_REGISTER
	if (!isregistered_real())
		registered = 0;
	int cpeonly = iscpe();
#else
	int cpeonly = 0;
#endif

#ifdef HAVE_MADWIFI
#ifdef HAVE_NOWIFI
	int wifi = 0;
#else
	int wifi = haswifi();
#endif
#endif
#ifdef HAVE_MADWIFI
	int wimaxwifi = 0;
#endif
#ifdef HAVE_ERC
	static char menu_s[8][12][32] = {
		{"index.asp", "DDNS.asp", "", "", "", "", "", "", "", "", "", ""},	//
		{"Wireless_Basic.asp", "WL_WPATable.asp", "", "", "", "", "", "", "", "", "", ""},	//
		{"ForwardSpec.asp", "", "", "", "", "", "", "", "", "", "", ""},	//
		{"Filters.asp", "", "", "", "", "", "", "", "", "", "", ""},	//
		{"Management.asp", "", "", "", "", "", "", "", "", "", "", ""},	//
		{"", "", "", "", "", "", "", "", "", "", "", ""},	//
		{"", "", "", "", "", "", "", "", "", "", "", ""},	//
		{"", "", "", "", "", "", "", "", "", "", "", ""},	//
		{"", "", "", "", "", "", "", "", "", "", "", ""}	//
	};
	/*
	 * real name is bmenu.menuname[i][j] 
	 */
	static char menuname_s[8][13][32] = {
		{"setup", "setupbasic", "setupddns", "", "", "", "", "", "", "", "", "", ""},	//
		{"wireless", "wirelessBasic", "wirelessSecurity", "", "", "", "", "", "", "", "", "", ""},	//
		{"applications", "applicationspforwarding", "", "", "", "", "", "", "", "", "", "", ""},	//
		{"accrestriction", "webaccess", "", "", "", "", "", "", "", "", "", "", ""},	//
		{"admin", "adminManagement", "", "", "", "", "", "", "", "", "", "", ""},	//
		{"", "", "", "", "", "", "", "", "", "", "", "", ""},	//
		{"", "", "", "", "", "", "", "", "", "", "", "", ""},	//
		{"", "", "", "", "", "", "", "", "", "", "", "", ""},	//
		{"", "", "", "", "", "", "", "", "", "", "", "", ""}	//
	};

#elif HAVE_IPR

#endif

	static char menu_t[8][12][32] = {
		{"index.asp", "DDNS.asp", "WanMAC.asp", "Routing.asp", "Vlan.asp", "Networking.asp", "eop-tunnel.asp", "", "", "", "", ""},	// 
		{"Wireless_Basic.asp", "SuperChannel.asp", "WiMAX.asp", "Wireless_radauth.asp", "WL_WPATable.asp", "AOSS.asp", "Wireless_MAC.asp", "Wireless_Advanced.asp", "Wireless_WDS.asp", "", "", ""},	//
		{"Services.asp", "FreeRadius.asp", "PPPoE_Server.asp", "PPTP.asp", "USB.asp", "NAS.asp", "Hotspot.asp", "Nintendo.asp", "Milkfish.asp", "Privoxy.asp", "", ""},	//
		{"Firewall.asp", "VPN.asp", "", "", "", "", "", "", "", "", "", ""},	//
		{"Filters.asp", "", "", "", "", "", "", "", "", "", "", ""},	//
		{"ForwardSpec.asp", "Forward.asp", "Triggering.asp", "UPnP.asp", "DMZ.asp", "QoS.asp", "P2P.asp", "", "", "", "", ""},	//
		{"Management.asp", "Alive.asp", "Diagnostics.asp", "Wol.asp", "Factory_Defaults.asp", "Upgrade.asp", "config.asp", "", "", "", "", ""},	//
		{"Status_Router.asp", "Status_Internet.asp", "Status_Lan.asp", "Status_Wireless.asp", "Status_SputnikAPD.asp", "Status_OpenVPN.asp", "Status_Bandwidth.asp", "Info.htm", "register.asp", "MyPage.asp", "Gpio.asp", "Status_CWMP.asp"}	//
	};
	/*
	 * real name is bmenu.menuname[i][j] 
	 */
	static char menuname_t[8][13][32] = {
		{"setup", "setupbasic", "setupddns", "setupmacclone", "setuprouting", "setupvlan", "networking", "setupeop", "", "", "", "", ""},	//
		{"wireless", "wirelessBasic", "wirelessSuperchannel", "wimax", "wirelessRadius", "wirelessSecurity",	//
#ifdef HAVE_WPS
		 "wirelessAossWPS",
#else
		 "wirelessAoss",
#endif
		 "wirelessMac", "wirelessAdvanced", "wirelessWds", "", "", ""},	//
		{"services", "servicesServices", "servicesRadius", "servicesPppoesrv", "servicesPptp", "servicesUSB", "servicesNAS", "servicesHotspot", "servicesNintendo", "servicesMilkfish", "servicesPrivoxy", "", ""},	//
		{"security", "firwall", "vpn", "", "", "", "", "", "", "", "", "", ""},	// 
		{"accrestriction", "webaccess", "", "", "", "", "", "", "", "", "", "", ""},	//
		{"applications", "applicationspforwarding", "applicationsprforwarding", "applicationsptriggering", "applicationsUpnp", "applicationsDMZ", "applicationsQoS", "applicationsP2P", "", "", "", "", ""},	//
		{"admin", "adminManagement", "adminAlive", "adminDiag", "adminWol", "adminFactory", "adminUpgrade", "adminBackup", "", "", "", "", ""},	//
		{"statu", "statuRouter", "statuInet", "statuLAN", "statuWLAN", "statuSputnik", "statuVPN", "statuBand", "statuSysInfo", "statuActivate", "statuMyPage", "statuGpio", "statuCWMP"}	//
	};
	static char menu[8][12][32];
	static char menuname[8][13][32];
	memcpy(menu, menu_t, 8 * 12 * 32);
	memcpy(menuname, menuname_t, 8 * 13 * 32);
#if HAVE_ERC
	if (!wp->userid) {
		memcpy(menu, menu_s, 8 * 12 * 32);
		memcpy(menuname, menuname_s, 8 * 13 * 32);
	}
#endif
#ifdef HAVE_IPR
	if (!wp->userid) {
		sprintf(&menu[0][2][0], "");	// setup - mac cloning
		//sprintf(&menu[0][4][0], "");  // setup - routing / test!
		sprintf(&menu[2][4][0], "");	// services - USB
		sprintf(&menu[2][5][0], "");	// services - NAS
		sprintf(&menu[2][6][0], "");	// services - Hotspot
		sprintf(&menu[6][2][0], "");	// administration - commands
		sprintf(&menu[6][5][0], "");	// administration - upgrade
	}
	sprintf(&menu[2][9][0], "");	// services - anchorfree
#endif
#ifdef HAVE_CORENET
	sprintf(&menuname[0][0][0], "setupnetw");
	sprintf(&menuname[6][0][0], "adminman");
#endif
//fprintf(stderr,"generate menu content\n");
#ifdef HAVE_MADWIFI
#if defined(HAVE_BUFFALO) && !defined(HAVE_ATH9K)
	sprintf(&menu[1][8][0], "");
	sprintf(&menuname[1][9][0], "");
#else
	// fill up WDS
	int ifcount = getdevicecount();
	if (ifcount > 4)
		ifcount = 4;	//truncate to max of 4
	int a;

	for (a = 0; a < ifcount; a++) {
		sprintf(&menu[1][a + 8][0], "Wireless_WDS-ath%d.asp", a);
		if (ifcount == 1)
			sprintf(&menuname[1][a + 9][0], "wirelessWds");
		else
			sprintf(&menuname[1][a + 9][0], "wirelessWds%d", a);
	}
#endif
#else
#ifdef HAVE_ERC
	if (wp->userid) {
#endif

		int ifcount = get_wl_instances();
		int a;

		for (a = 0; a < ifcount; a++) {
			sprintf(&menu[1][a * 2 + 7][0], "Wireless_Advanced-wl%d.asp", a);
			sprintf(&menu[1][a * 2 + 8][0], "Wireless_WDS-wl%d.asp", a);
			if (ifcount == 1) {
				sprintf(&menuname[1][a * 2 + 8][0], "wirelessAdvanced");
				sprintf(&menuname[1][a * 2 + 9][0], "wirelessWds");
			} else {
				sprintf(&menuname[1][a * 2 + 8][0], "wirelessAdvancedwl%d", a);
				sprintf(&menuname[1][a * 2 + 9][0], "wirelessWdswl%d", a);
			}
		}
#ifdef HAVE_ERC
	}
#endif
#endif
//fprintf(stderr,"generate menu header content\n");

	int i, j;

	websWrite(wp, "<div id=\"menu\">\n");
	websWrite(wp, " <div id=\"menuMain\">\n");
	websWrite(wp, "  <ul id=\"menuMainList\">\n");
#ifdef HAVE_WAVESAT
	wimaxwifi = 1;
#endif

#define MAXMENU 8
#define MAXSUBMENU 12

	for (i = 0; i < MAXMENU; i++) {
		if (strlen(menu[i][0]) == 0)
			continue;
#ifdef HAVE_MADWIFI
		if (!wifi && !wimaxwifi && !strcmp(menu[i][0], "Wireless_Basic.asp"))
			i++;
#endif
#ifdef HAVE_CORENET
		if (!strcmp(menu[i][0], "Firewall.asp") || !strcmp(menu[i][0], "Filters.asp") || !strcmp(menu[i][0], "ForwardSpec.asp"))	// jump over
			// Corenet
			i++;
#endif
		if (i >= MAXMENU)
			break;
//fprintf(stderr,"generate menu %s\n",menu[i][0]);
		if (!strcmp(menu[i][0], mainmenu)) {
#ifdef HAVE_MADWIFI
			if (!wifi && wimaxwifi && !strcmp(menu[i][0], "Wireless_Basic.asp"))
				websWrite(wp, "   <li class=\"current\"><span><strong><script type=\"text/javascript\">Capture(bmenu.wimax)</script></strong></span>\n");
			else
#endif
				websWrite(wp, "   <li class=\"current\"><span><strong><script type=\"text/javascript\">Capture(bmenu.%s)</script></strong></span>\n", menuname[i][0]);
			websWrite(wp, "    <div id=\"menuSub\">\n");
			websWrite(wp, "     <ul id=\"menuSubList\">\n");

			for (j = 0; j < MAXSUBMENU; j++) {
#ifdef HAVE_MADWIFI
				if (!wifi && !strncmp(menu[i][j], "Wireless_Basic.asp", 8))
					j++;
#ifndef HAVE_SUPERCHANNEL
				if (!strcmp(menu[i][j], "SuperChannel.asp"))	// jump over
					// PPTP in
					// micro
					// build
					j++;
#else
				if (!strcmp(menu[i][j], "SuperChannel.asp") && (issuperchannel() || !wifi))	// jump 
					// over 
					// PPTP 
					// in 
					// micro 
					// build
					j++;
#endif
#else
				if (!strcmp(menu[i][j], "SuperChannel.asp"))	// jump over
					// PPTP in
					// micro
					// build
					j++;
#endif
#ifndef HAVE_WAVESAT
				if (!strcmp(menu[i][j], "WiMAX.asp"))	// jump over
					// WiMAX
					j++;
#else
				if (!wimaxwifi && !strcmp(menu[i][j], "WiMAX.asp"))	// jump 
					// over 
					// WiMAX
					j++;
#endif
#ifndef HAVE_AOSS
				if (!strcmp(menu[i][j], "AOSS.asp"))	// jump over
					// AOSS
					j++;
#endif
#ifdef HAVE_MADWIFI
				if (!wifi && !strcmp(menu[i][j], "WL_WPATable.asp"))	// jump 
					// over 
					// PPTP 
					// in 
					// micro 
					// build
					j++;
				if (!strcmp(menu[i][j], "Wireless_radauth.asp"))
					j++;
				if (!wifi && !strncmp(menu[i][j], "Wireless_MAC.asp", 8))
					j++;
				if (!strncmp(menu[i][j], "Wireless_Advanced", 17))
					j++;
				if ((!wifi || cpeonly)
				    && !strncmp(menu[i][j], "Wireless_WDS", 12))
					j++;
				if (!wifi && !strcmp(menu[i][j], "Status_Wireless.asp"))
					j++;

#endif
				if ((!vlan_supp) && !strcmp(menu[i][j], "Vlan.asp"))	// jump 
					// over 
					// VLANs 
					// if 
					// vlan 
					// not 
					// supported
					j++;
#ifndef HAVE_FREERADIUS
				if (!strcmp(menu[i][j], "FreeRadius.asp"))
					j++;
#endif
#ifndef HAVE_PPPOESERVER
				if (!strcmp(menu[i][j], "PPPoE_Server.asp"))
					j++;
#endif
#ifdef HAVE_MICRO
				if (!strcmp(menu[i][j], "PPTP.asp"))	// jump over PPTP in
					// micro build
					j++;
#endif
#ifndef HAVE_USB
				if (!strcmp(menu[i][j], "USB.asp"))	// jump over USB
					j++;
#endif
#ifndef HAVE_NAS_SERVER
				if (!strcmp(menu[i][j], "NAS.asp"))	// jump over NAS
					j++;
#endif
#ifdef HAVE_GLAUCO
				if (!strcmp(menu[i][j], "Factory_Defaults.asp"))
					j++;
				if (!strcmp(menu[i][j], "Upgrade.asp"))
					j++;
#endif
#ifdef HAVE_SANSFIL
				if (!strcmp(menu[i][j], "Hotspot.asp"))
					j++;
#endif
#ifndef HAVE_SPOTPASS
				if (!strcmp(menu[i][j], "Nintendo.asp"))	// jump over
					// Nintendo
					j++;
#endif
#ifndef HAVE_MILKFISH
				if (!strcmp(menu[i][j], "Milkfish.asp"))
					j++;
#endif
//#ifdef HAVE_WIKINGS
//                              if (!strcmp(menu[i][j], "AnchorFree.asp"))
//                                      j++;
//#endif
#ifndef HAVE_PRIVOXY
				if (!strcmp(menu[i][j], "Privoxy.asp"))
					j++;
#endif
//#ifdef HAVE_ESPOD
//                              if (!strcmp(menu[i][j], "AnchorFree.asp"))
//                                      j++;
//#endif
//#ifdef HAVE_CARLSONWIRELESS
//                              if (!strcmp(menu[i][j], "AnchorFree.asp"))
//                                      j++;
//#endif
#ifndef HAVE_WOL
				if (!strcmp(menu[i][j], "Wol.asp"))
					j++;
#endif
#ifndef HAVE_EOP_TUNNEL
				if (!strcmp(menu[i][j], "eop-tunnel.asp"))
					j++;
#endif
#ifndef HAVE_VLANTAGGING
				if (!strcmp(menu[i][j], "Networking.asp"))
					j++;
#endif
#ifndef HAVE_CTORRENT
				if (!strcmp(menu[i][j], "P2P.asp"))
					j++;
#endif
				if ((!sputnik) && !strcmp(menu[i][j], "Status_SputnikAPD.asp"))	// jump 
					// over 
					// Sputnik
					j++;
				if ((!openvpn) && !strcmp(menu[i][j], "Status_OpenVPN.asp"))	// jump 
					// over 
					// OpenVPN
					j++;
				if ((!auth) && !strcmp(menu[i][j], "Info.htm"))	// jump 
					// over 
					// Sys-Info
					j++;
				if ((registered) && !cpeonly && !strcmp(menu[i][j], "register.asp"))	// jump 
					// over 
					// register.asp
					j++;
				if ((!strlen(nvram_safe_get("mypage_scripts"))) && !strcmp(menu[i][j], "MyPage.asp"))	// jump 
					// over 
					// MyPage.asp
					j++;
#ifndef HAVE_STATUS_GPIO
				if (!strcmp(menu[i][j], "Gpio.asp"))
					j++;
#endif
#ifndef HAVE_FREECWMP
				if (!strcmp(menu[i][j], "Status_CWMP.asp"))
					j++;
#endif
				if (j >= MAXSUBMENU)
					break;
#ifdef HAVE_MADWIFI
				if (!strcmp(menu[i][j], submenu)
				    && (strlen(menu[i][j])
					&& !strcmp(menu[i][j], "Wireless_Basic.asp")
					&& !wifi && wimaxwifi)) {
					websWrite(wp, "      <li><span><strong><script type=\"text/javascript\">Capture(bmenu.wimax)</script></strong></span></li>\n");
				}
#endif
				else if (!strcmp(menu[i][j], submenu)
					 && (strlen(menu[i][j]))) {
					websWrite(wp, "      <li><span><strong><script type=\"text/javascript\">Capture(bmenu.%s)</script></strong></span></li>\n", menuname[i][j + 1]);
				}
#ifdef HAVE_HTTPS		// until https will allow upgrade and backup
#ifdef HAVE_MATRIXSSL
				else if ((strlen(menu[i][j]) != 0) && (do_ssl)
					 && ((!strcmp(menu[i][j], "Upgrade.asp")
					      || (!strcmp(menu[i][j], "config.asp"))))) {
					websWrite(wp, "      <script type=\"text/javascript\">\n//<![CDATA[\n");
					websWrite(wp,
						  "      document.write(\"<li><a style=\\\"cursor:pointer\\\" title=\\\"\" + errmsg.err46 + \"\\\" onclick=\\\"alert(errmsg.err45)\\\" ><em>\" + bmenu.%s + \"</em></a></li>\");\n",
						  menuname[i][j + 1]);
					websWrite(wp, "      \n//]]>\n</script>\n");
				}
#endif
#endif
#ifdef HAVE_MADWIFI
				else if (strlen(menu[i][j])
					 && !strcmp(menu[i][j], "Wireless_Basic.asp")
					 && !wifi && wimaxwifi) {
					websWrite(wp, "      <li><a href=\"WiMAX.asp\"><strong><script type=\"text/javascript\">Capture(bmenu.wimax)</script></strong></a></li>\n");
				}
#endif
				else if (strlen(menu[i][j])) {
					websWrite(wp, "      <li><a href=\"%s\"><strong><script type=\"text/javascript\">Capture(bmenu.%s)</script></strong></a></li>\n", menu[i][j], menuname[i][j + 1]);
				}
			}
			websWrite(wp, "     </ul>\n");
			websWrite(wp, "    </div>\n");
			websWrite(wp, "    </li>\n");
		}
#ifdef HAVE_MADWIFI
		else if (!strcmp(menu[i][0], "Wireless_Basic.asp") && !wifi && wimaxwifi) {
			websWrite(wp, "      <li><a href=\"WiMAX.asp\"><strong><script type=\"text/javascript\">Capture(bmenu.wimax)</script></strong></a></li>\n");
		}
#endif
		else {
			websWrite(wp, "   <li><a href=\"%s\"><strong><script type=\"text/javascript\">Capture(bmenu.%s)</script></strong></a></li>\n", menu[i][0], menuname[i][0]);
		}
	}
	websWrite(wp, "  </ul>\n");
	websWrite(wp, " </div>\n");
	websWrite(wp, "</div>\n");

	return;
}

void ej_do_pagehead(webs_t wp, int argc, char_t ** argv)	// Eko
{
	char *style = nvram_get("router_style");

#ifndef FASTWEB
	if (argc < 1) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	/*
	 * websWrite (wp, "<\?xml version=\"1.0\" encoding=\"%s\"\?>\n",
	 * live_translate("lang_charset.set")); IE Problem ... 
	 */ websWrite(wp,
		      "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n");
	websWrite(wp, "<html>\n");
	websWrite(wp, "\t<head>\n");
	websWrite(wp, "\t\t<meta http-equiv=\"Content-Type\" content=\"application/xhtml+xml; charset=%s\" />\n", live_translate("lang_charset.set"));
#ifndef HAVE_MICRO
	websWrite(wp, "\t\t<link rel=\"icon\" href=\"images/favicon.ico\" type=\"image/x-icon\" />\n");
	websWrite(wp, "\t\t<link rel=\"shortcut icon\" href=\"images/favicon.ico\" type=\"image/x-icon\" />\n");
#endif
	websWrite(wp, "\t\t<script type=\"text/javascript\" src=\"common.js\"></script>\n");
	websWrite(wp, "\t\t<script type=\"text/javascript\" src=\"lang_pack/english.js\"></script>\n");
#ifdef HAVE_LANGUAGE
	websWrite(wp, "\t\t<script type=\"text/javascript\" src=\"lang_pack/language.js\"></script>\n");
#endif
// temp
#ifdef HAVE_FREECWMP
	websWrite(wp, "\t\t<script type=\"text/javascript\" src=\"lang_pack/freecwmp-english.js\"></script>\n");
#endif
	websWrite(wp, "\t\t<link type=\"text/css\" rel=\"stylesheet\" href=\"style/%s/style.css\" />\n", style);
	websWrite(wp, "\t\t<!--[if IE]><link type=\"text/css\" rel=\"stylesheet\" href=\"style/%s/style_ie.css\" /><![endif]-->\n", style);

#ifdef HAVE_PWC
	websWrite(wp, "\t\t<script type=\"text/javascript\" src=\"js/prototype.js\"></script>\n");
	websWrite(wp, "\t\t<script type=\"text/javascript\" src=\"js/effects.js\"></script>\n");
	websWrite(wp, "\t\t<script type=\"text/javascript\" src=\"js/window.js\"></script>\n");
	websWrite(wp, "\t\t<script type=\"text/javascript\" src=\"js/window_effects.js\"></script>\n");
	websWrite(wp, "\t\t<link type=\"text/css\" rel=\"stylesheet\" href=\"style/pwc/default.css\" />\n");
	websWrite(wp, "\t\t<link type=\"text/css\" rel=\"stylesheet\" href=\"style/pwc/ddwrt.css\" />\n");
#endif
#ifdef HAVE_WIKINGS
	websWrite(wp, "\t\t<title>:::: Excel Networks ::::");
#elif HAVE_ESPOD
	websWrite(wp, "\t\t<title>ESPOD Technologies");
#elif HAVE_SANSFIL
	websWrite(wp, "\t\t<title>SANSFIL (build %s)", SVN_REVISION);
#else
	websWrite(wp, "\t\t<title>%s (build %s)", nvram_get("router_name"), SVN_REVISION);
#endif

	if (strlen(argv[0]) != 0) {
		websWrite(wp, " - %s", live_translate(argv[0]));
	}
	websWrite(wp, "</title>\n");

}

void ej_do_hpagehead(webs_t wp, int argc, char_t ** argv)	// Eko
{
	char *htitle;

#ifdef FASTWEB
	ejArgs(argc, argv, "%s", &htitle);
#else
	if (ejArgs(argc, argv, "%s", &htitle) < 1) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	websWrite(wp, "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n");
	if (!strcmp(htitle, "doctype_only"))
		return;		// stop here, for About.htm
	websWrite(wp, "<html>\n");
	websWrite(wp, "\t<head>\n");
	websWrite(wp, "\t\t<meta http-equiv=\"Content-Type\" content=\"application/xhtml+xml; charset=%s\" />\n", live_translate("lang_charset.set"));
	websWrite(wp, "\t\t<script type=\"text/javascript\" src=\"../common.js\"></script>\n");
	websWrite(wp, "\t\t<script type=\"text/javascript\" src=\"../lang_pack/english.js\"></script>\n");
#ifdef HAVE_LANGUAGE
	websWrite(wp, "\t\t<script type=\"text/javascript\" src=\"../lang_pack/language.js\"></script>\n");
#endif
	websWrite(wp, "\t\t<link type=\"text/css\" rel=\"stylesheet\" href=\"help.css\">\n");
	websWrite(wp, "\t\t<title>%s (build %s)", live_translate("share.help"), SVN_REVISION);
	websWrite(wp, " - %s</title>\n", live_translate(htitle));
	websWrite(wp, "\t</head>\n");

}

void ej_show_timeoptions(webs_t wp, int argc, char_t ** argv)	// Eko
{

	char timediffs[39][8] = {
		"-12", "-11", "-10", "-09.5", "-09", "-08", "-07", "-06", "-05",
		"-04.5", "-04",
		"-03.5", "-03", "-02", "-01", "+00",
		"+01", "+02", "+03", "+03.5", "+04", "+04.5", "+05",
		"+05.5", "+05.75", "+06", "+06.5", "+07", "+08",
		"+09", "+09.5", "+10", "+10.5", "+11", "+11.5",
		"+12", "+12.75", "+13", "+14"
	};

	char timezones[39][8] = {
		"-12:00", "-11:00", "-10:00", "-09:30", "-09:00", "-08:00",
		"-07:00",
		"-06:00", "-05:00", "-04:30", "-04:00", "-03:30",
		"-03:00", "-02:00", "-01:00", "", "+01:00",
		"+02:00", "+03:00", "+03:30", "+04:00", "+04:30",
		"+05:00", "+05:30", "+05:45", "+06:00", "+06:30",
		"+07:00", "+08:00", "+09:00", "+09:30", "+10:00",
		"+10:30", "+11:00", "+11:30", "+12:00", "+12:45",
		"+13:00", "+14:00"
	};

	int i;

	for (i = 0; i < 39; i++) {
		websWrite(wp, "<option value=\"%s\" %s>UTC%s</option>\n", timediffs[i], nvram_match("time_zone", timediffs[i]) ? "selected=\"selected\"" : "", timezones[i]);

	}

}

void ej_show_wanipinfo(webs_t wp, int argc, char_t ** argv)	// Eko
{
	char *wan_ipaddr;
	int wan_link;

	if (getWET() || nvram_match("wan_proto", "disabled")
	    || nvram_match("wan_proto", "bridge")) {
		websWrite(wp, ": %s", live_translate("share.disabled"));
		return;
	}

	wan_link = check_wan_link(0);
	char *wan_proto = nvram_safe_get("wan_proto");
	if (!strcmp(wan_proto, "pptp")) {
		wan_ipaddr = wan_link ? nvram_safe_get("pptp_get_ip") : nvram_safe_get("wan_ipaddr");
	} else if (!strcmp(wan_proto, "pppoe")
#ifdef HAVE_PPPOATM
		   || !strcmp(wan_proto, "pppoa")
#endif
#ifdef HAVE_3G
		   || !strcmp(wan_proto, "3g")
#endif
#ifdef HAVE_IPETH
		   || !strcmp(wan_proto, "iphone")
#endif
	    ) {
		wan_ipaddr = wan_link ? nvram_safe_get("wan_ipaddr") : "0.0.0.0";
#ifdef HAVE_L2TP
	} else if (nvram_match("wan_proto", "l2tp")) {
		wan_ipaddr = wan_link ? nvram_safe_get("l2tp_get_ip") : nvram_safe_get("wan_ipaddr");
#endif
	} else {
		wan_ipaddr = nvram_safe_get("wan_ipaddr");
	}

	websWrite(wp, "&nbsp;IP: %s", wan_ipaddr);

	return;
}

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_selected("wan_proto", "dhcp",); %> produces: selected="selected"
 * <% nvram_selected_js("wan_proto", "dhcp"); %> produces: selected=\"selected\"
 * <% nvram_selected("wan_proto", "static"); %> does not produce
 */
void ej_nvram_selected(webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
	if (argc < 2) ;
	{
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	if (nvram_match(argv[0], argv[1])) {
		websWrite(wp, "selected=\"selected\"");
	}
	return;
}

void ej_nvram_selected_js(webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
	if (argc < 2) ;
	{
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	if (nvram_match(argv[0], argv[1])) {
		websWrite(wp, "selected=\\\"selected\\\"");
	}
	return;
}

void ej_getrebootflags(webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_RB500
	websWrite(wp, "1");
#elif HAVE_MAGICBOX
	websWrite(wp, "2");
#elif HAVE_RB600
	websWrite(wp, "2");
#elif HAVE_FONERA
	websWrite(wp, "2");
#elif HAVE_MERAKI
	websWrite(wp, "2");
#elif HAVE_LS2
	websWrite(wp, "2");
#elif HAVE_LS5
	websWrite(wp, "2");
#elif HAVE_WHRAG108
	websWrite(wp, "2");
#elif HAVE_TW6600
	websWrite(wp, "2");
#elif HAVE_CA8
	websWrite(wp, "2");
#elif HAVE_GATEWORX
	websWrite(wp, "1");
#elif HAVE_X86
	websWrite(wp, "1");
#elif HAVE_WHR300HP
	websWrite(wp, "3");
#elif HAVE_WZR300HP
	websWrite(wp, "3");
#elif HAVE_WZR600DHP
	websWrite(wp, "3");
#else
	websWrite(wp, "0");
#endif
} void ej_tran(webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
	if (argc != 1)
		return;
#endif
	websWrite(wp, "<script type=\"text/javascript\">Capture(%s)</script>", argv[0]);
	return;
}

/*
 * Example:
 * wan_proto=dhcp
 * <% nvram_checked("wan_proto", "dhcp"); %> produces: checked="checked"
 * <% nvram_checked_js("wan_proto", "dhcp"); %> produces: checked=\"checked\"
 * <% nvram_checked("wan_proto", "static"); %> does not produce
 */ void ej_nvram_checked(webs_t wp, int argc, char_t ** argv)
{

#ifdef FASTWEB
	if (argc < 2) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	if (nvram_match(argv[0], argv[1])) {
		websWrite(wp, "checked=\"checked\"");
	}

	return;
}

void ej_nvram_checked_js(webs_t wp, int argc, char_t ** argv)
{

#ifndef FASTWEB
	if (argc < 2) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	if (nvram_match(argv[0], argv[1])) {
		websWrite(wp, "checked=\\\"checked\\\"");
	}

	return;
}

void ej_make_time_list(webs_t wp, int argc, char_t ** argv)
{
	int i, st, en;
	char ic[16];

#ifndef FASTWEB
	if (argc < 3) {
		websError(wp, 400, "Insufficient args\n");
		return;
	}
#endif
	st = atoi(argv[1]);
	en = atoi(argv[2]);

	for (i = st; i <= en; i++) {
		sprintf(ic, "%d", i);
		websWrite(wp, "<option value=\"%d\" %s >%02d</option>\n", i, nvram_match(argv[0], ic) ? "selected=\"selected\"" : "", i);
	}

	return;
}

#ifdef HAVE_CPUTEMP
void ej_get_cputemp(webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_BCMMODERN

	static int tempcount = 0;
	char buf[WLC_IOCTL_SMLEN];
	char buf2[WLC_IOCTL_SMLEN];
	int ret;
	unsigned int *ret_int = NULL;
	unsigned int *ret_int2 = NULL;
	static double tempavg_24 = 0.000;
	static double tempavg_50 = 0.000;
	static double tempavg_max = 0.000;
	int no2 = 0, no5 = 0;
	strcpy(buf, "phy_tempsense");
	strcpy(buf2, "phy_tempsense");

	if (nvram_match("wl0_net_mode", "disabled") || (ret = wl_ioctl("eth1", WLC_GET_VAR, buf, sizeof(buf)))) {
		no2 = 1;
	}

	if (nvram_match("wl1_net_mode", "disabled") || (ret = wl_ioctl("eth2", WLC_GET_VAR, buf2, sizeof(buf2)))) {
		no5 = 1;
	}

	ret_int = (unsigned int *)buf;
	ret_int2 = (unsigned int *)buf2;

	if (tempcount == -2) {
		tempcount++;
		tempavg_24 = *ret_int;
		tempavg_50 = *ret_int2;
		if (tempavg_24 < 0.0)
			tempavg_24 = 0.0;
		if (tempavg_50 < 0.0)
			tempavg_50 = 0.0;
	} else {
		if (tempavg_24 < 10.0 && *ret_int > 0.0)
			tempavg_24 = *ret_int;
		if (tempavg_50 < 10.0 && *ret_int2 > 0.0)
			tempavg_50 = *ret_int2;

		if (tempavg_24 > 200.0 && *ret_int > 0.0)
			tempavg_24 = *ret_int;
		if (tempavg_50 > 200.0 && *ret_int2 > 0.0)
			tempavg_50 = *ret_int2;

		tempavg_24 = (tempavg_24 * 4 + *ret_int) / 5;
		tempavg_50 = (tempavg_50 * 4 + *ret_int2) / 5;
	}

	int cputemp = 1;
#ifdef HAVE_NORTHSTAR
	cputemp = 0;
	FILE *fp = fopen("/proc/dmu/temperature", "rb");
	if (fp) {
		fscanf(fp, "%d", &cputemp);
		fclose(fp);
		websWrite(wp, "CPU %d.%d &#176;C / ", cputemp / 10, cputemp % 10);
	}
#endif
	if (no2 && no5 && cputemp)
		websWrite(wp, "%s", live_translate("status_router.notavail"));	// no 
	else if (no2)
		websWrite(wp, "WL1 %4.2f &#176;C", tempavg_50 * 0.5 + 20.0);
	else if (no5)
		websWrite(wp, "WL0 %4.2f &#176;C", tempavg_24 * 0.5 + 20.0);
	else
		websWrite(wp, "WL0 %4.2f &#176;C / WL1 %4.2f &#176;C", tempavg_24 * 0.5 + 20.0, tempavg_50 * 0.5 + 20.0);
#else
#ifdef HAVE_GATEWORX
	int TEMP_MUL = 100;

	if (getRouterBrand() == ROUTER_BOARD_GATEWORX_SWAP)
		TEMP_MUL = 200;

	FILE *fp = fopen("/sys/devices/platform/IXP4XX-I2C.0/i2c-adapter:i2c-0/0-0028/temp_input",
			 "rb");
	if (!fp)
		fp = fopen("/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0028/temp1_input", "rb");
#elif HAVE_LAGUNA
	int TEMP_MUL = 10;
	FILE *fp = fopen("/sys/bus/i2c/devices/0-0029/temp0_input", "rb");
#elif HAVE_VENTANA
	int TEMP_MUL = 10;
	FILE *fp = fopen("/sys/bus/i2c/devices/0-0029/temp0_input", "rb");
#else
#define TEMP_MUL 1000
#ifdef HAVE_X86
	FILE *fp = fopen("/sys/devices/platform/i2c-1/1-0048/temp1_input", "rb");
#else
	FILE *fp = fopen("/sys/devices/platform/i2c-0/0-0048/temp1_input", "rb");
#endif
#endif

	if (fp == NULL) {
		websWrite(wp, "%s", live_translate("status_router.notavail"));	// no 
		// i2c 
		// lm75 
		// found
		return;
	}
	int temp;

	fscanf(fp, "%d", &temp);
	fclose(fp);
	int high = temp / TEMP_MUL;
	int low = (temp - (high * TEMP_MUL)) / (TEMP_MUL / 10);

	websWrite(wp, "%d.%d &#176;C", high, low);	// no i2c lm75 found
#endif
}

void ej_show_cpu_temperature(webs_t wp, int argc, char_t ** argv)
{
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(status_router.cputemp)</script></div>\n");
	websWrite(wp, "<span id=\"cpu_temp\">");
	ej_get_cputemp(wp, argc, argv);
	websWrite(wp, "</span>&nbsp;\n");
	websWrite(wp, "</div>\n");
}
#endif
#ifdef HAVE_VOLT
void ej_get_voltage(webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_LAGUNA
	FILE *fp = fopen("/sys/bus/i2c/devices/0-0029/in0_input", "rb");
#elif HAVE_VENTANA
	FILE *fp = fopen("/sys/bus/i2c/devices/0-0029/in0_input", "rb");
#else
	FILE *fp = fopen("/sys/devices/platform/IXP4XX-I2C.0/i2c-adapter:i2c-0/0-0028/volt",
			 "rb");
	if (!fp)
		fp = fopen("/sys/devices/platform/IXP4XX-I2C.0/i2c-0/0-0028/in1_input", "rb");
#endif
	if (fp == NULL) {
		websWrite(wp, "%s", live_translate("status_router.notavail"));	// no 
		// i2c 
		// lm75 
		// found
		return;
	}
	int temp;

	fscanf(fp, "%d", &temp);
	fclose(fp);
	// temp*=564;
	int high = temp / 1000;
	int low = (temp - (high * 1000)) / 100;

	websWrite(wp, "%d.%d V", high, low);	// no i2c lm75 found
}

void ej_show_voltage(webs_t wp, int argc, char_t ** argv)
{
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(status_router.inpvolt)</script></div>\n");
	websWrite(wp, "<span id=\"voltage\">");
	ej_get_voltage(wp, argc, argv);
	websWrite(wp, "</span>&nbsp;\n");
	websWrite(wp, "</div>\n");
}
#endif
static void showencstatus(webs_t wp, char *prefix)
{
	char akm[64];

	sprintf(akm, "%s_akm", prefix);
	websWrite(wp, "<div class=\"setting\">\n");
	websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.encrypt)</script>&nbsp;-&nbsp;<script type=\"text/javascript\">Capture(share.intrface)</script>&nbsp;%s</div>\n", prefix);
	websWrite(wp, "<script type=\"text/javascript\">");
	if (nvram_match(akm, "disabled")) {
		websWrite(wp, "Capture(share.disabled)");
		websWrite(wp, "</script>");
	} else {
		websWrite(wp, "Capture(share.enabled)");
		websWrite(wp, "</script>,&nbsp;");

		if (nvram_match(akm, "psk"))
			websWrite(wp, "WPA Personal");
		if (nvram_match(akm, "wpa"))
			websWrite(wp, "WPA Enterprise");
		if (nvram_match(akm, "psk2"))
			websWrite(wp, "WPA2 Personal");
		if (nvram_match(akm, "wpa2"))
			websWrite(wp, "WPA2 Enterprise");
		if (nvram_match(akm, "psk psk2"))
			websWrite(wp, "WPA2 Personal Mixed");
		if (nvram_match(akm, "wpa wpa2"))
			websWrite(wp, "WPA Enterprise Mixed");
		if (nvram_match(akm, "radius"))
			websWrite(wp, "RADIUS");
		if (nvram_match(akm, "wep"))
			websWrite(wp, "WEP");
		if (nvram_match(akm, "8021X"))
			websWrite(wp, "802.1x");
	}

	websWrite(wp, "\n</div>\n");
	return;
}

void ej_get_txpower(webs_t wp, int argc, char_t ** argv)
{
	char txpwr[32];
	char m[32];
	int txpower;
	char mode[32];

	strncpy(m, nvram_safe_get("wifi_display"), 4);
	m[4] = 0;
	sprintf(mode, "%s_net_mode", m);
	if (nvram_match(mode, "disabled")) {
		txpower = 0;
		websWrite(wp, "%s", live_translate("wl_basic.radio_off"));
	} else {

		sprintf(txpwr, "%s_txpwr", m);
#ifdef HAVE_MADWIFI
		txpower = wifi_gettxpower(m);
#elif HAVE_RT2880
		txpower = atoi(nvram_safe_get(txpwr));
#else				//broadcom
		txpower = bcm_gettxpower(m);
#endif
#ifdef HAVE_BUFFALO
		get_txpower_extended(wp, txpower, m);
#else
#ifdef HAVE_MADWIFI
		websWrite(wp, "%d dBm", txpower);
#elif HAVE_RT2880
		websWrite(wp, "%d mW", txpower);
#else				//broadcom
		websWrite(wp, "%d mW", txpower);
#endif
#endif
	}
}

void ej_getencryptionstatus(webs_t wp, int argc, char_t ** argv)
{
	char *mode = nvram_safe_get("wifi_display");

	showencstatus(wp, mode);
} void ej_getwirelessstatus(webs_t wp, int argc, char_t ** argv)
{
	char var[32];
	char m[32];
	int showap = 0, showcli = 0;

	strncpy(m, nvram_safe_get("wifi_display"), 4);
	m[4] = 0;
	sprintf(var, "%s_mode", m);

	if (nvram_match(var, "ap") || nvram_match(var, "wdsap")) {
		showap = 1;	// "Clients"
	} else {
		showcli = 1;	// "Access Point"
		sprintf(var, "%s_vifs", m);
		if (strlen(nvram_safe_get(var)) > 0)
			showap = 1;	// " & Clients"
	}

	if (showcli)
		websWrite(wp, "<script type=\"text/javascript\">Capture(info.ap)</script>");
	if (showcli && showap)
		websWrite(wp, " & ");
	if (showap)
		websWrite(wp, "<script type=\"text/javascript\">Capture(status_wireless.legend3)</script>");

}

void ej_getwirelessssid(webs_t wp, int argc, char_t ** argv)
{
	char ssid[32];

	sprintf(ssid, "%s_ssid", nvram_safe_get("wifi_display"));
	tf_webWriteESCNV(wp, ssid);

} void ej_getwirelessmode(webs_t wp, int argc, char_t ** argv)
{
	char mode[32];

	sprintf(mode, "%s_mode", nvram_safe_get("wifi_display"));

	websWrite(wp, "<script type=\"text/javascript\">");
	if (nvram_match(mode, "wet"))
		websWrite(wp, "Capture(wl_basic.clientBridge)");
	if (nvram_match(mode, "ap"))
		websWrite(wp, "Capture(wl_basic.ap)");
	if (nvram_match(mode, "sta"))
		websWrite(wp, "Capture(wl_basic.client)");
	if (nvram_match(mode, "infra"))
		websWrite(wp, "Capture(wl_basic.adhoc)");
	if (nvram_match(mode, "apsta"))
		websWrite(wp, "Capture(wl_basic.repeater)");
	if (nvram_match(mode, "apstawet"))
		websWrite(wp, "Capture(wl_basic.repeaterbridge)");
	if (nvram_match(mode, "wdssta"))
		websWrite(wp, "Capture(wl_basic.wdssta)");
	if (nvram_match(mode, "wdsap"))
		websWrite(wp, "Capture(wl_basic.wdsap)");
	websWrite(wp, "</script>&nbsp;\n");
}

void ej_getwirelessnetmode(webs_t wp, int argc, char_t ** argv)
{

	char mode[32];
	char m[32];

	strncpy(m, nvram_safe_get("wifi_display"), 4);
	m[4] = 0;
	sprintf(mode, "%s_net_mode", m);

	websWrite(wp, "<script type=\"text/javascript\">");
	if (nvram_match(mode, "disabled"))
		websWrite(wp, "Capture(share.disabled)");
	if (nvram_match(mode, "mixed"))
		websWrite(wp, "Capture(wl_basic.mixed)");
	if (nvram_match(mode, "bg-mixed"))
		websWrite(wp, "Capture(wl_basic.bg)");
	if (nvram_match(mode, "g-only"))
		websWrite(wp, "Capture(wl_basic.g)");
	if (nvram_match(mode, "b-only"))
		websWrite(wp, "Capture(wl_basic.b)");
	if (nvram_match(mode, "n-only"))
		websWrite(wp, "Capture(wl_basic.n)");
	if (nvram_match(mode, "a-only"))
		websWrite(wp, "Capture(wl_basic.a)");
	if (nvram_match(mode, "na-only"))
		websWrite(wp, "Capture(wl_basic.na)");
	if (nvram_match(mode, "ng-only"))
		websWrite(wp, "Capture(wl_basic.ng)");
	if (nvram_match(mode, "n2-only"))
		websWrite(wp, "Capture(wl_basic.n2)");
	if (nvram_match(mode, "n5-only"))
		websWrite(wp, "Capture(wl_basic.n5)");
	if (nvram_match(mode, "ac-only"))
		websWrite(wp, "Capture(wl_basic.ac)");
	websWrite(wp, "</script>&nbsp;\n");
}

void ej_show_openvpn_status(webs_t wp, int argc, char_t ** argv)
{
	websWrite(wp, "<fieldset>\n<legend><script type=\"text/javascript\">Capture(share.state)</script></legend>\n");

	system2("/etc/openvpnstate.sh > /tmp/.temp");
	FILE *in = fopen("/tmp/.temp", "r");

	while (!feof(in)) {
		int b = getc(in);

		if (b != EOF)
			wfputc(b, wp);
	}
	fclose(in);
	websWrite(wp, "</fieldset><br />");
	websWrite(wp, "<fieldset>\n<legend><script type=\"text/javascript\">Capture(share.statu)</script></legend>\n");
	system2("/etc/openvpnstatus.sh > /tmp/.temp");
	in = fopen("/tmp/.temp", "r");
	while (!feof(in)) {
		int b = getc(in);

		if (b != EOF)
			wfputc(b, wp);
	}
	fclose(in);
	websWrite(wp, "</fieldset><br />");
	websWrite(wp, "<fieldset>\n<legend><script type=\"text/javascript\">Capture(log.legend)</script></legend>\n");
	system2("/etc/openvpnlog.sh > /tmp/.temp");
	in = fopen("/tmp/.temp", "r");
	while (!feof(in)) {
		int b = getc(in);

		if (b != EOF)
			wfputc(b, wp);
	}
	fclose(in);
	websWrite(wp, "</fieldset><br />");

}

void ej_radio_on(webs_t wp, int argc, char_t ** argv)
{
	int radiooff = -1;

#ifdef HAVE_MADWIFI
	char *ifname = nvram_safe_get("wifi_display");

	if (strlen(ifname) > 0) {
		int state = get_radiostate(ifname);

		switch (state) {
		case 1:
			websWrite(wp, "1");
			break;
		default:
			websWrite(wp, "0");
			break;
		}
	} else {
		websWrite(wp, "0");
	}
#elif HAVE_RT2880

	int state = get_radiostate("wl0");

	switch (state) {
	case 1:
		websWrite(wp, "1");
		break;
	default:
		websWrite(wp, "0");
		break;
	}
#else
	char name[32];
	sprintf(name, "%s_ifname", nvram_safe_get("wifi_display"));

	char *ifname = nvram_safe_get(name);

	wl_ioctl(ifname, WLC_GET_RADIO, &radiooff, sizeof(int));

	switch ((radiooff & WL_RADIO_SW_DISABLE)) {
	case 0:
		websWrite(wp, "1");
		break;
	default:
		websWrite(wp, "0");
		break;
	}
#endif
}

void ej_get_radio_state(webs_t wp, int argc, char_t ** argv)
{
	int radiooff = -1;

#ifdef HAVE_MADWIFI
	char *ifname = nvram_safe_get("wifi_display");

	if (strlen(ifname) > 0) {
		int state = get_radiostate(ifname);

		switch (state) {
		case 1:
			websWrite(wp, "%s", live_translate("wl_basic.radio_on"));
			break;
		case -1:
			websWrite(wp, "%s", live_translate("share.unknown"));
			break;
		default:	// 1: software disabled, 2: hardware
			// disabled, 3: both are disabled
			websWrite(wp, "%s", live_translate("wl_basic.radio_off"));
			break;
		}
	} else {
		websWrite(wp, "%s", live_translate("share.unknown"));
	}
#elif HAVE_RT2880

	int state = get_radiostate("wl0");

	switch (state) {
	case 1:
		websWrite(wp, "%s", live_translate("wl_basic.radio_on"));
		break;
	case -1:
		websWrite(wp, "%s", live_translate("share.unknown"));
		break;
	default:		// 1: software disabled, 2: hardware
		// disabled, 3: both are disabled
		websWrite(wp, "%s", live_translate("wl_basic.radio_off"));
		break;
	}
#else
	char name[32];
	sprintf(name, "%s_ifname", nvram_safe_get("wifi_display"));

	char *ifname = nvram_safe_get(name);

	wl_ioctl(ifname, WLC_GET_RADIO, &radiooff, sizeof(int));

	switch ((radiooff & WL_RADIO_SW_DISABLE)) {
	case 0:
		websWrite(wp, "%s", live_translate("wl_basic.radio_on"));
		break;
	case -1:
		websWrite(wp, "%s", live_translate("share.unknown"));
		break;
	default:		// 1: software disabled, 2: hardware
		// disabled, 3: both are disabled
		websWrite(wp, "%s", live_translate("wl_basic.radio_off"));
		break;
	}
#endif
}

void ej_dumparptable(webs_t wp, int argc, char_t ** argv)
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
	int conn_count = 0;

	if ((f = fopen("/proc/net/arp", "r")) != NULL) {
		while (fgets(buf, sizeof(buf), f)) {
			if (sscanf(buf, "%15s %*s %*s %17s %*s %s", ip, mac, landev) != 3)
				continue;
			if ((strlen(mac) != 17)
			    || (strcmp(mac, "00:00:00:00:00:00") == 0))
				continue;
			if (strcmp(landev, nvram_safe_get("wan_iface")) == 0)
				continue;	// skip all but LAN arp entries
			strcpy(hostname, "*");	// set name to *

			/*
			 * count open connections per IP 
			 */
			if ((conn = fopen("/proc/net/ip_conntrack", "r")) != NULL) {
				strcpy(ip2, ip);
				strcat(ip2, " ");

				while (fgets(buf, sizeof(buf), conn)) {
					if (strstr(buf, ip2))
						conn_count++;
				}
				fclose(conn);
			}

			/*
			 * end count 
			 */

			/*
			 * do nslookup 
			 */

			// struct servent *servp;
			// char buf1[256];
			// 
			// getHostName (buf1, ip);
			// if (strcmp(buf1, "unknown"))
			// strcpy (hostname, buf1);
			/*
			 * end nslookup 
			 */

			/*
			 * look into hosts file for hostnames (static leases) 
			 */
			if ((host = fopen("/tmp/hosts", "r")) != NULL && !strcmp(hostname, "*")) {
				while (fgets(buf, sizeof(buf), host)) {
					sscanf(buf, "%15s %*s", fullip);

					if (!strcmp(ip, fullip)) {
						sscanf(buf, "%*15s %s", hostname);
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

			if (!strcmp(hostname, "*")
			    && nvram_match("dhcp_dnsmasq", "1")
			    && nvram_match("dhcpd_usenvram", "0")) {
				if (!(host = fopen("/tmp/dnsmasq.leases", "r")))
					host = fopen("/jffs/dnsmasq.leases", "r");

				if (host) {

					while (fgets(buf, sizeof(buf), host)) {
						sscanf(buf, "%*s %*s %15s %*s", fullip);

						if (strcmp(ip, fullip) == 0) {
							sscanf(buf, "%*s %*s %*s %s", hostname);
						}
					}
					fclose(host);
				}
			}
			/*
			 * end dnsmasq.leases check 
			 */

			/*
			 * check nvram for dnsmasq leases in nvram if hostname is still
			 * unknown 
			 */

			if (!strcmp(hostname, "*")
			    && nvram_match("dhcp_dnsmasq", "1")
			    && nvram_match("dhcpd_usenvram", "1")) {
				sscanf(nvram_nget("dnsmasq_lease_%s", ip), "%*s %*s %*s %s", hostname);
			}
			/*
			 * end nvram check 
			 */

			websWrite(wp, "%c'%s','%s','%s','%d'", (count ? ',' : ' '), hostname, ip, mac, conn_count);
			++count;
			conn_count = 0;
		}
		fclose(f);
	}
}

#ifdef HAVE_PPPOESERVER

void ej_dumppppoe(webs_t wp, int argc, char_t ** argv)
{
	FILE *in = fopen("/tmp/pppoe_connected", "rb");
	if (!in)
		return;
	char pid[32];
	char ifname[32];
	char local[32];
	char remote[32];
	char peer[64];
	int count = 0;
	while (fscanf(in, "%s %s %s %s", pid, ifname, local, peer) == 4) {
		websWrite(wp, "%c\"%s\",\"%s\",\"%s\",\"%s\"", count ? ',' : ' ', ifname, peer, local, pid);
		count++;
		if (feof(in))
			break;
	}
	fclose(in);
	return;
}

#endif

#ifdef HAVE_EOP_TUNNEL

void ej_show_eop_tunnels(webs_t wp, int argc, char_t ** argv)
{

	int tun;
	char temp[32];

	for (tun = 1; tun < 11; tun++) {

		websWrite(wp, "<fieldset>\n");
		websWrite(wp, "<legend><script type=\"text/javascript\">Capture(eoip.tunnel)</script> %d</legend>\n", tun);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.srv)</script></div>\n");
		sprintf(temp, "oet%d_en", tun);
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idoet%d', true)\" /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
			  temp, (nvram_match(temp, "1") ? "checked=\"checked\"" : ""), tun);
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idoet%d', false)\" /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
			  temp, (nvram_match(temp, "0") ? "checked=\"checked\"" : ""), tun);
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div id=\"idoet%d\">\n", tun);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.remoteIP)</script></div>\n");
		websWrite(wp, "<input type=\"hidden\" name=\"oet%d_rem\" value=\"0.0.0.0\"/>\n", tun);
		sprintf(temp, "oet%d_rem", tun);
		websWrite(wp,
			  "<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_0\" onblur=\"valid_range(this,0,255,eoip.remoteIP)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_1\" onblur=\"valid_range(this,0,255,eoip.tunnelID)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_2\" onblur=\"valid_range(this,0,255,eoip.tunnelID)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_3\" onblur=\"valid_range(this,1,254,eoip.tunnelID)\" value=\"%d\" />\n",
			  temp, get_single_ip(nvram_safe_get(temp), 0), temp, get_single_ip(nvram_safe_get(temp), 1), temp, get_single_ip(nvram_safe_get(temp), 2), temp, get_single_ip(nvram_safe_get(temp), 3));
		websWrite(wp, "</div>\n");
/*
	websWrite( wp, "<div class=\"setting\">\n" );
	websWrite( wp,
		   "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.tunnelID)</script></div>\n" );
		  	sprintf( temp, "oet%d_id", tun );
	websWrite( wp,
		   "<input size=\"4\" maxlength=\"3\" class=\"num\" name=\"%s\" onblur=\"valid_range(this,0,999,eoip.tunnelID)\" value=\"%s\" />\n",
		   temp, nvram_get( temp ) );
	websWrite( wp, "</div>\n" );

	websWrite( wp, "<div class=\"setting\">\n" );
	websWrite( wp,
		   "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.comp)</script></div>\n" );
	sprintf( temp, "oet%d_comp", tun );
	websWrite( wp,
		   "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
		   temp,
		   ( nvram_match( temp, "1" ) ? "checked=\"checked\"" :
		     "" ) );
	websWrite( wp,
		   "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
		   temp,
		   ( nvram_match( temp, "0" ) ? "checked=\"checked\"" :
		     "" ) );
	websWrite( wp, "</div>\n" );
	websWrite( wp, "<div class=\"setting\">\n" );
	websWrite( wp,
		   "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.passtos)</script></div>\n" );
	sprintf( temp, "oet%d_pt", tun );
	websWrite( wp,
		   "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
		   temp,
		   ( nvram_match( temp, "1" ) ? "checked=\"checked\"" :
		     "" ) );
	websWrite( wp,
		   "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
		   temp,
		   ( nvram_match( temp, "0" ) ? "checked=\"checked\"" :
		     "" ) );
	websWrite( wp, "</div>\n" );
	websWrite( wp, "<div class=\"setting\">\n" );
	websWrite( wp,
		   "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.frag)</script></div>\n" );
	sprintf( temp, "oet%d_fragment", tun );
	websWrite( wp,
		   "<input size=\"4\" maxlength=\"4\" class=\"num\" name=\"%s\" onblur=\"valid_range(this,0,1500,eoip.frag)\" value=\"%s\" />\n",
		   temp, nvram_get( temp ) );
	websWrite( wp, "</div>\n" );
	websWrite( wp, "<div class=\"setting\">\n" );
	websWrite( wp,
		   "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.mssfix)</script></div>\n" );
	sprintf( temp, "oet%d_mssfix", tun );
	websWrite( wp,
		   "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
		   temp,
		   ( nvram_match( temp, "1" ) ? "checked=\"checked\"" :
		     "" ) );
	websWrite( wp,
		   "<input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
		   temp,
		   ( nvram_match( temp, "0" ) ? "checked=\"checked\"" :
		     "" ) );
	websWrite( wp, "</div>\n" );
	websWrite( wp, "<div class=\"setting\">\n" );
	websWrite( wp,
		   "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.shaper)</script></div>\n" );
	sprintf( temp, "oet%d_shaper", tun );
	websWrite( wp,
		   "<input size=\"6\" maxlength=\"6\" class=\"num\" name=\"%s\" onblur=\"valid_range(this,0,100000,eoip.shaper)\" value=\"%s\" />\n",
		   temp, nvram_get( temp ) );
	websWrite( wp, "</div>\n" );
*/
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(eoip.bridging)</script></div>\n");
		sprintf(temp, "oet%d_bridged", tun);
		websWrite(wp,
			  "<input class=\"spaceradio\" type=\"radio\" value=\"1\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idbridged%d', false)\" /><script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;\n",
			  temp, (nvram_match(temp, "1") ? "checked=\"checked\"" : ""), tun);
		websWrite(wp,
			  " <input class=\"spaceradio\" type=\"radio\" value=\"0\" name=\"%s\" %s onclick=\"show_layer_ext(this, 'idbridged%d', true)\" /><script type=\"text/javascript\">Capture(share.disable)</script>\n",
			  temp, (nvram_match(temp, "0") ? "checked=\"checked\"" : ""), tun);
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div id=\"idbridged%d\">\n", tun);
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.ip)</script></div>\n");
		websWrite(wp, "<input type=\"hidden\" name=\"oet%d_ipaddr\" value=\"0.0.0.0\"/>\n", tun);
		sprintf(temp, "oet%d_ipaddr", tun);
		websWrite(wp,
			  "<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_0\" onblur=\"valid_range(this,0,255,share.ip)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_1\" onblur=\"valid_range(this,0,255,share.ip)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_2\" onblur=\"valid_range(this,0,255,share.ip)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_3\" onblur=\"valid_range(this,1,254,share.ip)\" value=\"%d\" />\n",
			  temp, get_single_ip(nvram_safe_get(temp), 0), temp, get_single_ip(nvram_safe_get(temp), 1), temp, get_single_ip(nvram_safe_get(temp), 2), temp, get_single_ip(nvram_safe_get(temp), 3));
		websWrite(wp, "</div>\n");
		websWrite(wp, "<div class=\"setting\">\n");
		websWrite(wp, "<div class=\"label\"><script type=\"text/javascript\">Capture(share.subnet)</script></div>\n");
		websWrite(wp, "<input type=\"hidden\" name=\"oet%d_netmask\" value=\"0.0.0.0\"/>\n", tun);
		sprintf(temp, "oet%d_netmask", tun);
		websWrite(wp,
			  "<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_0\" onblur=\"valid_range(this,0,255,share.subnet)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_1\" onblur=\"valid_range(this,0,255,share.subnet)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_2\" onblur=\"valid_range(this,0,255,share.subnet)\" value=\"%d\" />.<input size=\"3\" maxlength=\"3\" class=\"num\" name=\"%s_3\" onblur=\"valid_range(this,0,254,share.subnet)\" value=\"%d\" />\n",
			  temp, get_single_ip(nvram_safe_get(temp), 0), temp, get_single_ip(nvram_safe_get(temp), 1), temp, get_single_ip(nvram_safe_get(temp), 2), temp, get_single_ip(nvram_safe_get(temp), 3));
		websWrite(wp, "</div>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "</div>\n");
		websWrite(wp, "</fieldset><br/>\n");
	}
}
#endif
int tf_webWriteESC(webs_t wp, const char *value)
{
	char buf[512];
	int n;
	int r;
	const char *c;

	n = 0;
	r = 0;
	for (c = value; *c; c++) {
		if ((isprint(*c)) && (*c != '"') && (*c != '&')
		    && (*c != '<') && (*c != '>') && (*c != '\'')
		    && (*c != '\\')) {
			buf[n++] = *c;
		} else {
			sprintf(buf + n, "&#%d;", *c);
			n += strlen(buf + n);
		}
		if (n > (sizeof(buf) - 10)) {	// ! extra space for &...
			buf[n] = 0;
			n = 0;
			r += wfputs(buf, wp);
		}
	}
	if (n > 0) {
		buf[n] = 0;
		r += wfputs(buf, wp);
	}
	wfflush(wp);
	return r;
}

int tf_webWriteESCNV(webs_t wp, const char *nvname)
{
	return tf_webWriteESC(wp, nvram_safe_get(nvname));
}

int tf_webWriteJS(webs_t wp, const char *s)
{
	char buf[512];
	int n;
	int r;

	n = 0;
	r = 0;
	for (; *s; s++) {
		if (*s == '<') {
			sprintf(buf + n, "&lt;");
			n += 4;
		} else if (*s == '>') {
			sprintf(buf + n, "&gt;");
			n += 4;
		} else if ((*s != '"') && (*s != '\\') && (*s != '/')
			   && (*s != '*') && (*s != '\'') && (isprint(*s))) {
			buf[n++] = *s;
		} else {
			sprintf(buf + n, "\\x%02x", *s);
			n += 4;
		}
		if (n > (sizeof(buf) - 10)) {	// ! extra space for \xHH
			buf[n] = 0;
			n = 0;
			r += wfputs(buf, wp);
		}
	}
	if (n > 0) {
		buf[n] = 0;
		r += wfputs(buf, wp);
	}
	wfflush(wp);
	return r;
}

#if defined(HAVE_MINIDLNA) || defined(HAVE_NAS_SERVER)

struct fsentry {
	char fs[32];
	char mp[64];
	char fstype[16];
	char perms[4];
	struct fsentry *next;
};

struct fsentry *parsefsentry(char line[256])
{

	struct fsentry *entry = calloc(1, sizeof(struct fsentry));
	char *token, *perm;
	int tokcount = 0;

	line[strlen(line) - 1] = '\0';	// replace new line with null
	token = strtok(line, " ");
	while (token != NULL) {
		// check for values
		if (tokcount == 0) {
			strcpy(entry->fs, token);
		} else if (tokcount == 2) {
			if (!strncmp(token, "/tmp/proftpd", 12)) {
				free(entry);
				return NULL;	//skip it
			}
			if (!strncmp(token, "/tmp/mnt/", 9)) {
				char newpath[64];
				strcpy(newpath, token);
				char *slash = strrchr(newpath, '/') + 1;
				sprintf(entry->mp, "/mnt/%s", slash);	// convert symbolic link to absolute path
			} else {
				strcpy(entry->mp, token);
			}
		} else if (tokcount == 4) {
			strcpy(entry->fstype, token);
		} else if (tokcount == 5) {
			perm = strtok(token, ",");
			strncpy(entry->perms, perm + 1, strlen(perm) - 1);
		}
		// next token
		token = strtok(NULL, " ");
		tokcount++;
	}
	return entry;
}

struct fsentry *getfsentries()
{

	char line[512];
	FILE *fp;
	struct fsentry *list, *tmplist, *current;
	int count = 0;

	if ((fp = popen("mount", "r"))) {
		//current = list;
		while (fgets(line, sizeof(line), fp)) {
			//fprintf(stderr, "[MOUNTS] %s\n", line);
			tmplist = parsefsentry(line);
			if (!tmplist)
				continue;
			if (count == 0) {
				list = tmplist;
				current = list;
			} else {
				current->next = tmplist;
				current = current->next;
			}
			count++;
		}
		pclose(fp);
	}
	struct fsentry *entry = calloc(1, sizeof(struct fsentry));
	strcpy(entry->fs, "/mnt");
	strcpy(entry->fstype, "dummy");
	strcpy(entry->perms, "rw");
	strcpy(entry->mp, "/mnt");
	current->next = entry;
	current = current->next;
	return list;
}

#endif

#ifdef HAVE_MINIDLNA
#include <dlna.h>

void ej_dlna_sharepaths(webs_t wp, int argc, char_t ** argv)
{

	struct fsentry *fs, *current;
	struct dlna_share *cs, *csnext;
	char buffer[64], number[4], perms[16];
	int found, rows = 0;

	fs = getfsentries();
	struct dlna_share *dlnashares = getdlnashares();

	// share count var
	for (cs = dlnashares; cs; cs = cs->next) {
		rows++;
	}
	rows--;
	websWrite(wp, "	<input type=\"hidden\" name=\"dlna_shares_count\" id=\"dlna_shares_count\" value=\"%d\">\n", rows);
	rows = 5;

	websWrite(wp, "	<input type=\"hidden\" name=\"dlna_shares_count_limit\" id=\"dlna_shares_count_limit\" value=\"%d\">\n", rows);
	rows = 0;

	// table header
	websWrite(wp, "	<table id=\"dlna_shares\" class=\"table center\" summary=\"dlna share table\">\n");
	websWrite(wp, "		<tr><th colspan=\"5\"><script type=\"text/javascript\">Capture(service.samba3_shares)</script></th></tr>\n");
	websWrite(wp, "		<tr>\n");
	websWrite(wp, "			<th><script type=\"text/javascript\">Capture(service.samba3_share_path)</script></th>\n");
	websWrite(wp, "			<th><script type=\"text/javascript\">Capture(service.dlna_type_audio)</script></th>\n");
	websWrite(wp, "			<th><script type=\"text/javascript\">Capture(service.dlna_type_video)</script></th>\n");
	websWrite(wp, "			<th><script type=\"text/javascript\">Capture(service.dlna_type_images)</script></th>\n");
	websWrite(wp, "			<th style=\"width: 50px;\">&nbsp;</th>\n");
	websWrite(wp, "		</tr>\n");

	for (cs = dlnashares; cs; cs = csnext) {

		if (rows == 0) {
			// dummy entry
			sprintf(buffer, "%s", "id=\"dlna_shares_row_template\" style=\"display: none;\"");
			//sprintf(number, '\0');
			number[0] = '\0';
		} else {
			sprintf(buffer, "id=\"dlna_shares_row_%d\"", rows);
			sprintf(number, "_%d", rows);
		}

		websWrite(wp, "		<tr %s>\n", buffer);

		// display filesystems to mount
		found = 0;
		//sprintf( perms, "");
		perms[0] = '\0';
		websWrite(wp,
			  "			<td id=\"n_dlna_mp%s\" style=\"width: 17.816em;\"><select name=\"dlnashare_mp%s\" id=\"dlnashare_mp%s\" style=\"width: 100%%;\" onchange=\"setDlnaShareAccessOptions(this);\">\n",
			  number, number, number);
		websWrite(wp, "				<option value=\"\" rel=\"\">-</option>\n");
		//fprintf(stderr, "[SAMBA] FS %s:%s public:%d\n", cs->label, cs->mp, cs->public );
		for (current = fs; current; current = current->next) {
			if (strcmp(current->fstype, "squashfs")
			    && strcmp(current->fstype, "rootfs")
			    && strcmp(current->fstype, "proc")
			    && strcmp(current->fstype, "sysfs")
			    && strcmp(current->fstype, "sysdebug")
			    && strcmp(current->fstype, "debugfs")
			    && strcmp(current->fstype, "ramfs")
			    && strcmp(current->fstype, "tmpfs")
			    && strcmp(current->fstype, "devpts")
			    && strcmp(current->fstype, "usbfs")) {
				// adjust the rights
				if ( /*rows == 0 || */ !strcmp(current->mp, "")) {
					sprintf(buffer, "%s", "");
				} else if (!strcmp(current->perms, "rw")) {
					sprintf(buffer, "%s", "\"rw\",\"ro\"");
				} else {
					sprintf(buffer, "%s", "\"ro\"");
				}

				if (!strcmp(current->mp, cs->mp)) {
					found = 1;
					sprintf(perms, "%s", current->perms);
				}

				websWrite(wp,
					  "				<option value=\"%s\" rel='{\"fstype\":\"%s\",\"perms\":[%s],\"avail\":1}' %s>%s</option>\n",
					  current->mp, current->fstype, buffer, strcmp(current->mp, cs->mp) ? "" : "selected=\"selected\"", current->mp);
			}
		}
		// fs not available -> add stored entry for display
		if (found == 0 && rows > 0) {
			websWrite(wp, "				<option value=\"%s\" rel='{\"fstype\":\"\",\"perms\":[\"%s\"],\"avail\":0}' selected>[not available!]</option>\n", cs->mp, cs->mp);
		}
		websWrite(wp, "				</select></td>\n");
		websWrite(wp,
			  "				<td style=\"width: 25px; text-align: center;\"><input type=\"checkbox\" name=\"dlnashare_audio%s\" id=\"dlnashare_audio%s\" value=\"1\" %s></td>\n",
			  number, number, cs->types & TYPE_AUDIO ? "checked" : "");
		websWrite(wp,
			  "				<td style=\"width: 25px; text-align: center;\"><input type=\"checkbox\" name=\"dlnashare_video%s\" id=\"dlnashare_video%s\" value=\"1\" %s></td>\n",
			  number, number, cs->types & TYPE_VIDEO ? "checked" : "");
		websWrite(wp,
			  "				<td style=\"width: 25px; text-align: center;\"><input type=\"checkbox\" name=\"dlnashare_images%s\" id=\"dlnashare_images%s\" value=\"1\" %s></td>\n",
			  number, number, cs->types & TYPE_IMAGES ? "checked" : "");

		websWrite(wp, "				<td style=\"width: 50px; text-align: center;\">\n");
		websWrite(wp, "					<input type=\"button\" class=\"button\" name=\"dlnashare_del%s\" value=\"Remove\"  style=\"width: 100%%;\" onclick=\"removeDlnaShare(this);\">\n", number);
		websWrite(wp, "				</td>\n");
		websWrite(wp, "			</tr>\n");

		rows++;
		csnext = cs->next;
		free(cs);
	}

	websWrite(wp, "		</table>\n");

	// add button
	websWrite(wp, "<div id=\"dlna_shares_add\" style=\"text-align: center;\"><input type=\"button\" class=\"button\" name=\"share_add\" value=\"Add Share\" onclick=\"addDlnaShare();\" /></div>");

	for (current = fs; fs; current = fs) {
		fs = current->next;
		free(current);
	}
}

#endif

#ifdef HAVE_NAS_SERVER

#include <samba3.h>

void ej_samba3_sharepaths(webs_t wp, int argc, char_t ** argv)
{

	struct fsentry *fs, *current;
	struct samba3_share *cs, *csnext;
	char buffer[64], number[4], perms[16];
	int found, rows = 0;

	fs = getfsentries();
	struct samba3_share *samba3shares = getsamba3shares();

	// share count var
	for (cs = samba3shares; cs; cs = cs->next) {
		rows++;
	}
	rows--;
	websWrite(wp, "	<input type=\"hidden\" name=\"samba_shares_count\" id=\"samba_shares_count\" value=\"%d\">\n", rows);
	rows = 5;

	websWrite(wp, "	<input type=\"hidden\" name=\"samba_shares_count_limit\" id=\"samba_shares_count_limit\" value=\"%d\">\n", rows);
	rows = 0;

	// table header
	websWrite(wp, "	<table id=\"samba_shares\" class=\"table center\" summary=\"samba share table\">\n");
	websWrite(wp, "		<tr><th colspan=\"6\"><script type=\"text/javascript\">Capture(service.samba3_shares)</script></th></tr>\n");
	websWrite(wp, "		<tr>\n");
	websWrite(wp, "			<th><script type=\"text/javascript\">Capture(service.samba3_share_path)</script></th>\n");
	websWrite(wp, "			<th><script type=\"text/javascript\">Capture(service.samba3_share_subdir)</script></th>\n");
	websWrite(wp, "			<th><script type=\"text/javascript\">Capture(service.samba3_share_label)</script></th>\n");
	websWrite(wp, "			<th><script type=\"text/javascript\">Capture(service.samba3_share_public)</script></th>\n");
	websWrite(wp, "			<th><script type=\"text/javascript\">Capture(service.samba3_share_access)</script></th>\n");
	websWrite(wp, "			<th style=\"width: 50px;\">&nbsp;</th>\n");
	websWrite(wp, "		</tr>\n");

	for (cs = samba3shares; cs; cs = csnext) {

		if (rows == 0) {
			// dummy entry
			sprintf(buffer, "%s", "id=\"samba_shares_row_template\" style=\"display: none;\"");
			//sprintf(number, '\0');
			number[0] = '\0';
		} else {
			sprintf(buffer, "id=\"samba_shares_row_%d\"", rows);
			sprintf(number, "_%d", rows);
		}

		websWrite(wp, "		<tr %s>\n", buffer);

		// display filesystems to mount
		found = 0;
		//sprintf( perms, "");
		perms[0] = '\0';
		websWrite(wp,
			  "			<td id=\"n_share_mp%s\" style=\"width: 17.816em;\"><select name=\"smbshare_mp%s\" id=\"smbshare_mp%s\" style=\"width: 100%%;\" onchange=\"setSambaShareAccessOptions(this);\">\n",
			  number, number, number);
		websWrite(wp, "				<option value=\"\" rel=\"\">-</option>\n");
		//fprintf(stderr, "[SAMBA] FS %s:%s public:%d\n", cs->label, cs->mp, cs->public );
		for (current = fs; current; current = current->next) {
			if (strcmp(current->fstype, "squashfs")
			    && strcmp(current->fstype, "rootfs")
			    && strcmp(current->fstype, "proc")
			    && strcmp(current->fstype, "sysfs")
			    && strcmp(current->fstype, "sysdebug")
			    && strcmp(current->fstype, "debugfs")
			    && strcmp(current->fstype, "ramfs")
			    && strcmp(current->fstype, "tmpfs")
			    && strcmp(current->fstype, "devpts")
			    && strcmp(current->fstype, "usbfs")) {
				// adjust the rights
				if ( /*rows == 0 || */ !strcmp(current->mp, "")) {
					sprintf(buffer, "%s", "");
				} else if (!strcmp(current->perms, "rw")) {
					sprintf(buffer, "%s", "\"rw\",\"ro\"");
				} else {
					sprintf(buffer, "%s", "\"ro\"");
				}

				if (!strcmp(current->mp, cs->mp)) {
					found = 1;
					sprintf(perms, "%s", current->perms);
				}

				websWrite(wp,
					  "				<option value=\"%s\" rel='{\"fstype\":\"%s\",\"perms\":[%s],\"avail\":1}' %s>%s</option>\n",
					  current->mp, current->fstype, buffer, strcmp(current->mp, cs->mp) ? "" : "selected=\"selected\"", current->mp);
			}
		}
		// fs not available -> add stored entry for display
		if (found == 0 && rows > 0) {
			websWrite(wp, "				<option value=\"%s\" rel='{\"fstype\":\"\",\"perms\":[\"%s\"],\"avail\":0}' selected>%s [not available!]</option>\n", cs->mp, cs->access_perms, cs->mp);
			sprintf(perms, "%s", cs->access_perms);
		}
		websWrite(wp, "				</select></td>\n");
		websWrite(wp,
			  "				<td style=\"width: 1%%;\"><input type=\"text\" name=\"smbshare_subdir%s\" id=\"smbshare_subdir%s\" value=\"%s\" style=\"width: 150px;\" onChange=\"updateSambaUserShare(this);\" /></td>\n",
			  number, number, cs->sd);
		websWrite(wp,
			  "				<td style=\"width: 1%%;\"><input type=\"text\" name=\"smbshare_label%s\" id=\"smbshare_label%s\" value=\"%s\" style=\"width: 120px;\" onChange=\"updateSambaUserShare(this);\" /></td>\n",
			  number, number, cs->label);
		websWrite(wp,
			  "				<td style=\"width: 25px; text-align: center;\"><input type=\"checkbox\" name=\"smbshare_public%s\" id=\"smbshare_public%s\" value=\"1\" %s></td>\n",
			  number, number, cs->public == 1 ? "checked" : "");
		websWrite(wp, "				<td>\n");
		websWrite(wp,
			  "					<select name=\"smbshare_access_perms%s\" id=\"smbshare_access_perms%s\" style=\"width: 100%%;\"%s>\n",
			  number, number, !strcmp(perms, "") ? " disabled" : "");
		if (rows == 0 || strcmp(perms, "")) {
			websWrite(wp, "						<option value=\"rw\"%s>Read/Write</option>\n", !strcmp(cs->access_perms, "rw") ? " selected" : "");
			websWrite(wp, "						<option value=\"ro\"%s>Read Only</option>\n", !strcmp(cs->access_perms, "ro") ? " selected" : "");
		}
		websWrite(wp, "					</select>\n");
		websWrite(wp, "					<input type=\"hidden\" name=\"smbshare_access_perms_prev_%d\" value=\"%s\">\n", rows, cs->access_perms);
		websWrite(wp, "				</td>\n");
		websWrite(wp, "				<td style=\"width: 50px; text-align: center;\">\n");
		websWrite(wp, "					<input type=\"button\" class=\"button\" name=\"smbshare_del%s\" value=\"Remove\"  style=\"width: 100%%;\" onclick=\"removeSambaShare(this);\">\n", number);
		websWrite(wp, "				</td>\n");
		websWrite(wp, "			</tr>\n");

		rows++;
		csnext = cs->next;
		free(cs);
	}

	websWrite(wp, "		</table>\n");

	// add button
	websWrite(wp, "<div id=\"samba_shares_add\" style=\"text-align: center;\"><input type=\"button\" class=\"button\" name=\"share_add\" value=\"Add Share\" onclick=\"addSambaShare();\" /></div>");

	for (current = fs; fs; current = fs) {
		fs = current->next;
		free(current);
	}
}

void ej_samba3_users(webs_t wp, int argc, char_t ** argv)
{

	struct samba3_share *cs, *csnext;
	struct samba3_shareuser *csu, *csunext;
	struct samba3_user *samba3users, *cu, *cunext;
	char buffer[64], number[4];
	int rows = 0, usershares = 0;

	samba3users = getsamba3users();
	struct samba3_share *samba3shares = getsamba3shares();

	// share count var
	for (cu = samba3users; cu; cu = cu->next) {
		rows++;
	}
	rows--;
	websWrite(wp, "	<input type=\"hidden\" name=\"samba_users_count\" id=\"samba_users_count\" value=\"%d\">\n", rows);
	rows = 10;

	websWrite(wp, "	<input type=\"hidden\" name=\"samba_users_count_limit\" id=\"samba_users_count_limit\" value=\"%d\">\n", rows);
	rows = 0;

	// table header
	websWrite(wp, "	<table id=\"samba_users\" class=\"table center\" summary=\"samba user table\">\n");

	websWrite(wp, "		<tr><th colspan=\"6\"><script type=\"text/javascript\">Capture(service.samba3_users)</script></th></tr>\n");
	websWrite(wp, "		<tr>\n");
	websWrite(wp, "			<th>username</th>\n");
	websWrite(wp, "			<th style=\"width:180px;\">password</th>\n");
	websWrite(wp, "			<th><script type=\"text/javascript\">Capture(service.samba3_user_shares)</script></th>\n");
	websWrite(wp, "			<th>samba</th>\n");
	websWrite(wp, "			<th>ftp</th>\n");

	websWrite(wp, "			<th style=\"width:50px;\">&nbsp;</th>\n");
	websWrite(wp, "		</tr>\n");

	for (cu = samba3users; cu; cu = cunext) {

		if (rows == 0) {
			// dummy entry
			sprintf(buffer, "%s", "id=\"n_smbuser_template\" style=\"display: none;\"");
			//sprintf(number, '\0');
			number[0] = '\0';
		} else {
			sprintf(buffer, "id=\"samba_users_row_%d\"", rows);
			sprintf(number, "_%d", rows);
		}

		websWrite(wp, "		<tr %s>\n", buffer);

		websWrite(wp, "			<td id=\"n_smbuser_user\" valign=\"top\" width=\"1%%\" align=\"center\">\n");
		websWrite(wp, "				<input type=\"text\" name=\"smbuser_username%s\" value=\"%s\" size=\"20\">\n", number, cu->username);
		websWrite(wp, "			</td>\n");

		websWrite(wp, "			<td id=\"n_smbuser_pass\" valign=\"top\" align=\"left\">\n");
		websWrite(wp, "				<input type=\"password\" name=\"smbuser_password%s\" id=\"smbuser_password%s\" value=\"%s\" size=\"12\">&nbsp;\n", number, number, cu->password);
		//websWrite(wp, "                               <div style=\"float: left;padding-top: 2px;\">\n");
		websWrite(wp,
			  "					<input type=\"checkbox\" name=\"smbuser_password_unmask%s\" value=\"0\" onclick=\"setElementMask('smbuser_password' + this.name.substr(23, this.name.length - 23), this.checked);\" />Unmask\n",
			  number, number);
		//websWrite(wp, "                               </div>\n");
		websWrite(wp, "			</td>\n");

		//fprintf( stderr, "[USERS] %s:%s\n", cu->username, cu->password );
		if (rows == 0) {
			websWrite(wp, "			<td id=\"n_smbuser_shareaccess\" valign=\"top\">\n");
			websWrite(wp, "				<div id=\"n_smbuser_share\"><input type=\"checkbox\" value=\"1\">&nbsp;<span>&nbsp</span></div>\n");
			websWrite(wp, "			</td>\n");
			websWrite(wp, "			<td style=\"width: 25px; text-align: center;\">\n");
			websWrite(wp, "				        <input type=\"checkbox\" name=\"smbuser_samba%s\" value=\"1\">\n", number);
			websWrite(wp, "			</td>\n");
			websWrite(wp, "			<td style=\"width: 25px; text-align: center;\">\n");
			websWrite(wp, "				        <input type=\"checkbox\" name=\"smbuser_ftp%s\" value=\"1\">\n", number);
			websWrite(wp, "			</td>\n");
		} else {
			websWrite(wp, "			<td id=\"n_smbuser_shareaccess\" valign=\"top\">\n");
			usershares = 0;
			for (cs = samba3shares; cs; cs = cs->next) {
				buffer[0] = '\0';
				for (csu = cs->users; csu; csu = csu->next) {
					if (!strcmp(csu->username, cu->username)) {
						//fprintf( stderr, "[USERSHARES] %s: %s\n", cs->label, csu->username );
						sprintf(buffer, " checked");
					}
				}
				if (usershares > 0) {
					websWrite(wp,
						  "				<div id=\"n_smbuser_share\"><input type=\"checkbox\" name=\"smbshare_%d_user_%d\"%s value=\"1\">&nbsp;<span>%s</span></div>\n",
						  usershares, rows, buffer, cs->label);
				}
				usershares++;
			}
			websWrite(wp, "			</td>\n");
			websWrite(wp, "			<td style=\"width: 25px; text-align: center;\">\n");
			websWrite(wp, "				<input type=\"checkbox\" name=\"smbuser_samba%s\" value=\"1\" %s>\n", number, cu->sharetype & SHARETYPE_SAMBA ? "checked" : "");
			websWrite(wp, "			</td>\n");

			websWrite(wp, "			<td style=\"width: 25px; text-align: center;\">\n");
			websWrite(wp, "				<input type=\"checkbox\" name=\"smbuser_ftp%s\" value=\"1\" %s>\n", number, cu->sharetype & SHARETYPE_FTP ? "checked" : "");
			websWrite(wp, "			</td>\n");
		}

		websWrite(wp, "			<td valign=\"top\" style=\"width: 50px; text-align: center;\">\n");
		websWrite(wp,
			  "				<input type=\"button\" class=\"button\" name=\"smbuser_del%s\" value=\"Remove\" style=\"width: 100%%;\" onclick=\"removeTableEntry('samba_users', this);\">\n",
			  number);
		websWrite(wp, "			</td>\n");

		websWrite(wp, "		</tr>\n");
		rows++;

		cunext = cu->next;
		free(cu);
	}
	for (cs = samba3shares; cs; cs = csnext) {
		for (csu = cs->users; csu; csu = csunext) {
			csunext = csu->next;
			free(csu);
		}
		csnext = cs->next;
		free(csnext);
	}

	websWrite(wp, "		</table>\n");

	// add button
	websWrite(wp, "<div id=\"samba_users_add\" style=\"text-align: center;\"><input type=\"button\" class=\"button\" name=\"user_add\" value=\"Add User\" onclick=\"addSambaUser();\" /></div>");

	// free memory

}
#endif

#ifdef HAVE_UPNP
// changed by steve
// writes javascript-string safe text

// <% tf_upnp(); %>
// returns all "forward_port#" nvram entries containing upnp port forwardings
void ej_tf_upnp(webs_t wp, int argc, char_t ** argv)
{
	int i;
	int len, pos, count;
	char *temp;

	if (nvram_match("upnp_enable", "1")) {
		for (i = 0; i < 50; i++) {
			websWrite(wp, (i > 0) ? ",'" : "'");

			// fix: some entries are missing the desc. - this breaks the
			// upnp.asp page, so we add ,*
			temp = nvram_nget("forward_port%d", i);
			count = 0;
			len = strlen(temp);

			for (pos = len; pos != 0; pos--) {
				if (temp[pos] == ',')
					count++;
			}
			tf_webWriteJS(wp, temp);
			if (count == 2)
				websWrite(wp, ",*");

			websWrite(wp, "'");
		}
	}
}

// end changed by steve
#endif

//extern void show_onlineupdates(webs_t wp, int argc, char_t ** argv);

void ej_show_upgrade_options(webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_BUFFALO
#ifndef HAVE_FREECWMP
	show_onlineupdates(wp, argc, argv);
#endif
#endif
} extern char *request_url;

void ej_getsetuppage(webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_BUFFALO
	if (endswith(request_url, ".asp") || endswith(request_url, ".htm")
	    || endswith(request_url, ".html")) {
		websWrite(wp, "%s", request_url);
	} else {
		websWrite(wp, "SetupAssistant.asp");
	}
#else
	websWrite(wp, "Info.htm");
#endif
} void ej_wan_if_status(webs_t wp, int argc, char_t ** argv)
{
#ifdef HAVE_DSL_CPE_CONTROL
	char *annex = nvram_safe_get("annex");
	websWrite(wp, "<fieldset>\n");
	websWrite(wp, "  <legend>DSL Status</legend>\n");
	websWrite(wp, "  <div class=\"setting\">\n");
	websWrite(wp, "    <div class=\"label\"><script type=\"text/javascript\">Capture(dsl.annex)</script></div>\n");
	websWrite(wp, "    <span>%c</span>\n", toupper(annex[0]));
	websWrite(wp, "  </div>\n");
	websWrite(wp, "  <div class=\"setting\">\n");
	websWrite(wp, "    <div class=\"label\"><script type=\"text/javascript\">Capture(dsl.iface_status)</script></div>\n");
	websWrite(wp, "    <span id=\"dsl_iface_status\">%s</span>\n", nvram_safe_get("dsl_iface_status"));
	websWrite(wp, "  </div>\n");
	websWrite(wp, "  <div class=\"setting\">\n");
	websWrite(wp, "    <div class=\"label\"><script type=\"text/javascript\">Capture(dsl.datarate)</script></div>\n");
	websWrite(wp, "    <span id=\"dsl_datarate_ds\">%11.2f</span> MBit / <span id=\"dsl_datarate_us\">%11.2f</span> MBit\n", atof(nvram_safe_get("dsl_datarate_ds")), atof(nvram_safe_get("dsl_datarate_us")));
	websWrite(wp, "  </div>\n");
	websWrite(wp, "  <div class=\"setting\">\n");
	websWrite(wp, "    <div class=\"label\"><script type=\"text/javascript\">Capture(dsl.snr)</script></div>\n");
	websWrite(wp, "    <span id=\"dsl_snr_up\">%d</span> dB / <span id=\"dsl_snr_down\">%d</span> dB\n", atoi(nvram_safe_get("dsl_snr_up")), atoi(nvram_safe_get("dsl_snr_down")));
	websWrite(wp, "  </div>\n");
	websWrite(wp, "</fieldset>\n");
	websWrite(wp, "<br />\n");
#endif
}

#ifdef HAVE_SPOTPASS
void ej_spotpass_servers(webs_t wp, int argc, char_t ** argv)
{
	char url[128], proto[8], ports[64];
	char dummy1[1], dummy2[8];
	int port1, port2;
	char *ptr;
	char *serverlist = (char *)
	    safe_malloc(strlen(nvram_default_get("spotpass_servers", "")) + 1);

	strcpy(serverlist, nvram_get("spotpass_servers"));
	ptr = strtok(serverlist, "|");
	while (ptr != NULL) {
		if (sscanf(ptr, "%s %s %s %s %d %d", &dummy1, &url, &proto, &dummy2, &port1, &port2) == 6) {
			websWrite(wp, "%s %s %d,%d", url, proto, port1, port2);
		} else if (sscanf(ptr, "%s %s %s %d %d", &dummy1, &url, &proto, &port1, &port2) == 5) {
			websWrite(wp, "%s %s %d,%d", url, proto, port1, port2);
		} else if (sscanf(ptr, "%s %s %s", &url, &proto, &ports) == 3) {
			websWrite(wp, "%s %s %s", url, proto, ports);
		}
		ptr = strtok(NULL, "|");
		if (ptr != NULL) {
			websWrite(wp, "\n");
		}
	}
}
#endif
#ifdef HAVE_STATUS_GPIO
void ej_show_status_gpio_output(webs_t wp, int argc, char_t ** argv)
{
	char *var, *next, *rgpio, *gpio_name;
	char nvgpio[32], gpio_new_name[32];

	char *gpios = nvram_safe_get("gpio_outputs");
	var = (char *)malloc(strlen(gpios) + 1);
	if (var != NULL) {
		if (gpios != NULL) {
			foreach(var, gpios, next) {
				sprintf(nvgpio, "gpio%s", var);
				sprintf(gpio_new_name, "gpio%s_name", var);
				rgpio = nvram_nget("gpio%s", var);
				if (strlen(rgpio) == 0)
					nvram_set(nvgpio, "0");

				rgpio = nvram_nget("gpio%s", var);
				gpio_name = nvram_nget("gpio%s_name", var);
				// enable
				websWrite(wp, "<div class=\"label\">%s (%s)</div>", nvgpio, gpio_name);
				websWrite(wp, "<input type=text maxlength=\"17\" size=\"17\" id=\"%s\" name=\"%s\" value=\"%s\">", gpio_new_name, gpio_new_name, gpio_name);
				websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" name=\"%s\" value=\"1\" %s />\n", nvgpio, nvram_match(nvgpio, "1") ? "checked=\"checked\"" : "");
				websWrite(wp, "<script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;");
				//disable 
				websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" name=\"%s\" value=\"0\" %s />\n", nvgpio, nvram_match(nvgpio, "0") ? "checked=\"checked\"" : "");
				websWrite(wp, "<script type=\"text/javascript\">Capture(share.disable)</script><br>");
			}
		}
		free(var);
	}
}

void ej_show_status_gpio_input(webs_t wp, int argc, char_t ** argv)
{
	char *var, *next, *rgpio, *gpio_name;
	char nvgpio[32], gpio_new_name[32];

	char *gpios = nvram_safe_get("gpio_inputs");
	var = (char *)malloc(strlen(gpios) + 1);
	if (var != NULL) {
		if (gpios != NULL) {
			foreach(var, gpios, next) {
				sprintf(nvgpio, "gpio%s", var);
				gpio_name = nvram_nget("gpio%s_name", var);
				sprintf(gpio_new_name, "gpio%s_name", var);

				// enable
				websWrite(wp, "<div class=\"label\">%s</div>", nvgpio);
				websWrite(wp, "<input maxlength=\"17\" size=\"17\" id=\"%s\" name=\"%s\" value=\"%s\">", gpio_new_name, gpio_new_name, gpio_name);

				websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" name=\"%s\" value=\"1\" disabled=\"true\" %s />\n", nvgpio, !get_gpio(atoi(var)) ? "checked=\"checked\"" : "");
				websWrite(wp, "<script type=\"text/javascript\">Capture(share.enable)</script>&nbsp;");
				//Disable
				websWrite(wp, "<input class=\"spaceradio\" type=\"radio\" name=\"%s\" value=\"0\" disabled=\"true\" %s />\n", nvgpio, get_gpio(atoi(var)) ? "checked=\"checked\"" : "");
				websWrite(wp, "<script type=\"text/javascript\">Capture(share.disable)</script><br>");
			}
		}
		free(var);
	}
}

#endif
