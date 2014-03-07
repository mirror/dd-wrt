#define VALIDSOURCE 1

#ifdef WEBS
#include <webs.h>
#include <uemf.h>
#include <ej.h>
#else				/* !WEBS */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <httpd.h>
#include <errno.h>
#endif				/* WEBS */

#include <proto/ethernet.h>
#include <fcntl.h>
#include <signal.h>
#include <time.h>
#include <sys/klog.h>
#include <sys/wait.h>
#include <cyutils.h>
#include <support.h>
#include <cy_conf.h>
// #ifdef EZC_SUPPORT
#include <ezc.h>
// #endif
#include <broadcom.h>
#include <wlutils.h>
#include <netdb.h>
#include <utils.h>
#include <stdarg.h>
#include <sha1.h>
#ifdef HAVE_SAMBA_SERVER
#include <jansson.h>
#endif

extern char *(*websGetVar) (webs_t wp, char *var, char *d);

void wan_proto(webs_t wp)
{
	char *enable;

	enable = websGetVar(wp, "wan_proto", NULL);
	nvram_set("wan_proto", enable);
}

#ifdef FILTER_DEBUG
extern FILE *debout;

#define D(a) fprintf(debout,"%s\n",a); fflush(debout);
#else
#define D(a)
#endif

void dhcpfwd(webs_t wp)
{
	char *enable;

	enable = websGetVar(wp, "dhcpfwd_enable", NULL);
	nvram_set("dhcpfwd_enable", enable);

}

#ifdef HAVE_CCONTROL

void execute(webs_t wp);

{
	char command[256];
	char *var = websGetVar(wp, "command", "");

	sysprintf("%s > /tmp/.result");
}

#endif
void clone_mac(webs_t wp)
{
	nvram_set("clone_wan_mac", "1");
}

/*
 * Delete lease 
 */
void delete_leases(webs_t wp)
{
	char *iface;
	char *ip;
	char *mac;

	if (nvram_match("lan_proto", "static"))
		return;

	if (nvram_match("fon_enable", "1")
	    || (nvram_match("chilli_nowifibridge", "1")
		&& nvram_match("chilli_enable", "1"))) {
		iface = nvram_safe_get("wl0_ifname");
	} else {
		if (nvram_match("chilli_enable", "1"))
			iface = nvram_safe_get("wl0_ifname");
		else
			iface = nvram_safe_get("lan_ifname");
	}
	//todo. detect correct interface

	ip = websGetVar(wp, "ip_del", NULL);
	mac = websGetVar(wp, "mac_del", NULL);

	sysprintf("dhcp_release %s %s %s", iface, ip, mac);
}

#if defined(HAVE_PPTPD) || defined(HAVE_PPPOESERVER)
void delete_pptp(webs_t wp)
{
	char *iface;
	iface = websGetVar(wp, "if_del", NULL);
	if (iface)
		sysprintf("kill %s", iface);
}
#endif
void save_wifi(webs_t wp)
{
	// fprintf (stderr, "save wifi\n");
	char *var = websGetVar(wp, "wifi_display", NULL);

	if (var) {
		nvram_set("wifi_display", var);
	}
}

void dhcp_renew(webs_t wp)
{
	killall("igmprt", SIGTERM);
	killall("udhcpc", SIGUSR1);
}

void dhcp_release(webs_t wp)
{

	killall("igmprt", SIGTERM);
	nvram_set("wan_ipaddr", "0.0.0.0");
	nvram_set("wan_netmask", "0.0.0.0");
	nvram_set("wan_gateway", "0.0.0.0");
	nvram_set("wan_get_dns", "");
	nvram_set("wan_lease", "0");

	unlink("/tmp/get_lease_time");
	unlink("/tmp/lease_time");

}

void stop_ppp(webs_t wp)
{
	unlink("/tmp/ppp/log");
	unlink("/tmp/ppp/link");
}

void validate_filter_tod(webs_t wp)
{
	char buf[256] = "";
	char tod_buf[20];
	struct variable filter_tod_variables[] = {
	      {argv:ARGV("20")},
	      {argv:ARGV("0", "1", "2")},

	}, *which;

	char *day_all, *week0, *week1, *week2, *week3, *week4, *week5, *week6;
	char *time_all, *start_hour, *start_min, *end_hour, *end_min;
	int _start_hour, _start_min, _end_hour, _end_min;
	char time[20];
	int week[7];
	int i, flag = -1, dash = 0;
	char filter_tod[] = "filter_todXXX";
	char filter_tod_buf[] = "filter_tod_bufXXX";

	which = &filter_tod_variables[0];

	day_all = websGetVar(wp, "day_all", "0");
	week0 = websGetVar(wp, "week0", "0");
	week1 = websGetVar(wp, "week1", "0");
	week2 = websGetVar(wp, "week2", "0");
	week3 = websGetVar(wp, "week3", "0");
	week4 = websGetVar(wp, "week4", "0");
	week5 = websGetVar(wp, "week5", "0");
	week6 = websGetVar(wp, "week6", "0");
	time_all = websGetVar(wp, "time_all", "0");
	start_hour = websGetVar(wp, "start_hour", "0");
	start_min = websGetVar(wp, "start_min", "0");
	// start_time = websGetVar (wp, "start_time", "0");
	end_hour = websGetVar(wp, "end_hour", "0");
	end_min = websGetVar(wp, "end_min", "0");
	// end_time = websGetVar (wp, "end_time", "0");

	// if(atoi(time_all) == 0)
	// if(!start_hour || !start_min || !start_time || !end_hour || !end_min
	// || !end_time)
	// return 1;

	if (atoi(day_all) == 1) {
		strcpy(time, "0-6");
		strcpy(tod_buf, "7");
	} else {
		week[0] = atoi(week0);
		week[1] = atoi(week1);
		week[2] = atoi(week2);
		week[3] = atoi(week3);
		week[4] = atoi(week4);
		week[5] = atoi(week5);
		week[6] = atoi(week6);
		strcpy(time, "");

		for (i = 0; i < 7; i++) {
			if (week[i] == 1) {
				if (i == 6) {
					if (dash == 0 && flag == 1)
						sprintf(time + strlen(time), "%c", '-');
					sprintf(time + strlen(time), "%d", i);
				} else if (flag == 1 && dash == 0) {
					sprintf(time + strlen(time), "%c", '-');
					dash = 1;
				} else if (dash == 0) {
					sprintf(time + strlen(time), "%d", i);
					flag = 1;
					dash = 0;
				}
			} else {
				if (!strcmp(time, ""))
					continue;
				if (dash == 1)
					sprintf(time + strlen(time), "%d", i - 1);
				if (flag != 0)
					sprintf(time + strlen(time), "%c", ',');
				flag = 0;
				dash = 0;
			}
		}
		if (time[strlen(time) - 1] == ',')
			time[strlen(time) - 1] = '\0';

		snprintf(tod_buf, sizeof(tod_buf), "%s %s %s %s %s %s %s", week0, week1, week2, week3, week4, week5, week6);
	}
	if (atoi(time_all) == 1) {
		_start_hour = 0;
		_start_min = 0;
		_end_hour = 23;
		_end_min = 59;
	} else {
		_start_hour = atoi(start_hour);
		_start_min = atoi(start_min);
		_end_hour = atoi(end_hour);
		_end_min = atoi(end_min);
	}

	sprintf(buf, "%d:%d %d:%d %s", _start_hour, _start_min, _end_hour, _end_min, time);
	snprintf(filter_tod, sizeof(filter_tod), "filter_tod%s", nvram_safe_get("filter_id"));
	snprintf(filter_tod_buf, sizeof(filter_tod_buf), "filter_tod_buf%s", nvram_safe_get("filter_id"));

	nvram_set(filter_tod, buf);
	nvram_set(filter_tod_buf, tod_buf);
	D("everything okay");

}

void applytake(char *value)
{
	if (value && !strcmp(value, "ApplyTake")) {
		nvram_commit();
		service_restart();
	}
}

void save_policy(webs_t wp)
{
	char *f_id, *f_name, *f_status, *f_status2;
	char buf[256] = "";
	char *value = websGetVar(wp, "action", "");
	struct variable filter_variables[] = {
	      {argv:ARGV("1", "10")},
	      {argv:ARGV("0", "1", "2")},
	      {argv:ARGV("deny", "allow")},

	}, *which;
	char filter_buf[] = "filter_ruleXXX";

	D("save policy");
	which = &filter_variables[0];
	f_id = websGetVar(wp, "f_id", NULL);
	f_name = websGetVar(wp, "f_name", NULL);
	f_status = websGetVar(wp, "f_status", NULL);	// 0=>Disable /
	// 1,2=>Enable
	f_status2 = websGetVar(wp, "f_status2", NULL);	// deny=>Deny /
	// allow=>Allow
	if (!f_id || !f_name || !f_status || !f_status2) {
		D("invalid");
		return;
	}
	if (!valid_range(wp, f_id, &which[0])) {
		D("invalid");
		return;
	}
	if (!valid_choice(wp, f_status, &which[1])) {
		D("invalid");
		return;
	}
	if (!valid_choice(wp, f_status2, &which[2])) {
		D("invalid");
		return;
	}

	validate_filter_tod(wp);

	snprintf(filter_buf, sizeof(filter_buf), "filter_rule%s", nvram_safe_get("filter_id"));

	// Add $DENY to decide that users select Allow or Deny, if status is
	// Disable // 2003/10/21
	snprintf(buf, sizeof(buf), "$STAT:%s$NAME:%s$DENY:%d$$", f_status, f_name, !strcmp(f_status2, "deny") ? 1 : 0);

	nvram_set(filter_buf, buf);
	applytake(value);

	D("okay");
}

void validate_filter_policy(webs_t wp, char *value, struct variable *v)
{
	char *f_id = websGetVar(wp, "f_id", NULL);

	if (f_id)
		nvram_set("filter_id", f_id);
	else
		nvram_set("filter_id", "1");

	save_policy(wp);
}

char *num_to_protocol(int num)
{
	switch (num) {
	case 1:
		return "icmp";
	case 6:
		return "tcp";
	case 17:
		return "udp";
	case 23:
		return "both";
	case 99:
		return "l7";
	case 100:
		return "p2p";
#ifdef HAVE_OPENDPI
	case 101:
		return "dpi";
#endif
	default:
		return "unknown";
	}
}

/*
 * Format: 21:21:tcp:FTP(&nbsp;)500:1000:both:TEST1 
 */

void validate_services_port(webs_t wp)
{
	char *buf = (char *)safe_malloc(8192);
	char *services = (char *)safe_malloc(8192);
	memset(buf, 0, 8192);
	memset(services, 0, 8192);
	char *cur = buf, *svcs = NULL;

	char *services_array = websGetVar(wp, "services_array0", NULL);

	// char *services_length = websGetVar (wp, "services_length0", NULL);
	char word[1026], *next;
	char delim[] = "(&nbsp;)";
	char var[32] = "";
	int index = 0;
	do {
		snprintf(var, 31, "services_array%d", index++);
		svcs = websGetVar(wp, var, NULL);
		if (svcs)
			strcat(services, svcs);

	}
	while (svcs);

	services_array = services;

	split(word, services_array, next, delim) {
		int from, to, proto;
		char name[80];

		if (sscanf(word, "%d:%d:%d:%s", &from, &to, &proto, name) != 4)
			continue;

		cur +=
		    snprintf(cur, buf + 8192 - cur,
			     "%s$NAME:%03d:%s$PROT:%03d:%s$PORT:%03d:%d:%d",
			     cur == buf ? "" : "<&nbsp;>", strlen(name), name, strlen(num_to_protocol(proto)), num_to_protocol(proto), (int)(get_int_len(from) + get_int_len(to) + strlen(":")), from, to);
	}

	// segment filter_services into <= 1024 byte lengths
	cur = buf;
	// fprintf (stderr, "cur=%s\n", cur);

	memcpy(word, cur, 1024);
	word[1025] = 0;
	nvram_set("filter_services", word);
	cur += 1024;

	if (strlen(cur) > 0) {
		nvram_set("filter_services_1", cur);
	}
	free(services);
	free(buf);
	// nvram_set ("filter_services", cur);
	D("okay");
}

void save_services_port(webs_t wp)
{
	validate_services_port(wp);
	char *value = websGetVar(wp, "action", "");
	applytake(value);
}

void delete_policy(webs_t wp, int which)
{
	D("delete policy");

	nvram_nset("", "filter_rule%d", which);
	nvram_nset("", "filter_tod%d", which);
	nvram_nset("", "filter_tod_buf%d", which);
	nvram_nset("", "filter_web_host%d", which);
	nvram_nset("", "filter_web_url%d", which);
	nvram_nset("", "filter_ip_grp%d", which);
	nvram_nset("", "filter_mac_grp%d", which);
	nvram_nset("", "filter_port_grp%d", which);
	nvram_nset("", "filter_dport_grp%d", which);

	D("okay");
}

void single_delete_policy(webs_t wp)
{
	char *id = nvram_safe_get("filter_id");

	D("single delete policy");
	delete_policy(wp, atoi(id));
	D("okay");
	return;
}

void summary_delete_policy(webs_t wp)
{
	int i;

	D("summary delete policy");
	for (i = 1; i <= 10; i++) {
		char filter_sum[] = "sumXXX";
		char *sum;

		snprintf(filter_sum, sizeof(filter_sum), "sum%d", i);
		sum = websGetVar(wp, filter_sum, NULL);
		if (sum)
			delete_policy(wp, i);
	}
	D("okay");
}

void addDeletion(char *word)
{
	if (!strlen(word) > 0)
		return;

	char *oldarg = nvram_get("action_service_arg1");

	if (oldarg && strlen(oldarg) > 0) {
		char *newarg = safe_malloc(strlen(oldarg) + strlen(word) + 2);

		sprintf(newarg, "%s %s", oldarg, word);
		nvram_set("action_service_arg1", newarg);
		free(newarg);
	} else
		nvram_set("action_service_arg1", word);
}

void delete_old_routes(void)
{
	char word[256], *next;
	char ipaddr[20], netmask[20], gateway[20], met[20], ifn[20];

	sleep(1);
	foreach(word, nvram_safe_get("action_service_arg1"), next) {
		strcpy(ipaddr, strtok(word, ":"));
		strcpy(netmask, strtok(NULL, ":"));
		strcpy(gateway, strtok(NULL, ":"));
		strcpy(met, strtok(NULL, ":"));
		strcpy(ifn, strtok(NULL, ":"));

		route_del(ifn, atoi(met) + 1, ipaddr, gateway, netmask);
	}
}

void delete_static_route(webs_t wp)
{
	addAction("routing");
	nvram_set("nowebaction", "1");
	char *buf = safe_malloc(2500);
	char *buf_name = safe_malloc(2500);

	memset(buf, 0, 2500);
	memset(buf_name, 0, 2500);
	char *cur = buf;
	char *cur_name = buf_name;
	static char word[256], *next;
	static char word_name[256], *next_name;
	char *page = websGetVar(wp, "route_page", NULL);
	char *value = websGetVar(wp, "action", "");
	int i = 0;
	char *performance = nvram_safe_get("static_route");
	char *performance2 = nvram_safe_get("static_route_name");

	foreach(word, performance, next) {
		if (i == atoi(page)) {
			addDeletion(word);
			i++;
			continue;
		}

		cur += snprintf(cur, buf + 2500 - cur, "%s%s", cur == buf ? "" : " ", word);

		i++;
	}

	i = 0;
	foreach(word_name, performance2, next_name) {
		if (i == atoi(page)) {
			i++;
			continue;
		}
		cur_name += snprintf(cur_name, buf_name + 2500 - cur_name, "%s%s", cur_name == buf_name ? "" : " ", word_name);

		i++;
	}

	nvram_set("static_route", buf);
	nvram_set("static_route_name", buf_name);
	free(buf_name);
	free(buf);
	applytake(value);
	return;
}

extern void gen_key(char *genstr, int weptype);

extern unsigned char key128[4][13];
extern unsigned char key64[4][5];


static void generate_wep_key_single(char *prefix, char *passphrase, char *bit, char *tx)
{

	int i;
	char buf[256];
	char var[80];

	if (!prefix || !bit || !passphrase || !tx)
		return;

	gen_key(passphrase, atoi(bit));

	nvram_set("generate_key", "1");

	if (atoi(bit) == 64) {
		char key1[27] = "";
		char key2[27] = "";
		char key3[27] = "";
		char key4[27] = "";

		for (i = 0; i < 5; i++)
			sprintf(key1 + (i << 1), "%02X", key64[0][i]);
		for (i = 0; i < 5; i++)
			sprintf(key2 + (i << 1), "%02X", key64[1][i]);
		for (i = 0; i < 5; i++)
			sprintf(key3 + (i << 1), "%02X", key64[2][i]);
		for (i = 0; i < 5; i++)
			sprintf(key4 + (i << 1), "%02X", key64[3][i]);

		snprintf(buf, sizeof(buf), "%s:%s:%s:%s:%s:%s", passphrase, key1, key2, key3, key4, tx);
		// nvram_set("wl_wep_gen_64",buf);
		cprintf("buf = %s\n", buf);
		sprintf(var, "%s_wep_gen", prefix);

		nvram_set(var, buf);
		nvram_nset(key1, "%s_key1", prefix);
		nvram_nset(key2, "%s_key2", prefix);
		nvram_nset(key3, "%s_key3", prefix);
		nvram_nset(key4, "%s_key4", prefix);
	} else if (atoi(bit) == 128) {
		char key1[27] = "";
		char key2[27] = "";
		char key3[27] = "";
		char key4[27] = "";

		for (i = 0; i < 13; i++)
			sprintf(key1 + (i << 1), "%02X", key128[0][i]);
		key1[26] = 0;

		for (i = 0; i < 13; i++)
			sprintf(key2 + (i << 1), "%02X", key128[1][i]);
		key2[26] = 0;

		for (i = 0; i < 13; i++)
			sprintf(key3 + (i << 1), "%02X", key128[2][i]);
		key3[26] = 0;

		for (i = 0; i < 13; i++)
			sprintf(key4 + (i << 1), "%02X", key128[3][i]);
		key4[26] = 0;
		// cprintf("passphrase[%s]\n", passphrase);
		// filter_name(passphrase, new_passphrase, sizeof(new_passphrase),
		// SET);
		// cprintf("new_passphrase[%s]\n", new_passphrase);
		cprintf("key1 = %s\n", key1);
		cprintf("key2 = %s\n", key2);
		cprintf("key3 = %s\n", key3);
		cprintf("key4 = %s\n", key4);

		snprintf(buf, sizeof(buf), "%s:%s:%s:%s:%s:%s", passphrase, key1, key2, key3, key4, tx);
		cprintf("buf = %s\n", buf);
		// nvram_set("wl_wep_gen_128",buf);
		sprintf(var, "%s_wep_gen", prefix);
		nvram_set(var, buf);
		nvram_nset(key1, "%s_key1", prefix);
		nvram_nset(key2, "%s_key2", prefix);
		nvram_nset(key3, "%s_key3", prefix);
		nvram_nset(key4, "%s_key4", prefix);
	}
	return;
}

void generate_wep_key(webs_t wp)
{
	char *prefix, *passphrase, *bit, *tx;

#ifdef HAVE_MADWIFI
	prefix = websGetVar(wp, "security_varname", "ath0");
#else
	prefix = websGetVar(wp, "security_varname", "wl");
#endif
	char var[80];

	sprintf(var, "%s_wep_bit", prefix);
	bit = websGetVar(wp, var, NULL);
	if (bit != NULL)
		nvram_set("wl_wep_bit", bit);
	sprintf(var, "%s_passphrase", prefix);
	passphrase = websGetVar(wp, var, NULL);
	sprintf(var, "%s_key", prefix);
	tx = websGetVar(wp, var, NULL);
	cprintf("gen wep key: bits = %s\n", bit);

	generate_wep_key_single(prefix, passphrase, bit, tx);
}


void copytonv(webs_t wp, const char *fmt, ...)
{
	char varbuf[64];
	va_list args;

	va_start(args, (char *)fmt);
	vsnprintf(varbuf, sizeof(varbuf), fmt, args);
	va_end(args);

	char *wl = websGetVar(wp, varbuf, NULL);

	if (wl)
		nvram_set(varbuf, wl);
}

void copytonv2(webs_t wp, char *prefix_get, char *prefix_set, char *name)
{
	char tmpname[64];

	sprintf(tmpname, "%s_%s", prefix_get, name);

	char *wl = websGetVar(wp, tmpname, NULL);

	sprintf(tmpname, "%s_%s", prefix_set, name);

	if (wl)
		nvram_set(tmpname, wl);
}

void copytonv2_wme(webs_t wp, char *prefix_get, char *prefix_set, char *name, int maxindex)
{
	char tmpvalue[128] = "";
	char tmpname[64];
	char *next;
	char *wl;
	int i;

	for (i = 0; i <= maxindex; i++) {
		sprintf(tmpname, "%s_%s%d", prefix_get, name, i);
		wl = websGetVar(wp, tmpname, NULL);
		if (wl) {
			strcat(tmpvalue, wl);
			strcat(tmpvalue, " ");
		}
	}

	sprintf(tmpname, "%s_%s", prefix_set, name);
	strtrim_right(tmpvalue, ' ');
	nvram_set(tmpname, tmpvalue);
}

extern int get_merge_ipaddr(webs_t wp, char *name, char *ipaddr);

static void save_secprefix(webs_t wp, char *prefix)
{
	char n[80];
	char radius[80];
	char p2[80];

	strcpy(p2, prefix);
	if (contains(prefix, '.'))
		rep(p2, '.', 'X');	// replace invalid characters for sub ifs

#ifdef HAVE_WPA_SUPPLICANT

/*_8021xtype
_8021xuser
_8021xpasswd
_8021xca
_8021xpem
_8021xprv
*/
	copytonv(wp, "%s_8021xtype", prefix);
	copytonv(wp, "%s_tls8021xuser", prefix);
	copytonv(wp, "%s_tls8021xanon", prefix);
	copytonv(wp, "%s_tls8021xpasswd", prefix);
	copytonv(wp, "%s_tls8021xphase2", prefix);
	copytonv(wp, "%s_tls8021xca", prefix);
	copytonv(wp, "%s_tls8021xpem", prefix);
	copytonv(wp, "%s_tls8021xprv", prefix);
	copytonv(wp, "%s_tls8021xaddopt", prefix);
	copytonv(wp, "%s_peap8021xuser", prefix);
	copytonv(wp, "%s_peap8021xanon", prefix);
	copytonv(wp, "%s_peap8021xpasswd", prefix);
	copytonv(wp, "%s_tls8021xkeyxchng", prefix);
	copytonv(wp, "%s_peap8021xphase2", prefix);
	copytonv(wp, "%s_peap8021xca", prefix);
	copytonv(wp, "%s_peap8021xaddopt", prefix);
	copytonv(wp, "%s_ttls8021xuser", prefix);
	copytonv(wp, "%s_ttls8021xanon", prefix);
	copytonv(wp, "%s_ttls8021xpasswd", prefix);
	copytonv(wp, "%s_ttls8021xphase2", prefix);
	copytonv(wp, "%s_ttls8021xca", prefix);
	copytonv(wp, "%s_ttls8021xaddopt", prefix);
	copytonv(wp, "%s_leap8021xuser", prefix);
	copytonv(wp, "%s_leap8021xanon", prefix);
	copytonv(wp, "%s_leap8021xpasswd", prefix);
	copytonv(wp, "%s_leap8021xphase2", prefix);
	copytonv(wp, "%s_leap8021xaddopt", prefix);

#endif

	copytonv(wp, "%s_crypto", prefix);
	copytonv(wp, "%s_wpa_psk", prefix);
	copytonv(wp, "%s_wpa_gtk_rekey", prefix);
	sprintf(n, "%s_radius_ipaddr", prefix);
	if (get_merge_ipaddr(wp, n, radius))
		nvram_set(n, radius);
	copytonv(wp, "%s_radius_port", prefix);
	copytonv(wp, "%s_radius_key", prefix);

	sprintf(n, "%s_radius2_ipaddr", prefix);
	if (get_merge_ipaddr(wp, n, radius))
		nvram_set(n, radius);
	copytonv(wp, "%s_radius2_port", prefix);
	copytonv(wp, "%s_radius2_key", prefix);
#ifdef HAVE_MADWIFI
	copytonv(wp, "%s_acct", prefix);
	sprintf(n, "%s_acct_ipaddr", prefix);
	if (get_merge_ipaddr(wp, n, radius))
		nvram_set(n, radius);
	copytonv(wp, "%s_acct_port", prefix);
	copytonv(wp, "%s_acct_key", prefix);
#endif
	copytonv(wp, "%s_radmactype", prefix);

	sprintf(n, "%s_authmode", prefix);
	char *authmode = websGetVar(wp, n, "");
	if (strlen(authmode) == 0) {
		nvram_set(n, "open");
	} else {
		copytonv(wp, n);
	}
	sprintf(n, "%s_key1", prefix);
	char *key1 = websGetVar(wp, n, "");

	copytonv(wp, n);
	sprintf(n, "%s_key2", prefix);
	char *key2 = websGetVar(wp, n, "");

	copytonv(wp, n);
	sprintf(n, "%s_key3", prefix);
	char *key3 = websGetVar(wp, n, "");

	copytonv(wp, n);
	sprintf(n, "%s_key4", prefix);
	char *key4 = websGetVar(wp, n, "");

	copytonv(wp, n);
	sprintf(n, "%s_passphrase", prefix);
	char *pass = websGetVar(wp, n, "");

	copytonv(wp, n);
	sprintf(n, "%s_key", prefix);
	char *tx = websGetVar(wp, n, "");
	if (strlen(tx) == 0) {
		nvram_set(n, "1");
	} else {
		copytonv(wp, n);
	}
	sprintf(n, "%s_wep_bit", prefix);
	copytonv(wp, n);
	char buf[128];

	snprintf(buf, sizeof(buf), "%s:%s:%s:%s:%s:%s", pass, key1, key2, key3, key4, tx);
	sprintf(n, "%s_wep_buf", prefix);
	nvram_set(n, buf);

	sprintf(n, "%s_security_mode", p2);
	char n2[80];

	sprintf(n2, "%s_akm", prefix);
	char *v = websGetVar(wp, n, NULL);

	if (v) {
		char auth[32];
		char wep[32];

		sprintf(auth, "%s_auth_mode", prefix);
		sprintf(wep, "%s_wep", prefix);
		if (!strcmp(v, "wep")) {
			nvram_set(auth, "none");
			nvram_set(wep, "enabled");
		} else if (!strcmp(v, "radius")) {
			nvram_set(auth, "radius");
			nvram_set(wep, "enabled");
		} else {
			nvram_set(auth, "none");
			nvram_set(wep, "disabled");
		}
		nvram_set(n2, v);
	}

	copytonv(wp, n);

}

static int security_save_prefix(webs_t wp, char *prefix)
{

	save_secprefix(wp, prefix);
	char *next;
	char var[80];
	char *vifs = nvram_nget("%s_vifs", prefix);

	if (vifs == NULL)
		return 0;
	foreach(var, vifs, next) {
		save_secprefix(wp, var);
	}
	// nvram_commit ();
	return 0;
}

void security_save(webs_t wp)
{
	char *value = websGetVar(wp, "action", "");

#ifdef HAVE_MADWIFI
	int dc = getdevicecount();
	int i;

	for (i = 0; i < dc; i++) {
		char b[16];

		sprintf(b, "ath%d", i);
		security_save_prefix(wp, b);
	}
#else
	int dc = get_wl_instances();
	int i;

	for (i = 0; i < dc; i++) {
		char b[16];

		sprintf(b, "wl%d", i);
		security_save_prefix(wp, b);
	}
#endif
	applytake(value);
}

extern struct wl_client_mac *wl_client_macs;

void add_active_mac(webs_t wp)
{
	int i, count = 0;
	int msize = 4608;	// 18 chars * 256 entries
	char *buf = malloc(msize);
	char *cur = buf;
	memset(buf, 0, msize);
	char *ifname = websGetVar(wp, "ifname", NULL);

	nvram_set("wl_active_add_mac", "1");

	for (i = 0; i < MAX_LEASES + 2; i++) {
		char active_mac[] = "onXXX";
		char *index = NULL;

		snprintf(active_mac, sizeof(active_mac), "%s%d", "on", i);
		index = websGetVar(wp, active_mac, NULL);
		if (!index)
			continue;

		count++;

		cur += snprintf(cur, buf + msize - cur, "%s%s", cur == buf ? "" : " ", wl_client_macs[atoi(index)].hwaddr);
	}
	for (i = 0; i < MAX_LEASES + 2; i++) {
		char active_mac[] = "offXXX";
		char *index;

		snprintf(active_mac, sizeof(active_mac), "%s%d", "off", i);
		index = websGetVar(wp, active_mac, NULL);
		if (!index)
			continue;

		count++;
		cur += snprintf(cur, buf + msize - cur, "%s%s", cur == buf ? "" : " ", wl_client_macs[atoi(index)].hwaddr);
	}
	char acmac[32];
	sprintf(acmac, "%s_active_mac", ifname);
	nvram_set(acmac, buf);
	if (!strcmp(ifname, "wl0"))
		nvram_set("wl_active_mac", buf);
	free(buf);
}

void removeLineBreak(char *startup)
{
	int i = 0;
	int c = 0;

	for (i = 0; i < strlen(startup); i++) {
		if (startup[i] == '\r')
			continue;
		startup[c++] = startup[i];
	}
	startup[c++] = 0;

}

void ping_startup(webs_t wp)
{
	char *startup = websGetVar(wp, "ping_ip", NULL);
	if (startup) {
		// filter Windows <cr>ud
		removeLineBreak(startup);

		nvram_set("rc_startup", startup);
		nvram_commit();
		nvram2file("rc_startup", "/tmp/.rc_startup");
		chmod("/tmp/.rc_startup", 0700);
	}
	return;

}

void ping_shutdown(webs_t wp)
{
	char *shutdown = websGetVar(wp, "ping_ip", NULL);
	if (shutdown) {
		// filter Windows <cr>ud
		removeLineBreak(shutdown);

		nvram_set("rc_shutdown", shutdown);
		nvram_commit();
		nvram2file("rc_shutdown", "/tmp/.rc_shutdown");
		chmod("/tmp/.rc_shutdown", 0700);
	}
	return;

}

void ping_firewall(webs_t wp)
{
	char *firewall = websGetVar(wp, "ping_ip", NULL);
	if (firewall) {
		// filter Windows <cr>ud
		removeLineBreak(firewall);
		nvram_set("rc_firewall", firewall);
		nvram_commit();
		nvram2file("rc_firewall", "/tmp/.rc_firewall");
		chmod("/tmp/.rc_firewall", 0700);
	}
	return;
}

void ping_custom(webs_t wp)
{
	char *custom = websGetVar(wp, "ping_ip", NULL);
	if (custom) {
		// filter Windows <cr>ud
		unlink("/tmp/custom.sh");
		removeLineBreak(custom);
		nvram_set("rc_custom", custom);
		nvram_commit();
		if (nvram_invmatch("rc_custom", "")) {
			nvram2file("rc_custom", "/tmp/custom.sh");
			chmod("/tmp/custom.sh", 0700);
		}
	}

	return;
}

void ping_wol(webs_t wp)
{
	char *wol_type = websGetVar(wp, "wol_type", NULL);

	unlink(PING_TMP);

	if (!wol_type || !strcmp(wol_type, ""))
		return;

	if (!strcmp(wol_type, "update")) {
		char *wol_hosts = websGetVar(wp, "wol_hosts", NULL);

		if (!wol_hosts || !strcmp(wol_hosts, ""))
			return;

		nvram_set("wol_hosts", wol_hosts);
		nvram_set("wol_cmd", "");
		return;
	}

	char *manual_wol_mac = websGetVar(wp, "manual_wol_mac", NULL);
	char *manual_wol_network = websGetVar(wp, "manual_wol_network", NULL);
	char *manual_wol_port = websGetVar(wp, "manual_wol_port", NULL);

	if (!strcmp(wol_type, "manual")) {
		nvram_set("manual_wol_mac", manual_wol_mac);
		nvram_set("manual_wol_network", manual_wol_network);
		nvram_set("manual_wol_port", manual_wol_port);
	}

	char wol_cmd[256] = { 0 };
	snprintf(wol_cmd, sizeof(wol_cmd), "/usr/sbin/wol -v -i %s -p %s %s", manual_wol_network, manual_wol_port, manual_wol_mac);
	nvram_set("wol_cmd", wol_cmd);

	// use Wol.asp as a debugging console
#ifdef HAVE_REGISTER
	if (!isregistered_real())
		return;
#endif
	sysprintf("%s > %s 2>&1 &", wol_cmd, PING_TMP);

}

void diag_ping_start(webs_t wp)
{
	char *ip = websGetVar(wp, "ping_ip", NULL);

	if (!ip || !strcmp(ip, ""))
		return;

	unlink(PING_TMP);
	nvram_set("ping_ip", ip);

	setenv("PATH", "/sbin:/bin:/usr/sbin:/usr/bin", 1);
#ifdef HAVE_REGISTER
	if (!isregistered_real())
		return;
#endif
	sysprintf("alias ping=\'ping -c 3\'; eval \"%s\" > %s 2>&1 &", ip, PING_TMP);

	return;
}

void diag_ping_stop(webs_t wp)
{
	killall("ping", SIGKILL);
}

void diag_ping_clear(webs_t wp)
{
	unlink(PING_TMP);
}

void save_wireless_advanced(webs_t wp)
{
	char set_prefix[8];
	char prefix[8];
	char *wlface = websGetVar(wp, "interface", NULL);

	if (!strcmp(wlface, "wl0"))
		sprintf(set_prefix, "%s", "wl");
	else
		sprintf(set_prefix, "%s", wlface);

	sprintf(prefix, wlface);

	copytonv2(wp, prefix, set_prefix, "auth");
	copytonv2(wp, prefix, set_prefix, "rateset");
	copytonv2(wp, prefix, set_prefix, "nmcsidx");
	copytonv2(wp, prefix, set_prefix, "rate");
	copytonv2(wp, prefix, set_prefix, "gmode_protection");
	copytonv2(wp, prefix, set_prefix, "frameburst");
	copytonv2(wp, prefix, set_prefix, "bcn");
	copytonv2(wp, prefix, set_prefix, "dtim");
	copytonv2(wp, prefix, set_prefix, "frag");
	copytonv2(wp, prefix, set_prefix, "rts");
	copytonv2(wp, prefix, set_prefix, "maxassoc");
	copytonv2(wp, prefix, set_prefix, "ap_isolate");
	copytonv2(wp, prefix, set_prefix, "plcphdr");
	copytonv2(wp, prefix, set_prefix, "shortslot");
	copytonv2(wp, prefix, set_prefix, "afterburner");
	copytonv2(wp, prefix, set_prefix, "btc_mode");
	copytonv2(wp, prefix, set_prefix, "wme");
	copytonv2(wp, prefix, set_prefix, "wme_no_ack");
	copytonv2_wme(wp, prefix, set_prefix, "wme_ap_bk", 5);
	copytonv2_wme(wp, prefix, set_prefix, "wme_ap_be", 5);
	copytonv2_wme(wp, prefix, set_prefix, "wme_ap_vi", 5);
	copytonv2_wme(wp, prefix, set_prefix, "wme_ap_vo", 5);
	copytonv2_wme(wp, prefix, set_prefix, "wme_sta_bk", 5);
	copytonv2_wme(wp, prefix, set_prefix, "wme_sta_be", 5);
	copytonv2_wme(wp, prefix, set_prefix, "wme_sta_vi", 5);
	copytonv2_wme(wp, prefix, set_prefix, "wme_sta_vo", 5);
	copytonv2_wme(wp, prefix, set_prefix, "wme_txp_bk", 4);
	copytonv2_wme(wp, prefix, set_prefix, "wme_txp_be", 4);
	copytonv2_wme(wp, prefix, set_prefix, "wme_txp_vi", 4);
	copytonv2_wme(wp, prefix, set_prefix, "wme_txp_vo", 4);

	return;

}

void save_wds(webs_t wp)
{
	char *wds_enable_val, wds_enable_var[32] = { 0 };
	int h = 0;
	char *interface = websGetVar(wp, "interface", NULL);

	for (h = 1; h <= MAX_WDS_DEVS; h++) {
		sprintf(wds_enable_var, "%s_wds%d_enable", interface, h);
		wds_enable_val = websGetVar(wp, wds_enable_var, NULL);
		nvram_set(wds_enable_var, wds_enable_val);
	}
	sprintf(wds_enable_var, "%s_br1_enable", interface);
	wds_enable_val = websGetVar(wp, wds_enable_var, NULL);
	nvram_set(wds_enable_var, wds_enable_val);

	sprintf(wds_enable_var, "%s_br1_nat", interface);
	wds_enable_val = websGetVar(wp, wds_enable_var, NULL);
	nvram_set(wds_enable_var, wds_enable_val);

	return;

}

int get_svc(char *svc, char *protocol, char *ports)
{
	char word[1024], *next;
	char delim[] = "<&nbsp;>";
	char *services;
	// services = nvram_safe_get("filter_services");
	services = get_filter_services();

	split(word, services, next, delim) {
		int len = 0;
		char *name, *prot, *port;
		int from = 0, to = 0;

		if ((name = strstr(word, "$NAME:")) == NULL || (prot = strstr(word, "$PROT:")) == NULL || (port = strstr(word, "$PORT:")) == NULL)
			continue;

		/*
		 * $NAME 
		 */
		if (sscanf(name, "$NAME:%3d:", &len) != 1)
			return -1;

		strncpy(name, name + sizeof("$NAME:nnn:") - 1, len);
		name[len] = '\0';

		if (strcasecmp(svc, name))
			continue;

		/*
		 * $PROT 
		 */
		if (sscanf(prot, "$PROT:%3d:", &len) != 1)
			return -1;

		strncpy(protocol, prot + sizeof("$PROT:nnn:") - 1, len);
		protocol[len] = '\0';

		/*
		 * $PORT 
		 */
		if (sscanf(port, "$PORT:%3d:", &len) != 1)
			return -1;

		strncpy(ports, port + sizeof("$PORT:nnn:") - 1, len);
		ports[len] = '\0';

		if (sscanf(ports, "%d:%d", &from, &to) != 2) {
			free(services);
			return -1;
		}

		if (strcasecmp(svc, name) == 0) {
			free(services);
			return 0;
		}
	}
	free(services);

	return -1;
}

void qos_add_svc(webs_t wp)
{
	char *var = websGetVar(wp, "wshaper_enable", NULL);

	if (var != NULL)
		nvram_set("wshaper_enable", var);

	char protocol[100] = { 0 }, ports[100] = {
	0};
	char *add_svc = websGetVar(wp, "add_svc", NULL);
	char *svqos_svcs = nvram_safe_get("svqos_svcs");
	char new_svcs[4096] = { 0 };
	int i = 0;

	memset(new_svcs, 0, sizeof(new_svcs));

	if (get_svc(add_svc, protocol, ports))
		return;

	if (strcmp(protocol, "l7") == 0) {
		int slen = strlen(add_svc);

		for (i = 0; i < slen; i++)
			add_svc[i] = tolower(add_svc[i]);
	}
#ifdef HAVE_OPENDPI
	if (strcmp(protocol, "dpi") == 0) {
		int slen = strlen(add_svc);

		for (i = 0; i < slen; i++)
			add_svc[i] = tolower(add_svc[i]);
	}
#endif

	/*
	 * if this service exists, return an error 
	 */
	if (strstr(svqos_svcs, add_svc))
		return;

	if (strlen(svqos_svcs) > 0)
		snprintf(new_svcs, 4095, "%s %s %s %s 30 |", svqos_svcs, add_svc, protocol, ports);
	else
		snprintf(new_svcs, 4095, "%s %s %s 30 |", add_svc, protocol, ports);

	if (strlen(new_svcs) >= sizeof(new_svcs))
		return;

	nvram_set("svqos_svcs", new_svcs);
	nvram_commit();
}

void qos_add_ip(webs_t wp)
{
	char *var = websGetVar(wp, "wshaper_enable", NULL);

	if (var != NULL)
		nvram_set("wshaper_enable", var);

	char *add_ip0 = websGetVar(wp, "svqos_ipaddr0", NULL);
	char *add_ip1 = websGetVar(wp, "svqos_ipaddr1", NULL);
	char *add_ip2 = websGetVar(wp, "svqos_ipaddr2", NULL);
	char *add_ip3 = websGetVar(wp, "svqos_ipaddr3", NULL);
	char *add_nm = websGetVar(wp, "svqos_netmask", NULL);
	char add_ip[19] = { 0 };
	char *svqos_ips = nvram_safe_get("svqos_ips");
	char new_ip[4096] = { 0 };

	memset(new_ip, 0, sizeof(new_ip));

	snprintf(add_ip, 19, "%s.%s.%s.%s/%s", add_ip0, add_ip1, add_ip2, add_ip3, add_nm);

	/*
	 * if this ip exists, return an error 
	 */
	if (strstr(svqos_ips, add_ip))
		return;
#ifdef HAVE_AQOS
	snprintf(new_ip, 4095, "%s %s 100 100 0 0 |", svqos_ips, add_ip);
#else
	snprintf(new_ip, 4095, "%s %s 30 |", svqos_ips, add_ip);
#endif
	if (strlen(new_ip) >= sizeof(new_ip))
		return;

	nvram_set("svqos_ips", new_ip);
	nvram_commit();

}

void qos_add_mac(webs_t wp)
{
	char *var = websGetVar(wp, "wshaper_enable", NULL);

	if (var != NULL)
		nvram_set("wshaper_enable", var);

	char *add_mac0 = websGetVar(wp, "svqos_hwaddr0", NULL);
	char *add_mac1 = websGetVar(wp, "svqos_hwaddr1", NULL);
	char *add_mac2 = websGetVar(wp, "svqos_hwaddr2", NULL);
	char *add_mac3 = websGetVar(wp, "svqos_hwaddr3", NULL);
	char *add_mac4 = websGetVar(wp, "svqos_hwaddr4", NULL);
	char *add_mac5 = websGetVar(wp, "svqos_hwaddr5", NULL);
	char add_mac[19] = { 0 };
	char *svqos_macs = nvram_safe_get("svqos_macs");
	char new_mac[4096] = { 0 };

	memset(new_mac, 0, sizeof(new_mac));

	snprintf(add_mac, 18, "%s:%s:%s:%s:%s:%s", add_mac0, add_mac1, add_mac2, add_mac3, add_mac4, add_mac5);

	/*
	 * if this mac exists, return an error 
	 */
	if (strstr(svqos_macs, add_mac))
		return;
#ifdef HAVE_AQOS
	snprintf(new_mac, 4095, "%s %s 100 100 user 0 0 |", svqos_macs, add_mac);
#else
	snprintf(new_mac, 4095, "%s %s 30 |", svqos_macs, add_mac);
#endif
	if (strlen(new_mac) >= sizeof(new_mac))
		return;

	nvram_set("svqos_macs", new_mac);
	nvram_commit();

}

void qos_save(webs_t wp)
{
	char *value = websGetVar(wp, "action", "");
	char svqos_var[4096] = { 0 };
	char svqos_pktstr[30] = { 0 };
	char field[32] = { 0 };
	char *name, *data, *level, *level2, *lanlevel, *prio, *delete, *pktopt;
	int no_svcs = atoi(websGetVar(wp, "svqos_nosvcs", NULL));
	int no_ips = atoi(websGetVar(wp, "svqos_noips", NULL));
	int no_macs = atoi(websGetVar(wp, "svqos_nomacs", NULL));
	int i = 0, j = 0;

	/*
	 * reused wshaper fields - see src/router/rc/wshaper.c 
	 */

	data = websGetVar(wp, "wshaper_enable", NULL);
	nvram_set("wshaper_enable", data);

	if (strcmp(data, "0") == 0) {
		addAction("qos");
		nvram_set("nowebaction", "1");
		applytake(value);
		return;
	}
//      nvram_set("enable_game", websGetVar(wp, "enable_game", NULL));
	nvram_set("svqos_defaults", websGetVar(wp, "svqos_defaults", NULL));
	nvram_set("default_uplevel", websGetVar(wp, "default_uplevel", NULL));
	nvram_set("default_downlevel", websGetVar(wp, "default_downlevel", NULL));
	nvram_set("default_lanlevel", websGetVar(wp, "default_lanlevel", NULL));
	nvram_set("wshaper_downlink", websGetVar(wp, "wshaper_downlink", NULL));
	nvram_set("wshaper_uplink", websGetVar(wp, "wshaper_uplink", NULL));
	nvram_set("wshaper_dev", websGetVar(wp, "wshaper_dev", NULL));
	nvram_set("qos_type", websGetVar(wp, "qos_type", NULL));

#if defined(HAVE_CODEL) || defined(HAVE_FQ_CODEL)
	nvram_set("svqos_aqd", websGetVar(wp, "qos_aqd", NULL));
#endif

	// nvram_commit ();

	/*
	 * tcp-packet flags
	 */
	memset(svqos_pktstr, 0, sizeof(svqos_pktstr));

	pktopt = websGetVar(wp, "svqos_pktack", NULL);
	if (pktopt)
		strcat(svqos_pktstr, "ACK | ");
	pktopt = websGetVar(wp, "svqos_pktsyn", NULL);
	if (pktopt)
		strcat(svqos_pktstr, "SYN | ");
	pktopt = websGetVar(wp, "svqos_pktfin", NULL);
	if (pktopt)
		strcat(svqos_pktstr, "FIN | ");
	pktopt = websGetVar(wp, "svqos_pktrst", NULL);
	if (pktopt)
		strcat(svqos_pktstr, "RST | ");

	nvram_set("svqos_pkts", svqos_pktstr);

	/*
	 * services priorities 
	 */
	memset(svqos_var, 0, sizeof(svqos_var));

	for (i = 0; i < no_svcs; i++) {
		char protocol[100], ports[100];

		memset(protocol, 0, 100);
		memset(ports, 0, 10);

		snprintf(field, 31, "svqos_svcdel%d", i);
		delete = websGetVar(wp, field, NULL);

		if (delete && strlen(delete) > 0)
			continue;

		snprintf(field, 31, "svqos_svcname%d", i);
		name = websGetVar(wp, field, NULL);

		snprintf(field, 31, "svqos_svcprio%d", i);
		level = websGetVar(wp, field, NULL);

		if (get_svc(name, protocol, ports))
			continue;

		if (strcmp(protocol, "l7") == 0) {
			int slen = strlen(name);

			for (j = 0; j < slen; j++)
				name[j] = tolower(name[j]);
		}
#ifdef HAVE_OPENDPI
		if (strcmp(protocol, "dpi") == 0) {
			int slen = strlen(name);

			for (j = 0; j < slen; j++)
				name[j] = tolower(name[j]);
		}
#endif
		if (strlen(svqos_var) > 0)
			sprintf(svqos_var, "%s %s %s %s %s |", svqos_var, name, protocol, ports, level);
		else
			sprintf(svqos_var, "%s %s %s %s |", name, protocol, ports, level);

	}

	if (strlen(svqos_var) <= sizeof(svqos_var))
		nvram_set("svqos_svcs", svqos_var);
	// nvram_commit ();
	memset(svqos_var, 0, sizeof(svqos_var));

	/*
	 * IP priorities 
	 */
	for (i = 0; i < no_ips; i++) {

		snprintf(field, 31, "svqos_ipdel%d", i);
		delete = websGetVar(wp, field, NULL);

		if (delete && strlen(delete) > 0)
			continue;

		snprintf(field, 31, "svqos_ip%d", i);
		data = websGetVar(wp, field, NULL);

#ifndef HAVE_AQOS
		snprintf(field, 31, "svqos_ipprio%d", i);
		level = websGetVar(wp, field, NULL);
		if (strlen(svqos_var) > 0)
			sprintf(svqos_var, "%s %s %s |", svqos_var, data, level);
		else
			sprintf(svqos_var, "%s %s |", data, level);
#else
		snprintf(field, 31, "svqos_ipprio%d", i);
		prio = websGetVar(wp, field, NULL);

		snprintf(field, 31, "svqos_ipup%d", i);
		level = websGetVar(wp, field, NULL);
		snprintf(field, 31, "svqos_ipdown%d", i);
		level2 = websGetVar(wp, field, NULL);
		snprintf(field, 31, "svqos_iplanlvl%d", i);
		lanlevel = websGetVar(wp, field, NULL);

		if (strlen(svqos_var) > 0)
			sprintf(svqos_var, "%s %s %s %s %s %s |", svqos_var, data, level, level2, lanlevel, prio);
		else
			sprintf(svqos_var, "%s %s %s %s %s |", data, level, level2, lanlevel, prio);

#endif

	}

	if (strlen(svqos_var) <= sizeof(svqos_var))
		nvram_set("svqos_ips", svqos_var);
	// nvram_commit ();
	memset(svqos_var, 0, sizeof(svqos_var));

	/*
	 * MAC priorities 
	 */
	for (i = 0; i < no_macs; i++) {
		snprintf(field, 31, "svqos_macdel%d", i);
		delete = websGetVar(wp, field, NULL);

		if (delete && strlen(delete) > 0)
			continue;

		snprintf(field, 31, "svqos_mac%d", i);
		data = websGetVar(wp, field, NULL);

#ifndef HAVE_AQOS
		snprintf(field, 31, "svqos_macprio%d", i);
		level = websGetVar(wp, field, NULL);

		if (strlen(svqos_var) > 0)
			sprintf(svqos_var, "%s %s %s |", svqos_var, data, level);
		else
			sprintf(svqos_var, "%s %s |", data, level);
#else
		snprintf(field, 31, "svqos_macprio%d", i);
		prio = websGetVar(wp, field, NULL);

		snprintf(field, 31, "svqos_macup%d", i);
		level = websGetVar(wp, field, NULL);
		snprintf(field, 31, "svqos_macdown%d", i);
		level2 = websGetVar(wp, field, NULL);
		snprintf(field, 31, "svqos_maclanlvl%d", i);
		lanlevel = websGetVar(wp, field, NULL);

		if (strlen(svqos_var) > 0)
			sprintf(svqos_var, "%s %s %s %s user %s %s |", svqos_var, data, level, level2, lanlevel, prio);
		else
			sprintf(svqos_var, "%s %s %s user %s %s |", data, level, level2, lanlevel, prio);

#endif

	}

	if (strlen(svqos_var) <= sizeof(svqos_var))
		nvram_set("svqos_macs", svqos_var);
	// nvram_commit ();

	/*
	 * adm6996 LAN port priorities 
	 */
	nvram_set("svqos_port1prio", websGetVar(wp, "svqos_port1prio", NULL));
	nvram_set("svqos_port2prio", websGetVar(wp, "svqos_port2prio", NULL));
	nvram_set("svqos_port3prio", websGetVar(wp, "svqos_port3prio", NULL));
	nvram_set("svqos_port4prio", websGetVar(wp, "svqos_port4prio", NULL));

	nvram_set("svqos_port1bw", websGetVar(wp, "svqos_port1bw", NULL));
	nvram_set("svqos_port2bw", websGetVar(wp, "svqos_port2bw", NULL));
	nvram_set("svqos_port3bw", websGetVar(wp, "svqos_port3bw", NULL));
	nvram_set("svqos_port4bw", websGetVar(wp, "svqos_port4bw", NULL));

	addAction("qos");
	nvram_set("nowebaction", "1");
	applytake(value);

}

static void macro_add(char *a)
{
	cprintf("adding %s\n", a);

	char *count;
	int c;
	char buf[20];

	count = nvram_safe_get(a);
	cprintf("count = %s\n", count);
	if (count != NULL && strlen(count) > 0) {
		c = atoi(count);
		if (c > -1) {
			c++;
			sprintf(buf, "%d", c);
			cprintf("set %s to %s\n", a, buf);
			nvram_set(a, buf);
		}
	}
	return;
}

static void macro_rem(char *a, char *nv)
{
	char *count;
	int c, i, cnt;
	char buf[20];
	char *buffer, *b;

	cnt = 0;
	count = nvram_safe_get(a);
	if (count != NULL && strlen(count) > 0) {
		c = atoi(count);
		if (c > 0) {
			c--;
			sprintf(buf, "%d", c);
			nvram_set(a, buf);
			buffer = nvram_safe_get(nv);
			if (buffer != NULL) {
				int slen = strlen(buffer);

				b = safe_malloc(slen + 1);

				for (i = 0; i < slen; i++) {
					if (buffer[i] == ' ')
						cnt++;
					if (cnt == c)
						break;
					b[i] = buffer[i];
				}
				b[i] = 0;
				nvram_set(nv, b);
				free(b);
			}

		}
	}
	return;
}

void forward_remove(webs_t wp)
{
	macro_rem("forward_entries", "forward_port");
}

void forward_add(webs_t wp)
{
	macro_add("forward_entries");
}

void lease_remove(webs_t wp)
{
	macro_rem("static_leasenum", "static_leases");
}

void lease_add(webs_t wp)
{
	macro_add("static_leasenum");
}

#ifdef HAVE_PPPOESERVER
void chap_user_add(webs_t wp)
{
	char *var = websGetVar(wp, "pppoeserver_enabled", NULL);

	if (var != NULL)
		nvram_set("pppoeserver_enabled", var);
	macro_add("pppoeserver_chapsnum");
}

void chap_user_remove(webs_t wp)
{
	char *var = websGetVar(wp, "pppoeserver_enabled", NULL);

	if (var != NULL)
		nvram_set("pppoeserver_enabled", var);
	macro_rem("pppoeserver_chapsnum", "pppoeserver_chaps");
}
#endif

#ifdef HAVE_MILKFISH
void milkfish_user_add(webs_t wp)
{
	macro_add("milkfish_ddsubscribersnum");
}

void milkfish_user_remove(webs_t wp)
{
	macro_rem("milkfish_ddsubscribersnum", "milkfish_ddsubscribers");
}

void milkfish_alias_add(webs_t wp)
{
	macro_add("milkfish_ddaliasesnum");
}

void milkfish_alias_remove(webs_t wp)
{
	macro_rem("milkfish_ddaliasesnum", "milkfish_ddaliases");
}
#endif

void forwardspec_remove(webs_t wp)
{
	macro_rem("forwardspec_entries", "forward_spec");
}

void forwardspec_add(webs_t wp)
{
	macro_add("forwardspec_entries");
}

void trigger_remove(webs_t wp)
{
	macro_rem("trigger_entries", "port_trigger");
}

void trigger_add(webs_t wp)
{
	macro_add("trigger_entries");
}

int get_vifcount(char *prefix)
{
	char *next;
	char var[80];
	char wif[16];

	sprintf(wif, "%s_vifs", prefix);
	char *vifs = nvram_safe_get(wif);

	if (vifs == NULL)
		return 0;
	int count = 0;

	foreach(var, vifs, next) {
		count++;
	}
	return count;
}

#ifdef HAVE_GUESTPORT
int gp_action = 0;

void add_mdhcpd(char *iface, int start, int max, int leasetime)
{

	char mdhcpd[32];
	char *mdhcpds;
	int var[8];

	// add mdhcpd
	if (atoi(nvram_safe_get("mdhcpd_count")) > 0)
		sprintf(mdhcpd, " %s>On>%d>%d>%d", iface, start, max, leasetime);
	else
		sprintf(mdhcpd, "%s>On>%d>%d>%d", iface, start, max, leasetime);
	mdhcpds = safe_malloc(strlen(nvram_safe_get("mdhcpd")) + strlen(mdhcpd) + 2);
	sprintf(mdhcpds, "%s%s", nvram_safe_get("mdhcpd"), mdhcpd);
	nvram_set("mdhcpd", mdhcpds);
	free(mdhcpds);

	sprintf(var, "%d", atoi(nvram_safe_get("mdhcpd_count")) + 1);
	nvram_set("mdhcpd_count", var);
}

void remove_mdhcp(char *iface)
{

	char *start, *next, *pref, *suff;
	char *mdhcpds = safe_malloc(strlen(nvram_safe_get("mdhcpd")) + 1);
	int len;
	char var[4];

	strcpy(mdhcpds, nvram_safe_get("mdhcpd"));
	start = strstr(mdhcpds, iface);
	//fprintf(stderr, "checking.... %s -> %s %s\n", mdhcpds, iface, start);
	if (start) {
		len = strlen(mdhcpds) - strlen(start);
		if (len > 0) {
			pref = safe_malloc(len);
			strncpy(pref, mdhcpds, len - 1);
			pref[len - 1] = '\0';
		} else {
			pref = safe_malloc(1);
			pref[0] = '\0';
		}
		//fprintf(stderr, "[PREF] %s\n", pref);

		next = strchr(start, ' ');
		if (next) {
			// cut entry
			len = strlen(next);
			suff = safe_malloc(len + 1);
			strncpy(suff, next, len);
			suff[len - 1] = '\0';
		} else {
			// entry at the end?
			suff = safe_malloc(1);
			suff[0] = '\0';
		}

		free(mdhcpds);

		//fprintf(stderr, "[PREF/SUFF] %s %s\n", pref, suff);   
		len = strlen(pref) + strlen(suff);
		mdhcpds = safe_malloc(len + 2);
		sprintf(mdhcpds, "%s %s", pref, suff);
		mdhcpds[len + 1] = '\0';
		//fprintf(stderr, "[MDHCP] %s\n", mdhcpds);
		nvram_set("mdhcpd", mdhcpds);

		len = atoi(nvram_safe_get("mdhcpd_count"));
		if (len > 0) {
			len--;
			//fprintf(stderr, "[MDHCPDS] %d\n", len);
			sprintf(var, "%d", len);
			nvram_set("mdhcpd_count", var);
		}

		free(mdhcpds);
		free(pref);
		free(suff);
	}
}

void move_mdhcp(char *siface, char *tiface)
{

	char *start;
	char *mdhcpds = safe_malloc(strlen(nvram_safe_get("mdhcpd")) + 1);
	int i, len, pos;
	char iface[16];

	strcpy(mdhcpds, nvram_safe_get("mdhcpd"));
	start = strstr(mdhcpds, siface);
	if (start) {
		strcpy(iface, tiface);
		len = strlen(tiface);
		pos = strlen(mdhcpds) - strlen(start);
		for (i = 0; i < len; i++) {
			mdhcpds[pos + i] = iface[i];
		}
		//fprintf(stderr, "[MDHCPD] %s->%s %d %s\n", siface, tiface, pos, mdhcpds);
		nvram_set("mdhcpd", mdhcpds);
		free(mdhcpds);
	}
}

char *getFreeLocalIpNet()
{
	return "192.168.12.1";
}
#endif
void add_vifs_single(char *prefix, int device)
{
	int count = get_vifcount(prefix);

	if (count == 16)
		return;
	char vif[16];

	sprintf(vif, "%s_vifs", prefix);
	char *vifs = nvram_safe_get(vif);

	if (vifs == NULL)
		return;
	char *n = (char *)safe_malloc(strlen(vifs) + 8);
	char v[80];
	char v2[80];
#ifdef HAVE_GUESTPORT
	char guestport[16];
	sprintf(guestport, "guestport_%s", prefix);
#endif

#ifdef HAVE_MADWIFI
	// char *cou[] = { "a", "b", "c", "d", "e", "f" };
	sprintf(v, "ath%d.%d", device, count + 1);
#else
	sprintf(v, "wl%d.%d", device, count + 1);
#endif
	if (strlen(vifs) == 0)
		sprintf(n, "%s", v);
	else
		sprintf(n, "%s %s", vifs, v);
	sprintf(v2, "%s_closed", v);
	nvram_set(v2, "0");
	sprintf(v2, "%s_mode", v);
	nvram_set(v2, "ap");

	sprintf(v2, "%s_ap_isolate", v);
	nvram_set(v2, "0");
	sprintf(v2, "%s_ssid", v);
#ifdef HAVE_MAKSAT
#ifdef HAVE_MAKSAT_BLANK
	nvram_set(v2, "default_vap");
#else
	nvram_set(v2, "maksat_vap");
#endif
#elif defined(HAVE_SANSFIL)
	nvram_set(v2, "sansfil_vap");
#elif defined(HAVE_TRIMAX)
	nvram_set(v2, "m2m_vap");
#elif defined(HAVE_WIKINGS)
	nvram_set(v2, "Excel Networks_vap");
#elif defined(HAVE_ESPOD)
	nvram_set(v2, "ESPOD Technologies_vap");
#elif defined(HAVE_NEXTMEDIA)
	nvram_set(v2, "nextmedia_vap");
#elif defined(HAVE_TMK)
	nvram_set(v2, "KMT_vap");
#elif defined(HAVE_CORENET)
	nvram_set(v2, "corenet.ap");
#elif defined(HAVE_ONNET)
	nvram_set(v2, "OTAi_vap");
#elif defined(HAVE_KORENRON)
	nvram_set(v2, "WBR2000_vap");
#elif defined(HAVE_HDWIFI)
	nvram_set(v2, "hdwifi_vap");
#else
	nvram_set(v2, "dd-wrt_vap");
#endif
	sprintf(v2, "%s_vifs", prefix);
	nvram_set(v2, n);
	sprintf(v2, "%s_bridged", v);
	nvram_set(v2, "1");
	sprintf(v2, "%s_nat", v);
	nvram_set(v2, "1");
	sprintf(v2, "%s_ipaddr", v);
	nvram_set(v2, "0.0.0.0");
	sprintf(v2, "%s_netmask", v);
	nvram_set(v2, "0.0.0.0");

	sprintf(v2, "%s_gtk_rekey", v);
	nvram_set(v2, "3600");

	sprintf(v2, "%s_radius_port", v);
	nvram_set(v2, "1812");

	sprintf(v2, "%s_radius_ipaddr", v);
	nvram_set(v2, "0.0.0.0");
#ifdef HAVE_MADWIFI
	sprintf(v2, "%s_radius2_ipaddr", v);
	nvram_set(v2, "0.0.0.0");

	sprintf(v2, "%s_radius2_port", v);
	nvram_set(v2, "1812");

#endif
#ifdef HAVE_GUESTPORT
	char v3[80];
	if (gp_action == 1) {
		nvram_set(guestport, v);

		sprintf(v2, "%s_ssid", v);
#ifdef HAVE_WZRHPAG300NH
		if (has_5ghz(prefix))
			nvram_set(v2, "GuestPort_A");
		else
			nvram_set(v2, "GuestPort_G");
#else
		nvram_set(v2, "GuestPort");
#endif

		sprintf(v2, "%s_bridged", v);
		nvram_set(v2, "0");

		sprintf(v2, "%s_ipaddr", v);
		nvram_set(v2, getFreeLocalIpNet());

		sprintf(v2, "%s_netmask", v);
		nvram_set(v2, "255.255.255.0");

		sprintf(v2, "%s_security_mode", v);
		nvram_set(v2, "psk psk2");

		sprintf(v2, "%s_akm", v);
		nvram_set(v2, "psk psk2");

		sprintf(v2, "%s_crypto", v);
		nvram_set(v2, "tkip+aes");

		sprintf(v2, "%s_wpa_psk", v);
#ifdef HAVE_WZRHPAG300NH
		if (has_5ghz(prefix))
			sprintf(v3, "DEF-p_wireless_%s0_11a-wpapsk", prefix);
		else
			sprintf(v3, "DEF-p_wireless_%s0_11bg-wpapsk", prefix);
#else
		sprintf(v3, "DEF-p_wireless_%s_11bg-wpapsk", prefix);
#endif
		nvram_set(v2, getUEnv(v3));

		add_mdhcpd(v, 20, 200, 3600);
		//required to use mdhcpd
		nvram_set("dhcp_dnsmasq", "1");

		rep(v, '.', 'X');

		sprintf(v2, "%s_security_mode", v);
		nvram_set(v2, "psk psk2");

		sprintf(v2, "%s_crypto", v);
		nvram_set(v2, "tkip+aes");
	}
	gp_action = 0;
#endif

	// nvram_commit ();
	free(n);
}

void add_vifs(webs_t wp)
{
	char *prefix = websGetVar(wp, "iface", NULL);

	if (prefix == NULL)
		return;
	int devcount = prefix[strlen(prefix) - 1] - '0';
#ifdef HAVE_GUESTPORT
	if (!strcmp(websGetVar(wp, "gp_modify", ""), "add")) {
		gp_action = 1;
	}
#endif
	add_vifs_single(prefix, devcount);
}

void move_vif(char *prefix, char *svif, char *tvif)
{

	char filename[32];
	char command[64];

	//fprintf(stderr, "[VIFS] move %s -> %s\n", svif, tvif);
	sprintf(filename, "/tmp/.nvram_%s", svif);
	sprintf(command, "nvram show | grep %s_ > /tmp/.nvram_%s", svif, svif);
	//fprintf(stderr, "[VIFS] %s\n", command);
	system(command);

	FILE *fp;
	char line[80];
	char var[16];
	char tvifx[16];
	char nvram_var[32];
	char nvram_val[32];
	int len;
	int pos = 0;
	int xpos;

	strcpy(tvifx, tvif);
	rep(tvifx, '.', 'X');

	if (fp = fopen(filename, "r")) {
		while (fgets(line, sizeof(line), fp)) {
			pos = strcspn(line, "=");
			if (pos) {
				xpos = strcspn(line, "X");
				len = strlen(svif);
				strncpy(var, line + len, pos - len);
				var[pos - len] = '\0';
				if (xpos > 0 && xpos < pos) {
					sprintf(nvram_var, "%s%s", tvifx, var);
				} else {
					sprintf(nvram_var, "%s%s", tvif, var);
				}

				strncpy(nvram_val, line + pos + 1, strlen(line) - pos);
				nvram_val[strlen(line) - pos - 2] = '\0';
				//fprintf(stderr, "[VIF] %s %s\n", nvram_var, nvram_val);
				nvram_set(nvram_var, nvram_val);
			}
		}
		fclose(fp);
		unlink(filename);
	}
}

void remove_vifs_single(char *prefix)
{
	char wif[16];

	sprintf(wif, "%s_vifs", prefix);
	int o = -1;
	char *vifs = nvram_safe_get(wif);
	char copy[128];

	strcpy(copy, vifs);
	int i;
	int slen = strlen(copy);

#ifdef HAVE_GUESTPORT
	int q = 0;
	int j;
	int gp_found = 0;
	char vif[16], pvif[16];
	char guestport[16];
	sprintf(guestport, "guestport_%s", prefix);

	if (nvram_get(guestport)) {
		if (gp_action == 2) {
			for (i = 0; i <= slen; i++) {
				if (copy[i] == 0x20 || i == slen) {
					if (gp_found)
						strcpy(pvif, vif);
					if (o > 0)
						q = o + 1;
					o = i;
					for (j = 0; j < o - q; j++) {
						vif[j] = copy[j + q];
					}
					vif[j] = '\0';

					if (gp_found) {
						move_vif(prefix, vif, pvif);
					}

					if (nvram_match(guestport, vif))
						gp_found = 1;
				}
			}
			remove_mdhcp(nvram_get(guestport));
			nvram_unset(guestport);
		} else {
			o = slen;
			for (i = slen; i >= 0; i--) {
				if (copy[i] == 0x20 || i == 0) {
					if (gp_found)
						strcpy(pvif, vif);
					if (i == 0)
						q = i;
					else
						q = i + 1;
					for (j = 0; j < o - q; j++) {
						vif[j] = copy[j + q];
					}
					vif[j] = '\0';

					if (gp_found == slen) {
						move_vif(prefix, pvif, vif);
						nvram_set(guestport, vif);
						move_mdhcp(pvif, vif);
						gp_found = 0;
					}

					if (nvram_match(guestport, vif))
						gp_found = o;
					o = i;
				}
			}
		}
	}
	gp_action = 0;
#endif
	o = -1;
	for (i = 0; i < slen; i++) {
		if (copy[i] == 0x20)
			o = i;
	}

	if (o == -1) {
		nvram_set(wif, "");
	} else {
		copy[o] = 0;
		nvram_set(wif, copy);
	}
	// nvram_commit ();
#ifdef HAVE_AOSS
// must remove all aoss vap's if one of them is touched
	if (strlen(nvram_safe_get("aoss_vifs"))) {
		nvram_unset("ath0_vifs");
		nvram_unset("aoss_vifs");
		nvram_commit();
	}
	if (strlen(nvram_safe_get("aossa_vifs"))) {
		nvram_unset("ath1_vifs");
		nvram_unset("aossa_vifs");
		nvram_commit();
	}
#endif
}

void remove_vifs(webs_t wp)
{
	char *prefix = websGetVar(wp, "iface", NULL);
#ifdef HAVE_GUESTPORT
	if (!strcmp(websGetVar(wp, "gp_modify", ""), "remove")) {
		gp_action = 2;
	}
#endif
	remove_vifs_single(prefix);
}

#ifdef HAVE_BONDING
void add_bond(webs_t wp)
{
	static char word[256];
	char *next, *wordlist;
	int count = 0;
	int realcount = atoi(nvram_safe_get("bonding_count"));

	if (realcount == 0) {
		wordlist = nvram_safe_get("bondings");
		foreach(word, wordlist, next) {
			count++;
		}
		realcount = count;
	}
	realcount++;
	char var[32];

	sprintf(var, "%d", realcount);
	nvram_set("bonding_count", var);
	nvram_commit();
	return;
}

void del_bond(webs_t wp)
{
	static char word[256];
	int realcount = 0;
	char *next, *wordlist, *newwordlist;
	char *val = websGetVar(wp, "del_value", NULL);

	if (val == NULL)
		return;
	int todel = atoi(val);

	wordlist = nvram_safe_get("bondings");
	newwordlist = (char *)safe_malloc(strlen(wordlist));
	memset(newwordlist, 0, strlen(wordlist));
	int count = 0;

	foreach(word, wordlist, next) {
		if (count != todel) {
			strcat(newwordlist, word);
			strcat(newwordlist, " ");
		}
		count++;
	}

	char var[32];

	realcount = atoi(nvram_safe_get("bonding_count")) - 1;
	sprintf(var, "%d", realcount);
	nvram_set("bonding_count", var);
	nvram_set("bondings", newwordlist);
	nvram_commit();
	free(newwordlist);

	return;
}
#endif

#ifdef HAVE_OLSRD
void add_olsrd(webs_t wp)
{
	char *ifname = websGetVar(wp, "olsrd_ifname", NULL);

	if (ifname == NULL)
		return;
	char *wordlist = nvram_safe_get("olsrd_interfaces");
	char *addition = ">5.0>90.0>2.0>270.0>15.0>90.0>15.0>90.0";
	char *newadd = (char *)safe_malloc(strlen(wordlist) + strlen(addition) + strlen(ifname) + 2);
	if (strlen(wordlist) > 0) {
		strcpy(newadd, wordlist);
		strcat(newadd, " ");
		strcat(newadd, ifname);
	} else {
		strcpy(newadd, ifname);
	}
	strcat(newadd, addition);
	nvram_set("olsrd_interfaces", newadd);
	nvram_commit();
	free(newadd);
	return;
}

void del_olsrd(webs_t wp)
{
	char *del = websGetVar(wp, "olsrd_delcount", NULL);

	if (del == NULL)
		return;
	int d = atoi(del);
	char *wordlist = nvram_safe_get("olsrd_interfaces");
	char *newlist = (char *)safe_malloc(strlen(wordlist) + 1);

	memset(newlist, 0, strlen(wordlist));
	char *next;
	char word[128];
	int count = 0;

	foreach(word, wordlist, next) {
		if (count != d)
			sprintf(newlist, "%s %s", newlist, word);
		count++;
	}
	nvram_set("olsrd_interfaces", newlist);
	nvram_commit();
	free(newlist);
	return;
}

void save_olsrd(webs_t wp)
{
	char *wordlist = nvram_safe_get("olsrd_interfaces");
	char *newlist = (char *)safe_malloc(strlen(wordlist) + 512);

	memset(newlist, 0, strlen(wordlist) + 512);
	char *next;
	char word[64];

	foreach(word, wordlist, next) {
		char *interface = word;
		char *dummy = interface;

		strsep(&dummy, ">");
		char valuename[32];

		sprintf(valuename, "%s_hellointerval", interface);
		char *hellointerval = websGetVar(wp, valuename, "0");

		sprintf(valuename, "%s_hellovaliditytime", interface);
		char *hellovaliditytime = websGetVar(wp, valuename, "0");

		sprintf(valuename, "%s_tcinterval", interface);
		char *tcinterval = websGetVar(wp, valuename, "0");

		sprintf(valuename, "%s_tcvaliditytime", interface);
		char *tcvaliditytime = websGetVar(wp, valuename, "0");

		sprintf(valuename, "%s_midinterval", interface);
		char *midinterval = websGetVar(wp, valuename, "0");

		sprintf(valuename, "%s_midvaliditytime", interface);
		char *midvaliditytime = websGetVar(wp, valuename, "0");

		sprintf(valuename, "%s_hnainterval", interface);
		char *hnainterval = websGetVar(wp, valuename, "0");

		sprintf(valuename, "%s_hnavaliditytime", interface);
		char *hnavaliditytime = websGetVar(wp, valuename, "0");

		sprintf(newlist, "%s %s>%s>%s>%s>%s>%s>%s>%s>%s", newlist, interface, hellointerval, hellovaliditytime, tcinterval, tcvaliditytime, midinterval, midvaliditytime, hnainterval, hnavaliditytime);
	}
	nvram_set("olsrd_interfaces", newlist);
	nvram_commit();
	free(newlist);
	return;
}
#endif

#ifdef HAVE_VLANTAGGING

void save_networking(webs_t wp)
{
	char *value = websGetVar(wp, "action", "");
	int vlancount = atoi(nvram_safe_get("vlan_tagcount"));
	int bridgescount = atoi(nvram_safe_get("bridges_count"));
	int bridgesifcount = atoi(nvram_safe_get("bridgesif_count"));
	int mdhcpd_count = atoi(nvram_safe_get("mdhcpd_count"));

#ifdef HAVE_BONDING
	int bondcount = atoi(nvram_safe_get("bonding_count"));
#endif
	int i;

	// save vlan stuff
	char buffer[1024];

	memset(buffer, 0, 1024);
	for (i = 0; i < vlancount; i++) {
		char *ifname, *tag, *prio;
		char var[32];

		sprintf(var, "vlanifname%d", i);
		ifname = websGetVar(wp, var, NULL);
		if (!ifname)
			return;
		sprintf(var, "vlantag%d", i);
		tag = websGetVar(wp, var, NULL);
		if (!tag)
			return;
		sprintf(var, "vlanprio%d", i);
		prio = websGetVar(wp, var, NULL);
		if (!prio)
			return;
		strcat(buffer, ifname);
		strcat(buffer, ">");
		strcat(buffer, tag);
		strcat(buffer, ">");
		strcat(buffer, prio);
		if (i < vlancount - 1)
			strcat(buffer, " ");
	}
	nvram_set("vlan_tags", buffer);
	// save bonds
	memset(buffer, 0, 1024);
#ifdef HAVE_BONDING
	char *bondingnumber = websGetVar(wp, "bonding_number", NULL);

	if (bondingnumber)
		nvram_set("bonding_number", bondingnumber);
	char *bondingtype = websGetVar(wp, "bonding_type", NULL);

	if (bondingtype)
		nvram_set("bonding_type", bondingtype);
	for (i = 0; i < bondcount; i++) {
		char *ifname, *tag;
		char var[32];

		sprintf(var, "bondingifname%d", i);
		ifname = websGetVar(wp, var, NULL);
		if (!ifname)
			return;
		sprintf(var, "bondingattach%d", i);
		tag = websGetVar(wp, var, NULL);
		if (!tag)
			return;
		strcat(buffer, ifname);
		strcat(buffer, ">");
		strcat(buffer, tag);
		if (i < bondcount - 1)
			strcat(buffer, " ");
	}
	nvram_set("bondings", buffer);
	memset(buffer, 0, 1024);
#endif

	// save bridges

	for (i = 0; i < bridgescount; i++) {
		char *ifname, *tag, *prio, *mtu, *mcast;
		char var[32];
		char ipaddr[32];
		char netmask[32];
		char n[32];

		memset(ipaddr, 0, 32);
		memset(netmask, 0, 32);
		sprintf(var, "bridgename%d", i);
		ifname = websGetVar(wp, var, NULL);
		if (!ifname)
			return;
		sprintf(var, "bridgestp%d", i);
		tag = websGetVar(wp, var, NULL);
		if (!tag)
			return;
		sprintf(var, "bridgemcastbr%d", i);
		mcast = websGetVar(wp, var, NULL);
		if (!mcast){
			return;
		}else{
			sprintf(n, "%s_mcast", ifname);
			if (!strcmp(mcast,"Filtered"))
			    nvram_set(n, "1");
			else
			    nvram_set(n, "0");
		}
		sprintf(var, "bridgeprio%d", i);
		prio = websGetVar(wp, var, NULL);
		if (!prio)
			prio = "32768";
		if (strlen(prio) == 0)
			prio = "32768";

		sprintf(var, "bridgemtu%d", i);
		mtu = websGetVar(wp, var, NULL);
		if (!mtu)
			mtu = "1500";
		if (strlen(prio) == 0)
			mtu = "1500";

		sprintf(n, "%s_ipaddr", ifname);
		if (get_merge_ipaddr(wp, n, ipaddr))
			nvram_set(n, ipaddr);
		sprintf(n, "%s_netmask", ifname);
		if (get_merge_ipaddr(wp, n, netmask))
			nvram_set(n, netmask);

		strcat(buffer, ifname);
		strcat(buffer, ">");
		if (!strcmp(tag, "On"))
			strcat(buffer, "On");
		else
			strcat(buffer, "Off");
		strcat(buffer, ">");
		strcat(buffer, prio);
		strcat(buffer, ">");
		strcat(buffer, mtu);
		if (i < bridgescount - 1)
			strcat(buffer, " ");
	}
	nvram_set("bridges", buffer);
	// save bridge assignment
	memset(buffer, 0, 1024);
	for (i = 0; i < bridgesifcount; i++) {
		char *ifname, *tag, *prio;
		char var[32];

		sprintf(var, "bridge%d", i);
		ifname = websGetVar(wp, var, NULL);
		if (!ifname)
			return;
		sprintf(var, "bridgeif%d", i);
		tag = websGetVar(wp, var, NULL);
		if (!tag)
			return;
		sprintf(var, "bridgeifprio%d", i);
		prio = websGetVar(wp, var, NULL);
		if (!prio)
			prio = "128";
		if (strlen(prio) == 0)
			prio = "128";
		strcat(buffer, ifname);
		strcat(buffer, ">");
		strcat(buffer, tag);
		strcat(buffer, ">");
		strcat(buffer, prio);
		if (i < bridgesifcount - 1)
			strcat(buffer, " ");
	}
	nvram_set("bridgesif", buffer);
#ifdef HAVE_MDHCP
	// save multipe dhcp-servers
	memset(buffer, 0, 1024);
	// if (!interface || !start || !dhcpon || !max || !leasetime)
	for (i = 0; i < mdhcpd_count; i++) {
		char *mdhcpinterface, *mdhcpon, *mdhcpstart, *mdhcpmax, *mdhcpleasetime;
		char var[32];

		sprintf(var, "mdhcpifname%d", i);
		mdhcpinterface = websGetVar(wp, var, NULL);
		if (!mdhcpinterface)
			return;

		sprintf(var, "mdhcpon%d", i);
		mdhcpon = websGetVar(wp, var, NULL);
		if (!mdhcpon)
			return;

		sprintf(var, "mdhcpstart%d", i);
		mdhcpstart = websGetVar(wp, var, NULL);
		if (!mdhcpstart)
			return;

		sprintf(var, "mdhcpmax%d", i);
		mdhcpmax = websGetVar(wp, var, NULL);
		if (!mdhcpmax)
			return;

		sprintf(var, "mdhcpleasetime%d", i);
		mdhcpleasetime = websGetVar(wp, var, NULL);
		if (!mdhcpleasetime)
			return;

		strcat(buffer, mdhcpinterface);
		strcat(buffer, ">");
		strcat(buffer, mdhcpon);
		strcat(buffer, ">");
		strcat(buffer, mdhcpstart);
		strcat(buffer, ">");
		strcat(buffer, mdhcpmax);
		strcat(buffer, ">");
		strcat(buffer, mdhcpleasetime);
		if (i < mdhcpd_count - 1)
			strcat(buffer, " ");
	}
	nvram_set("mdhcpd", buffer);
#endif
#ifdef HAVE_PORTSETUP
	validate_portsetup(wp, NULL, NULL);
#endif

	applytake(value);
}

void add_vlan(webs_t wp)
{
	static char word[256];
	char *next, *wordlist;
	int count = 0;
	int realcount = atoi(nvram_safe_get("vlan_tagcount"));

	if (realcount == 0) {
		wordlist = nvram_safe_get("vlan_tags");
		foreach(word, wordlist, next) {
			count++;
		}
		realcount = count;
	}
	realcount++;
	char var[32];

	sprintf(var, "%d", realcount);
	nvram_set("vlan_tagcount", var);
	nvram_commit();
	return;
}

void del_vlan(webs_t wp)
{
	static char word[256];
	int realcount = 0;
	char *next, *wordlist, *newwordlist;
	char *val = websGetVar(wp, "del_value", NULL);

	if (val == NULL)
		return;
	int todel = atoi(val);

	wordlist = nvram_safe_get("vlan_tags");
	newwordlist = (char *)safe_malloc(strlen(wordlist));
	memset(newwordlist, 0, strlen(wordlist));
	int count = 0;

	foreach(word, wordlist, next) {
		if (count != todel) {
			strcat(newwordlist, word);
			strcat(newwordlist, " ");
		} else {
			char *port = word;
			char *tag = strsep(&port, ">");

			if (!tag || !port)
				break;
			char names[32];

			sprintf(names, "%s.%s", tag, port);
			eval("ifconfig", names, "down");
			eval("vconfig", "rem", names);
		}
		count++;
	}

	char var[32];

	realcount = atoi(nvram_safe_get("vlan_tagcount")) - 1;
	sprintf(var, "%d", realcount);
	nvram_set("vlan_tagcount", var);
	nvram_set("vlan_tags", newwordlist);
	nvram_commit();
	free(newwordlist);

	return;
}

void add_mdhcp(webs_t wp)
{
	static char word[256];
	char *next, *wordlist;
	int count = 0;
	int realcount = atoi(nvram_safe_get("mdhcpd_count"));

	if (realcount == 0) {
		wordlist = nvram_safe_get("mdhcpd");
		foreach(word, wordlist, next) {
			count++;
		}
		realcount = count;
	}
	realcount++;
	char var[32];

	sprintf(var, "%d", realcount);
	nvram_set("mdhcpd_count", var);
	nvram_commit();
	return;
}

void del_mdhcp(webs_t wp)
{
	static char word[256];
	int realcount = 0;
	char *next, *wordlist, *newwordlist;
	char *val = websGetVar(wp, "del_value", NULL);

	if (val == NULL)
		return;
	int todel = atoi(val);

	wordlist = nvram_safe_get("mdhcpd");
	newwordlist = (char *)safe_malloc(strlen(wordlist));
	memset(newwordlist, 0, strlen(wordlist));
	int count = 0;

	foreach(word, wordlist, next) {
		if (count != todel) {
			strcat(newwordlist, word);
			strcat(newwordlist, " ");
		}
		count++;
	}

	char var[32];

	realcount = atoi(nvram_safe_get("mdhcpd_count")) - 1;
	sprintf(var, "%d", realcount);
	nvram_set("mdhcpd_count", var);
	nvram_set("mdhcpd", newwordlist);
	nvram_commit();
	free(newwordlist);

	return;
}

void del_bridge(webs_t wp)
{
	static char word[256];
	int realcount = 0;
	char *next, *wordlist, *newwordlist;
	char *val = websGetVar(wp, "del_value", NULL);

	if (val == NULL)
		return;
	int todel = atoi(val);

	wordlist = nvram_safe_get("bridges");
	newwordlist = (char *)safe_malloc(strlen(wordlist));
	memset(newwordlist, 0, strlen(wordlist));
	int count = 0;

	foreach(word, wordlist, next) {
		if (count != todel) {
			strcat(newwordlist, word);
			strcat(newwordlist, " ");
		} else {
			char *port = word;
			char *tag = strsep(&port, ">");
			char *prio = port;

			strsep(&prio, ">");
			if (!tag || !port)
				continue;
			eval("ifconfig", tag, "down");
			eval("brctl", "delbr", tag);
		}
		count++;
	}

	realcount = atoi(nvram_safe_get("bridges_count")) - 1;
	char var[32];

	sprintf(var, "%d", realcount);
	nvram_set("bridges_count", var);
	nvram_set("bridges", newwordlist);
	nvram_commit();
	free(newwordlist);

	return;
}

void add_bridge(webs_t wp)
{
	static char word[256];
	char *next, *wordlist;
	int count = 0;
	int realcount = atoi(nvram_safe_get("bridges_count"));

	if (realcount == 0) {
		wordlist = nvram_safe_get("bridges");
		foreach(word, wordlist, next) {
			count++;
		}
		realcount = count;
	}
	realcount++;
	char var[32];

	sprintf(var, "%d", realcount);
	nvram_set("bridges_count", var);
	nvram_commit();
	return;
}

void del_bridgeif(webs_t wp)
{
	static char word[256];
	int realcount = 0;
	char *next, *wordlist, *newwordlist;
	char *val = websGetVar(wp, "del_value", NULL);

	if (val == NULL)
		return;
	int todel = atoi(val);

	wordlist = nvram_safe_get("bridgesif");
	newwordlist = (char *)safe_malloc(strlen(wordlist));
	memset(newwordlist, 0, strlen(wordlist));
	int count = 0;

	foreach(word, wordlist, next) {
		if (count != todel) {
			strcat(newwordlist, word);
			strcat(newwordlist, " ");
		}
		count++;
	}

	char var[32];

	realcount = atoi(nvram_safe_get("bridgesif_count")) - 1;
	sprintf(var, "%d", realcount);
	nvram_set("bridgesif_count", var);
	nvram_set("bridgesif", newwordlist);
	nvram_commit();
	free(newwordlist);

	return;
}

void add_bridgeif(webs_t wp)
{

	static char word[256];
	char *next, *wordlist;
	int count = 0;
	int realcount = atoi(nvram_safe_get("bridgesif_count"));

	if (realcount == 0) {
		wordlist = nvram_safe_get("bridgesif");
		foreach(word, wordlist, next) {
			count++;
		}
		realcount = count;
	}
	realcount++;
	char var[32];

	sprintf(var, "%d", realcount);
	nvram_set("bridgesif_count", var);
	nvram_commit();
	return;
}

#endif

static void save_prefix(webs_t wp, char *prefix)
{
	char n[80];

#ifdef HAVE_RELAYD
	char gwaddr[32];
	copytonv(wp, "%s_relayd_gw_auto", prefix);
	sprintf(n, "%s_relayd_gw_ipaddr", prefix);
	if (get_merge_ipaddr(wp, n, gwaddr))
		nvram_set(n, gwaddr);
#endif
#ifdef HAVE_IFL
	copytonv(wp, "%s_label", prefix);
	copytonv(wp, "%s_note", prefix);
#endif
#ifdef HAVE_MADWIFI
	char sifs[80];
	char turbo[80];
	char chanbw[80];
	char preamble[80];
	int cbwchanged = 0;
	copytonv(wp, "rate_control");
#endif
	sprintf(n, "%s_ssid", prefix);
	copytonv(wp, n);
	if (!strcmp(prefix, "wl0") || !strcmp(prefix, "wl1")) {
		char *wl = websGetVar(wp, n, NULL);

		cprintf("copy value %s which is [%s] to nvram\n", n, wl);
		if (wl) {
			if (!strcmp(prefix, "wl0"))
				nvram_set("wl_ssid", wl);
			else
				nvram_set("wl1_ssid", wl);
		}
	}
	copytonv(wp, "%s_distance", prefix);
#ifdef HAVE_MADWIFI
	{
		sprintf(n, "%s_txpwrdbm", prefix);
		char *sl = websGetVar(wp, n, NULL);

		if (sl) {
			int base = atoi(sl);
#ifdef HAVE_WIKINGS
/*			if (base > 28)
				base = 28;
#ifdef HAVE_SUB3
			if (base > 25)
				base = 25;
#endif
#ifdef HAVE_SUB6
			if (base > 22)
				base = 22;
#endif*/
#endif

#ifdef HAVE_ESPOD
			if (base > 30)
				base = 30;
#ifdef HAVE_SUB6
			if (base > 30)
				base = 30;
#endif
#ifdef HAVE_SUB3
			if (base > 28)
				base = 28;
#endif
#endif
			int txpower = base - wifi_gettxpoweroffset(prefix);

			if (txpower < 1)
				txpower = 1;
			sprintf(turbo, "%d", txpower);
			nvram_set(n, turbo);
		}
	}
	copytonv(wp, "%s_antgain", prefix);
	copytonv(wp, "%s_regulatory", prefix);
	sprintf(n, "%s_scanlist", prefix);
	{
		char *sl = websGetVar(wp, n, NULL);

		if (sl) {
			char *slc = (char *)safe_malloc(strlen(sl) + 1);

			strcpy(slc, sl);
			int i, sllen = strlen(slc);

			for (i = 0; i < sllen; i++) {
				if (slc[i] == ';')
					slc[i] = ' ';
				if (slc[i] == ',')
					slc[i] = ' ';
			}
			nvram_set(n, slc);
			free(slc);
		}
	}
#ifdef HAVE_MAKSAT
	copytonv(wp, "ath_specialmode");
#endif
	copytonv(wp, "%s_regdomain", prefix);

	copytonv(wp, "%s_rts", prefix);
	if (nvram_nmatch("1", "%s_rts", prefix)) {
		sprintf(turbo, "%s_rtsvalue", prefix);
		char *tw = websGetVar(wp, turbo, NULL);

		if (tw) {
			if (atoi(tw) < 1)
				tw = "1";
			if (atoi(tw) > 2346)
				tw = "2346";
			nvram_nset(tw, "%s_rtsvalue", prefix);
		}
	}
	copytonv(wp, "%s_protmode", prefix);
	copytonv(wp, "%s_minrate", prefix);
	copytonv(wp, "%s_maxrate", prefix);
	copytonv(wp, "%s_xr", prefix);
	copytonv(wp, "%s_outdoor", prefix);
//    copytonv( wp, "%s_compression", prefix ); // Atheros SuperG header
	// compression
	copytonv(wp, "%s_ff", prefix);	// ff = 0, Atheros SuperG fast
	// framing disabled, 1 fast framing
	// enabled
	copytonv(wp, "%s_diversity", prefix);
	copytonv(wp, "%s_preamble", prefix);
	copytonv(wp, "%s_wmm", prefix);
	copytonv(wp, "%s_txantenna", prefix);
	copytonv(wp, "%s_rxantenna", prefix);
	copytonv(wp, "%s_intmit", prefix);
	copytonv(wp, "%s_csma", prefix);
	copytonv(wp, "%s_noise_immunity", prefix);
	copytonv(wp, "%s_ofdm_weak_det", prefix);

	copytonv(wp, "%s_chanshift", prefix);
	copytonv(wp, "%s_doth", prefix);
	copytonv(wp, "%s_maxassoc", prefix);

	sprintf(chanbw, "%s_channelbw", prefix);
	char *cbw = websGetVar(wp, chanbw, NULL);

	if (cbw && !nvram_match(chanbw, cbw)) {
		cbwchanged = 1;
	}
	if (cbw)
		nvram_set(chanbw, cbw);

	copytonv(wp, "%s_xr", prefix);
	copytonv(wp, "%s_sifstime", prefix);
	copytonv(wp, "%s_preambletime", prefix);
	copytonv(wp, "%s_mtikie", prefix);
	copytonv(wp, "%s_cardtype", prefix);

#endif
	copytonv(wp, "%s_closed", prefix);
#ifdef HAVE_80211AC
	if (has_2ghz(prefix) && has_ac(prefix))
		copytonv(wp, "%s_turbo_qam", prefix);
	if (has_beamforming(prefix)) {
		copytonv(wp, "%s_txbf", prefix);
		copytonv(wp, "%s_itxbf", prefix);
	}
#endif

#ifndef HAVE_MADWIFI
	char *ifname = "wl0";

#ifndef HAVE_RT2880

	if (!strcmp(prefix, "wl0"))
		ifname = get_wl_instance_name(0);
	else if (!strcmp(prefix, "wl1"))
		ifname = get_wl_instance_name(1);
	else
		ifname = prefix;
#else
	ifname = getRADev(prefix);
#endif
	copytonv(wp, "%s_multicast", ifname);
	copytonv(wp, "%s_bridged", ifname);
	copytonv(wp, "%s_nat", prefix);
	copytonv(wp, "%s_isolation", prefix);

	char addr[32];

	sprintf(n, "%s_ipaddr", ifname);
	if (get_merge_ipaddr(wp, n, addr))
		nvram_set(n, addr);

	sprintf(n, "%s_netmask", ifname);
	if (get_merge_ipaddr(wp, n, addr))
		nvram_set(n, addr);
#else

	copytonv(wp, "%s_multicast", prefix);
	copytonv(wp, "%s_bridged", prefix);
	copytonv(wp, "%s_nat", prefix);
	copytonv(wp, "%s_isolation", prefix);

	char addr[32];

	sprintf(n, "%s_ipaddr", prefix);
	if (get_merge_ipaddr(wp, n, addr))
		nvram_set(n, addr);

	sprintf(n, "%s_netmask", prefix);
	if (get_merge_ipaddr(wp, n, addr))
		nvram_set(n, addr);

	copytonv(wp, "%s_duallink", prefix);
	sprintf(n, "%s_duallink_parent", prefix);
	if (get_merge_ipaddr(wp, n, addr))
		nvram_set(n, addr);

#endif

	copytonv(wp, "%s_ap_isolate", prefix);
	sprintf(n, "%s_mode", prefix);
	char *wl_newmode = websGetVar(wp, n, NULL);
	if (wl_newmode && (nvram_match(n, "sta") || nvram_match(n, "apsta")) && strcmp(wl_newmode, "sta") && strcmp(wl_newmode, "apsta"))
		notifywanChange();

	if (wl_newmode && nvram_match(n, "ap") && ( !strcmp(wl_newmode, "sta") || !strcmp(wl_newmode, "apsta") ) )
		notifywanChange();

	if (nvram_match(n, "sta")) {

		if (wl_newmode)
			if ((!strcmp(wl_newmode, "ap") || !strcmp(wl_newmode, "wdsap")
			     || !strcmp(wl_newmode, "infra") || !strcmp(wl_newmode, "wdssta"))
			    && nvram_invmatch("wan_proto", "3g")) {
				nvram_set("wan_proto", "disabled");
			}
	}
	copytonv(wp, n);
	if (!strcmp(prefix, "wl0") || !strcmp(prefix, "wl1")) {
		char *wl = websGetVar(wp, n, NULL);

		cprintf("copy value %s which is [%s] to nvram\n", n, wl);
		if (wl && !strcmp(prefix, "wl0"))
			nvram_set("wl_mode", wl);
#ifndef HAVE_MADWIFI
		if (strcmp(wl, "ap") && strcmp(wl, "apsta")
		    && strcmp(wl, "apstawet")) {
			nvram_nset("", "%s_vifs", prefix);
		}
#endif
	}
	int chanchanged = 0;

#ifdef HAVE_RT2880
	copytonv(wp, "%s_greenfield", prefix);
#endif

#ifndef HAVE_MADWIFI
	if (!strcmp(prefix, "wl0") || !strcmp(prefix, "wl1"))
#endif
	{
		sprintf(n, "%s_net_mode", prefix);
		if (!nvram_match(n, websGetVar(wp, n, ""))) {
			chanchanged = 1;
			copytonv(wp, n);
			char *value = websGetVar(wp, n, "");
			if (!strcmp(prefix, "wl0"))
				convert_wl_gmode(value, "wl");
			else
				convert_wl_gmode(value, prefix);
		}
	}
#ifdef HAVE_MADWIFI
	if (cbwchanged || chanchanged) {
		if (nvram_match(chanbw, "40")) {
			nvram_set(sifs, "8");
			nvram_set(preamble, "14");
		} else if (nvram_match(chanbw, "5")) {
			nvram_set(sifs, "64");
			nvram_set(preamble, "80");
		} else if (nvram_match(chanbw, "10")) {
			nvram_set(sifs, "32");
			nvram_set(preamble, "40");
		} else {
			nvram_set(sifs, "16");
			nvram_set(preamble, "20");
		}

	}
#endif

	copytonv(wp, "%s_nbw", prefix);
	copytonv(wp, "%s_nctrlsb", prefix);

	sprintf(n, "%s_channel", prefix);
	if (!strcmp(prefix, "wl0") || !strcmp(prefix, "wl1")) {
		char *wl = websGetVar(wp, n, NULL);

		cprintf("copy value %s which is [%s] to nvram\n", n, wl);
		if (wl && !strcmp(prefix, "wl0"))
			nvram_set("wl_channel", wl);
		else if (wl)
			nvram_set("wl1_channel", wl);
	}
	copytonv(wp, n);

	sprintf(n, "%s_wchannel", prefix);
	if (!strcmp(prefix, "wl0") || !strcmp(prefix, "wl1")) {
		char *wl = websGetVar(wp, n, NULL);

		cprintf("copy value %s which is [%s] to nvram\n", n, wl);
		if (wl && !strcmp(prefix, "wl0"))
			nvram_set("wl_wchannel", wl);
		else if (wl)
			nvram_set("wl1_wchannel", wl);

	}

	copytonv(wp, n);

#if defined(HAVE_NORTHSTAR) && !defined(HAVE_BUFFALO)
	sprintf(n, "wl_regdomain");
	char *reg = websGetVar(wp, n, NULL);
	if (reg) {
		if (strcmp(nvram_get("wl_regdomain"), reg)) {
			setRegulationDomain(reg);
			system("startservice lan");
		}
	}
#endif
	copytonv(wp, "wl_regdomain");

}

void wireless_join(webs_t wp)
{
	char *value = websGetVar(wp, "action", "");
	char *ssid = websGetVar(wp, "wl_ssid", NULL);
	if (ssid) {
		char *wifi = nvram_safe_get("wifi_display");
		if (strlen(wifi) > 0) {
			if (!strcmp(wifi, "ath0"))
				nvram_set("wl_ssid", ssid);
			if (!strcmp(wifi, "wl0"))
				nvram_set("wl_ssid", ssid);
			nvram_nset(ssid, "%s_ssid", wifi);
			nvram_set("cur_ssid", ssid);
			nvram_commit();
		}

	}
	applytake(value);
}

void wireless_save(webs_t wp)
{
	char *value = websGetVar(wp, "action", "");

	char *next;
	char var[80];

#ifndef HAVE_MADWIFI
	int c = get_wl_instances();
	int i;

	for (i = 0; i < c; i++) {
		char buf[16];

		sprintf(buf, "wl%d", i);
		save_prefix(wp, buf);
		char *vifs = nvram_nget("wl%d_vifs", i);
#else
	int c = getdevicecount();
	int i;

	for (i = 0; i < c; i++) {
		char buf[16];

		sprintf(buf, "ath%d", i);
		save_prefix(wp, buf);
		char *vifs = nvram_nget("ath%d_vifs", i);

#endif
		if (vifs == NULL)
			return;
		foreach(var, vifs, next) {
			save_prefix(wp, var);
		}
	}
	// nvram_commit ();
	applytake(value);
#ifdef HAVE_GUESTPORT
	system("stopservice firewall");
	system("startservice firewall");
#endif
}

void hotspot_save(webs_t wp)
{
#ifdef HAVE_TIEXTRA1
	chillispot_save(wp);
#endif
#ifdef HAVE_TIEXTRA2
	wifidogs_save(wp);
#endif
	validate_cgi(wp);
}

#ifdef HAVE_WIVIZ
void set_wiviz(webs_t wp)
{

	char *hopdwell = websGetVar(wp, "hopdwell", NULL);
	char *hopseq = websGetVar(wp, "hopseq", NULL);
	FILE *fp = fopen("/tmp/wiviz2-cfg", "wb");

	if (strstr(hopseq, ","))
		fprintf(fp, "channelsel=hop&");
	else
		fprintf(fp, "channelsel=%s&", hopseq);

	fprintf(fp, "hopdwell=%s&hopseq=%s\n", hopdwell, hopseq);

	nvram_set("hopdwell", hopdwell);
	nvram_set("hopseq", hopseq);

	fclose(fp);
	killall("wiviz", SIGUSR2);

}
#endif

void ttraff_erase(webs_t wp)
{
	char line[2048];
	char *name = NULL;

	system2("nvram show | grep traff- > /tmp/.ttraff");
	FILE *fp = fopen("/tmp/.ttraff", "r");

	if (fp == NULL) {
		return;
	}
	while (fgets(line, sizeof(line), fp) != NULL) {
		if (startswith(line, "traff-")) {
			name = strtok(line, "=");
			if (strlen(name) == 13)	//only unset ttraf-XX-XXXX
			{
				nvram_unset(name);
			}
		}
	}
	nvram_commit();
	unlink("/tmp/.ttraff");
}

void changepass(webs_t wp)
{
	char *value = websGetVar(wp, "http_username", NULL);
	char *pass = websGetVar(wp, "http_passwd", NULL);

	if (value && pass && strcmp(value, TMP_PASSWD)
	    && valid_name(wp, value, NULL)) {
		nvram_set("http_username", zencrypt(value));

		system2("/sbin/setpasswd");
#ifdef HAVE_IAS
		nvram_set("http_userpln", value);
#endif
	}

	if (pass && value && strcmp(pass, TMP_PASSWD)
	    && valid_name(wp, pass, NULL)) {
		nvram_set("http_passwd", zencrypt(pass));

		system2("/sbin/setpasswd");
#ifdef HAVE_IAS
		nvram_set("http_pwdpln", pass);
#endif
	}
	nvram_commit();
}

#ifdef HAVE_CHILLILOCAL

void user_remove(webs_t wp)
{
	macro_rem("fon_usernames", "fon_userlist");
}

void user_add(webs_t wp)
{
	macro_add("fon_usernames");
	// validate_userlist(wp);
}
#endif

#ifdef HAVE_RADLOCAL

void raduser_add(webs_t wp)
{
	int radcount = 0;
	char *radc = nvram_get("iradius_count");

	if (radc != NULL)
		radcount = atoi(radc);
	radcount++;
	char count[16];

	sprintf(count, "%d", radcount);
	nvram_set("iradius_count", count);
}
#endif

#ifdef HAVE_MILKFISH
void milkfish_sip_message(webs_t wp)
{
	char *message = websGetVar(wp, "sip_message", NULL);
	char *dest = websGetVar(wp, "sip_message_dest", NULL);
	int i;
	FILE *fp = fopen("/tmp/sipmessage", "wb");

	if (fp == NULL)
		return;
	char *host_key = message;

	i = 0;
	do {
		if (host_key[i] != 0x0D)
			fprintf(fp, "%c", host_key[i]);
	}
	while (host_key[++i]);
	putc(0xa, fp);
	fclose(fp);
	eval("milkfish_services", "simpledd", dest);
	return;
}
#endif

void set_security(webs_t wp)
{
	char *var = websGetVar(wp, "security_varname", "security_mode");

	cprintf("set security to %s\n", var);
	cprintf("security var = %s\n", websGetVar(wp, var, "disabled"));
	char *var2 = websGetVar(wp, var, "disabled");

	// rep(var,'X','.');
	nvram_set(var, var2);
}

void base64_encode(const unsigned char *in, size_t inlen, unsigned char *out, size_t outlen)
{
	static const char b64str[64] = "./ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";

	while (inlen && outlen) {
		*out++ = b64str[(in[0] >> 2) & 0x3f];
		if (!--outlen)
			break;
		*out++ = b64str[((in[0] << 4) + (--inlen ? in[1] >> 4 : 0)) & 0x3f];
		if (!--outlen)
			break;
		*out++ = (inlen ? b64str[((in[1] << 2) + (--inlen ? in[2] >> 6 : 0)) & 0x3f] : '=');
		if (!--outlen)
			break;
		*out++ = inlen ? b64str[in[2] & 0x3f] : '=';
		if (!--outlen)
			break;
		if (inlen)
			inlen--;
		if (inlen)
			in += 3;
	}

	if (outlen)
		*out = '\0';
}

char *request_freedns(char *user, char *password)
{
	unsigned char final[32];
	char un[128];

	unlink("/tmp/.hash");
	sprintf(un, "%s|%s", user, password);
	sha1_ctx_t context;

	sha1_begin(&context);
	sha1_hash(un, strlen(un), &context);
	sha1_end(final, &context);
	char request[128] = {
		0
	};
	int i;

	for (i = 0; i < 20; i++)
		sprintf(request, "%s%02x", request, final[i]);
	system2("rm -f /tmp/.hash");
	sysprintf("wget \"http://freedns.afraid.org/api/?action=getdyndns&sha=%s\" -O /tmp/.hash", request);
	FILE *in = fopen("/tmp/.hash", "rb");

	if (in == NULL)
		return NULL;
	while (getc(in) != '?' && feof(in) == 0) ;
	i = 0;
	char *hash = safe_malloc(64);

	if (feof(in)) {
		free(hash);
		return NULL;
	}
	for (i = 0; i < 36; i++)
		hash[i] = getc(in);
	fclose(in);
	hash[i++] = 0;
	return hash;
}

void ddns_save_value(webs_t wp)
{
	char *enable, *username, *passwd, *hostname, *dyndnstype, *wildcard, *custom, *conf, *url, *force, *wan_ip;
	struct variable ddns_variables[] = {
		{
	      argv:ARGV("0", "1", "2", "3", "4", "5", "6", "7", "8", "9")}, {
	      argv:							   ARGV("30")},
	}, *which;
	char _username[] = "ddns_username_X";
	char _passwd[] = "ddns_passwd_X";
	char _hostname[] = "ddns_hostname_X";
	char _dyndnstype[] = "ddns_dyndnstype_X";
	char _wildcard[] = "ddns_wildcard_X";
	char _custom[] = "ddns_custom_X";
	char _conf[] = "ddns_conf";
	char _url[] = "ddns_url";
	char _force[] = "ddns_force";
	char _wan_ip[] = "ddns_wan_ip";

	which = &ddns_variables[0];

	enable = websGetVar(wp, "ddns_enable", NULL);
	if (!enable && !valid_choice(wp, enable, &which[0])) {
		return;
	}
	int gethash = 0;

	switch (atoi(enable)) {
	case 0:
		// Disable
		nvram_set("ddns_enable", enable);
		return;
		break;
	case 1:
		// dyndns
		snprintf(_username, sizeof(_username), "%s", "ddns_username");
		snprintf(_passwd, sizeof(_passwd), "%s", "ddns_passwd");
		snprintf(_hostname, sizeof(_hostname), "%s", "ddns_hostname");
		snprintf(_dyndnstype, sizeof(_dyndnstype), "%s", "ddns_dyndnstype");
		snprintf(_wildcard, sizeof(_wildcard), "%s", "ddns_wildcard");
		break;
	case 2:
		// afraid
		snprintf(_username, sizeof(_username), "ddns_username_%s", enable);
		snprintf(_passwd, sizeof(_passwd), "ddns_passwd_%s", enable);
		snprintf(_hostname, sizeof(_hostname), "ddns_hostname_%s", enable);
		gethash = 1;
		break;
	case 3:		// zoneedit
	case 4:		// no-ip
	case 8:		// tzo
	case 9:		// dynSIP
		snprintf(_username, sizeof(_username), "ddns_username_%s", enable);
		snprintf(_passwd, sizeof(_passwd), "ddns_passwd_%s", enable);
		snprintf(_hostname, sizeof(_hostname), "ddns_hostname_%s", enable);
		break;
	case 5:
		// custom
		snprintf(_username, sizeof(_username), "ddns_username_%s", enable);
		snprintf(_passwd, sizeof(_passwd), "ddns_passwd_%s", enable);
		snprintf(_hostname, sizeof(_hostname), "ddns_hostname_%s", enable);
		snprintf(_custom, sizeof(_custom), "ddns_custom_%s", enable);
		snprintf(_conf, sizeof(_conf), "%s", "ddns_conf");
		snprintf(_url, sizeof(_url), "%s", "ddns_url");
		break;
	case 6:
		// 3322 dynamic : added botho 30/07/06
		snprintf(_username, sizeof(_username), "ddns_username_%s", enable);
		snprintf(_passwd, sizeof(_passwd), "ddns_passwd_%s", enable);
		snprintf(_hostname, sizeof(_hostname), "ddns_hostname_%s", enable);
		snprintf(_dyndnstype, sizeof(_dyndnstype), "ddns_dyndnstype_%s", enable);
		snprintf(_wildcard, sizeof(_wildcard), "ddns_wildcard_%s", enable);
		break;
	case 7:
		// easydns
		snprintf(_username, sizeof(_username), "ddns_username_%s", enable);
		snprintf(_passwd, sizeof(_passwd), "ddns_passwd_%s", enable);
		snprintf(_hostname, sizeof(_hostname), "ddns_hostname_%s", enable);
		snprintf(_wildcard, sizeof(_wildcard), "ddns_wildcard_%s", enable);
		break;
	}

	username = websGetVar(wp, _username, NULL);
	passwd = websGetVar(wp, _passwd, NULL);
	hostname = websGetVar(wp, _hostname, NULL);
	dyndnstype = websGetVar(wp, _dyndnstype, NULL);
	wildcard = websGetVar(wp, _wildcard, NULL);
	custom = websGetVar(wp, _custom, NULL);
	conf = websGetVar(wp, _conf, NULL);
	url = websGetVar(wp, _url, NULL);
	force = websGetVar(wp, _force, NULL);
	wan_ip = websGetVar(wp, _wan_ip, NULL);

	if (!username || !passwd || !hostname || !force || !wan_ip) {
		return;
	}

	if (atoi(force) < 1 || atoi(force) > 60) {
		force = "10";
	}

	nvram_set("ddns_enable", enable);
	nvram_set(_username, username);
	if (strcmp(passwd, TMP_PASSWD)) {
		nvram_set(_passwd, passwd);
	}
	if (gethash && !contains(hostname, ',')) {
		char hostn[128];
		char *hash = request_freedns(username, nvram_safe_get(_passwd));

		if (hash) {
			sprintf(hostn, "%s,%s", hostname, hash);
			nvram_set(_hostname, hostn);
			free(hash);
		} else {
			nvram_set(_hostname, "User/Password wrong");
		}
	} else
		nvram_set(_hostname, hostname);
	nvram_set(_dyndnstype, dyndnstype);
	nvram_set(_wildcard, wildcard);
	nvram_set(_custom, custom);
	nvram_set(_conf, conf);
	nvram_set(_url, url);
	nvram_set(_force, force);
	nvram_set(_wan_ip, wan_ip);

}

void ddns_update_value(webs_t wp)
{

}

void port_vlan_table_save(webs_t wp)
{
	int port = 0, vlan = 0, vlans[22], i;
	char portid[32], portvlan[64], *portval, buff[32], *c, *next, br0vlans[64], br1vlans[64], br2vlans[64];

	strcpy(portvlan, "");

	for (vlan = 0; vlan < 22; vlan++)
		vlans[vlan] = 0;

	vlans[16] = 1;

	for (port = 0; port < 5; port++) {
		for (vlan = 0; vlan < 22; vlan++) {
			snprintf(portid, 31, "port%dvlan%d", port, vlan);
			portval = websGetVar(wp, portid, "");

			if (vlan < 17 || vlan > 21)
				i = (strcmp(portval, "on") == 0);
			else
				i = (strcmp(portval, "on") != 0);

			if (i) {
				if (strlen(portvlan) > 0)
					strcat(portvlan, " ");

				snprintf(buff, 4, "%d", vlan);
				strcat(portvlan, buff);
				vlans[vlan] = 1;
			}
		}

		snprintf(portid, 31, "port%dvlans", port);
		nvram_set(portid, portvlan);
		strcpy(portvlan, "");
	}

	/*
	 * done with ports 0-4, now set up #5 automaticly 
	 */
	/*
	 * if a VLAN is used, it also gets assigned to port #5 
	 */
	for (vlan = 0; vlan < 17; vlan++) {
		if (vlans[vlan]) {
			if (strlen(portvlan) > 0)
				strcat(portvlan, " ");

			snprintf(buff, 4, "%d", vlan);
			strcat(portvlan, buff);
		}
	}

	nvram_set("port5vlans", portvlan);

	strcpy(br0vlans, "");
	c = nvram_safe_get("lan_ifnames");
	if (c) {
		foreach(portid, c, next) {
			if (!(strncmp(portid, "vlan", 4) == 0)
			    && !(strncmp(portid, "eth1", 4) == 0)) {
				if (strlen(br0vlans) > 0)
					strcat(br0vlans, " ");
				strcat(br0vlans, portid);
			}
		}
	}

	strcpy(br1vlans, "");
	c = nvram_safe_get("ub1_ifnames");
	if (c) {
		foreach(portid, c, next) {
			if (!(strncmp(portid, "vlan", 4) == 0)
			    && !(strncmp(portid, "eth1", 4) == 0)) {
				if (strlen(br1vlans) > 0)
					strcat(br1vlans, " ");
				strcat(br1vlans, portid);
			}
		}
	}

	strcpy(br2vlans, "");
	c = nvram_safe_get("ub2_ifnames");
	if (c) {
		foreach(portid, c, next) {
			if (!(strncmp(portid, "vlan", 4) == 0)
			    && !(strncmp(portid, "eth1", 4) == 0)) {
				if (strlen(br2vlans) > 0)
					strcat(br2vlans, " ");
				strcat(br2vlans, portid);
			}
		}
	}

	for (i = 0; i < 16; i++) {
		snprintf(buff, 31, "vlan%d", i);
		portval = websGetVar(wp, buff, "");

		switch (atoi(portval)) {
		case 0:
			if (strlen(br0vlans) > 0)
				strcat(br0vlans, " ");
			strcat(br0vlans, buff);
			break;
		case 1:
			if (strlen(br1vlans) > 0)
				strcat(br1vlans, " ");
			strcat(br1vlans, buff);
			break;
		case 2:
			if (strlen(br2vlans) > 0)
				strcat(br2vlans, " ");
			strcat(br2vlans, buff);
			break;
		}
	}

	strcpy(buff, "");

	switch (atoi(websGetVar(wp, "wireless", ""))) {
	case 0:
		if (strlen(br0vlans) > 0)
			strcat(br0vlans, " ");
		strcat(br0vlans, get_wdev());
		break;
	case 1:
		if (strlen(br1vlans) > 0)
			strcat(br1vlans, " ");
		strcat(br1vlans, get_wdev());
		break;
	case 2:
		if (strlen(br2vlans) > 0)
			strcat(br2vlans, " ");
		strcat(br2vlans, get_wdev());
		break;
	}

	snprintf(buff, 3, "%s", websGetVar(wp, "trunking", ""));

	nvram_set("lan_ifnames", br0vlans);
	// nvram_set("ub1_ifnames", br1vlans);
	// nvram_set("ub2_ifnames", br2vlans);
	nvram_set("trunking", buff);
	nvram_set("vlans", "1");

	nvram_commit();

}

static void save_macmode_if(webs_t wp, char *ifname)
{

	char macmode[32];
	char macmode1[32];

	sprintf(macmode, "%s_macmode", ifname);
	sprintf(macmode1, "%s_macmode1", ifname);
	rep(macmode1, '.', 'X');
	char *wl_macmode1, *wl_macmode;

	wl_macmode = websGetVar(wp, macmode, NULL);
	wl_macmode1 = websGetVar(wp, macmode1, NULL);

	if (!wl_macmode1)
		return;

	if (!strcmp(wl_macmode1, "disabled")) {
		nvram_set(macmode1, "disabled");
		nvram_set(macmode, "disabled");
	} else if (!strcmp(wl_macmode1, "other")) {
		if (!wl_macmode)
			nvram_set(macmode, "deny");
		else
			nvram_set(macmode, wl_macmode);
		nvram_set(macmode1, "other");
	}
}

void save_macmode(webs_t wp)
{
#ifndef HAVE_MADWIFI
	int c = get_wl_instances();
	char devs[32];
	int i;

	for (i = 0; i < c; i++) {
		sprintf(devs, "wl%d", i);
		save_macmode_if(wp, devs);
	}
#else
	int c = getdevicecount();
	char devs[32];
	int i;

	for (i = 0; i < c; i++) {
		sprintf(devs, "ath%d", i);
		save_macmode_if(wp, devs);
		char vif[32];

		sprintf(vif, "%s_vifs", devs);
		char var[80], *next;
		char *vifs = nvram_safe_get(vif);

		if (vifs != NULL)
			foreach(var, vifs, next) {
			save_macmode_if(wp, var);
			}
	}

#endif
	return;

}

// handle UPnP.asp requests / added 10
void tf_upnp(webs_t wp)
{
	char *v;
	char s[64];

	if (((v = websGetVar(wp, "remove", NULL)) != NULL) && (*v)) {
		if (strcmp(v, "all") == 0) {
			nvram_set("upnp_clear", "1");
		} else {
			int which = atoi(nvram_default_get("forward_cur", "0"));
			int i = atoi(v);
			char val[32];

			sprintf(val, "forward_port%d", i);
			int a;

			nvram_unset(val);
			for (a = i + 1; a < which; a++) {
				nvram_nset(nvram_nget("forward_port%d", a), "forward_port%d", a - 1);
			}
			which--;
			sprintf(val, "forward_port%d", which);
			nvram_unset(val);
			if (which < 0)
				which = 0;
			sprintf(val, "%d", which);
			nvram_set("forward_cur", val);
		}
		eval("stopservice", "firewall");
		eval("startservice", "firewall");	//restart firewall
	}

}

#ifdef HAVE_MINIDLNA
#include <dlna.h>
static void dlna_save(webs_t wp)
{
	int c, j;
	char var[128], val[128];
	json_t *entry = NULL;

	// dlna shares
	json_t *entries = json_array();
	int share_number = atoi(websGetVar(wp, "dlna_shares_count", "0"));
	for (c = 1; c <= share_number; c++) {
		entry = json_object();
		sprintf(var, "dlnashare_mp_%d", c);
		json_object_set_new(entry, "mp", json_string(websGetVar(wp, var, "")));
		int type = 0;
		sprintf(var, "dlnashare_audio_%d", c);
		if (atoi(websGetVar(wp, var, "0")))
			type |= TYPE_AUDIO;
		sprintf(var, "dlnashare_video_%d", c);
		if (atoi(websGetVar(wp, var, "0")))
			type |= TYPE_VIDEO;
		sprintf(var, "dlnashare_images_%d", c);
		if (atoi(websGetVar(wp, var, "0")))
			type |= TYPE_IMAGES;
		json_object_set_new(entry, "types", json_integer(type));
		json_array_append(entries, entry);
	}
	nvram_set("dlna_shares", json_dumps(entries, JSON_COMPACT));
	json_array_clear(entries);
}
#endif

#ifdef HAVE_NAS_SERVER
#include <samba3.h>
void nassrv_save(webs_t wp)
{
#ifdef HAVE_SAMBA_SERVER
	int c, j;
	char var[128], val[128];
	json_t *entry = NULL, *user_entries;

	// samba shares
	json_t *entries = json_array();
	int share_number = atoi(websGetVar(wp, "samba_shares_count", "0"));
	int user_number = atoi(websGetVar(wp, "samba_users_count", "0"));
	for (c = 1; c <= share_number; c++) {
		entry = json_object();
		sprintf(var, "smbshare_mp_%d", c);
		json_object_set_new(entry, "mp", json_string(websGetVar(wp, var, "")));
		sprintf(var, "smbshare_subdir_%d", c);
		json_object_set_new(entry, "sd", json_string(websGetVar(wp, var, "")));
		sprintf(var, "smbshare_label_%d", c);
		json_object_set_new(entry, "label", json_string(websGetVar(wp, var, "")));
		sprintf(var, "smbshare_public_%d", c);
		json_object_set_new(entry, "public", json_integer(atoi(websGetVar(wp, var, "0"))));
		sprintf(var, "smbshare_access_perms_%d", c);
		sprintf(val, "%s", websGetVar(wp, var, "-"));
		if (!strcmp(val, "-")) {
			sprintf(var, "smbshare_access_perms_prev_%d", c);
			sprintf(val, "%s", websGetVar(wp, var, "x"));
		}
		json_object_set_new(entry, "perms", json_string(val));
		user_entries = json_array();
		for (j = 1; j <= user_number; j++) {
			sprintf(var, "smbshare_%d_user_%d", c, j);
			if (!strcmp(websGetVar(wp, var, ""), "1")) {
				sprintf(var, "smbuser_username_%d", j);
				json_array_append(user_entries, json_string(websGetVar(wp, var, "")));
			}
		}
		json_object_set_new(entry, "users", user_entries);
		json_array_append(entries, entry);
	}
	//fprintf(stderr, "[SAVE NAS] %s\n", json_dumps( entries, JSON_COMPACT ) );
	nvram_set("samba3_shares", json_dumps(entries, JSON_COMPACT));
	json_array_clear(entries);

	entries = json_array();
	for (c = 1; c <= user_number; c++) {
		entry = json_object();
		sprintf(var, "smbuser_username_%d", c);
		json_object_set_new(entry, "user", json_string(websGetVar(wp, var, "")));
		sprintf(var, "smbuser_password_%d", c);
		json_object_set_new(entry, "pass", json_string(websGetVar(wp, var, "")));
		int type = 0;
		sprintf(var, "smbuser_samba_%d", c);
		if (atoi(websGetVar(wp, var, "0")))
			type |= SHARETYPE_SAMBA;
		sprintf(var, "smbuser_ftp_%d", c);
		if (atoi(websGetVar(wp, var, "0")))
			type |= SHARETYPE_FTP;
		json_object_set_new(entry, "type", json_integer(type));
		json_array_append(entries, entry);
	}
	//fprintf(stderr, "[SAVE NAS USERS] %s\n", json_dumps( entries, JSON_COMPACT ) );
	nvram_set("samba3_users", json_dumps(entries, JSON_COMPACT));
	json_array_clear(entries);
#endif
	char *value = websGetVar(wp, "action", "");

	// all other vars
	validate_cgi(wp);

	addAction("nassrv");
	nvram_set("nowebaction", "1");
#ifdef HAVE_MINIDLNA
	dlna_save(wp);
#endif

	applytake(value);
}
#endif

#ifdef HAVE_SPOTPASS
void nintendo_save(webs_t wp)
{

	char prefix[16] = "ath0";
	char var[32], param[32];
	int device = 0;

	int enabled = atoi(nvram_default_get("spotpass", "0"));

	device = prefix[strlen(prefix) - 1] - '0';

	// handle server list
	int count = 0;
	char *buffer = (char *)safe_malloc(strlen(websGetVar(wp, "spotpass_servers", "")) + 1);
	strcpy(buffer, websGetVar(wp, "spotpass_servers", ""));

	char *ptr = strtok(buffer, "\n");
	while (ptr != NULL) {
		count++;
		ptr = strtok(NULL, "\n");
	}
	char *serverlist = (char *)safe_malloc(strlen(websGetVar(wp, "spotpass_servers", "")) + (count * 2) + 1);
	char line[256], url[128], proto[8], mode[16], ports[64];
	int port1, port2, lines = 0;

	strcpy(buffer, websGetVar(wp, "spotpass_servers", ""));
	strcpy(serverlist, "\0");
	fprintf(stderr, "%s\n", buffer);
	ptr = strtok(buffer, "\n");
	while (ptr != NULL) {
		strcpy(line, "\0");
		if (sscanf(ptr, "%s %s %s %d %d", &url, &proto, &mode, &port1, &port2) == 5) {
			sprintf(line, "%s %s %d,%d", url, proto, port1, port2);
		} else if (sscanf(ptr, "%s %s %d %d", &url, &proto, &port1, &port2) == 4) {
			sprintf(line, "%s %s %d,%d", url, proto, port1, port2);
		} else if (sscanf(ptr, "%s %s %s", &url, &proto, &ports) == 3) {
			sprintf(line, "%s %s %s", url, proto, ports);
		}
		lines++;
		if (strlen(line) > 0) {
			strcat(serverlist, line);
			if (lines < count) {
				strcat(serverlist, "|");
			}
		}
		ptr = strtok(NULL, "\n");
	}
	nvram_set("spotpass_servers", serverlist);

	if (enabled == 0 && !strcmp(websGetVar(wp, "spotpass", "0"), "1")) {

		// check if vap is set
		if (!strcmp(nvram_default_get("spotpass_vif", ""), "")) {

			int count = get_vifcount(prefix) + 1;
			add_vifs_single(prefix, device);
			sprintf(var, "%s.%d", prefix, count);
			nvram_set("spotpass_vif", var);

			// set parameters for vap
			sprintf(param, "%s_ssid", var);
			nvram_set(param, "NintendoSpotPass1");
			sprintf(param, "%s_bridged", var);
			nvram_set(param, "0");
			sprintf(param, "%s_ipaddr", var);
			nvram_set(param, "192.168.12.1");
			sprintf(param, "%s_netmask", var);
			nvram_set(param, "255.255.255.0");
			sprintf(param, "%s_macmode", var);
			nvram_set(param, "allow");
			rep(param, '.', 'X');
			nvram_set(param, "allow");
			sprintf(param, "%s_macmode1", var);
			rep(param, '.', 'X');
			nvram_set(param, "other");
			sprintf(param, "%s_maclist", var);
			nvram_set(param, "A4:C0:E1:00:00:00/24");

			// dhcpd
			sprintf(param, "%s>On>20>200>60", var);
			nvram_set("mdhcpd", param);
			nvram_set("mdhcpd_count", "1");
		}

	} else if (enabled == 1 && !strcmp(websGetVar(wp, "spotpass", "0"), "0")) {

		if (strcmp(nvram_default_get("spotpass_vif", ""), "")) {
			sprintf(var, "%s.%%d", prefix);
			int index = 0;
			if (sscanf(nvram_get("spotpass_vif"), var, &index) == 1) {
				sprintf(var, "%s", nvram_get("spotpass_vif"));
				int count = get_vifcount(prefix);
				int index = var[strlen(var) - 1] - '0';
				while (get_vifcount(prefix) >= index) {
					remove_vifs_single(prefix);
				}
				nvram_set("spotpass_vif", "");

				nvram_set("mdhcpd", "");
				nvram_set("mdhcpd_count", "0");
			}
		}
	}

	if (atoi(websGetVar(wp, "spotpass", "")) != enabled) {
		addAction("wireless");
		nvram_set("nowebaction", "1");
	}

	nvram_set("spotpass", websGetVar(wp, "spotpass", "0"));

	char *value = websGetVar(wp, "action", "");

	//addAction("spotpass_start");
	applytake(value);
}
#endif
