/*

	Tomato Firmware
	Copyright (C) 2006 Jonathan Zarate

*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <stdarg.h>
#include <syslog.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <bcmnvram.h>
#include "shutils.h"
#include "shared.h"
#include "utils.h"


int get_wan_proto(void)
{
	const char *names[] = {	// order must be synced with def at shared.h
		"static",
		"dhcp",
		"heartbeat",
		"l2tp",
		"pppoe",
		"pptp"
	};
	int i;
	const char *p;
	
	p = nvram_safe_get("wan_proto");
	for (i = 0; i < 6; ++i) {
		if (strcmp(p, names[i]) == 0) return i + 1;
	}
	return WP_DISABLED;
}

int using_dhcpc(void)
{
	const char *proto = nvram_safe_get("wan_proto");
	return (strcmp(proto, "dhcp") == 0) || (strcmp(proto, "l2tp") == 0) || (strcmp(proto, "heartbeat") == 0);
}

void notice_set(const char *path, const char *format, ...)
{
	char p[256];
	char buf[2048];
	va_list args;

	mkdir("/var/notice", 0755);
	snprintf(p, sizeof(p), "/var/notice/%s", path);
	
	va_start(args, format);
	vsnprintf(buf, sizeof(buf), format, args);
	va_end(args);
	
	f_write_string(p, buf, 0, 0);
	if (buf[0]) syslog(LOG_INFO, "notice: %s", buf);
}


int check_wanup(void)
{
	int up = 0;
	int proto;
	char buf1[64];
	char buf2[64];
	const char *name;
    int f;
    struct ifreq ifr;
	
	proto = get_wan_proto();
	if (proto == WP_DISABLED) return 0;
	
	if ((proto == WP_PPTP) || (proto == WP_L2TP) || (proto == WP_PPPOE) || (proto == WP_HEARTBEAT)) {
		if (f_read_string("/tmp/ppp/link", buf1, sizeof(buf1)) > 0) {
			if (proto == WP_HEARTBEAT) {
				// contains pid of bpalogin
				if (strcmp(psname(atoi(buf1), buf2, sizeof(buf2)), "bpalogin") == 0) {
					up = 1;
				}
			}
			else {
				// contains the base name of a file in /var/run/ containing pid of a daemon
				snprintf(buf2, sizeof(buf2), "/var/run/%s.pid", buf1);
				if (f_read_string(buf2, buf1, sizeof(buf1)) > 0) {
					name = psname(atoi(buf1), buf2, sizeof(buf2));
					if (proto == WP_PPPOE) {
						if (strcmp(name, "pppoecd") == 0) up = 1;
					}
					else {
						if (strcmp(name, "pppd") == 0) up = 1;
					}						
				}
			}
			if (!up) {
				unlink("/tmp/ppp/link");
				cprintf("required daemon not found, assuming link is dead\n");
			}
		}
	}
	else if (!nvram_match("wan_ipaddr", "0.0.0.0")) {
		up = 1;
	}

	if ((up) && ((f = socket(AF_INET, SOCK_DGRAM, 0)) >= 0)) {
		strlcpy(ifr.ifr_name, nvram_safe_get("wan_iface"), sizeof(ifr.ifr_name));
		if (ioctl(f, SIOCGIFFLAGS, &ifr) < 0) up = 0;
		close(f);
		if ((ifr.ifr_flags & IFF_UP) == 0) up = 0;
	}

	return up;
}

const dns_list_t *get_dns(void)
{
	static dns_list_t dns;
	char s[512];
	int n;
	int i, j;
	struct in_addr ia;
	char d[4][16];

	dns.count = 0;	
	
	strlcpy(s, nvram_safe_get("wan_dns"), sizeof(s));
	if ((nvram_match("dns_addget", "1")) || (s[0] == 0)) {
		n = strlen(s);
		snprintf(s + n, sizeof(s) - n, " %s", nvram_safe_get("wan_get_dns"));
	}

	n = sscanf(s, "%15s %15s %15s %15s", d[0], d[1], d[2], d[3]);
	for (i = 0; i < n; ++i) {
		if (inet_pton(AF_INET, d[i], &ia) > 0) {
			for (j = dns.count - 1; j >= 0; --j) {
				if (dns.dns[j].s_addr == ia.s_addr) break;
			}
			if (j < 0) {
				dns.dns[dns.count++].s_addr = ia.s_addr;
				if (dns.count == 3) break;
			}
		}
	}

	return &dns;
}

// -----------------------------------------------------------------------------

void set_action(int a)
{
	int r = 3;
	while (f_write("/var/lock/action", &a, sizeof(a), 0, 0) != sizeof(a)) {
		sleep(1);
		if (--r == 0) return;
	}
	if (a != ACT_IDLE) sleep(2);
}

int check_action(void)
{
	int a;
	int r = 3;
	
	while (f_read("/var/lock/action", &a, sizeof(a)) != sizeof(a)) {
		sleep(1);
		if (--r == 0) return ACT_UNKNOWN;
	}
	return a;
}

int wait_action_idle(int n)
{
return 1;
/*
	while (n-- > 0) {
		if (check_action() == ACT_IDLE) return 1;
		sleep(1);
	}
	return 0;*/
}

// -----------------------------------------------------------------------------

int wl_client(void)
{
	return ((nvram_match("wl_mode", "sta")) || (nvram_match("wl_mode", "wet")));
}
