
/*
 * network.c
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <syslog.h>
#include <ctype.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
//#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <sys/sysinfo.h>

#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#ifdef __UCLIBC__
#include <error.h>
#endif
#include <time.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/mount.h>
#include <sys/stat.h>
#include <sys/reboot.h>
#include <sys/sysinfo.h>

#include <string.h>
#include <linux/version.h>

#include <linux/sockios.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <code_pattern.h>
#include <utils.h>
#include <wlutils.h>
#include <rc.h>
#include <cy_conf.h>
#include <utils.h>
#include <nvparse.h>
#include <etsockio.h>
#include <bcmparams.h>
#include <services.h>
#include <libbridge.h>

#include "../sysinit/devices/ethtools.c"

#ifdef HAVE_IPV6
#define evalip6(cmd, args...)                          \
	{                                              \
		if (nvram_match("ipv6_enable", "1")) { \
			eval_va(cmd, ##args, NULL);    \
		}                                      \
	}
#else
#define evalip6(...)
#endif

extern void genHosts(void);
extern int br_add_bridge(const char *brname);
extern int br_del_bridge(const char *brname);
extern int br_add_interface(const char *br, const char *dev);
extern int br_del_interface(const char *br, const char *dev);
extern int br_set_stp_state(const char *br, int stp_state);
void start_set_routes(void);
void config_macs(char *wlifname);
static void stop_ipv6_tunnel(char *wan_ifname);

#define PTABLE_MAGIC 0xbadc0ded
#define PTABLE_SLT1 1
#define PTABLE_SLT2 2
#define PTABLE_ACKW 3
#define PTABLE_ADHM 4
#define PTABLE_END 0xffffffff

/*
 * phy types 
 */
#define PHY_TYPE_A 0
#define PHY_TYPE_B 1
#define PHY_TYPE_G 2
#define PHY_TYPE_NULL 0xf

#define WL_IOCTL(name, cmd, buf, len) (wl_ioctl((name), (cmd), (buf), (len)))

#define TXPWR_MAX 1000
#define TXPWR_DEFAULT 70

#define IFUP (IFF_UP | IFF_RUNNING | IFF_BROADCAST | IFF_MULTICAST)
#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)
/*
 * configure loopback interface 
 */
void config_loopback(void)
{
	/*
	 * Bring up loopback interface 
	 */
	ifconfig("lo", IFUP, "127.0.0.1", "255.0.0.0");

	/*
	 * Add to routing table 
	 */
	route_add("lo", 0, "127.0.0.0", "0.0.0.0", "255.0.0.0");
}

char *getMacAddr(char *ifname, char *mac, size_t len)
{
	unsigned char hwbuff[16];
	int i = wl_hwaddr(ifname, hwbuff);

	if (i < 0)
		return NULL;
	snprintf(mac, len, "%02X:%02X:%02X:%02X:%02X:%02X", hwbuff[0], hwbuff[1], hwbuff[2], hwbuff[3], hwbuff[4], hwbuff[5]);
	return mac;
}

static unsigned long ptable[128];
static unsigned long kmem_offset;
static inline void wlc_get_mem_offset(void)
{
	FILE *f;
	char s[64];

	/*
	 * yes, i'm lazy ;) 
	 */
	f = popen("grep '\\[wl]' /proc/ksyms | sort", "r");
	if (fgets(s, 64, f) == 0) {
		return;
	}
	pclose(f);

	s[8] = 0;
	kmem_offset = strtoul(s, NULL, 16);

	/*
	 * sanity check 
	 */
	if (kmem_offset < 0xc0000000)
		kmem_offset = 0;
	return;
}

static int ptable_init(void)
{
	struct stat statbuf;
	int fd;

	if (ptable[0] == PTABLE_MAGIC)
		return 0;

	if ((fd = open("/etc/patchtable.bin", O_RDONLY)) < 0)
		return -1;

	if (fstat(fd, &statbuf) < 0)
		goto failed;

	if (statbuf.st_size < 512)
		goto failed;

	// if (lseek(fd, statbuf.st_size - 512, SEEK_SET) < 0) {
	// perror("lseek");
	// goto failed;
	// }

	if (read(fd, ptable, 512) < 512)
		goto failed;

	if (ptable[0] != PTABLE_MAGIC)
		goto failed;

	close(fd);

	wlc_get_mem_offset();
	if (kmem_offset == 0)
		return -1;

	return 0;

failed:
	close(fd);

	return -1;
}

static inline unsigned long wlc_kmem_read(unsigned long offset)
{
	int fd;
	unsigned long ret;

	if ((fd = open("/dev/kmem", O_RDONLY)) < 0)
		return -1;

	lseek(fd, 0x70000000, SEEK_SET);
	lseek(fd, (kmem_offset - 0x70000000) + offset, SEEK_CUR);
	read(fd, &ret, 4);
	close(fd);

	return ret;
}

static inline void wlc_kmem_write(unsigned long offset, unsigned long value)
{
	int fd;

	if ((fd = open("/dev/kmem", O_WRONLY)) < 0)
		return;

	lseek(fd, 0x70000000, SEEK_SET);
	lseek(fd, (kmem_offset - 0x70000000) + offset, SEEK_CUR);
	write(fd, &value, 4);
	close(fd);
}

static int wlc_patcher_getval(unsigned long key, unsigned long *val)
{
	unsigned long *pt = &ptable[1];
	unsigned long tmp;

	if (ptable_init() < 0) {
		fprintf(stderr, "Could not load the ptable\n");
		return -1;
	}

	while (*pt != PTABLE_END) {
		if (*pt == key) {
			tmp = wlc_kmem_read(pt[1]);

			if (tmp == pt[2])
				*val = 0xffffffff;
			else
				*val = tmp;

			return 0;
		}
		pt += 3;
	}

	return -1;
}

static int wlc_patcher_setval(unsigned long key, unsigned long val)
{
	unsigned long *pt = &ptable[1];

	if (ptable_init() < 0) {
		fprintf(stderr, "Could not load the ptable\n");
		return -1;
	}

	if (val != 0xffffffff)
		val = (pt[2] & ~(0xffff)) | (val & 0xffff);

	while (*pt != PTABLE_END) {
		if (*pt == key) {
			if (val == 0xffffffff) /* default */
				val = pt[2];

			wlc_kmem_write(pt[1], val);
		}
		pt += 3;
	}

	return 0;
}

/*
 * static int get_wlc_slottime(wlc_param param, void *data, void *value) {
 * int *val = (int *) value; int ret = 0;
 * 
 * ret = wlc_patcher_getval(PTABLE_SLT1, (unsigned long *) val); if (*val !=
 * 0xffffffff) *val &= 0xffff; } return ret; } 
 */
static int set_wlc_slottime(int value)
{
	int ret = 0;

	wlc_patcher_setval(PTABLE_SLT1, value);
	wlc_patcher_setval(PTABLE_SLT2, ((value == -1) ? value : value + 510));
	return ret;
}

static int wlc_noack(int value)
{
	int ret = 0;

	// if ((param & PARAM_MODE) == SET) {
	wlc_patcher_setval(PTABLE_ACKW, (value ? 1 : 0));
	// } else if ((param & PARAM_MODE) == GET) {
	// ret = wlc_patcher_getval(PTABLE_ACKW, (unsigned long *) val);
	// *val &= 0xffff;
	// *val = (*val ? 1 : 0);
	// }

	return ret;
}

#ifndef HAVE_MADWIFI
#ifndef HAVE_RT2880
#ifndef HAVE_RT61
static int notify_nas(char *type, char *ifname, char *action);
#endif
#endif
#endif

void run_dhcpc(char *wan_ifname, char *pidfile, char *script, int fork, int leasetime, int nodeconfig)
{
	char temp[12];

	pid_t pid;

	char *wan_hostname = nvram_safe_get("wan_hostname");
#ifdef HAVE_FREECWMP
	char *vendorclass = "dslforum.org";
#else
	char *vendorclass = nvram_safe_get("dhcpc_vendorclass");
#endif
	char *userclass = nvram_safe_get("dhcp_userclass");
	char *auth = nvram_safe_get("dhcp_authentication");
	char *clientid = nvram_safe_get("dhcp_clientid");
	char *requestip = nvram_safe_get("dhcpc_requestip");
	int use_extra = 0;

	symlink("/sbin/rc", "/tmp/dhcpc");
	if (!script)
		script = "/tmp/dhcpc";
	if (!pidfile) {
		pidfile = "/var/run/udhcpc.pid";
		use_extra = 1;
	}
	char *flags = NULL;
	if (!fork)
		flags = "-q";
	nvram_set("wan_get_dns", "");
	nvram_set("wan_gateway", "");
	nvram_set("wan_ipaddr", "");
	nvram_set("wan_netmask", "");
	nvram_set("wan_get_domain", "");
	stop_process("udhcpc", "DHCP client");

	char *dhcp_argv[] = {
		"udhcpc", "-i", wan_ifname, "-p", pidfile, "-s", script, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL,	  NULL, NULL,	    NULL, NULL,	   NULL, NULL,	 NULL, NULL, NULL, NULL, NULL, NULL, NULL,
		NULL,	  NULL, NULL,	    NULL, NULL,	   NULL, NULL,	 NULL, NULL, NULL, NULL, NULL, NULL,
	};

	int i = 7;

#ifdef HAVE_BUSYBOX_UDHCPC
	dhcp_argv[i++] = "-O";
	dhcp_argv[i++] = "routes";
	if (nvram_default_matchi("dhcpc_121", 1, 1)) {
		dhcp_argv[i++] = "-O";
		dhcp_argv[i++] = "msstaticroutes";
		dhcp_argv[i++] = "-O";
		dhcp_argv[i++] = "staticroutes";
	}
#ifdef HAVE_FREECWMP
	dhcp_argv[i++] = "-O";
	dhcp_argv[i++] = "vendorspecific";
#endif
#endif
	char *s_auth = NULL;
	char *s_clientid = NULL;
	char *s_userclass = NULL;

	if (nvram_match("wan_proto", "dhcp_auth")) {
		if (*auth) {
			dhcp_argv[i++] = "-x"; // authentication
			asprintf(&s_auth, "0x5a:%s", auth);
			dhcp_argv[i++] = s_auth;
		}
		if (*clientid) {
			dhcp_argv[i++] = "-x"; // client id
			asprintf(&s_clientid, "0x3d:%s", clientid);
			dhcp_argv[i++] = s_clientid;
		}
		if (*vendorclass) {
			dhcp_argv[i++] = "-V"; // vendor class
			dhcp_argv[i++] = vendorclass;
		}
		if (*userclass) {
			dhcp_argv[i++] = "-x"; // user class

			int c;
			size_t slen = 6 + (strlen(userclass) * 2);
			s_userclass = malloc(slen); // 5 bytes trailer, 2*string lenght for hex values + 1 zero termination
			snprintf(s_userclass, slen, "0x4d:");
			for (c = 0; c < strlen(userclass); c++) {
				snprintf(s_userclass, slen, "%s%02X", s_userclass, userclass[c]);
			}
			dhcp_argv[i++] = s_userclass;
		}
	}
	if (flags)
		dhcp_argv[i++] = flags;

	if (leasetime) {
		dhcp_argv[i++] = "-t";
		snprintf(temp, sizeof(temp), "%d", leasetime);
		dhcp_argv[i++] = temp;
	}
	if (nodeconfig)
		dhcp_argv[i++] = "-T";

	if (use_extra) {
		if (!nvram_match("wan_proto", "dhcp_auth")) {
			if (*vendorclass) {
				dhcp_argv[i++] = "-V";
				dhcp_argv[i++] = vendorclass;
			}
		}

		if (*requestip) {
			dhcp_argv[i++] = "-r";
			dhcp_argv[i++] = requestip;
		}
		char hostname[128];
		snprintf(hostname, sizeof(hostname), "hostname:%s", wan_hostname);
		if (*wan_hostname) {
			dhcp_argv[i++] = "-x";
			dhcp_argv[i++] = hostname;
		}
	}

#ifndef HAVE_MICRO
	if (nvram_match("wan_priority", "1") && isvlan(wan_ifname)) {
		eval("vconfig", "set_egress_map", wan_ifname, "0", "6");
		eval("vconfig", "set_egress_map", wan_ifname, "1", "0");
		insmod("nf_defrag_ipv6 nf_log_ipv6 ip6_tables nf_conntrack_ipv6 ip6table_filter ip6table_mangle xt_DSCP xt_CLASSIFY");
		eval("iptables", "-t", "mangle", "-D", "PREROUTING", "-i", wan_ifname, "-j", "MARK", "--set-mark", "0x100000");
		eval("iptables", "-t", "mangle", "-A", "PREROUTING", "-i", wan_ifname, "-j", "MARK", "--set-mark", "0x100000");

		eval("iptables", "-t", "mangle", "-D", "POSTROUTING", "-o", wan_ifname, "-j", "MARK", "--set-mark", "0x100000");
		eval("iptables", "-t", "mangle", "-A", "POSTROUTING", "-o", wan_ifname, "-j", "MARK", "--set-mark", "0x100000");

		eval("iptables", "-t", "mangle", "-D", "POSTROUTING", "-m", "--mark", "0x100000", "-j", "CLASSIFY", "--set-class",
		     "0:1");
		eval("iptables", "-t", "mangle", "-A", "POSTROUTING", "-m", "--mark", "0x100000", "-j", "CLASSIFY", "--set-class",
		     "0:1");

		evalip6("ip6tables", "-t", "mangle", "-D", "PREROUTING", "-i", wan_ifname, "-j", "MARK", "--set-mark", "0x100000");
		evalip6("ip6tables", "-t", "mangle", "-A", "PREROUTING", "-i", wan_ifname, "-j", "MARK", "--set-mark", "0x100000");
		evalip6("ip6tables", "-t", "mangle", "-D", "POSTROUTING", "-o", wan_ifname, "-j", "MARK", "--set-mark", "0x100000");
		evalip6("ip6tables", "-t", "mangle", "-A", "POSTROUTING", "-o", wan_ifname, "-j", "MARK", "--set-mark", "0x100000");

		evalip6("ip6tables", "-t", "mangle", "-D", "POSTROUTING", "-m", "mark", "--mark", "0x100000", "-j", "CLASSIFY",
			"--set-class", "0:1");
		evalip6("ip6tables", "-t", "mangle", "-A", "POSTROUTING", "-m", "mark", "--mark", "0x100000", "-j", "CLASSIFY",
			"--set-class", "0:1");

		evalip6("ip6tables", "-t", "mangle", "-D", "POSTROUTING", "-o", wan_ifname, "-p", "udp", "--dport", "547", "-j",
			"CLASSIFY", "--set-class", "0:0");
		evalip6("ip6tables", "-t", "mangle", "-A", "POSTROUTING", "-o", wan_ifname, "-p", "udp", "--dport", "547", "-j",
			"CLASSIFY", "--set-class", "0:0");
	}
#endif
	_log_evalpid(dhcp_argv, NULL, 0, &pid);

	if (s_auth)
		free(s_auth);
	if (s_clientid)
		free(s_clientid);
	if (s_userclass)
		free(s_userclass);
}

/*
 * Enable WET DHCP relay for ethernet clients 
 */
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
static int enable_dhcprelay(char *ifname)
{
	char name[80], *next;
#ifdef HAVE_DHDAP
	int is_dhd;
#endif /* __CONFIG_DHDAP__ */

	dprintf("%s\n", ifname);

	/*
	 * WET interface is meaningful only in bridged environment 
	 */
	if (strncmp(ifname, "br", 2) == 0) {
		foreach(name, nvram_safe_get("lan_ifnames"), next)
		{
			char mode[] = "wlXXXXXXXXXX_mode";
			int unit;
			/*
			 * get the instance number of the wl i/f 
			 */
			unit = get_wl_instance(name);
			if (unit == -1)
				continue;
			snprintf(mode, sizeof(mode), "wl%d_mode", unit);

			/*
			 * enable DHCP relay, there should be only one WET i/f 
			 */
			if (nvram_match(mode, "wet") || nvram_match(mode, "apstawet")) {
				uint32 ip;

				inet_aton(nvram_safe_get("lan_ipaddr"), (struct in_addr *)&ip);
#ifdef HAVE_DHDAP
				is_dhd = !dhd_probe(name);
				if (is_dhd) {
					dhd_iovar_setint(name, "wet_host_ipv4", ip);
				} else
#endif /* __CONFIG_DHDAP__ */
				{
					if (wl_iovar_setint(name, "wet_host_ipv4", ip))
						perror("wet_host_ipv4");
				}
				break;
			}
		}
	}
	return 0;
}
#endif

int isClient(void)
{
	if (getSTA())
		return 1;
	return 0;
}

void stop_wlconf(void)
{
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
	int cnt = get_wl_instances();
	int c;
#ifdef HAVE_QTN
	cnt = 1;
#endif
	for (c = 0; c < cnt; c++) {
		wlconf_down(get_wl_instance_name(c));
	}
#endif
}

void start_wlconf(void)
{
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
	int cnt = get_wl_instances();
	int c;
#ifdef HAVE_QTN
	cnt = 1;
#endif
	for (c = 0; c < cnt; c++) {
		if (cnt > 1)
			eval("wl", "-i", get_wl_instance_name(c), "interference", "0");
		if (!nvram_nmatch("disabled", "wl%d_net_mode", c))
			wlconf_up(get_wl_instance_name(c));
	}
#endif
}

// #ifdef HAVE_PORTSETUP
#if (defined(HAVE_RT2880) || defined(HAVE_RT61)) && !defined(HAVE_MT76)
#define IFMAP(a) getRADev(a)
#else
#define IFMAP(a) (a)
#endif

static void do_portsetup(char *lan, char *ifname)
{
	char var[64];
	char var2[64];
	char tmp[256];
	sprintf(var, "%s_bridged", IFMAP(ifname));
	if (nvram_default_matchi(var, 1, 1)) {
		br_add_interface(getBridge(IFMAP(ifname), tmp), IFMAP(ifname));
	} else {
		ifconfig(ifname, IFUP, nvram_nget("%s_ipaddr", IFMAP(ifname)), nvram_nget("%s_netmask", ifname));
		log_eval("gratarp", ifname);
	}
}

// #endif

#define PORTSETUPWAN(a)                                                  \
	if (*a && *(nvram_safe_get("wan_ifname2"))) {                    \
		strcpy(wan_ifname, nvram_safe_get("wan_ifname2"));       \
		nvram_set("wan_ifnames", nvram_safe_get("wan_ifname2")); \
	} else {                                                         \
		strcpy(wan_ifname, a);                                   \
		nvram_set("wan_ifnames", a);                             \
	}

void reset_hwaddr(char *ifname)
{
	char eabuf[32];
	/*
	 * Get current LAN hardware address 
	 */
	char macaddr[32];
	if (get_hwaddr(ifname, macaddr)) {
		if (!*(nvram_safe_get("lan_hwaddr")))
			nvram_set("lan_hwaddr", macaddr);
		if (getRouterBrand() == ROUTER_DLINK_DIR320) {
			if (strlen(nvram_safe_get("et0macaddr")) == 12) {
				char wlmac[32];

				strcpy(wlmac, nvram_safe_get("wl0_hwaddr"));
				MAC_SUB(wlmac);
				nvram_set("et0macaddr", wlmac);
				nvram_unset("lan_hwaddr");
				nvram_unset("wan_hwaddr");
				// fix dlink quirk, by restarting system. utils.c will
				// automaticly assign the et0macaddr then
				nvram_commit();
				eval("event", "5", "1", "15");
			}
		}
		if (!*(nvram_safe_get("et0macaddr"))) {
			char *def = nvram_safe_get("et0macaddr_safe");
			if (!*def)
				def = nvram_safe_get("lan_hwaddr");
#if defined(HAVE_RB500) || defined(HAVE_MAGICBOX) || defined(HAVE_LAGUNA) || defined(HAVE_VENTANA) || defined(HAVE_NEWPORT) || \
	defined(HAVE_RB600) || defined(HAVE_FONERA) || defined(HAVE_RT2880) || defined(HAVE_LS2) || defined(HAVE_LS5) ||       \
	defined(HAVE_SOLO51) || defined(HAVE_WHRAG108) || defined(HAVE_PB42) || defined(HAVE_LSX) || defined(HAVE_DANUBE) ||   \
	defined(HAVE_STORM) || defined(HAVE_OPENRISC) || defined(HAVE_ADM5120) || defined(HAVE_TW6600) || defined(HAVE_CA8) || defined(HAVE_IPQ6018) || \
	defined(HAVE_EROUTER)
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_XSCALE
#ifndef HAVE_GATEWORX
			nvram_set("et0macaddr", def);
#endif
#endif
		}
	}

	if (!*(nvram_safe_get("lan_hwaddr")))
		nvram_set(
			"lan_hwaddr",
			nvram_safe_get(
				"et0macaddr")); //after all fixes have been made, we set lan_hwaddr to et0macaddr to ensure equalness between all devices based first eth interface
	// lock mac address on bridge if possible
	set_hwaddr(ifname, nvram_safe_get("lan_hwaddr"));
}

#ifdef HAVE_3G
#define CANBRIDGE()                                                                                                      \
	(nvram_match("wan_proto", "disabled") || nvram_match("wan_proto", "3g") || nvram_match("wan_proto", "iphone") || \
	 nvram_match("wan_proto", "android"))
#else
#define CANBRIDGE() nvram_match("wan_proto", "disabled")
#endif

void start_lan(void)
{
	char mac[20];
	int s;
	struct ifreq ifr;
#ifdef HAVE_DHDAP
	int is_dhd;
#endif /*__CONFIG_DHDAP__ */
	char eabuf[32];
	char lan_ifname[64]; //= strdup(nvram_safe_get("lan_ifname"));
	char wan_ifname[64]; //= strdup(nvram_safe_get("wan_ifname"));
	char lan_ifnames[128]; //= strdup(nvram_safe_get("lan_ifnames"));
	char name[80];
	char *next;
	char realname[80];
	char wl_face[10];
	char macaddr[20];

	// don't let packages pass to iptables without ebtables loaded
	writeprocsysnet("bridge/bridge-nf-call-arptables", "0");
	writeprocsysnet("bridge/bridge-nf-call-ip6tables", "0");
	writeprocsysnet("bridge/bridge-nf-call-iptables", "0");

	// fix list of active client in webif
	writeprocsysnet("ipv4/neigh/default/gc_thresh1", "1");
	writeprocsysnet("ipv4/neigh/default/gc_interval", nvram_default_get("net.ipv4.neigh.default.gc_interval", "120"));

	strcpy(lan_ifname, nvram_safe_get("lan_ifname"));
	strcpy(wan_ifname, nvram_safe_get("wan_ifname"));
	strcpy(lan_ifnames, nvram_safe_get("lan_ifnames"));

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return;

#define nvram_setz(a, b) strcpy(a, b)
	nvram_setz(lan_ifname, "br0");
#ifdef HAVE_RB500
	nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 wlan0 wlan1 wlan2 wlan3 wlan4 wlan5");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}

	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
#endif

#ifdef HAVE_MAGICBOX
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}

	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
	MAC_ADD(mac);
	set_hwaddr("eth1", mac);
#endif
#ifdef HAVE_UNIWIP
	nvram_setz(lan_ifnames, "eth0 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}

	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
	MAC_ADD(mac);
#elif HAVE_E200
	nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}

	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
	MAC_ADD(mac);
#elif HAVE_EROUTER
	nvram_setz(lan_ifnames, "eth0 eth1 eth2");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}

	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
	MAC_ADD(mac);
#elif HAVE_MVEBU
	if (getRouterBrand() == ROUTER_WRT_1900AC) {
		if (getSTA() || getWET() || CANBRIDGE()) {
			nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
			PORTSETUPWAN("");
		} else {
			nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
			PORTSETUPWAN("eth1");
		}
	} else {
		if (getSTA() || getWET() || CANBRIDGE()) {
			nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
			PORTSETUPWAN("");
		} else {
			nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
			PORTSETUPWAN("eth0");
		}
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
	MAC_ADD(mac);
	set_hwaddr("eth1", mac);
#elif HAVE_R9000
	nvram_setz(lan_ifnames, "eth0 vlan1 vlan2 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("vlan2");
	}

	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_IPQ806X
	int board = getRouterBrand();
	switch (board) {
	case ROUTER_TRENDNET_TEW827:
		if (getSTA() || getWET() || CANBRIDGE()) {
			nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
			PORTSETUPWAN("");
		} else {
			nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
			PORTSETUPWAN("eth0");
		}
		strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
		break;
	case ROUTER_ASUS_AC58U:
	case ROUTER_LINKSYS_EA8300:
	case ROUTER_HABANERO:
		if (getSTA() || getWET() || CANBRIDGE()) {
			nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
			PORTSETUPWAN("");
		} else {
			nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
			PORTSETUPWAN("eth1");
		}
		strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
		break;
	default:
		if (getSTA() || getWET() || CANBRIDGE()) {
			nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
			PORTSETUPWAN("");
		} else {
			nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
			PORTSETUPWAN("eth0");
		}
		strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
		break;
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WDR4900
	nvram_setz(lan_ifnames, "vlan1 vlan2 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("vlan2");
	}

	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_RB600
	nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 wlan0 wlan1 wlan2 wlan3 wlan4 wlan5 wlan6 wlan7");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}

	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
	MAC_ADD(mac);
	set_hwaddr("eth1", mac);
#endif
#if defined(HAVE_FONERA) && !defined(HAVE_DIR300) && !defined(HAVE_WRT54G2) && !defined(HAVE_MR3202A) && !defined(HAVE_RTG32)
	if (getRouterBrand() == ROUTER_BOARD_FONERA2200) {
		nvram_setz(lan_ifnames, "vlan0 vlan1 wlan0");
		if (getSTA() || getWET() || CANBRIDGE()) {
			PORTSETUPWAN("");
		} else {
			PORTSETUPWAN("vlan1");
		}
	} else {
		nvram_setz(lan_ifnames, "eth0 wlan0");
		if (getSTA() || getWET() || CANBRIDGE()) {
			PORTSETUPWAN("");
		} else {
			PORTSETUPWAN("eth0");
		}
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_WRT54G2
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "vlan1 vlan2 wlan0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "vlan1 vlan2 wlan0");
		PORTSETUPWAN("vlan2");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_RTG32
	nvram_setz(lan_ifnames, "vlan1 vlan2 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("vlan2");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_DIR300
	nvram_setz(lan_ifnames, "vlan1 vlan2 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("vlan2");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_RS
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1 wlan2");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WA901
	nvram_setz(lan_ifnames, "eth0 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WR941
	nvram_setz(lan_ifnames, "vlan0 vlan1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("vlan1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WR1043
	nvram_setz(lan_ifnames, "vlan1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("vlan2");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("vlan1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_AP83
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WZRG450
	nvram_setz(lan_ifnames, "vlan1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("vlan2");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("vlan1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WZRHPAG300NH
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WR810N
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_DIR632
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_AP94
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_HORNET
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
#ifdef HAVE_MAKSAT
		PORTSETUPWAN("eth0");
#elif HAVE_ONNET
		PORTSETUPWAN("eth0");
#else
		PORTSETUPWAN("eth1");
#endif
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WNR2000
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_E325N
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_XD9531
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WR615N
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_E355AC
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_XD3200
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("vlan2");
	}
	nvram_setz(lan_ifnames, "vlan1 wlan0 wlan1");
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_E380AC
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_AP120C
	nvram_setz(lan_ifnames, "eth0 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WR650AC
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_DIR862
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_CPE880
	nvram_setz(lan_ifnames, "vlan2 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("vlan1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WILLY
	nvram_setz(lan_ifnames, "eth0 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_MMS344
	nvram_setz(lan_ifnames, "vlan1 vlan2 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("vlan2");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_ARCHERC7V4
	nvram_setz(lan_ifnames, "vlan1 vlan2 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("vlan2");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WZR450HP2
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WDR3500
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_JWAP606
	nvram_setz(lan_ifnames, "eth0 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_LIMA
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_DW02_412H
	nvram_setz(lan_ifnames, "vlan1 vlan2 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("vlan2");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_RAMBUTAN
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WASP
	nvram_setz(lan_ifnames, "vlan1 vlan2 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("vlan2");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WDR2543
	nvram_setz(lan_ifnames, "vlan1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("vlan2");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("vlan1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WHRHPGN
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_JJAP93
	nvram_setz(lan_ifnames, "eth0 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_JJAP005
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_JJAP501
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_AC722
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_AC622
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_JA76PF
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_ALFAAP94
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1 wlan2 wlan3");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_JWAP003
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_DAP2230
	nvram_setz(lan_ifnames, "eth0 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WA901V5
	nvram_setz(lan_ifnames, "eth0 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WR941V6
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WR841V9
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_DIR615E
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WA901v1
	nvram_setz(lan_ifnames, "eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_ERC
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_CARAMBOLA
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("vlan1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WR710
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WR703
	nvram_setz(lan_ifnames, "eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WA7510
	nvram_setz(lan_ifnames, "eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WR741
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_DAP3310
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_DAP3410
	nvram_setz(lan_ifnames, "vlan1 vlan2 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("vlan2");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_UBNTM
	int brand = getRouterBrand();
	int devnum = 2;
	int vlan = 0;
	switch (brand) {
	case ROUTER_BOARD_BS2M:
	case ROUTER_BOARD_UNIFI:
	case ROUTER_UBNT_UAPAC:
	case ROUTER_BOARD_BS5M:
	case ROUTER_BOARD_R2M:
	case ROUTER_BOARD_R5M:
	case ROUTER_UBNT_POWERBEAMAC_GEN2:
		devnum = 1;
		break;
	case ROUTER_BOARD_NS2M:
	case ROUTER_BOARD_NS5M:
	case ROUTER_BOARD_AIRROUTER:
	case ROUTER_UBNT_UAPACPRO:
		devnum = 2;
		break;
	case ROUTER_BOARD_NS5MXW:
	case ROUTER_UBNT_NANOAC:
		devnum = 2;
		vlan = 1;
		break;
	default:
		devnum = 2;
		break;
	}

	if (vlan) {
		if (brand == ROUTER_BOARD_NS5MXW)
			nvram_setz(lan_ifnames, "vlan1 wlan0");
		else
			nvram_setz(lan_ifnames, "vlan1 vlan2 wlan0");
	} else if (devnum == 2)
		nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
	else
		nvram_setz(lan_ifnames, "eth0 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		if (vlan) {
			if (!*(nvram_safe_get("wan_ifname2")))
				nvram_set("wan_ifname2", "vlan2");
			PORTSETUPWAN("vlan2");
		} else {
			if (!*(nvram_safe_get("wan_ifname2")))
				nvram_set("wan_ifname2", "eth0");
			PORTSETUPWAN("eth0");
		}
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));

#elif HAVE_WP546
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_LSX
	nvram_setz(lan_ifnames, "eth0 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_DANUBE
	nvram_setz(lan_ifnames, "eth0 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		stop_atm();
		start_atm();
		PORTSETUPWAN("nas0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_RT2880
	int rb = getRouterBrand();
	if (rb == ROUTER_BOARD_ECB9750 || rb == ROUTER_BOARD_EAP9550 || rb == ROUTER_BOARD_TECHNAXX3G) // lets load
	{
		nvram_setz(lan_ifnames, "eth2 ra0 ba0");
		if (getSTA() || getWET() || CANBRIDGE()) {
			PORTSETUPWAN("");
		} else {
			PORTSETUPWAN("eth2");
		}
	} else {
		nvram_setz(lan_ifnames, "vlan1 vlan2 ra0 ba0");
		if (getSTA() || getWET() || CANBRIDGE()) {
			PORTSETUPWAN("");
		} else {
			PORTSETUPWAN("vlan2");
		}
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth2", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_WBD222
	nvram_setz(lan_ifnames, "eth0 eth1 eth2 wlan0 wlan1 wlan2");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_STORM
	nvram_setz(lan_ifnames, "eth0 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_OPENRISC
	nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_NORTHSTAR
#ifdef HAVE_DHDAP
	nvram_setz(lan_ifnames, "vlan1 vlan2 eth1 eth2 eth3");
#else
	nvram_setz(lan_ifnames, "vlan1 vlan2 eth1 eth2");
#endif
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("vlan2");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("vlan1", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#if defined(HAVE_NEWPORT)
	nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 wlan0 wlan1 wlan2 wlan3 wlan4 wlan5 wlan6");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif defined(HAVE_LAGUNA)
	nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 wlan0 wlan1 wlan2 wlan3");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif defined(HAVE_IPQ6018)
	nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth4");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth4", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif defined(HAVE_VENTANA)
	nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 wlan0 wlan1 wlan2 wlan3 wlan4 wlan5 wlan6");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_ADM5120

	if (getRouterBrand() == ROUTER_BOARD_WP54G || getRouterBrand() == ROUTER_BOARD_NP28G) {
		nvram_setz(lan_ifnames, "eth0 eth1 wlan0");
		if (getSTA() || getWET() || CANBRIDGE()) {
			PORTSETUPWAN("");
		} else {
			PORTSETUPWAN("eth1");
		}
	} else {
		nvram_setz(lan_ifnames, "eth0 wlan0");
		if (getSTA() || getWET() || CANBRIDGE()) {
			PORTSETUPWAN("");
		} else {
			PORTSETUPWAN("eth0");
		}
	}

	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_MR3202A
	nvram_setz(lan_ifnames, "vlan1 vlan2 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("vlan2");
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif

#if defined(HAVE_LS2) || defined(HAVE_SOLO51)
#if defined(HAVE_NS2) || defined(HAVE_BS2) || defined(HAVE_LC2) || defined(HAVE_BS2HP) || defined(HAVE_MS2) || \
	defined(HAVE_PICO2) || defined(HAVE_PICO2HP)
	nvram_setz(lan_ifnames, "eth0 wlan0");
#else
	nvram_setz(lan_ifnames, "vlan0 vlan2 wlan0");
#endif
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
#if defined(HAVE_NS2) || defined(HAVE_BS2) || defined(HAVE_LC2) || defined(HAVE_BS2HP) || defined(HAVE_MS2) || \
	defined(HAVE_PICO2) || defined(HAVE_PICO2HP)
		PORTSETUPWAN("eth0");
#else
#ifdef HAVE_BWRG1000
		PORTSETUPWAN("vlan2");
#else
		PORTSETUPWAN("vlan0");
#endif
#endif
	}

	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_LS5
	nvram_setz(lan_ifnames, "eth0 wlan0");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}

	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_TW6600
	nvram_setz(lan_ifnames, "eth0 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}

	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_PB42
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth0");
	}

	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_WHRAG108
	nvram_setz(lan_ifnames, "eth0 eth1 wlan0 wlan1");
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		PORTSETUPWAN("eth1");
	}

	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_CA8
	if (getRouterBrand() == ROUTER_BOARD_CA8PRO) {
		nvram_setz(lan_ifnames, "vlan0 vlan1 wlan0");
	} else if (getRouterBrand() == ROUTER_BOARD_RCAA01) {
		nvram_setz(lan_ifnames, "vlan0 vlan1 wlan0 wlan1");
	} else if (getRouterBrand() == ROUTER_BOARD_RDAT81) {
		nvram_setz(lan_ifnames, "eth0 wlan0 wlan1");
	} else {
		nvram_setz(lan_ifnames, "eth0 wlan0");
	}
	if (getSTA() || getWET() || CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
		if (getRouterBrand() == ROUTER_BOARD_CA8PRO || getRouterBrand() == ROUTER_BOARD_RCAA01) {
			nvram_setz(lan_ifnames, "vlan0 vlan1 wlan0");
			PORTSETUPWAN("vlan1");
		} else {
			nvram_setz(lan_ifnames, "eth0 wlan0");
			PORTSETUPWAN("eth0");
		}
	}

	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_GATEWORX
	if (getRouterBrand() == ROUTER_BOARD_GATEWORX_SWAP) {
		nvram_setz(lan_ifnames, "ixp0 eth0 eth1 wlan0 wlan1 wlan2 wlan3 ofdm");
		PORTSETUPWAN("");
	} else if (getRouterBrand() == ROUTER_BOARD_GATEWORX_GW2345) {
		nvram_setz(lan_ifnames, "ixp0 ixp1 eth0 eth1 wlan0 wlan1 wlan2 wlan3 ofdm");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "ixp0 ixp1 eth0 eth1 wlan0 wlan1 wlan2 wlan3 ofdm");
		PORTSETUPWAN("");
	}
	if (getSTA() || getWET() || CANBRIDGE()) {
	} else {
		if (getRouterBrand() == ROUTER_BOARD_GATEWORX_SWAP) {
			PORTSETUPWAN("ixp0");
		} else if (getRouterBrand() == ROUTER_BOARD_GATEWORX_GW2345) {
#ifdef HAVE_MI424WR
			PORTSETUPWAN("ixp0");
#else
			PORTSETUPWAN("ixp1");
#endif
		} else {
#ifdef HAVE_XIOCOM
			PORTSETUPWAN("ixp0");
#else
			PORTSETUPWAN("ixp1");
#endif
		}
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("ixp0", macaddr));
#endif
#ifdef HAVE_X86
#ifdef HAVE_NOWIFI
	nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10");
#else
	if (nvram_matchi("wifi_bonding", 1))
		nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10 bond0");
	else
		nvram_setz(
			lan_ifnames,
			"eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10 wlan0 wlan1 wlan2 wlan3 wlan4 wlan5 wlan6 wlan7 wlan8");

#endif
	if (getSTA() || getWET()) {
		PORTSETUPWAN("");
	} else if (CANBRIDGE()) {
		PORTSETUPWAN("");
	} else {
#ifdef HAVE_GW700
		PORTSETUPWAN("eth1");
#else
		PORTSETUPWAN("eth0");
#endif
	}

	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", get_hwaddr("eth0", macaddr));
#endif

	if (!nvram_match("lan_ifname", lan_ifname) || !nvram_match("wan_ifname", wan_ifname) ||
	    !nvram_match("lan_ifnames", lan_ifnames)) {
		nvram_set("lan_ifname", lan_ifname);
		nvram_set("wan_ifname", wan_ifname);
		nvram_set("lan_ifnames", lan_ifnames);
		nvram_async_commit();
	}

	cprintf("lan ifname = %s\n", lan_ifname);
	cprintf("lan ifnames = %s\n", lan_ifnames);
	cprintf("wan ifname = %s\n", wan_ifname);

	// If running in client-mode, remove old WAN-configuration
	if (getSTA()) {
		// #ifdef HAVE_SKYTRON
		// ifconfig(wan_ifname,IFUP,"172.16.1.1","255.255.255.0");
		// #else
		ifconfig(wan_ifname, IFUP, "0.0.0.0", NULL);
		// #endif
	}
	// find wireless interface
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	diag_led(DIAG, STOP_LED); // stop that blinking
#endif
	strcpy(wl_face, get_wdev());
#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880) || defined(HAVE_RT61)
#ifndef HAVE_NOWIFI
	void deconfigure_wifi(void);

	deconfigure_wifi();
#endif
#else
	wlconf_down(wl_face);
#endif
#ifdef HAVE_WAVESAT
	deconfigure_wimax();
#endif

	/*
	 * you gotta bring it down before you can set its MAC 
	 */
	cprintf("configure wl_face %s\n", wl_face);
	ifconfig(wl_face, 0, 0, 0);
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)

	if (nvram_matchi("mac_clone_enable", 1) && nvram_invmatch("def_whwaddr", "00:00:00:00:00:00") &&
	    nvram_invmatch("def_whwaddr", "")) {
		ether_atoe(nvram_safe_get("def_whwaddr"), ifr.ifr_hwaddr.sa_data);

	} else {
		int instance = get_wl_instance(wl_face);
		getWirelessMac(mac, instance);

		ether_atoe(mac, ifr.ifr_hwaddr.sa_data);

		if (nvram_match("wl0_hwaddr", "") || !nvram_exists("wl0_hwaddr")) {
			nvram_set("wl0_hwaddr", mac);
			nvram_async_commit();
		}
	}
	/*
	 * Write wireless mac 
	 */
	cprintf("Write wireless mac\n");
	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
	strncpy(ifr.ifr_name, wl_face, IFNAMSIZ);

	eval("wl", "-i", wl_face, "down");
	if (ioctl(s, SIOCSIFHWADDR, &ifr) == -1)
		perror("Write wireless mac fail : ");
	else
		cprintf("Write wireless mac successfully\n");
	eval("wl", "-i", wl_face, "up");
	config_macs(wl_face);
#endif
	if (getSTA()) {
		char mac[20];

		getWANMac(mac);

		nvram_set("wan_hwaddr", mac);
	}

	cprintf("wl_face up %s\n", wl_face);
	ifconfig(wl_face, IFUP, 0, 0);
#ifdef HAVE_MICRO
	br_init();
#endif
	/*
	 * Bring up bridged interface 
	 */
#ifdef HAVE_EAD
	char *eadline = NULL;

#endif
	char *wanstate = NULL;
	if (getSTA() || getWET() || CANBRIDGE() && !nvram_match("vlans", "1")) {
		wanstate = set_wan_state(0);
	} else {
		if (!nvram_match("vlans", "1"))
			set_wan_state(1);
	}
	if (strncmp(lan_ifname, "br0", 3) == 0) {
		br_add_bridge(lan_ifname);
		eval("ifconfig", lan_ifname, "promisc");
		char word[256];
		br_set_stp_state(lan_ifname, getBridgeSTP(lan_ifname, word));
		br_set_bridge_max_age(lan_ifname, getBridgeMaxAge(lan_ifname));
		br_set_bridge_forward_delay(lan_ifname, getBridgeForwardDelay(lan_ifname));
#ifdef HAVE_EAD
		eval("killall", "-9", "ead");
#endif

		foreach(name, lan_ifnames, next)
		{
			int ex = ifexists(name);
			if (!ex)
				continue;
#ifdef HAVE_EAD
#if defined(HAVE_RT2880) || defined(HAVE_MADWIFI) || defined(HAVE_RT61)
			if (strncmp(name, "wlan", 4) && strncmp(name, "ra", 2))
#else
			if (wl_probe(name))
#endif
			{
				if (!eadline) {
					eadline = malloc(64);
					snprintf(eadline, 64, "-d %s", name);
				} else {
					eadline = realloc(eadline, strlen(eadline) + 64);
					strcat(eadline, " -d ");
					strcat(eadline, name);
				}
			}
#endif
			fprintf(stderr, "check %s\n", name);
			if (nvram_match("wan_ifname", name))
				continue;
#if defined(HAVE_MADWIFI) && !defined(HAVE_RB500) && !defined(HAVE_XSCALE) && !defined(HAVE_LAGUNA) && !defined(HAVE_VENTANA) && \
	!defined(HAVE_NEWPORT) && !defined(HAVE_MAGICBOX) && !defined(HAVE_RB600) && !defined(HAVE_FONERA) &&                    \
	!defined(HAVE_WHRAG108) && !defined(HAVE_X86) && !defined(HAVE_LS2) && !defined(HAVE_LS5) && !defined(HAVE_CA8) &&       \
	!defined(HAVE_TW6600) && !defined(HAVE_PB42) && !defined(HAVE_LSX) && !defined(HAVE_DANUBE) && !defined(HAVE_STORM) &&   \
	!defined(HAVE_OPENRISC) && !defined(HAVE_ADM5120) && !defined(HAVE_RT2880) && !defined(HAVE_SOLO51) &&                   \
	!defined(HAVE_EROUTER) && !defined(HAVE_IPQ806X) && !defined(HAVE_R9000) && !defined(HAVE_IPQ6018)
			if (!strcmp(name, "eth2")) {
				strcpy(realname, "wlan0");
			} else
#endif
				strcpy(realname, name);

			fprintf(stderr, "name=[%s] lan_ifname=[%s]\n", realname, lan_ifname);

			/*
			 * Bring up interface 
			 */
			if (ifconfig(realname, IFUP, "0.0.0.0", NULL)) {
				continue;
			}
			// set proper mtu

			if (strncmp(realname, "wlan", 4) != 0) { // this is not an ethernet driver
				eval("ifconfig", realname,
				     "down"); //fixup for some ethernet drivers
			}
			eval("ifconfig", realname, "mtu", getMTU(realname));
			eval("ifconfig", realname, "txqueuelen", getTXQ(realname));
			if (!nvram_nmatch("", "%s_hwaddr", realname))
				set_hwaddr(realname, nvram_nget("%s_hwaddr", realname));

			if (strncmp(realname, "wlan", 4) != 0) { // this is not an ethernet driver
				eval("ifconfig", realname,
				     "up"); //fixup for some ethernet drivers
			}

			/*
			 * Set the logical bridge address to that of the first interface 
			 */

#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
			strncpy(ifr.ifr_name, lan_ifname, IFNAMSIZ);
			if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0 &&
			    (memcmp(ifr.ifr_hwaddr.sa_data, "\0\0\0\0\0\0", ETHER_ADDR_LEN) == 0 ||
			     (((ifr.ifr_hwaddr.sa_data[0] & 0x01) == 0) && ((ifr.ifr_hwaddr.sa_data[1] & 0x02) == 0x02))) &&
			    strcmp(wl_face, realname) == 0) {
				strncpy(ifr.ifr_name, realname, IFNAMSIZ);
				if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0) {
					strncpy(ifr.ifr_name, lan_ifname, IFNAMSIZ);
					ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
					ioctl(s, SIOCSIFHWADDR, &ifr);
					cprintf("=====> set %s hwaddr to %s\n", lan_ifname, realname);
				} else
					perror(lan_ifname);
			} else
				perror(lan_ifname);
#endif
			char wl_name[] = "wlXXXXXXXXXX_mode";
			int unit;
			char tmp[256];
			/*
			 * If not a wl i/f then simply add it to the bridge 
			 */
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
			int conf = wlconf_up(name);
#else
			int conf = strcmp(name, "wl0") ? -1 : 0;
#endif
			switch (conf) {
			case -1:
				do_portsetup(lan_ifname, name);
				break;
			case -2: //ignore
				br_del_interface(lan_ifname, name);
				break;
			case 0:
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
				if (nvram_matchi("mac_clone_enable", 1) && nvram_invmatch("def_whwaddr", "00:00:00:00:00:00") &&
				    nvram_invmatch("def_whwaddr", "")) {
					ether_atoe(nvram_safe_get("def_whwaddr"), ifr.ifr_hwaddr.sa_data);

				} else {
					int instance = get_wl_instance(name);
					getWirelessMac(mac, instance);

					ether_atoe(mac, ifr.ifr_hwaddr.sa_data);

					if (instance == -1)
						continue; // no wireless device
					if (nvram_nmatch("", "wl%d_hwaddr", instance) || !nvram_nget("wl%d_hwaddr", instance)) {
						nvram_nset(mac, "wl%d_hwaddr", instance);
						nvram_async_commit();
						ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
						strncpy(ifr.ifr_name, name, IFNAMSIZ);

						eval("wl", "-i", name, "down");
						if (ioctl(s, SIOCSIFHWADDR, &ifr) == -1)
							perror("Write wireless mac fail : ");
						else
							cprintf("Write wireless mac successfully\n");
						eval("wl", "-i", name, "up");
						config_macs(name);
					}
				}
#endif
				/*
				 * get the instance number of the wl i/f 
				 */

#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880) || defined(HAVE_RT61)
				unit = 0;
#else
				unit = get_wl_instance(name);
#endif
				snprintf(wl_name, sizeof(wl_name), "wl%d_mode", unit);
				/*
				 * Do not attach the main wl i/f if in wds or client/adhoc 
				 */
				led_control(LED_BRIDGE, LED_OFF);
				if (nvram_match(wl_name, "wet") || nvram_match(wl_name, "apstawet")) {
					ifconfig(name, IFUP | IFF_ALLMULTI, NULL, NULL); // from
					// up
					br_add_interface(getBridge(IFMAP(name), tmp), name);
					led_control(LED_BRIDGE, LED_ON);
					/* Enable host DHCP relay */
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
					if (nvram_matchi("lan_dhcp", 1)) {
#ifdef HAVE_DHDAP
						is_dhd = !dhd_probe(name);
						if (is_dhd) {
							char macbuf[sizeof("wet_host_mac") + 1 + ETHER_ADDR_LEN];
							dhd_iovar_setbuf(name, "wet_host_mac", ifr.ifr_hwaddr.sa_data,
									 ETHER_ADDR_LEN, macbuf, sizeof(macbuf));
						} else
#endif /* __CONFIG_DHDAP__ */
						{
							wl_iovar_set(name, "wet_host_mac", ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
						}
					}
					/* Enable WET DHCP relay if requested */
					if (nvram_matchi("dhcp_relay",
							 1)) // seems to fix some dhcp problems, also Netgear does it this way
					{
						enable_dhcprelay(lan_ifname);
					}
#endif
				}
#ifdef HAVE_WAVESAT
				if (nvram_match(wl_name, "bridge")) {
					ifconfig(name, IFUP | IFF_ALLMULTI, NULL, NULL); // from
					// up
					br_add_interface(getBridge(IFMAP(name), tmp), name);
					led_control(LED_BRIDGE, LED_ON);
				}
#endif

				if (nvram_match(wl_name, "ap")) {
					do_portsetup(lan_ifname, name);
					// br_add_interface (getBridge (name), name); //eval
					// ("brctl", "addif", lan_ifname, name);
				}
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
				wl_scan_params_t params;

				bzero(&params, sizeof(params));

				/*
				 * use defaults (same parameters as wl scan) 
				 */

				memset(&params.bssid, 0xff, sizeof(params.bssid));

				params.bss_type = DOT11_BSSTYPE_ANY;
				params.scan_type = 0;
				params.nprobes = -1;
				params.active_time = -1;
				params.passive_time = -1;
				params.home_time = -1;
				params.channel_num = 0;
#endif

				if (nvram_match(wl_name, "apsta")) {
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
					// eval ("wl", "ap", "0");
					eval("wl", "-i", name, "ap", "0");
					// eval ("wl", "infra", "1");
					eval("wl", "-i", name, "infra", "1");
					wl_ioctl(wl_name, WLC_SCAN, &params, 64);
					wlconf_up(name);
#endif
					// eval("wlconf", name, "up");
					ifconfig(name, IFUP | IFF_ALLMULTI, NULL, NULL);
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
					// eval ("wl", "ap", "0");
					eval("wl", "-i", name, "ap", "0");
					// eval ("wl", "ssid", nvram_get ("wl0_ssid"));
					eval("wl", "-i", name, "ssid", nvram_nget("wl%d_ssid", get_wl_instance(name)));
					// eval ("brctl", "addif", lan_ifname, name);
#endif
				}

				/*
				 * if client/wet mode, turn off ap mode et al 
				 */
				if (nvram_match(wl_name, "infra")) {
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
					// eval ("wl", "ap", "0");
					eval("wl", "-i", name, "ap", "0");
					// eval ("wl", "infra", "0");
					eval("wl", "-i", name, "infra", "0");
					wl_ioctl(wl_name, WLC_SCAN, &params, 64);
					wlconf_up(name);
#endif
					// eval ("wl", "infra", "0");
					eval("wl", "-i", name, "infra", "0");
					// eval ("wl", "ssid", nvram_safe_get ("wl0_ssid"));
					ifconfig(name, IFUP | IFF_ALLMULTI, NULL, NULL);
					eval("wl", "-i", name, "ssid", nvram_nget("wl%d_ssid", get_wl_instance(name)));
					do_portsetup(lan_ifname, name);
				}

				if (nvram_match(wl_name, "sta")) {
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
					// eval ("wl", "ap", "0");
					eval("wl", "-i", name, "ap", "0");
					// eval ("wl", "infra", "1");
					eval("wl", "-i", name, "infra", "1");
					wlconf_up(name);
					wl_ioctl(name, WLC_SCAN, &params, 64);
#endif
					// eval("wlconf", name, "up");
					ifconfig(name, IFUP | IFF_ALLMULTI, NULL, NULL);
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
					// eval ("wl", "ap", "0");
					eval("wl", "-i", name, "ap", "0");
					// eval ("wl", "ssid", nvram_get ("wl0_ssid"));
					eval("wl", "-i", name, "ssid", nvram_nget("wl%d_ssid", get_wl_instance(name)));
#endif
				}
#ifdef HAVE_WAVESAT
				if (nvram_match(wl_name, "router")) {
					do_portsetup(lan_ifname, name);
					// br_add_interface (getBridge (name), name); //eval
					// ("brctl", "addif", lan_ifname, name);
				}
#endif
			}
		}
	}
	if (wanstate)
		br_add_interface(lan_ifname, wanstate);

#ifdef HAVE_EAD
	if (eadline && *eadline) {
		sysprintf("ead %s -B", eadline);
		free(eadline);
	}
#endif

	/*
	 * specific non-bridged lan i/f 
	 */
	if (strcmp(lan_ifname, "")) { // FIXME
		/*
		 * Bring up interface 
		 */
		ifconfig(lan_ifname, IFUP, NULL, NULL);
		eval("ifconfig", lan_ifname, "promisc");
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
		/*
		 * config wireless i/f 
		 */
		if (!wlconf_up(lan_ifname)) {
			char tmp[100], prefix[] = "wanXXXXXXXXXX_";
			int unit;

			/*
			 * get the instance number of the wl i/f 
			 */
			unit = get_wl_instance(lan_ifname);
			snprintf(prefix, sizeof(prefix), "wl%d_", unit);
			/*
			 * Receive all multicast frames in WET mode 
			 */
			if (nvram_match(strcat_r(prefix, "mode", tmp), "sta"))
				ifconfig(lan_ifname, IFUP | IFF_ALLMULTI, NULL, NULL);
			if (nvram_match(strcat_r(prefix, "mode", tmp), "apsta"))
				ifconfig(lan_ifname, IFUP | IFF_ALLMULTI, NULL, NULL);
		}
#endif
	}

	/*
	 * Bring up and configure LAN interface 
	 */

	eval("ifconfig", lan_ifname, "promisc");
#if defined(HAVE_FONERA) || defined(HAVE_CA8) && !defined(HAVE_MR3202A)
	if (getRouterBrand() != ROUTER_BOARD_FONERA2200 && getRouterBrand() != ROUTER_BOARD_CA8PRO &&
	    getRouterBrand() != ROUTER_BOARD_RCAA01)
		if (nvram_match("wlan0_mode", "sta") || nvram_match("wlan0_mode", "wdssta") ||
		    nvram_match("wlan0_mode", "wdssta_mtik") || nvram_match("wlan0_mode", "wet") || CANBRIDGE())
#endif
			eval("ifconfig", "eth0:0", "down");

	close(s);

#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880) || defined(HAVE_RT61)

#if (defined(HAVE_RT2880) || defined(HAVE_RT61)) && !defined(HAVE_MT76)
#define getWifi(a) a
#define getWDSSTA() NULL
#endif
#ifndef HAVE_NOWIFI
	if (nvram_matchi("mac_clone_enable", 1) && nvram_invmatch("def_hwaddr", "00:00:00:00:00:00") &&
	    nvram_invmatch("def_hwaddr", "")) {
#ifdef HAVE_MADWIFI
		char *wifi = "wifi0";
#else
		char *wifi = "ra0";
#endif
		eval("ifconfig", wifi, "down");
		set_hwaddr(wifi, nvram_safe_get("def_whwaddr"));
		//              eval("ifconfig", wifi, "up");
	}
	if (nvram_matchi("mac_clone_enable", 1) && nvram_invmatch("def_whwaddr", "00:00:00:00:00:00") &&
	    nvram_invmatch("def_whwaddr", "")) {
#ifdef HAVE_MADWIFI
		char *wifi = "wifi0";
#else
		char *wifi = "ra0";
#endif
		eval("ifconfig", wifi, "down");
		set_hwaddr(wifi, nvram_safe_get("def_whwaddr"));
		//              eval("ifconfig", wifi, "up");
	}
	ifconfig(lan_ifname, IFUP, nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));
	void configure_wifi(void);
	configure_wifi();
#endif
#endif
#ifdef HAVE_WAVESAT
	configure_wimax();
#endif
	nvram_set("sta_ifname", getSTA());
	reset_hwaddr(lan_ifname);
	ifconfig(lan_ifname, IFUP, nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));
#ifdef HAVE_QTN
	start_qtn(); //bootup quantenna firmware
#endif
	log_eval("gratarp", lan_ifname);

	cprintf("%s %s\n", nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));

#ifndef HAVE_MADWIFI
#ifndef HAVE_RT2880
#ifndef HAVE_RT61
	int cnt = get_wl_instances();
	int c;

	for (c = 0; c < cnt; c++) {
#ifdef HAVE_MADWIFI
		char br1enable[32];
		char br1ipaddr[32];
		char br1netmask[32];

		sprintf(br1enable, "wlan%d_br1_enable", c);
		sprintf(br1ipaddr, "wlan%d_br1_ipaddr", c);
		sprintf(br1netmask, "wlan%d_br1_netmask", c);
#else
		char br1enable[32];
		char br1ipaddr[32];
		char br1netmask[32];

		sprintf(br1enable, "wl%d_br1_enable", c);
		sprintf(br1ipaddr, "wl%d_br1_ipaddr", c);
		sprintf(br1netmask, "wl%d_br1_netmask", c);
#endif
		if (!nvram_exists(br1enable))
			nvram_seti(br1enable, 0);
		if (!nvram_exists(br1ipaddr))
			nvram_set(br1ipaddr, "0.0.0.0");
		if (!nvram_exists(br1netmask))
			nvram_set(br1netmask, "255.255.255.0");
		if (nvram_matchi(br1enable, 1)) {
			ifconfig("br1", 0, 0, 0);

			// eval ("ifconfig", "br1", "down");
			br_del_bridge("br1");
			br_add_bridge("br1");
			char word[256];

			br_set_stp_state("br1", getBridgeSTP("br1", word));
			br_set_bridge_max_age("br1", getBridgeMaxAge("br1"));
			br_set_bridge_forward_delay("br1", getBridgeForwardDelay("br1"));

			/*
			 * Bring up and configure br1 interface 
			 */
			if (nvram_invmatch(br1ipaddr, "0.0.0.0")) {
				ifconfig("br1", IFUP, nvram_safe_get(br1ipaddr), nvram_safe_get(br1netmask));

				br_set_stp_state("br1", getBridgeSTP("br1", word));
				sleep(2);
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
				notify_nas("lan", "br1", "up");
#endif
			}
		}
	}

	/*
	 * Sveasoft - Bring up and configure wds interfaces 
	 */
	/*
	 * logic - if separate ip defined bring it up 
	 */
	/*
	 * else if flagged for br1 and br1 is enabled add to br1 
	 */
	/*
	 * else add it to the br0 bridge 
	 */
	for (c = 0; c < cnt; c++) {
		for (s = 1; s <= MAX_WDS_DEVS; s++) {
			char wdsvarname[32] = { 0 };
			char wdsdevname[32] = { 0 };
			char *dev;

#ifdef HAVE_MADWIFI
			char br1enable[32];

			sprintf(wdsvarname, "wlan%d_wds%d_enable", c, s);
			sprintf(wdsdevname, "wlan%d_wds%d_if", c, s);
			sprintf(br1enable, "wlan%d_br1_enable", c);
			if (!nvram_exists(wdsvarname))
				nvram_seti(wdsvarname, 0);
#else
			char br1enable[32];

			sprintf(wdsvarname, "wl%d_wds%d_enable", c, s);
			sprintf(wdsdevname, "wl%d_wds%d_if", c, s);
			sprintf(br1enable, "wl%d_br1_enable", c);
			if (!nvram_exists(wdsvarname))
				nvram_seti(wdsvarname, 0);
#endif
			dev = nvram_safe_get(wdsdevname);
			if (!*dev)
				continue;
			ifconfig(dev, 0, 0, 0);

			// eval ("ifconfig", dev, "down");
			if (nvram_matchi(wdsvarname, 1)) {
				char *wdsip;
				char *wdsnm;
				char wdsbc[32] = { 0 };
#ifdef HAVE_MADWIFI
				wdsip = nvram_nget("wlan%d_wds%d_ipaddr", c, s);
				wdsnm = nvram_nget("wlan%d_wds%d_netmask", c, s);
#else
				wdsip = nvram_nget("wl%d_wds%d_ipaddr", c, s);
				wdsnm = nvram_nget("wl%d_wds%d_netmask", c, s);
#endif

				snprintf(wdsbc, sizeof(wdsbc), "%s", wdsip);
				get_broadcast(wdsbc, sizeof(wdsbc), wdsnm);
				eval("ifconfig", dev, wdsip, "broadcast", wdsbc, "netmask", wdsnm, "up");
			} else if (nvram_matchi(wdsvarname, 2) && nvram_matchi(br1enable, 1)) {
				eval("ifconfig", dev, "up");
				sleep(1);
				br_add_interface("br1", dev);
			} else if (nvram_matchi(wdsvarname, 3)) {
				ifconfig(dev, IFUP, 0, 0);
				sleep(1);
				char tmp[256];
				br_add_interface(getBridge(dev, tmp), dev);
			}
		}
	}
#endif
#endif
#endif
#ifdef HAVE_XSCALE
#define HAVE_RB500
#endif
#ifdef HAVE_LAGUNA
#define HAVE_RB500
#endif
#ifdef HAVE_VENTANA
#define HAVE_RB500
#endif
#ifdef HAVE_IPQ6018
#define HAVE_RB500
#endif
#ifdef HAVE_NEWPORT
#define HAVE_RB500
#endif
#ifdef HAVE_EROUTER
#define HAVE_RB500
#endif
#ifdef HAVE_PB42
#define HAVE_RB500
#endif
#ifdef HAVE_LSX
#define HAVE_RB500
#endif
#ifdef HAVE_DANUBE
#define HAVE_RB500
#endif
#ifdef HAVE_STORM
#define HAVE_RB500
#endif
#ifdef HAVE_OPENRISC
#define HAVE_RB500
#endif
#ifdef HAVE_ADM5120
#define HAVE_RB500
#endif
#ifdef HAVE_MAGICBOX
#define HAVE_RB500
#endif
#ifdef HAVE_RB600
#define HAVE_RB500
#endif
#ifdef HAVE_RT2880
#define HAVE_RB500
#endif
#ifdef HAVE_FONERA
#define HAVE_RB500
#endif
#ifdef HAVE_LS2
#define HAVE_RB500
#endif
#ifdef HAVE_SOLO51
#define HAVE_RB500
#endif
#ifdef HAVE_LS5
#define HAVE_RB500
#endif
#ifdef HAVE_WHRAG108
#define HAVE_RB500
#endif
#ifdef HAVE_TW6600
#define HAVE_RB500
#endif
#ifdef HAVE_CA8
#define HAVE_RB500
#endif
#ifndef HAVE_RB500
	/*
	 * Set QoS mode 
	 */
#if 0
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) >= 0) {
		int i, qos;
		caddr_t ifrdata;
		struct ethtool_drvinfo info;

		qos = (strcmp(nvram_safe_get("wl_wme"), "on")) ? 0 : 1;
		for (i = 1; i <= DEV_NUMIFS; i++) {
			ifr.ifr_ifindex = i;
			if (ioctl(s, SIOCGIFNAME, &ifr))
				continue;
			if (ioctl(s, SIOCGIFHWADDR, &ifr))
				continue;
			if (ifr.ifr_hwaddr.sa_family != ARPHRD_ETHER)
				continue;
			/*
			 * get flags 
			 */
			if (ioctl(s, SIOCGIFFLAGS, &ifr))
				continue;
			/*
			 * if up(wan not up yet at this point) 
			 */
			if (ifr.ifr_flags & IFF_UP) {
				ifrdata = ifr.ifr_data;
				bzero(&info, sizeof(info));
				info.cmd = ETHTOOL_GDRVINFO;
				ifr.ifr_data = (caddr_t) & info;
				if (ioctl(s, SIOCETHTOOL, &ifr) >= 0) {
					/*
					 * currently only need to set QoS to et devices 
					 */
#ifndef HAVE_80211AC //http://svn.dd-wrt.com/ticket/2943
					if (!strncmp(info.driver, "et", 2)) {
						ifr.ifr_data = (caddr_t) & qos;
						ioctl(s, SIOCSETCQOS, &ifr);
					}
#endif
				}
				ifr.ifr_data = ifrdata;
			}
		}
		close(s);
	}
#endif
#undef HAVE_RB500
#endif
	/*
	 * Sveasoft - set default IP gateway defined 
	 */
	if (strcmp(nvram_safe_get("lan_gateway"), "0.0.0.0"))
		eval("ip", "route", "add", "default", "via", nvram_safe_get("lan_gateway"), "dev", "br0");

#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
	for (c = 0; c < cnt; c++) {
		eval("wl", "-i", get_wl_instance_name(c), "vlan_mode", "0");
	}
#endif
	/*
	 * Bring up local host interface 
	 */
	config_loopback();

	/*
	 * Set additional lan static routes if need 
	 */
	start_set_routes();
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
	for (c = 0; c < cnt; c++) {
		eval("wl", "-i", get_wl_instance_name(c), "radio", nvram_nmatch("disabled", "wl%d_net_mode", c) ? "off" : "on");
#ifndef HAVE_80211AC
		eval("wl", "-i", get_wl_instance_name(c), "down");
		eval("wl", "-i", get_wl_instance_name(c), "up");
#endif
	}
#endif
	/*
	 * Disable wireless will cause diag led blink, so we want to stop it. 
	 */
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	if (check_hw_type() == BCM4712_CHIP) {
		diag_led(DIAG, STOP_LED);
		/*
		 * Light or go out the DMZ led even if there is no wan ip. 
		 */
		if (nvram_invmatch("dmz_ipaddr", "") && nvram_invmatchi("dmz_ipaddr", 0))
			diag_led(DMZ, START_LED);
		else
			diag_led(DMZ, STOP_LED);
	}
#endif
	{
		char word[256];
		br_set_stp_state("br0", getBridgeSTP("br0", word));
		genHosts();
	}
#ifdef HAVE_MICRO
	br_shutdown();
#endif

#ifdef HAVE_DHDAP
	int ifs = get_wl_instances();
	int i;
	char *vifs;
	char *nxt;
	char var[80];

	for (i = 0; i < ifs; i++) {
		vifs = nvram_nget("wl%d_vifs", i);
		if (vifs != NULL) {
			foreach(var, vifs, nxt)
			{
				eval("wl", "-i", var, "bss", "up");
			}
		}
	}
#endif
#if defined(HAVE_R9000) || defined(HAVE_IPQ806X)
	start_postnetwork();
#endif
}

void stop_lan(void)
{
	char *lan_ifname = nvram_safe_get("lan_ifname");
	char name[80], *next;

	cprintf("%s\n", lan_ifname);
	/*
	 * Bring down LAN interface 
	 */
	ifconfig(lan_ifname, 0, NULL, NULL);
#ifdef HAVE_MICRO
	br_init();
#endif

#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
	int cnt = get_wl_instances();
	int c;
	char vifs_name[32];

	for (c = 0; c < cnt; c++) {
		sprintf(vifs_name, "wl%d_vifs", c);
		char *vifs = nvram_safe_get(vifs_name);
		foreach(name, vifs, next)
		{
			br_del_interface(lan_ifname, vifs);
			ifconfig(vifs, 0, NULL, NULL);
		}
	}

#endif
	/*
	 * Bring down bridged interfaces 
	 */
	if (strncmp(lan_ifname, "br", 2) == 0) {
		char *lanifnames = nvram_safe_get("lan_ifnames");
		foreach(name, lanifnames, next)
		{
			if (nvram_match("wan_ifname", name))
				continue;
			if (!ifexists(name))
				continue;
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
			wlconf_down(name);
#endif
			ifconfig(name, 0, NULL, NULL);
			br_del_interface(lan_ifname, name);
		}
		br_del_bridge(lan_ifname);
	}
	/*
	 * Bring down specific interface 
	 */
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
	else if (strcmp(lan_ifname, ""))
		wlconf_down(lan_ifname);
#endif
#ifdef HAVE_MICRO
	br_shutdown();
#endif

	cprintf("done\n");
}

#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

int wan_valid(char *ifname)
{
	char name[80], *next;

	foreach(name, nvram_safe_get("wan_ifnames"), next) if (ifname && !strcmp(ifname, name)) return 1;

	if (getSTA() && !strcmp(getSTA(), ifname))
		return 1;

	return 0;
}

void start_force_to_dial(void);

static void vdsl_fuckup(char *ifname)
{
	fprintf(stderr, "[VDSL FUCK]\n");
	sleep(20);
	int s;
	struct ifreq ifr;
	char mac[32];
	char eabuf[32];
	eval("ifconfig", ifname, "down");
	if (nvram_exists("wan_hwaddr"))
		strcpy(mac, nvram_safe_get("wan_hwaddr"));
	else
		strcpy(mac, get_hwaddr(ifname, eabuf));
	MAC_SUB(mac);
	set_hwaddr(ifname, mac);
	eval("ifconfig", ifname, "up");
}

void run_wan(int status)
{
	FILE *fp;
	char wan_if_buffer[33];
	char *wan_ifname = safe_get_wan_face(wan_if_buffer);
	char *wan_proto = nvram_safe_get("wan_proto");
	int s;
	struct ifreq ifr;
	if (isClient()) {
		char *ifn = getSTA();
		int count = 10;
		while ((count--) > 0) // wait until wan is available (10 sek max)
		{
			if (ifexists(ifn)) {
				break;
			}
			sleep(1);
		}
	}
	rmmod("n_hdlc");

	if (strcmp(wan_ifname, "wwan0")) {
		eval("ifconfig", nvram_safe_get("wan_ifname"), "allmulti", "promisc");
	}

	start_firewall(); // start firewall once, to fix problem with rules which should exist even before wan is up
	// wan test mode
	if (nvram_matchi("wan_testmode", 1)) {
		status = 0; // avoid redialing
		fprintf(stderr, "[SERVICE WAN] testmode\n");
	}
#ifdef HAVE_PPPOE
	char *pppoe_wan_ifname = nvram_safe_get("wan_ifname");

#ifdef HAVE_MULTICAST
	if ((!nvram_matchi("dtag_vlan8", 1) && nvram_matchi("dtag_bng", 1)) || nvram_matchi("wan_vdsl", 0))
		stop_igmprt();
#endif
#ifdef HAVE_UDPXY
	if (!nvram_matchi("udpxy_enable", 1))
		stop_udpxy();
#endif

#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
	if (getWET()) {
		dns_to_resolv();
		stop_process_monitor();
		start_process_monitor();
		return;
	}
	if (isClient()) {
		pppoe_wan_ifname = getSTA();
	}
#else
	if (isClient()) {
		pppoe_wan_ifname = getSTA();
		int count = 10;
		while ((count--) > 0) {
		}
	}
#endif
#endif
	// fprintf(stderr,"%s %s\n", wan_ifname, wan_proto);

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return;
	/*
	 * Check PPPoE version, RP or linksys 
	 */
	char *ethname = wan_ifname;
#ifdef HAVE_PPPOE
	if (nvram_match("wan_proto", "pppoe"))
		ethname = pppoe_wan_ifname;
	else
#endif
#ifdef HAVE_PPPOEDUAL
		if (nvram_match("wan_proto", "pppoe_dual"))
		ethname = pppoe_wan_ifname;
#endif
#ifdef HAVE_L2TP
	if (nvram_match("wan_proto", "l2tp"))
		ethname = pppoe_wan_ifname;
	else
#endif
#ifdef HAVE_PPTP
		if (nvram_match("wan_proto", "pptp"))
		ethname = pppoe_wan_ifname;
#endif

	strncpy(ifr.ifr_name, ethname, IFNAMSIZ);

	/*
	 * Set WAN hardware address before bringing interface up 
	 */
	bzero(ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);

	ifconfig(ethname, 0, NULL, NULL);
	// fprintf(stderr,"%s %s\n", wan_ifname, wan_proto);
	char *wlifname = getSTA();

	if (!wlifname) {
		wlifname = getWET();
	}

	char mac[20];

	if (nvram_matchi("mac_clone_enable", 1) && nvram_invmatch("def_hwaddr", "00:00:00:00:00:00") &&
	    nvram_invmatch("def_hwaddr", "")) {
		ether_atoe(nvram_safe_get("def_hwaddr"), ifr.ifr_hwaddr.sa_data);
	} else {
		if (wlifname &&
		    (!strcmp(ethname, wlifname) || nvram_match("wan_proto", "l2tp") || nvram_match("wan_proto", "pppoe") ||
		     nvram_match("wan_proto", "pppoe_dual") || nvram_match("wan_proto", "pptp"))) // sta mode
		{
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
			int instance = get_wl_instance(wlifname);
			getWirelessMac(mac, instance);
#else
			getWirelessMac(mac, 0);
#endif
			ether_atoe(mac, ifr.ifr_hwaddr.sa_data);
		} else {
#ifdef HAVE_X86
			ioctl(s, SIOCGIFHWADDR, &ifr);
#else
			getWANMac(mac);
			ether_atoe(mac, ifr.ifr_hwaddr.sa_data);
#endif
		}
	}

	if (!nvram_match("wan_proto", "disabled")) {
		if (memcmp(ifr.ifr_hwaddr.sa_data, "\0\0\0\0\0\0", ETHER_ADDR_LEN)) {
			ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)

			if (wlifname && !strcmp(ethname, wlifname))
				eval("wl", "-i", ethname, "down");
			else if (strcmp(ethname, "br0"))
				eval("ifconfig", ethname, "down");

			ioctl(s, SIOCSIFHWADDR, &ifr);
#else
			if (!wlifname && strcmp(wan_ifname, "br0") && strcmp(wan_ifname, "wwan0")) {
				eval("ifconfig", ethname, "down");
				ioctl(s, SIOCSIFHWADDR, &ifr);
			}
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
			if (wlifname && !strcmp(ethname, wlifname)) {
				eval("wl", "-i", ethname, "up");
				config_macs(ethname);
			}
#endif
			cprintf("Write WAN mac successfully\n");
		} else
			perror("Write WAN mac fail : \n");
	}
	// fprintf(stderr,"%s %s\n", wan_ifname, wan_proto);

	/*
	 * Set MTU 
	 */
	init_mtu(wan_proto); // add by honor 2002/12/27
	// fprintf(stderr,"%s %s\n", wan_ifname, wan_proto);

	// Set our Interface to the right MTU
	int mtu = atoi(getMTU(ethname));
#ifdef HAVE_PPPOE
	if (nvram_match("wan_proto", "pppoe")) {
		ifr.ifr_mtu = mtu; // default ethernet frame size
	} else
#endif
#ifdef HAVE_PPPOEDUAL
		if (nvram_match("wan_proto", "pppoe_dual")) {
		ifr.ifr_mtu = mtu; // default ethernet frame size
	} else
#endif
#ifdef HAVE_PPTP
		if (nvram_match("wan_proto", "pptp")) {
		ifr.ifr_mtu = mtu; // default ethernet frame size
	} else
#endif
#ifdef HAVE_L2TP
		if (nvram_match("wan_proto", "l2tp")) {
		ifr.ifr_mtu = mtu; // default ethernet frame size
	} else
#endif
	{
		int smtu = nvram_geti("wan_mtu");
		if (smtu == 1500)
			smtu = mtu;
		ifr.ifr_mtu = smtu;
	}
	// fprintf(stderr,"set mtu for %s to %d\n",ifr.ifr_name,ifr.ifr_mtu);
	ioctl(s, SIOCSIFMTU, &ifr);
	eval("ifconfig", wan_ifname, "txqueuelen", getTXQ(wan_ifname));

	if (strcmp(wan_proto, "disabled") == 0) {
		char eabuf[32];
		if (get_hwaddr(ifr.ifr_name, eabuf)) {
			nvram_set("wan_hwaddr", eabuf);
		}
		close(s);
		wan_done(wan_ifname);
		return;
	}

	/*
	 * Bring up WAN interface 
	 */
	ifconfig(ethname, IFUP, NULL, NULL);
	if (nvram_match("wl0_mode", "infra")) {
		eval("wl", "infra", "0");
		eval("wl", "ssid", nvram_safe_get("wl0_ssid"));
	}

	set_host_domain_name();

	// Remove the current value of pppd_pppifname
	nvram_set("pppd_pppifname", "");

#ifdef HAVE_3G
	nvram_set("3gdata", "");
#endif
	/*
	 * Configure WAN interface 
	 */
#ifdef HAVE_3G
	if ((strcmp(wan_proto, "3g") == 0)) {
		if (!nvram_matchi("usb_enable", 1)) {
			nvram_seti("usb_enable",
				   1); //  simply enable it, otherwise 3g might not work
			nvram_async_commit();
			load_drivers(1);
		}

		stop_dhcpc();
#ifdef HAVE_PPTP
		stop_pptp();
#endif
		mkdir("/tmp/ppp", 0777);
		symlink("/sbin/rc", "/tmp/ppp/ip-up");
		symlink("/sbin/rc", "/tmp/ppp/ip-down");
		unlink("/tmp/ppp/log");
		unlink("/tmp/ppp/connect-log");
		unlink("/tmp/ppp/set-pppoepid");
		get3GControlDevice();
		char *controldevice = nvram_safe_get("3gcontrol");
		int timeout = 5;
		char wsel[16];
		char wsbuf[30];
		sprintf(wsel, "");

		// #if defined(HAVE_CAMBRIA) || defined(HAVE_LAGUNA)
		int wan_select = 1;
		if (*(nvram_safe_get("wan_select"))) {
			wan_select = atoi(nvram_safe_get("wan_select"));
			if (wan_select != 1) {
				sprintf(wsel, "_%d", wan_select);
			}
		}
#ifdef HAVE_LIBMBIM
		if (controldevice && !strcmp(controldevice, "mbim")) {
#ifdef HAVE_REGISTER
			if (registered_has_cap(27))
#else
			if (0)
#endif
			{
				if (pidof("mbim-connect.sh") < 0) {
					dd_loginfo("mbim", "STARTING mbim-status.sh\n");
					sysprintf("mbim-connect.sh");
				}
				if (status != REDIAL) {
					start_redial();
				}
			} else {
				nvram_set(
					"wan_3g_mode",
					"Software feature not licenced. Please contact the vendor for a valid licence<br><a href=\"/register.asp\">Use Registration here</A>");
			}
		} else
#endif
// #endif
#ifdef HAVE_UQMI
			if (controldevice && (!strcmp(controldevice, "qmi") || !strcmp(controldevice, "qmiraw"))) {
			/* disconnect network */
			int clientid = 0;
			FILE *fp = fopen("/tmp/qmi-clientid", "rb");
			if (fp) {
				fscanf(fp, "%d", &clientid);
				fclose(fp);
				char wdsid[32];
				sprintf(wdsid, "wds,%d", clientid);
				eval("uqmi", "-d", "/dev/cdc-wdm0", "--set-client-id", wdsid, "--release-client-id", "wds");
			}
			clientid = 0;
			sprintf(wsbuf, "wan_roaming%s", wsel);
			if (nvram_match(wsbuf, "1")) {
				sysprintf("/usr/sbin/uqmi -d /dev/cdc-wdm0 --set-network-roaming any");
			} else {
				sysprintf("/usr/sbin/uqmi -d /dev/cdc-wdm0 --set-network-roaming off");
			}
			sprintf(wsbuf, "wan_conmode%s", wsel);
			if (nvram_match(wsbuf, "6"))
				sysprintf("uqmi -d /dev/cdc-wdm0 --set-network-modes lte");
			if (nvram_match(wsbuf, "4"))
				sysprintf("uqmi -d /dev/cdc-wdm0 --set-network-modes gsm,umts");
			if (nvram_match(wsbuf, "3"))
				sysprintf("uqmi -d /dev/cdc-wdm0 --set-network-modes umts,gsm");
			if (nvram_match(wsbuf, "2"))
				sysprintf("uqmi -d /dev/cdc-wdm0 --set-network-modes gsm");
			if (nvram_match(wsbuf, "1"))
				sysprintf("uqmi -d /dev/cdc-wdm0 --set-network-modes umts");
			if (nvram_match(wsbuf, "0"))
				sysprintf("uqmi -d /dev/cdc-wdm0 --set-network-modes all");

			//set pin
			sprintf(wsbuf, "wan_pin%s", wsel);
			if (*(nvram_safe_get(wsbuf))) {
				sysprintf("/usr/sbin/uqmi -d /dev/cdc-wdm0 --verify-pin1 %s >/tmp/qmiping.log 2>&1",
					  nvram_safe_get(wsbuf));
			}
			//set apn and dial
			fp = popen("/usr/sbin/uqmi -d /dev/cdc-wdm0 --get-client-id wds", "r");
			fscanf(fp, "%d", &clientid);
			pclose(fp);
			fp = fopen("/tmp/qmi-clientid", "wb");
			fprintf(fp, "%d", clientid);
			fclose(fp);

			fp = fopen("/tmp/qmi-connect.sh", "wb");
			fprintf(fp, "#!/bin/sh\n");
			fprintf(fp, "REG=0\n");
			fprintf(fp, "COUNT=2\n");
			fprintf(fp, "while [ $REG = 0 ]\n");
			fprintf(fp, "do\n");
			fprintf(fp, "if [ $COUNT = 0 ]\n");
			fprintf(fp, "then\n");
			fprintf(fp, " exit\n");
			fprintf(fp, "fi\n");
			fprintf(fp, "CLIENTID=`cat /tmp/qmi-clientid`\n");
			fprintf(fp,
				"REG=`uqmi -d /dev/cdc-wdm0 --set-client-id wds,${CLIENTID} --keep-client-id wds --get-serving-system|grep registered|wc -l`\n");
			fprintf(fp, "if [ $REG = 0 ]\n");
			fprintf(fp, "then\n");
			fprintf(fp, "echo \"not yet registered to the network (${COUNT})\" | logger -t wan_dial\n");
			fprintf(fp, "fi\n");
			fprintf(fp, "COUNT=$(($COUNT - 1))\n");
			fprintf(fp, "sleep 5\n");
			fprintf(fp, "done\n");
			sprintf(wsbuf, "ppp_username%s", wsel);
			char *username = nvram_safe_get(wsbuf);
			sprintf(wsbuf, "ppp_passwd%s", wsel);
			char *passwd = nvram_safe_get(wsbuf);
			sprintf(wsbuf, "wan_apn%s", wsel);
			if (*username && *passwd) {
				fprintf(fp,
					"uqmi -d /dev/cdc-wdm0 --set-client-id wds,${CLIENTID} --start-network --apn %s --auth-type both --username %s --password %s --keep-client-id wds\n",
					nvram_safe_get(wsbuf), username, passwd);
			} else {
				fprintf(fp,
					"uqmi -d /dev/cdc-wdm0 --set-client-id wds,${CLIENTID} --start-network --apn %s --auth-type both --keep-client-id wds\n",
					nvram_safe_get(wsbuf));
			}
			if (!strcmp(controldevice, "qmiraw"))
				sysprintf("echo Y > /sys/class/net/wwan0/qmi/raw_ip");
			fprintf(fp,
				"uqmi -d /dev/cdc-wdm0 --set-client-id wds,${CLIENTID} --keep-client-id wds --get-data-status | grep '^\"connected' | wc -l >/tmp/qmistatus\n");
			fprintf(fp, "(echo QMISTATUS ; cat /tmp/qmistatus) | logger\n");
			// fprintf(fp,"ifconfig wwan0 up\n");
			// fprintf(fp,"ln -s /sbin/rc /tmp/udhcpc\n");
			// fprintf(fp,"udhcpc -i wwan0 -p /var/run/udhcpc.pid -s /tmp/udhcpc\n");
			fclose(fp);
			chmod("/tmp/qmi-connect.sh", 0700);
			sysprintf("/tmp/qmi-connect.sh >/tmp/qmi-connect.out 2>&1");
			eval("ifconfig", "wwan0", "up");
			run_dhcpc("wwan0", NULL, NULL, 1, 0, 0);
			if (status != REDIAL) {
				start_redial();
			}

		} else
#elif HAVE_LIBQMI
		if (controldevice && (!strcmp(controldevice, "qmi") || !strcmp(controldevice, "qmiraw"))) {
			if (nvram_matchi("wan_conmode", 6))
				sysprintf("qmicli -d /dev/cdc-wdm0 --nas-set-network-mode=LTE");
			//              if (nvram_match("wan_conmode","5")) //unsupported and useless. i dont know what that means
			//                  sysprintf("qmicli -d /dev/cdc-wdm0 --nas-set-network-mode=LTE");
			if (nvram_matchi("wan_conmode", 4))
				sysprintf("qmicli -d /dev/cdc-wdm0 --nas-set-network-mode=GSMUMTS");
			if (nvram_matchi("wan_conmode", 3))
				sysprintf("qmicli -d /dev/cdc-wdm0 --nas-set-network-mode=UMTSGSM");
			if (nvram_matchi("wan_conmode", 2))
				sysprintf("qmicli -d /dev/cdc-wdm0 --nas-set-network-mode=GSM");
			if (nvram_matchi("wan_conmode", 1))
				sysprintf("qmicli -d /dev/cdc-wdm0 --nas-set-network-mode=UMTS");
			if (nvram_matchi("wan_conmode", 0))
				sysprintf("qmicli -d /dev/cdc-wdm0 --nas-set-network-mode=ANY");

			//set pin
			sysprintf("qmicli -d /dev/cdc-wdm0 --dms-uim-verify-pin=PIN,%s", nvram_safe_get("wan_pin"));
			//set apn and dial
			FILE *fp = fopen("/tmp/qmi-network.conf", "wb");
			fprintf(fp, "APN=%s", nvram_safe_get("wan_apn"));
			if (*(nvram_safe_get("ppp_username")) && *(nvram_safe_get("ppp_passwd"))) {
				fprintf(fp, ",BOTH,%s,%s\n", nvram_safe_get("ppp_username"), nvram_safe_get("ppp_passwd"));
			} else {
				fprintf(fp, "\n");
			}
			fclose(fp);

			eval("qmi-network", "/dev/cdc-wdm0",
			     "stop"); //release it before
			eval("qmi-network", "/dev/cdc-wdm0", "start");
			if (!strcmp(controldevice, "qmiraw"))
				sysprintf("echo Y > /sys/class/net/wwan0/qmi/raw_ip");
			eval("ifconfig", "wwan0", "up");
			run_dhcpc("wwan0", NULL, NULL, 1, 0, 0);
			if (status != REDIAL) {
				start_redial();
			}

		} else
#endif
			if (controldevice && !strcmp(controldevice, "hso")) {

		} else {
			if (nvram_match("3gnmvariant", "1")) {
				sleep(3);
				sprintf(wsbuf, "wan_roaming%s", wsel);
				if (nvram_match(wsbuf, "1")) {
					sysprintf("COMGTATC='AT^SYSCONFIG=16,3,1,4' comgt -s -d %s /etc/comgt/atcommand.comgt",
						  controldevice);
				} else {
					sysprintf("COMGTATC='AT^SYSCONFIG=16,3,0,4' comgt -s -d %s /etc/comgt/atcommand.comgt",
						  controldevice);
				}
			}
			/* Set APN Necseeary before PIN */
			sprintf(wsbuf, "wan_apn%s", wsel);
			if (*(nvram_safe_get(wsbuf))) {
				sprintf(wsbuf, "wan_dial%s", wsel);
				if (!nvram_match(wsbuf, "2")) {
					sprintf(wsbuf, "wan_apn%s", wsel);
					if (nvram_match("3gdata", "sierradirectip")) {
						sysprintf(
							"export COMGTAPN=\"%s\";export COMGTPROF=3 ; comgt -s -d %s /etc/comgt/dip-apn.comgt",
							nvram_safe_get(wsbuf), controldevice);
					} else {
						sysprintf("export COMGTAPN=\"%s\";comgt -s -d %s APN", nvram_safe_get(wsbuf),
							  controldevice);
					}
				}
			}

			/* init PIN */
			sprintf(wsbuf, "wan_pin%s", wsel);
			if (*(nvram_safe_get(wsbuf)))
				sysprintf("export COMGTPIN=%s;comgt -s -d %s PIN", nvram_safe_get(wsbuf), controldevice);
			// set netmode, even if it is auto, should be set every time, the stick might save it
			// some sticks, don't save it ;-)
			if (*(nvram_safe_get("3gnmvariant"))) {
				int netmode;
				int netmodetoggle;
				sprintf(wsbuf, "wan_conmode%s", wsel);
				netmode = atoi(nvram_default_get(wsbuf, "0"));
				if (netmode == 5) {
					if (*(nvram_safe_get("3gnetmodetoggle"))) {
						netmodetoggle = nvram_geti("3gnetmodetoggle");
						if (netmodetoggle == 1) {
							// 2g
							netmode = 2;
							nvram_seti("3gnetmodetoggle", 0);
						} else {
							// auto
							netmode = 0;
							nvram_seti("3gnetmodetoggle", 1);
						}
					} else {
						// auto
						netmode = 0;
						nvram_seti("3gnetmodetoggle", 1);
					}
				}
				if (netmode == 0 || netmode == 6) {
					printf("3g setting netmode with variant %s to mode %d\n", nvram_safe_get("3gnmvariant"), 4);
					sysprintf(
						"export COMGNMVARIANT=%s;export COMGTNM=%d;comgt -d %s -s /etc/comgt/netmode.comgt >/tmp/comgt-netmode.out",
						nvram_safe_get("3gnmvariant"), 4, controldevice);
					sysprintf("comgt -d %s -s /etc/comgt/cgatt.comgt\n", controldevice);
				}
				sysprintf(
					"export COMGNMVARIANT=%s;export COMGTNM=%d;comgt -d %s -s /etc/comgt/netmode.comgt >/tmp/comgt-netmode.out",
					nvram_safe_get("3gnmvariant"), netmode, controldevice);
				printf("3g setting netmode with variant %s to mode %d\n", nvram_safe_get("3gnmvariant"), netmode);
			}
			// Wait for device to attach to the provider network
			int retcgatt = 0;
			// Lets open option file and enter all the parameters.
			sprintf(wsbuf, "ppp_username%s", wsel);
			char *username = nvram_safe_get(wsbuf);
			sprintf(wsbuf, "ppp_passwd%s", wsel);
			char *passwd = nvram_safe_get(wsbuf);
			if (nvram_match("3gdata", "sierradirectip")) {
				sysprintf("comgt -d %s -s /etc/comgt/hangup-dip.comgt\n", controldevice);
				// eval("ifconfig", "wwan0", "up");
				if (*username && *passwd) {
					if (!strcmp(username, "chap")) {
						sysprintf(
							"export COMGTPASSWORD=\"%s\"; export COMGTUSERNAME=\"%s\";export COMGTPROF=3; comgt -s -d %s /etc/comgt/dip-auth-chap.comgt",
							passwd, username, controldevice);
					} else {
						sysprintf(
							"export COMGTPASSWORD=\"%s\"; export COMGTUSERNAME=\"%s\";export COMGTPROF=3; comgt -s -d %s /etc/comgt/dip-auth.comgt",
							passwd, username, controldevice);
					}
				}

				sysprintf("export COMGTPROF=3 ; comgt -s -d %s /etc/comgt/dip-prof.comgt", controldevice);
				sysprintf("comgt -d %s -s /etc/comgt/dial-dip.comgt >/tmp/comgt-dial.out 2>&1\n", controldevice);
				rmmod("sierra_net");
				insmod("sierra_net");
				// sysprintf("echo 1 > /proc/sys/net/ipv6/conf/wwan0/disable_ipv6\n");
				run_dhcpc("wwan0", NULL, NULL, 1, 0, 0);
				if (status != REDIAL) {
					start_redial();
				}
			} else {
				if (*username && *passwd) {
					if ((fp = fopen("/tmp/ppp/chap-secrets", "w"))) {
						fprintf(fp, "\"%s\" * \"%s\" *\n", username, passwd);
						fclose(fp);
						chmod("/tmp/ppp/chap-secrets", 0600);
					}
					if ((fp = fopen("/tmp/ppp/pap-secrets", "w"))) {
						fprintf(fp, "\"%s\" * \"%s\" *\n", username, passwd);
						fclose(fp);
						chmod("/tmp/ppp/pap-secrets", 0600);
					}
				}
				fp = fopen("/tmp/ppp/options.pppoe", "w");
				if (*username && *passwd) {
					fprintf(fp, "chap-secrets /tmp/ppp/chap-secrets\n");
					fprintf(fp, "pap-secrets /tmp/ppp/pap-secrets\n");
				}

				fprintf(fp, "defaultroute\n");
				fprintf(fp, "usepeerdns\n");
				fprintf(fp, "noipdefault\n");
				fprintf(fp, "noauth\n");
				fprintf(fp, "ipcp-max-failure 30\n");
				if (nvram_match("mtu_enable", "1")) {
					if (nvram_geti("wan_mtu") > 0) {
						fprintf(fp, "mtu %s\n", nvram_safe_get("wan_mtu"));
						fprintf(fp, "mru %s\n", nvram_safe_get("wan_mtu"));
					}
				}
				fprintf(fp, "crtscts\n");
				fprintf(fp, "460800\n");
				sprintf(wsbuf, "wan_dial%s", wsel);
				char *dial = NULL;
				switch (nvram_geti(wsbuf)) {
				case 0:
					dial = "ATD*99***1#";
					break;
				case 1:
					dial = "ATD*99#";
					break;
				case 2:
					dial = "ATDT#777";
					break;
				case 3:
					dial = "ATD*99***3#";
					break;
				case 4:
					dial = "ATD*99***2#";
					break;
				case 5:
					dial = "ATD*99***4#";
					break;
				default:
					dial = "*99***1#";
					break;
				}
				fprintf(fp,
					"connect \"COMGTDIAL='%s' /usr/sbin/comgt -s -d %s /etc/comgt/dial.comgt >/tmp/comgt.out 2>&1\"\n",
					dial, nvram_safe_get("3gdata"));
				if (*username)
					fprintf(fp, "user '%s'\n", username);
				if (*passwd)
					fprintf(fp, "password '%s'\n", passwd);
				fprintf(fp, "%s\n", nvram_safe_get("3gdata"));

				fclose(fp);
				start_pppmodules();

				log_eval("pppd", "file", "/tmp/ppp/options.pppoe");

				/*
				 * Pretend that the WAN interface is up 
				 */
				if (nvram_matchi("ppp_demand", 1)) {
					/*
					 * Wait for ppp0 to be created 
					 */
					while (ifconfig("ppp0", IFUP, NULL, NULL) && timeout--)
						sleep(1);
					strncpy(ifr.ifr_name, "ppp0", IFNAMSIZ);

					/*
					 * Set temporary IP address 
					 */
					timeout = 3;
					while (ioctl(s, SIOCGIFADDR, &ifr) && timeout--) {
						perror("ppp0");
						printf("Wait ppp inteface to init (1) ...\n");
						sleep(1);
					};
					char client[32];

					nvram_set("wan_ipaddr", inet_ntop(AF_INET, &sin_addr(&ifr.ifr_addr), client, 16));
					nvram_set("wan_netmask", "255.255.255.255");

					/*
					 * Set temporary P-t-P address 
					 */
					timeout = 3;
					while (ioctl(s, SIOCGIFDSTADDR, &ifr) && timeout--) {
						perror("ppp0");
						printf("Wait ppp inteface to init (2) ...\n");
						sleep(1);
					}
					const char *peer = inet_ntop(AF_INET, &sin_addr(&ifr.ifr_dstaddr), client, 16);

					nvram_set("wan_gateway", peer);

					wan_done("ppp0");

					// if user press Connect" button from web, we must force to dial
					if (nvram_match("action_service", "start_3g")) {
						sleep(3);
						start_force_to_dial();
						nvram_unset("action_service");
					}
				} else {
					if (status != REDIAL) {
						start_redial();
					}
				}
			}
		}
	} else
#endif
#ifdef HAVE_PPPOE
		if ((strcmp(wan_proto, "pppoe") == 0)) {
		char username[80], passwd[80];
		char idletime[20], retry_num[20];

		snprintf(idletime, sizeof(idletime), "%d", nvram_geti("ppp_idletime") * 60);
		snprintf(retry_num, sizeof(retry_num), "%d", (nvram_geti("ppp_redialperiod") / 5) - 1);

		snprintf(username, sizeof(username), "%s", nvram_safe_get("ppp_username"));
		snprintf(passwd, sizeof(passwd), "%s", nvram_safe_get("ppp_passwd"));

		mkdir("/tmp/ppp", 0777);
		int timeout = 5;

		// Lets open option file and enter all the parameters.
		fp = fopen("/tmp/ppp/options.pppoe", "w");
		// rp-pppoe kernelmode plugin
		if (nvram_matchi("ppp_demand", 1))
			fprintf(fp, "connect true\n");

#if defined(HAVE_ADM5120) && !defined(HAVE_WP54G) && !defined(HAVE_NP28G)
		fprintf(fp, "plugin /lib/rp-pppoe.so\n");
#else
		fprintf(fp, "plugin /usr/lib/rp-pppoe.so\n");
#endif
		if (nvram_invmatch("pppoe_service", ""))
			fprintf(fp, " rp_pppoe_service %s", nvram_safe_get("pppoe_service"));
		if (nvram_invmatch("pppoe_host_uniq", ""))
			fprintf(fp, " host-uniq %s", nvram_safe_get("pppoe_host_uniq"));
		fprintf(fp, "\n");
		char vlannic[32];
		char tvnic[32];

		if (!strncmp(pppoe_wan_ifname, "vlan", 4)) {
			if (nvram_matchi("wan_vdsl", 1)) {
				char *ifn = enable_dtag_vlan(1);

				if (nvram_matchi("dtag_vlan8", 1) && nvram_matchi("dtag_bng", 0)) {
					sprintf(vlannic, "%s.0008", ifn);
					if (!ifexists(vlannic)) {
						eval("vconfig", "set_name_type", "DEV_PLUS_VID");
						eval("vconfig", "add", ifn, "8");
						eval("ifconfig", vlannic, "up");
					}
					nvram_set("tvnicfrom", vlannic);
					symlink("/sbin/rc", "/tmp/dhcpc_tv");
					run_dhcpc(vlannic, "/var/run/udhcpc_tv.pid", "/tmp/dhcpc_tv", 1, 0, 0);
				}
				sprintf(vlannic, "%s.0007", ifn);
				if (!ifexists(vlannic)) {
					eval("vconfig", "set_name_type", "DEV_PLUS_VID");
					eval("vconfig", "add", ifn, "7");
					eval("ifconfig", vlannic, "up");
				}

				fprintf(fp, "nic-%s\n", vlannic);
				//                              vdsl_fuckup(vlannic);   /* work around for DTAG DSLAMS */
			} else {
				char *ifn = enable_dtag_vlan(0);

				sprintf(vlannic, "%s.0007", ifn);
				if (ifexists(vlannic))
					eval("vconfig", "rem", vlannic);
				sprintf(vlannic, "%s.0008", ifn);
				if (ifexists(vlannic))
					eval("vconfig", "rem", vlannic);
				fprintf(fp, "nic-%s\n", pppoe_wan_ifname);
			}

		} else {
			if (nvram_matchi("wan_vdsl", 1)) // Deutsche Telekom
			// VDSL2 Vlan 7 Tag
			{
				if (nvram_matchi("dtag_vlan8", 1) && nvram_matchi("dtag_bng", 0)) {
					sprintf(vlannic, "%s.0008", pppoe_wan_ifname);
					if (!ifexists(vlannic)) {
						eval("vconfig", "set_name_type", "DEV_PLUS_VID");
						eval("vconfig", "add", pppoe_wan_ifname, "8");
						eval("ifconfig", vlannic, "up");
					}
					nvram_set("tvnicfrom", vlannic);
					symlink("/sbin/rc", "/tmp/dhcpc_tv");
					run_dhcpc(vlannic, "/var/run/udhcpc_tv.pid", "/tmp/dhcpc_tv", 1, 0, 0);
				}
				sprintf(vlannic, "%s.0007", pppoe_wan_ifname);
				if (!ifexists(vlannic)) {
					eval("vconfig", "set_name_type", "DEV_PLUS_VID");
					eval("vconfig", "add", pppoe_wan_ifname, "7");
					eval("ifconfig", vlannic, "up");
				}
				fprintf(fp, "nic-%s\n", vlannic);
				//                              vdsl_fuckup(vlannic);   /* work around for DTAG DSLAMS */
			} else {
				sprintf(vlannic, "%s.0008", pppoe_wan_ifname);
				if (ifexists(vlannic))
					eval("vconfig", "rem", vlannic);
				sprintf(vlannic, "%s.0007", pppoe_wan_ifname);
				if (ifexists(vlannic))
					eval("vconfig", "rem", vlannic);
				fprintf(fp, "nic-%s\n", pppoe_wan_ifname);
			}
		}

		// Those are default options we use + user/passwd
		// By using user/password options we dont have to deal with chap/pap
		// secrets files.
		if (nvram_matchi("ppp_compression", 1)) {
			fprintf(fp, "mppc\n");
		} else {
			fprintf(fp, "noccp\n");
			fprintf(fp, "nomppc\n");
		}
		fprintf(fp, "noipdefault\n" //
			    "noauth\n" //
			    "defaultroute\n" //
			    "noaccomp\n" //
			    "nobsdcomp\n" //
			    "nodeflate\n"
			    // "debug\n"
			    // "maxfail 0\n"
			    // "nocrtscts\n"
			    // "sync\n"
			    // "local\n"
			    // "noixp\n"
			    // "lock\n"
			    // "noproxyarp\n"
			    // "ipcp-accept-local\n"
			    // "ipcp-accept-remote\n"
			    // "nodetach\n"
			    "nopcomp\n");
		// "novj\n"
		// "novjccomp\n");
		if (nvram_invmatch("ppp_mppe", ""))
			fprintf(fp, "%s\n", nvram_safe_get("ppp_mppe"));
		else
			fprintf(fp, "nomppe\n");
		if (nvram_matchi("ppp_mlppp", 1))
			fprintf(fp, "mp\n");
		fprintf(fp,
			"usepeerdns\nuser '%s'\n" //
			"password '%s'\n",
			username, passwd);

		// This is a tricky one. When used it could improve speed of PPPoE
		// but not all ISP's can support it.
		// default-asyncmap escapes all control characters. By using asyncmap
		// 0 PPPD will not escape any control characters
		// Not all ISP's can handle this. By default use default-asyncmap
		// and if ppp_asyncmap=1 do not escape
		if (nvram_matchi("ppp_asyncmap", 1))
			fprintf(fp, "asyncmap 0\n");
		else
			fprintf(fp, "default-asyncmap\n");

		// Allow users some control on PPP interface MTU and MRU
		// If pppoe_ppp_mtu > 0 will set mtu of pppX interface to the value
		// in the nvram variable
		// If pppoe_ppp_mru > 0 will set mru of pppX interface to the value
		// in the nvram variable
		// if none is specified PPPD will autonegotiate the values with ISP
		// (sometimes not desirable)
		// Do not forget this should be at least 8 bytes less then physycal
		// interfaces mtu.

		// if MRU is not Auto force MTU/MRU of interface to value selected by
		// theuser on web page
		if (nvram_matchi("mtu_enable", 1)) {
			if (nvram_geti("wan_mtu") > 0) {
				fprintf(fp, "mtu %s\n", nvram_safe_get("wan_mtu"));
				fprintf(fp, "mru %s\n", nvram_safe_get("wan_mtu"));
			}

		} else {
			// If MRU set to Auto we still allow custom MTU/MRU settings for
			// expirienced users
			if (nvram_invmatch("pppoe_ppp_mtu", ""))
				if (nvram_geti("pppoe_ppp_mtu") > 0)
					fprintf(fp, "mtu %s\n", nvram_safe_get("pppoe_ppp_mtu"));
			if (nvram_invmatch("pppoe_ppp_mru", ""))
				if (nvram_geti("pppoe_ppp_mru") > 0)
					fprintf(fp, "mru %s\n", nvram_safe_get("pppoe_ppp_mru"));
		}

		// Allow runtime debugging
		if (nvram_matchi("ppp_debug", 1))
			fprintf(fp, "debug\n");

		// Demand dial.. This is not pretty.
		// The first problems i see is that if connection is lost it would
		// take PPPoE (idletime * 2) * 3 to notice it.
		// In other words if idle is set to 30 seconds, it would take 30*2*3
		// (180) seconds to detect the lost connection.
		// We have to increase the lcp-echo-interval to idletime*2 so that we
		// do not upset the idletime counter.
		// When not using demand dialing, it only takes 15 seconds to detect
		// the lost connection.
		if (nvram_matchi("ppp_demand", 1))
			fprintf(fp,
				"demand\n" //
				"idle %s\n" //
				"10.112.112.112:10.112.112.113\n" //
				"lcp-echo-interval %d\n" //
				"lcp-echo-failure 10\n" //
				"lcp-echo-adaptive\n" //
				"ipcp-accept-remote\n" //
				"ipcp-accept-local\n" //
				"ktune\n",
				idletime, atoi(idletime) * 2);
		else
			fprintf(fp, "persist\n" //
				    "lcp-echo-interval 3\n" //
				    "lcp-echo-failure 20\n" //
				    "lcp-echo-adaptive\n");
#ifdef HAVE_IPV6
		if (nvram_matchi("ipv6_enable", 1)) {
			fprintf(fp, "+ipv6\n");
			if (nvram_matchi("ppp_demand", 1))
				fprintf(fp, "ipv6 ,::123\n");
		}
#endif

		fclose(fp);

		symlink("/sbin/rc", "/tmp/ppp/ip-up");
		symlink("/sbin/rc", "/tmp/ppp/ip-down");
		unlink("/tmp/ppp/log");

		// Clean pppoe linksys client files - Added by ice-man (Wed Jun 1)
		unlink("/tmp/ppp/connect-log");
		unlink("/tmp/ppp/set-pppoepid");

		stop_dhcpc();
#ifdef HAVE_PPTP
		stop_pptp();
#endif
		stop_process("pppd", "PPP daemon");
		start_pppmodules();
		log_eval("pppd", "file", "/tmp/ppp/options.pppoe");

		// This is horrible.
		// What if pppoe recconects with ppp1?

		/*
		 * Pretend that the WAN interface is up 
		 */
		if (nvram_matchi("ppp_demand", 1)) {
			/*
			 * Wait for ppp0 to be created 
			 */
			while (ifconfig("ppp0", IFUP, NULL, NULL) && timeout--)
				sleep(1);
			strncpy(ifr.ifr_name, "ppp0", IFNAMSIZ);

			/*
			 * Set temporary IP address 
			 */
			timeout = 3;
			while (ioctl(s, SIOCGIFADDR, &ifr) && timeout--) {
				perror("ppp0");
				printf("Wait ppp inteface to init (1) ...\n");
				sleep(1);
			};
			char client[32];

			nvram_set("wan_ipaddr", inet_ntop(AF_INET, &sin_addr(&ifr.ifr_addr), client, 16));
			nvram_set("wan_netmask", "255.255.255.255");

			/*
			 * Set temporary P-t-P address 
			 */
			timeout = 3;
			while (ioctl(s, SIOCGIFDSTADDR, &ifr) && timeout--) {
				perror("ppp0");
				printf("Wait ppp inteface to init (2) ...\n");
				sleep(1);
			}
			const char *peer = inet_ntop(AF_INET, &sin_addr(&ifr.ifr_dstaddr), client, 16);

			nvram_set("wan_gateway", peer);

			wan_done("ppp0");

			// if user press Connect" button from web, we must force to dial
			if (nvram_match("action_service", "start_pppoe")) {
				sleep(3);
				start_force_to_dial();
				nvram_unset("action_service");
			}
		} else {
			if (status != REDIAL) {
				start_redial();
			}
		}
	} else
#endif
#ifdef HAVE_PPPOEDUAL
		if (strcmp(wan_proto, "pppoe_dual") == 0) {
		if (nvram_matchi("pptp_iptv", 1))
			nvram_set("tvnicfrom", nvram_safe_get("wan_iface"));
		else
			nvram_unset("tvnicfrom");

		if (nvram_matchi("pptp_use_dhcp", 1)) {
			nvram_set("wan_get_dns", "");
			char *pppoe_dual_ifname = nvram_safe_get("wan_ifname");
			if (isClient()) {
				pppoe_dual_ifname = getSTA();
			}

			run_dhcpc(pppoe_dual_ifname, NULL, NULL, 1, 0, 0);
		} else {
			char *wan_iface = nvram_safe_get("wan_iface");
			struct dns_lists *dns_list = NULL;
			int i = 0;

			if (isClient())
				wan_iface = getSTA();
			if (wan_iface != NULL)
				ifconfig(wan_iface, IFUP, nvram_safe_get("wan_ipaddr_static"),
					 nvram_safe_get("wan_netmask_static"));

			dns_to_resolv();
			dns_list = get_dns_list(0);

			if (dns_list) {
				for (i = 0; i < dns_list->num_servers; i++)
					route_add(wan_iface, 0, dns_list->dns_server[i].ip,
						  nvram_safe_get("pptp_wan_gateway_static"), "255.255.255.255");
				free_dns_list(dns_list);
			}
			route_del(wan_iface, 0, "0.0.0.0", nvram_safe_get("pptp_wan_gateway_static"), "0.0.0.0");

			start_firewall();
			run_pppoe_dual(status);
		}
	} else
#endif
#ifdef HAVE_MODEMBRIDGE
		if ((strcmp(wan_proto, "bridge") == 0)) {
		stop_atm();
		start_atm();
		br_add_interface("br0", "nas0");
		wan_done("nas0");
	} else
#endif
#ifdef HAVE_PPPOATM
		if ((strcmp(wan_proto, "pppoa") == 0)) {
		char username[80], passwd[80];
		char idletime[20], retry_num[20];
		stop_atm();
		start_atm();

		snprintf(idletime, sizeof(idletime), "%d", nvram_geti("ppp_idletime") * 60);
		snprintf(retry_num, sizeof(retry_num), "%d", (nvram_geti("ppp_redialperiod") / 5) - 1);

		snprintf(username, sizeof(username), "%s", nvram_safe_get("ppp_username"));
		snprintf(passwd, sizeof(passwd), "%s", nvram_safe_get("ppp_passwd"));

		mkdir("/tmp/ppp", 0777);
		int timeout = 5;

		// Lets open option file and enter all the parameters.
		fp = fopen("/tmp/ppp/options.pppoa", "w");

		if (nvram_matchi("ppp_demand", 1))
			fprintf(fp, "connect true\n");

		fprintf(fp, "plugin /usr/lib/pppoatm.so %s.%s %s", nvram_safe_get("vpi"), nvram_safe_get("vci"),
			nvram_matchi("atm_encaps", 0) ? "llc-encaps" : "vc-encaps");
		fprintf(fp, "\n");

		// Those are default options we use + user/passwd
		// By using user/password options we dont have to deal with chap/pap
		// secrets files.
		if (nvram_matchi("ppp_compression", 1)) {
			fprintf(fp, "mppc\n");
		} else {
			fprintf(fp, "noccp\n");
			fprintf(fp, "nomppc\n");
		}
		fprintf(fp, "noipdefault\n" //
			    "noauth\n" //
			    "defaultroute\n" //
			    "noaccomp\n" //
			    "nobsdcomp\n" //
			    "nodeflate\n" //
			    "nopcomp\n");
		if (nvram_invmatch("ppp_mppe", ""))
			fprintf(fp, "%s\n", nvram_safe_get("ppp_mppe"));
		else
			fprintf(fp, "nomppe\n");
		if (nvram_matchi("ppp_mlppp", 1))
			fprintf(fp, "mp\n");
		fprintf(fp,
			"usepeerdns\nuser '%s'\n" //
			"password '%s'\n",
			username, passwd);

		if (nvram_matchi("ppp_asyncmap", 1))
			fprintf(fp, "asyncmap 0\n");
		else
			fprintf(fp, "default-asyncmap\n");

		if (nvram_matchi("mtu_enable", 1)) {
			if (nvram_geti("wan_mtu") > 0) {
				fprintf(fp, "mtu %s\n", nvram_safe_get("wan_mtu"));
				fprintf(fp, "mru %s\n", nvram_safe_get("wan_mtu"));
			}

		} else {
			// If MRU set to Auto we still allow custom MTU/MRU settings for
			// expirienced users
			if (nvram_invmatch("pppoe_ppp_mtu", ""))
				if (nvram_geti("pppoe_ppp_mtu") > 0)
					fprintf(fp, "mtu %s\n", nvram_safe_get("pppoe_ppp_mtu"));
			if (nvram_invmatch("pppoe_ppp_mru", ""))
				if (nvram_geti("pppoe_ppp_mru") > 0)
					fprintf(fp, "mru %s\n", nvram_safe_get("pppoe_ppp_mru"));
		}

		if (nvram_matchi("ppp_debug", 1))
			fprintf(fp, "debug\n");

		if (nvram_matchi("ppp_demand", 1))
			fprintf(fp,
				"demand\n" //
				"idle %s\n" //
				"10.112.112.112:10.112.112.113\n" //
				"lcp-echo-interval %d\n" //
				"lcp-echo-failure 10\n" //
				"lcp-echo-adaptive\n"
				"ipcp-accept-remote\n" //
				"ipcp-accept-local\n" //
				"ktune\n",
				idletime, atoi(idletime) * 2);
		else
			fprintf(fp, "persist\n" //
				    "lcp-echo-interval 3\n" //
				    "lcp-echo-failure 20\n" //
				    "lcp-echo-adaptive\n");

		fclose(fp);

		symlink("/sbin/rc", "/tmp/ppp/ip-up");
		symlink("/sbin/rc", "/tmp/ppp/ip-down");
		unlink("/tmp/ppp/log");

		// Clean pppoe linksys client files - Added by ice-man (Wed Jun 1)
		unlink("/tmp/ppp/connect-log");
		unlink("/tmp/ppp/set-pppoepid");

		stop_dhcpc();
#ifdef HAVE_PPTP
		stop_pptp();
#endif
		stop_process("pppd", "PPP daemon");
		start_pppmodules();
		log_eval("pppd", "file", "/tmp/ppp/options.pppoa");

		/*
		 * Pretend that the WAN interface is up 
		 */
		if (nvram_matchi("ppp_demand", 1)) {
			/*
			 * Wait for ppp0 to be created 
			 */
			while (ifconfig("ppp0", IFUP, NULL, NULL) && timeout--)
				sleep(1);
			strncpy(ifr.ifr_name, "ppp0", IFNAMSIZ);

			/*
			 * Set temporary IP address 
			 */
			timeout = 3;
			while (ioctl(s, SIOCGIFADDR, &ifr) && timeout--) {
				perror("ppp0");
				printf("Wait ppp inteface to init (1) ...\n");
				sleep(1);
			};
			char client[32];

			nvram_set("wan_ipaddr", inet_ntop(AF_INET, &sin_addr(&ifr.ifr_addr), client, 16));
			nvram_set("wan_netmask", "255.255.255.255");

			/*
			 * Set temporary P-t-P address 
			 */
			timeout = 3;
			while (ioctl(s, SIOCGIFDSTADDR, &ifr) && timeout--) {
				perror("ppp0");
				printf("Wait ppp inteface to init (2) ...\n");
				sleep(1);
			}
			const char *peer = inet_ntop(AF_INET, &sin_addr(&ifr.ifr_dstaddr), client, 16);

			nvram_set("wan_gateway", peer);

			wan_done("ppp0");

			// if user press Connect" button from web, we must force to dial
			if (nvram_match("action_service", "start_pppoa")) {
				sleep(3);
				start_force_to_dial();
				nvram_unset("action_service");
			}
		} else {
			if (status != REDIAL) {
				start_redial();
			}
		}
	} else
#endif
		if (strcmp(wan_proto, "dhcp") == 0 || strcmp(wan_proto, "dhcp_auth") == 0) {
		run_dhcpc(wan_ifname, NULL, NULL, 1, 0, 0);
	}
#ifdef HAVE_IPETH
	else if (strcmp(wan_proto, "iphone") == 0) {
		if (!nvram_matchi("usb_enable", 1)) {
			nvram_seti("usb_enable",
				   1); //  simply enable it, otherwise 3g might not work
			nvram_async_commit();
			load_drivers(1);
		}
		insmod("ipheth");
		stop_process("ipheth-loop", "IPhone Pairing daemon");
		stop_process("usbmuxd", "IPhone Mux daemon");
		stop_process("udhcpc", "DHCP Client");
		log_eval("usbmuxd");
		log_eval("ipheth-pair");
		eval("ifconfig", "iph0", "up");
		run_dhcpc("iph0", NULL, NULL, 1, 0, 0);
		if (status != REDIAL) {
			start_redial();
		}
	} else if (strcmp(wan_proto, "android") == 0) {
		if (!nvram_matchi("usb_enable", 1)) {
			nvram_seti("usb_enable",
				   1); //  simply enable it, otherwise 3g might not work
			nvram_async_commit();
			load_drivers(1);
		}
		insmod("usbnet");
		insmod("cdc_ether");
		insmod("cdc_ncm");
		insmod("rndis_host");
		int deadcount = 10;
		while (deadcount--) {
			eval("ifconfig", "usb0", "up");
			eval("ifconfig", "wwan0", "up");
			if (ifexists("usb0"))
				run_dhcpc("usb0", NULL, NULL, 1, 0, 0);
			else if (ifexists("wwan0"))
				run_dhcpc("wwan0", NULL, NULL, 1, 0, 0);
			else {
				sleep(1);
				continue;
			}
		}
		if (status != REDIAL) {
			start_redial();
		}
	}
#endif
#ifdef HAVE_PPTP
	else if (strcmp(wan_proto, "pptp") == 0) {
		if (nvram_matchi("pptp_iptv", 1))
			nvram_set("tvnicfrom", nvram_safe_get("wan_iface"));
		else
			nvram_unset("tvnicfrom");

		run_pptp(status);
	}
#endif
#ifdef HAVE_L2TP
	else if (strcmp(wan_proto, "l2tp") == 0) {
		if (nvram_matchi("pptp_iptv", 1))
			nvram_set("tvnicfrom", nvram_safe_get("wan_iface"));
		else
			nvram_unset("tvnicfrom");

		if (nvram_matchi("l2tp_use_dhcp", 1)) {
			nvram_set("wan_get_dns", "");
			char *l2tp_ifname = nvram_safe_get("wan_ifname");
			if (isClient()) {
				l2tp_ifname = getSTA();
			}

			run_dhcpc(l2tp_ifname, NULL, NULL, 1, 0, 0);
		} else {
			run_l2tp(status);
		}
	}
#endif
#ifdef HAVE_HEARTBEAT
	else if (strcmp(wan_proto, "heartbeat") == 0) {
		run_dhcpc(wan_ifname, NULL, NULL, 1, 0, 0);
	}
#endif
	else {
		ifconfig(wan_ifname, IFUP, nvram_safe_get("wan_ipaddr"), nvram_safe_get("wan_netmask"));
		wan_done(wan_ifname);
		log_eval("gratarp", wan_ifname);
	}
	cprintf("dhcp client ready\n");

	/*
	 * Get current WAN hardware address 
	 */
	if (!strncmp(wan_ifname, "ppp", 3))
		strncpy(ifr.ifr_name, nvram_safe_get("wan_ifname"), IFNAMSIZ);
	else
		strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);
	cprintf("get current hardware adress");
	{
		char eabuf[32];
		if (get_hwaddr(ifr.ifr_name, eabuf)) {
			nvram_set("wan_hwaddr", eabuf);
			//fprintf(stderr,"write wan addr %s\n",nvram_safe_get("wan_hwaddr"));
		}
	}

	close(s);

	// set_ip_forward('1');

	// ===================================================================================
	// Tallest move herei(from "start_lan" function ). Fixed when wireless
	// disable, wireless LED dosen't off.
	// ===================================================================================
	/*
	 * Disable wireless will cause diag led blink, so we want to stop it. 
	 */

	cprintf("diag led control\n");
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)
	if ((check_hw_type() == BCM4712_CHIP) || (check_hw_type() == BCM5325E_CHIP)) {
		// Barry will put disable WLAN here
		if (nvram_match("wl_gmode", "-1")) {
			diag_led(WL, STOP_LED);
#if 0
			eval("wl", "led", "0", "0", "1");
			eval("wl", "led", "0", "1", "1");
#endif
		}
		diag_led(DIAG, STOP_LED);
	}
	/*
	 * Light or go out the DMZ led even if there is no wan ip. 
	 */
	if (nvram_matchi("dmz_enable", 1) && nvram_invmatch("dmz_ipaddr", "") && nvram_invmatchi("dmz_ipaddr", 0))
		diag_led(DMZ, START_LED);
	else
		diag_led(DMZ, STOP_LED);
#endif
	cprintf("%s %s\n", nvram_safe_get("wan_ipaddr"), nvram_safe_get("wan_netmask"));

	if (nvram_match("wan_proto", "l2tp")) {
		/*
		 * Delete all default routes 
		 */
		while (route_del(safe_get_wan_face(wan_if_buffer), 0, NULL, NULL, NULL) == 0)
			;
	}
	cprintf("wep handling\n");
	cprintf("disable stp if needed\n");
#ifdef HAVE_MICRO
	br_init();
#endif
	{
		char word[256];

		br_set_stp_state("br0", getBridgeSTP("br0", word));
	}
#ifdef HAVE_MICRO
	br_shutdown();
#endif

	cprintf("done()()()\n");
}

void start_wan_boot(void)
{
	run_wan(BOOT);
}

void start_wan_redial(void)
{
	run_wan(REDIAL);
}

void start_wan_service(void)
{
	stop_process_monitor();
	stop_ddns();
	cprintf("start process monitor\n");
	start_process_monitor();
	cprintf("start ddns\n");
	start_ddns();
#ifdef HAVE_SPEEDCHECKER
	stop_speedchecker();
	start_speedchecker();
#endif
}

#ifdef HAVE_IPV6
static const char *ipv6_router_address(struct in6_addr *in6addr, char *addr6, socklen_t len)
{
	char *p;
	struct in6_addr addr;
	if (!addr6)
		return NULL;
	addr6[0] = '\0';

	if ((p = nvram_safe_get("ipv6_addr")) && *p) {
		inet_pton(AF_INET6, p, &addr);
	} else if ((p = nvram_safe_get("ipv6_prefix")) && *p) {
		inet_pton(AF_INET6, p, &addr);
		addr.s6_addr16[7] = htons(0x0001);
	} else {
		return addr6;
	}

	inet_ntop(AF_INET6, &addr, addr6, len);

	return addr6;
}

static void start_ipv6_tunnel(char *wan_ifname)
{
	char *remote_endpoint = nvram_safe_get("ipv6_tun_end_ipv4");
	char *tun_client_ipv6 = nvram_safe_get("ipv6_tun_client_addr");
	char *tun_client_pref = nvram_safe_get("ipv6_tun_client_addr_pref");
	char *ipv6_prefix = nvram_safe_get("ipv6_prefix");
	char *ipv6_pf_len = nvram_safe_get("ipv6_pf_len");

	int mtu = nvram_default_geti("wan_mtu", 1500) - 20;

	stop_ipv6_tunnel(wan_ifname);

	if (nvram_invmatch("ipv6_mtu", ""))
		mtu = nvram_geti("ipv6_mtu");
	eval("ip", "tunnel", "add", "ip6tun", "mode", "sit", "ttl", "64", "local", get_wan_ipaddr(), "remote", remote_endpoint);
	char s_mtu[32];
	sprintf(s_mtu, "%d", mtu);
	eval("ip", "link", "set", "ip6tun", "mtu", s_mtu);
	eval("ip", "link", "set", "ip6tun", "up");
	char clientip[64];
	char prefix[64];
	sprintf(clientip, "%s/%s", tun_client_ipv6, tun_client_pref);
	sprintf(prefix, "%s/%s", ipv6_prefix, ipv6_pf_len);
	eval("ip", "-6", "addr", "add", clientip, "dev", "ip6tun");
	eval("ip", "-6", "addr", "add", prefix, "dev", nvram_safe_get("lan_ifname"));
	eval("ip", "-6", "route", "add", "2000::/3", "dev", "ip6tun");
}

static void stop_ipv6_tunnel(char *wan_ifname)
{
	if (nvram_match("ipv6_typ", "ipv6rd") || nvram_match("ipv6_typ", "ipv6to4")) {
		eval("ip", "tunnel", "del", wan_ifname);
	}

	if (nvram_match("ipv6_typ", "ipv6in4")) {
		eval("ip", "tunnel", "del", "ip6tun");
	}

	if (nvram_match("ipv6_typ", "ipv6to4") || nvram_match("ipv6_typ", "ipv6rd")) {
		eval("ip", "-6", "addr", "flush", "dev", nvram_safe_get("lan_ifname"), "scope", "global");
	}
}

static void start_wan6_done(char *wan_ifname)
{
	if (nvram_matchi("ipv6_enable", 0))
		return;

	eval("ip", "-6", "addr", "flush", "scope", "global");

	if (nvram_match("ipv6_typ", "ipv6native")) {
		if (nvram_match("wan_proto", "disabled")) {
			sysprintf("echo 2 > /proc/sys/net/ipv6/conf/%s/accept_ra", nvram_safe_get("lan_ifname"));
		} else {
			sysprintf("echo 2 > /proc/sys/net/ipv6/conf/%s/accept_ra", wan_ifname);
		}

		char ip[INET6_ADDRSTRLEN + 4];
		const char *p;
		char addr6[INET6_ADDRSTRLEN];

		p = ipv6_router_address(NULL, addr6, sizeof(addr6));
		if (p && *p) {
			snprintf(ip, sizeof(ip), "%s/%d", p, nvram_geti("ipv6_pf_len") ?: 64);
			eval("ip", "-6", "addr", "add", ip, "dev", nvram_safe_get("lan_ifname"));
		}
		if (nvram_match("wan_proto", "disabled")) {
			eval("ip", "route", "add", "::/0", "dev", nvram_safe_get("lan_ifname"), "metric", "2048");
		} else {
			eval("ip", "route", "add", "::/0", "dev", wan_ifname, "metric", "2048");
		}
	}

	if (nvram_match("ipv6_typ", "ipv6pd")) {
		sysprintf("echo 2 > /proc/sys/net/ipv6/conf/%s/accept_ra", wan_ifname);
		eval("stopservice", "dhcp6c", "-f");
		eval("startservice", "dhcp6c", "-f");
		if (nvram_match("wan_proto", "disabled")) {
			eval("ip", "route", "add", "::/0", "dev", nvram_safe_get("lan_ifname"), "metric", "2048");
		} else {
			eval("ip", "route", "add", "::/0", "dev", wan_ifname, "metric", "2048");
		}
	}

	if (nvram_match("ipv6_typ", "ipv6in4")) {
		stop_ipv6_tunnel(wan_ifname);
		start_ipv6_tunnel(wan_ifname);
	}

	eval("stopservice", "dhcp6s", "-f");
	eval("startservice", "dhcp6s", "-f");
}
#endif
void wan_done(char *wan_ifname)
{
	if (nvram_matchi("wan_testmode", 1)) {
		fprintf(stderr, "[WAN IF] testmode: skipping wan_done\n");
		return;
	}

	cprintf("%s %s\n", wan_ifname, nvram_safe_get("wan_proto"));

	if (nvram_match("wan_proto", "l2tp")) {
		/*
		 * Delete all default routes 
		 */
		while (route_del(nvram_safe_get("wan_ifname"), 0, NULL, NULL, NULL) == 0)
			;
	}

	/*
	 * Delete all default routes 
	 */
	while (route_del(wan_ifname, 0, NULL, NULL, NULL) == 0)
		;

	if ((nvram_match("wan_proto", "pppoe")) && check_wan_link(1)) {
		while (route_del(nvram_safe_get("wan_ifname_1"), 0, NULL, NULL, NULL) == 0)
			;
	}
#ifdef HAVE_PPPOEDUAL
	if ((nvram_match("wan_proto", "pppoe_dual")) && check_wan_link(1)) {
		while (route_del(nvram_safe_get("wan_ifname_1"), 0, NULL, NULL, NULL) == 0)
			;
	}
#endif
#ifdef HAVE_PPPOATM
	if ((nvram_match("wan_proto", "pppoa")) && check_wan_link(1)) {
		while (route_del(nvram_safe_get("wan_ifname_1"), 0, NULL, NULL, NULL) == 0)
			;
	}
#endif

	if (nvram_invmatch("wan_proto", "disabled")) {
		int timeout = 5;

		/*
		 * Set default route to gateway if specified 
		 */
		char *gateway = nvram_match("wan_proto", "pptp") ? nvram_safe_get("pptp_get_ip") : nvram_safe_get("wan_gateway");
		if (strcmp(gateway, "0.0.0.0")) {
			//                      route_add(wan_ifname, 0, gateway, NULL,"255.255.255.255");

			while (route_add(wan_ifname, 0, "0.0.0.0", gateway, "0.0.0.0") && timeout--) {
				if ((nvram_match("wan_proto", "pppoe") || nvram_match("wan_proto", "pppoa") ||
				     nvram_match("wan_proto", "pppoe_dual")) &&
				    nvram_matchi("ppp_demand", 1)) {
					printf("Wait ppp interface to init (3) ...\n");
					sleep(1);
				} else
					break;
			}
		}
	}

	if (nvram_match("wan_proto", "pptp")) {
		route_del(nvram_safe_get("wan_iface"), 0, nvram_safe_get("wan_gateway"), NULL, "255.255.255.255");
		route_del(nvram_safe_get("wan_iface"), 0, nvram_safe_get("pptp_server_ip"), NULL, "255.255.255.255");
		route_add(nvram_safe_get("wan_iface"), 0, nvram_safe_get("pptp_get_ip"), NULL, "255.255.255.255");
	} else if (nvram_match("wan_proto", "l2tp")) {
		route_del(nvram_safe_get("wan_iface"), 0, nvram_safe_get("wan_gateway"), NULL, "255.255.255.255");
		route_add(nvram_safe_get("wan_iface"), 0, nvram_safe_get("l2tp_get_ip"), NULL, "255.255.255.255");
		if (nvram_matchi("l2tp_use_dhcp", 1))
			route_add(nvram_safe_get("wan_ifname"), 0, nvram_safe_get("l2tp_server_ip"),
				  nvram_safe_get("wan_gateway_buf"),
				  "255.255.255.255"); // fixed
	}

	/*
	 * save dns to resolv.conf 
	 */
	cprintf("dns to resolv\n");
	dns_to_resolv();

	cprintf("restart dhcp server\n");
	/*
	 * Restart DHCP server 
	 */
#ifdef HAVE_IPV6
	start_wan6_done(wan_ifname);
#endif
	cprintf("restart dns proxy\n");
	/*
	 * Restart DNS proxy 
	 */

	cprintf("start firewall\n");

	/*
	 * Set additional wan static routes if need 
	 */

	start_set_routes();
	cprintf("routes done\n");

#ifdef HAVE_UPNP
	stop_upnp();
#endif
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	stop_zebra();
#endif
	cprintf("start zebra\n");
#if defined(HAVE_BIRD) || defined(HAVE_QUAGGA)
	start_zebra();
#endif
	cprintf("start upnp\n");
#ifdef HAVE_UPNP
	start_upnp();
#endif
	// cprintf ("start cron\n");
	// start_OAcron ();
	cprintf("start wshaper\n");
	stop_wland();
	start_wland();
	if (nvram_match("wan_proto", "pptp")) {
		if (nvram_invmatch("pptp_customipup", "")) {
			// We not going to assume that /tmp/ppp is created..
			mkdir("/tmp/ppp", 0700);

			// Create our custom pptp ipup script and change its attributes
			writenvram("pptp_customipup", "/tmp/ppp/sh_pptp_customipup");
			chmod("/tmp/ppp/sh_pptp_customipup", 0744);

			// Execute our custom ipup script
			eval("/tmp/ppp/sh_pptp_customipup");
		}
	}
	cprintf("std on\n");
#ifdef HAVE_MICRO
	br_init();
#endif

	{
		char word[256];
		br_set_stp_state(nvram_safe_get("lan_ifname"), getBridgeSTP(nvram_safe_get("lan_ifname"), word));
	}
#ifdef HAVE_MICRO
	br_shutdown();
#endif

	cprintf("check wan link\n");
	if (check_wan_link(0))
		SET_LED(GOT_IP);
	else if ((!check_wan_link(0)) && nvram_match("wan_proto", "auto")) {
		SET_LED(GET_IP_ERROR);
	}
	/*
	 * check ip addresses for validity 
	 */
	uint32 wanip;
	uint32 wannm;

	inet_aton(get_wan_ipaddr(), (struct in_addr *)&wanip);
	inet_aton(nvram_safe_get("wan_netmask"), (struct in_addr *)&wannm);
	uint32 lanip;
	uint32 lannm;

	inet_aton(nvram_safe_get("lan_ipaddr"), (struct in_addr *)&lanip);
	inet_aton(nvram_safe_get("lan_netmask"), (struct in_addr *)&lannm);

	if (wanip != 0 && !nvram_match("wan_proto", "disabled")) {
		int iperror = 0;

		if ((wanip & wannm) == (lanip & wannm))
			iperror = 1;
		if ((lanip & lannm) == (wanip & lannm))
			iperror = 1;
		if (iperror)
			eval("ledtool",
			     "5"); // blink 5 times the 3 time interval
	}
	/*
	 * end 
	 */

	cprintf("running custom DD-WRT ipup scripts\n");
	runStartup(".ipup");
	cprintf("trigger gpio");

	led_control(LED_CONNECTED, LED_OFF);
	if (!nvram_match("wan_proto", "disabled")) {
		led_control(LED_CONNECTED, LED_ON);
		dd_loginfo("wan", "WAN is up. IP: %s\n", get_wan_ipaddr());
		if (nvram_match("wan_proto", "3g")) {
			stop_redial();
			start_redial();
		}
	}
#ifdef HAVE_3G
#if defined(HAVE_TMK) || defined(HAVE_BKM)
	char *gpio3g;
	if (nvram_match("wan_proto", "3g")) {
		gpio3g = nvram_safe_get("gpio3g");
		if (*gpio3g)
			set_gpio(atoi(gpio3g), 1);
	} else {
		if (!nvram_match("wan_proto", "disabled")) {
			gpio3g = nvram_safe_get("gpiowancable");
			if (*gpio3g)
				set_gpio(atoi(gpio3g), 1);
		} else {
			gpio3g = nvram_safe_get("gpio3g");
			if (*gpio3g)
				set_gpio(atoi(gpio3g), 0);
			gpio3g = nvram_safe_get("gpiowancable");
			if (*gpio3g)
				set_gpio(atoi(gpio3g), 0);
		}
	}
#endif
#endif

	unsigned sys_uptime;
	struct sysinfo info;
	sysinfo(&info);
	sys_uptime = info.uptime;
	FILE *up;

	up = fopen("/tmp/.wanuptime", "w");
	fprintf(up, "%u", sys_uptime);
	fclose(up);

	cprintf("done\n");

	nvram_set("wan_iface", nvram_safe_get("wan_ifname"));

#ifdef HAVE_DNSMASQ
	restart_dnsmasq();
#endif
#ifdef HAVE_SMARTDNS
	stop_smartdns();
	start_smartdns();
#endif

#ifdef HAVE_OPENVPN
	cprintf("starting openvpn\n");
	stop_openvpn_wandone();
	start_openvpn();
	cprintf("done\n");

#endif

#ifdef HAVE_ANTAIRA_AGENT
	cprintf("starting antaira-agent\n");
	stop_antaira_agent();
	start_antaira_agent();
	cprintf("done\n");
#endif

#ifdef HAVE_STRONGSWAN
	stop_strongswan();
	start_strongswan();
#endif
#ifdef HAVE_OPENVPN
	stop_openvpnserverwan();
	start_openvpnserverwan();
#endif
#ifdef HAVE_DHCPFORWARD
	stop_dhcpfwd();
	start_dhcpfwd();
#endif
#ifdef HAVE_MILKFISH
	if (nvram_matchi("milkfish_enabled", 1)) {
		cprintf("starting milkfish netup script\n");
		eval("/etc/config/milkfish.netup");
	}
#endif
#ifdef HAVE_SPUTNIK_APD
	stop_sputnik();
	start_sputnik();
#endif

#ifdef HAVE_TOR
	stop_tor();
	start_tor();
#endif

#ifdef HAVE_IPVS
	stop_ipvs();
	start_ipvs();
#endif

#ifdef HAVE_SPEEDCHECKER
	stop_speedchecker();
	start_speedchecker();
#endif
#ifdef HAVE_RADVD
	stop_radvd();
	start_radvd();
#endif
	stop_unbound();
	start_unbound();

#ifdef HAVE_MICRO
	br_shutdown();
#endif
#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880) || defined(HAVE_RT61)
#ifndef HAVE_NOWIFI
	start_hostapdwan();
#endif
#endif
	cprintf("start igmp proxy\n");
#ifdef HAVE_MULTICAST
	if ((!nvram_matchi("dtag_vlan8", 1) && !nvram_matchi("dtag_bng", 1)) || nvram_matchi("wan_vdsl", 0))
		stop_igmprt();
	start_igmprt();
#endif
	cprintf("ready\n");
#ifdef HAVE_UDPXY
	if (!nvram_matchi("udpxy_enable", 1))
		stop_udpxy();
	start_udpxy();
#endif
//      start_anchorfree();
//      start_anchorfreednat();
#ifdef HAVE_MADWIFI
#ifndef HAVE_NOWIFI
	start_duallink();
#endif
#endif
	/*
	 * Start firewall 
	 */
	start_firewall();
	if (nvram_matchi("ipv6_enable", 1) && nvram_match("ipv6_typ", "ipv6in4")) {
#ifdef HAVE_CURL
		eval("/usr/bin/curl", "-s", "-k", nvram_safe_get("ipv6_tun_upd_url"), "-o", "/tmp/tunnelstat");
		FILE *fp = fopen("/tmp/tunnelstat", "r");
		if (fp) {
			fclose(fp);
		} else {
			eval("/usr/bin/curl", "-s", "-k", nvram_safe_get("ipv6_tun_upd_url"), "-o", "/tmp/tunnelstat");
		}
#else
		eval("wget", nvram_safe_get("ipv6_tun_upd_url"), "-O", "/tmp/tunnelstat");
#endif
#ifdef HAVE_IPV6
		if (nvram_matchi("wshaper_enable", 1)) {
			stop_ipv6_tunnel(wan_ifname);
			start_ipv6_tunnel(wan_ifname);
		}
#endif
	}
	start_wan_service();
	nvram_seti("wanup", 1);
}

void stop_wan(void)
{
	char wan_if_buffer[33];

	char *wan_ifname = safe_get_wan_face(wan_if_buffer);

	nvram_seti("wanup", 0);

	led_control(LED_CONNECTED, LED_OFF);
	unlink("/tmp/.wanuptime");

	cprintf("%s %s\n", wan_ifname, nvram_safe_get("wan_proto"));
#ifdef HAVE_OPENVPN
	stop_openvpnserverwan();
	stop_openvpn_wandone();
#endif
#ifdef HAVE_ANTAIRA_AGENT
	stop_antaira_agent();
#endif
#ifdef HAVE_DHCPFORWARD
	stop_dhcpfwd();
#endif
#ifdef HAVE_3G
	if (nvram_match("3gdata", "sierradirectip")) {
		sysprintf("comgt -d %s -s /etc/comgt/hangup-dip.comgt\n", nvram_safe_get("3gcontrol"));
	}
	if (nvram_match("3gdata", "mbim")) {
		sysprintf("/jffs/bin/stop-mbim.sh", nvram_safe_get("3gcontrol"));
	}
// todo:
// #ifdef HAVE_UQMI
//      if (nvram_match("3gdata", "qmi")) {
//      }
// #endif
#endif
	/*
	 * Stop firewall 
	 */
	stop_firewall();
#ifdef HAVE_IPV6
	stop_ipv6_tunnel(wan_ifname);
#endif
	/*
	 * Kill any WAN client daemons or callbacks 
	 */
#ifdef HAVE_PPPOE
	stop_pppoe();
#endif
#ifdef HAVE_PPPOEDUAL
	stop_pppoe_dual();
#endif
#ifdef HAVE_L2TP
	stop_l2tp();
#endif
	stop_dhcpc();
#ifdef HAVE_HEARTBEAT
	stop_heartbeat();
#endif
#ifdef HAVE_PPTP
	stop_pptp();
#endif
#ifdef HAVE_SPUTNIK_APD
	stop_sputnik();
#endif
	stop_ntpc();
	stop_redial();
	nvram_set("wan_get_dns", "");

	// Reset pppd's pppX interface
	nvram_set("pppd_pppifname", "");

	/*
	 * Bring down WAN interfaces 
	 */
	ifconfig(wan_ifname, 0, NULL, NULL);
#ifdef HAVE_MODEMBRIDGE
	br_del_interface("br0", "nas0");
#endif
	eval("ifconfig", wan_ifname, "down"); // to allow for MAC clone to
	// take effect
#ifdef HAVE_PPP
#endif

	cprintf("done\n");
}

static void apply_rules(char *method, char *pbr)
{
	char word[512], *tmp;
	foreach(word, pbr, tmp)
	{
		char cmd[256] = { 0 };
		char add[256];
		char del[256];
		strcpy(add, "ip rule ");
		strcat(add, method);
		strcpy(del, "ip rule ");
		strcat(del, "del");
		GETENTRYBYIDX(s_flags, word, 0);
		int flags = 0;
		if (s_flags)
			sscanf(s_flags, "%X", &flags);
		int not = flags & 0x1;
		int from_en = flags & 0x2;
		int to_en = flags & 0x4;
		int priority_en = flags & 0x8;
		int tos_en = flags & 0x10;
		int fwmark_en = flags & 0x20;
		int realms_en = flags & 0x40;
		int table_en = flags & 0x80;
		int suppress_prefixlength_en = flags & 0x100;
		int iif_en = flags & 0x200;
		int nat_en = flags & 0x400;
		int type_en = flags & 0x800;
		int ipproto_en = flags & 0x1000;
		int sport_en = flags & 0x2000;
		int dport_en = flags & 0x4000;
		int oif_en = flags & 0x8000;
		GETENTRYBYIDX_DEL(from, word, 1, ":");
		GETENTRYBYIDX_DEL(to, word, 2, ":");
		GETENTRYBYIDX_DEL(priority, word, 3, ":");
		GETENTRYBYIDX_DEL(tos, word, 4, ":");
		GETENTRYBYIDX_DEL(fwmark, word, 5, ":");
		GETENTRYBYIDX_DEL(realms, word, 6, ":");
		GETENTRYBYIDX_DEL(table, word, 7, ":");
		GETENTRYBYIDX_DEL(suppress_prefixlength, word, 8, ":");
		GETENTRYBYIDX_DEL(iif, word, 9, ":");
		GETENTRYBYIDX_DEL(nat, word, 10, ":");
		GETENTRYBYIDX_DEL(type, word, 11, ":");
		GETENTRYBYIDX_DEL(ipproto, word, 12, ":");
		GETENTRYBYIDX_DEL(sport, word, 13, ":");
		GETENTRYBYIDX_DEL(dport, word, 14, ":");
		GETENTRYBYIDX_DEL(oif, word, 15, ":");
		if (not )
			sprintf(cmd, "%s %s", cmd, "not");
		if (from_en && from)
			sprintf(cmd, "%s from %s", cmd, from);
		if (to_en && to)
			sprintf(cmd, "%s to %s", cmd, to);
		if (priority_en && priority)
			sprintf(cmd, "%s priority %s", cmd, priority);
		if (tos_en && tos)
			sprintf(cmd, "%s tos %s", cmd, tos);
		if (fwmark_en && fwmark)
			sprintf(cmd, "%s fwmark %s", cmd, fwmark);
		if (realms_en && realms)
			sprintf(cmd, "%s realms %s", cmd, realms);
		if (table_en && table)
			sprintf(cmd, "%s table %s", cmd, table);
		if (suppress_prefixlength_en && suppress_prefixlength)
			sprintf(cmd, "%s suppress_prefixlength %s", cmd, suppress_prefixlength);
		if (iif_en && iif)
			sprintf(cmd, "%s iif %s", cmd, iif);
		if (oif_en && oif)
			sprintf(cmd, "%s oif %s", cmd, oif);
		if (nat_en && nat)
			sprintf(cmd, "%s nat %s", cmd, nat);
		if (type_en && type)
			sprintf(cmd, "%s type %s", cmd, type);
		if (ipproto_en && ipproto)
			sprintf(cmd, "%s ipproto %s", cmd, ipproto);
		if (sport_en && sport)
			sprintf(cmd, "%s sport %s", cmd, sport);
		if (dport_en && dport)
			sprintf(cmd, "%s dport %s", cmd, dport);
		if (strlen(cmd)) {
			strcat(add, cmd);
			if (!strcmp(method, "add")) {
				strcat(del, cmd);
				dd_debug(DEBUG_CONSOLE, "%s\n", del);
				system(del);
			}
			dd_debug(DEBUG_CONSOLE, "%s\n", add);
			system(add);
		}
	}
}

void start_set_routes(void)
{
	char word[512], *tmp;

	if (!nvram_match("lan_gateway", "0.0.0.0")) {
		eval("route", "del", "default");
		eval("route", "add", "default", "gw", nvram_safe_get("lan_gateway"));
	}
	char *defgateway;

	if (nvram_match("wan_proto", "pptp"))
		defgateway = nvram_safe_get("pptp_get_ip");
	else if (nvram_match("wan_proto", "l2tp"))
		defgateway = nvram_safe_get("l2tp_get_ip");
	else
		defgateway = nvram_safe_get("wan_gateway");
	if (strcmp(defgateway, "0.0.0.0") && !nvram_match("wan_proto", "disabled")) {
		eval("route", "del", "default");
		eval("route", "add", "default", "gw", defgateway);
	}
#ifdef HAVE_MICRO
	char *sr = nvram_safe_get("static_route");
	foreach(word, sr, tmp)
	{
		GETENTRYBYIDX_DEL(ipaddr, word, 0, ":");
		GETENTRYBYIDX_DEL(netmask, word, 1, ":");
		GETENTRYBYIDX_DEL(gateway, word, 2, ":");
		GETENTRYBYIDX_DEL(metric, word, 3, ":");
		GETENTRYBYIDX_DEL(ifname, word, 4, ":");
		if (!ipaddr || !netmask || !gateway || !metric || !ifname)
			continue;
		if (!strcmp(ipaddr, "0.0.0.0") && !strcmp(gateway, "0.0.0.0"))
			continue;
		if (!strcmp(ipaddr, "0.0.0.0")) {
			eval("route", "del", "default");
			eval("route", "add", "default", "gw", gateway);
		} else if (!strcmp(ifname, "any")) {
			eval("route", "add", "-net", ipaddr, "netmask", netmask, "gw", gateway, "metric", metric);
		} else
			route_add(ifname, atoi(metric) + 1, ipaddr, gateway, netmask);
	}
#else
	char *sr = nvram_safe_get("static_route");
	foreach(word, sr, tmp)
	{
		GETENTRYBYIDX_DEL(ipaddr, word, 0, ":");
		GETENTRYBYIDX_DEL(netmask, word, 1, ":");
		GETENTRYBYIDX_DEL(gateway, word, 2, ":");
		GETENTRYBYIDX_DEL(metric, word, 3, ":");
		GETENTRYBYIDX_DEL(ifname, word, 4, ":");

		GETENTRYBYIDX_DEL(s_flags, word, 6, ":");
		int flags = 0;
		if (s_flags)
			sscanf(s_flags, "%X", &flags);
		int src_en = flags & 0x1;
		int scope_en = flags & 0x2;
		int table_en = flags & 0x4;
		int mtu_en = flags & 0x8;
		int advmss_en = flags & 0x10;
		GETENTRYBYIDX_DEL(src, word, 7, ":");
		GETENTRYBYIDX_DEL(scope, word, 8, ":");
		GETENTRYBYIDX_DEL(table, word, 9, ":");
		GETENTRYBYIDX_DEL(mtu, word, 10, ":");
		GETENTRYBYIDX_DEL(advmss, word, 11, ":");
		if (!ipaddr || !netmask || !gateway || !metric || !ifname)
			continue;
		if (!strcmp(ipaddr, "0.0.0.0") && !strcmp(gateway, "0.0.0.0"))
			continue;
		if (!strcmp(ipaddr, "0.0.0.0") && !table_en) {
			eval("route", "del", "default");
			eval("route", "add", "default", "gw", gateway);
		}
		char cmd[256] = { 0 };
		sprintf(cmd, "ip route add to %s/%d", ipaddr, getmask(netmask));
		if (strcmp(gateway, "0.0.0.0"))
			sprintf(cmd, "%s via %s", cmd, gateway);
		if (strcmp(ifname, "any"))
			sprintf(cmd, "%s dev %s", cmd, ifname);
		if (strcmp(metric, "0"))
			sprintf(cmd, "%s metric %s", cmd, metric);
		if (src_en && src)
			sprintf(cmd, "%s src %s", cmd, src);
		if (scope_en && scope)
			sprintf(cmd, "%s scope %s", cmd, scope);
		if (table_en && table)
			sprintf(cmd, "%s table %s", cmd, table);
		if (mtu_en && mtu)
			sprintf(cmd, "%s mtu %s", cmd, mtu);
		if (advmss_en && advmss)
			sprintf(cmd, "%s advmss %s", cmd, advmss);
		dd_debug(DEBUG_CONSOLE, "%s\n", cmd);
		system(cmd);
	}
#endif
	FILE *old = fopen("/tmp/pbr_old", "rb");
	if (old) {
		fseek(old, 0, SEEK_END);
		int len = ftell(old);
		rewind(old);
		if (len) {
			char *buf = malloc(len + 1);
			fread(buf, 1, len, old);
			buf[len] = 0;
			apply_rules("del", buf);
			free(buf);
		}
		fclose(old);
		eval("rm", "-f", "/tmp/pbr_old");
	}
	char *pbr = nvram_safe_get("pbr_rule");
	apply_rules("add", pbr);
	if (f_exists("/tmp/tvrouting"))
		system("sh /tmp/tvrouting");
	if (f_exists("/tmp/udhcpstaticroutes"))
		system("sh /tmp/udhcpstaticroutes");
}

#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
static int notify_nas(char *type, char *ifname, char *action)
{
	char *argv[] = { "nas4not", type, ifname, action, NULL, /* role */
			 NULL, /* crypto */
			 NULL, /* auth */
			 NULL, /* passphrase */
			 NULL, /* ssid */
			 NULL };
	char *str = NULL;
	int retries = 10;
	char tmp[100], prefix[] = "wlXXXXXXXXXX_";
	int unit;
	char remote[ETHER_ADDR_LEN];
	char ssid[48], pass[80], auth[16], crypto[16], role[8];
	int i;

	/*
	 * the wireless interface must be configured to run NAS 
	 */
	unit = get_wl_instance(ifname);
	if (unit == -1)
		return 0;
	snprintf(prefix, sizeof(prefix), "wl%d_", unit);
	if (nvram_match(strcat_r(prefix, "akm", tmp), "") && nvram_match(strcat_r(prefix, "auth_mode", tmp), "none"))
		return 0;

	while (retries-- > 0 && !(str = file2str("/tmp/nas.wl0lan.pid")))
		sleep(1);
	if (!str) {
		return -1;
	}
	free(str);
	sleep(3);
	/*
	 * find WDS link configuration 
	 */
	wl_ioctl(ifname, WLC_WDS_GET_REMOTE_HWADDR, remote, ETHER_ADDR_LEN);
	for (i = 0; i < MAX_NVPARSE; i++) {
		char mac[ETHER_ADDR_STR_LEN];
		uint8 ea[ETHER_ADDR_LEN];

		if (get_wds_wsec(unit, i, mac, role, crypto, auth, ssid, pass) && ether_atoe(mac, ea) &&
		    !bcmp(ea, remote, ETHER_ADDR_LEN)) {
			argv[4] = role;
			argv[5] = crypto;
			argv[6] = auth;
			argv[7] = pass;
			argv[8] = ssid;
			break;
		}
	}

	/*
	 * did not find WDS link configuration, use wireless' 
	 */
	if (i == MAX_NVPARSE) {
		/*
		 * role 
		 */
		argv[4] = "auto";
		/*
		 * crypto 
		 */
		argv[5] = nvram_safe_get(strcat_r(prefix, "crypto", tmp));
		/*
		 * auth mode 
		 */
		argv[6] = nvram_safe_get(strcat_r(prefix, "akm", tmp));
		/*
		 * passphrase 
		 */
		argv[7] = nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp));
		/*
		 * ssid 
		 */
		argv[8] = nvram_safe_get(strcat_r(prefix, "ssid", tmp));
	}
	int pid;

	return _evalpid(argv, ">/dev/console", 0, &pid);
}
#endif

void stop_hotplug_net(void)
{
}

static void writenet(char *path, int cpumask, char *ifname)
{
	char dev[64];
	snprintf(dev, sizeof(dev), "/sys/class/net/%s/%s", ifname, path);

	int fd = open(dev, O_WRONLY);
	if (fd < 0)
		return;
	char mask[32];
	sprintf(mask, "%x", cpumask);
	write(fd, mask, strlen(mask));
	close(fd);
}

#ifdef HAVE_MADWIFI
#ifdef HAVE_MAC80211_COMPRESS

static void set_frame_compression(char *prefix, char *interface)
{
	char compr[32];
	sprintf(compr, "%s_fc_th", prefix);
	char *threshold = nvram_default_get(compr, "512"); // minimum framesize frequired for compression
	sprintf(compr, "%s_fc", prefix);
	if (nvram_default_matchi(compr, 1, 0)) {
		eval("iw", "dev", interface, "set", "compr", "lzo", threshold);
	} else if (nvram_default_matchi(compr, 2, 0)) {
		eval("iw", "dev", interface, "set", "compr", "lzma", threshold);
	} else if (nvram_default_matchi(compr, 3, 0)) {
		eval("iw", "dev", interface, "set", "compr", "lz4", threshold);
	} else if (nvram_default_matchi(compr, 4, 0)) {
		eval("iw", "dev", interface, "set", "compr", "zstd", threshold);
	} else {
		eval("iw", "dev", interface, "set", "compr", "off");
	}
}
#else
static void set_frame_compression(char *prefix, char *interface)
{
}
#endif
#endif
void set_mesh_params(char *interface);

void start_hotplug_net(void)
{
	char *interface, *action;

	interface = getenv("INTERFACE");
	if (!interface)
		return;
	action = getenv("ACTION");
	if (!action)
		return;
#ifdef _SC_NPROCESSORS_ONLN
	int cpucount = sysconf(_SC_NPROCESSORS_ONLN);
#else
	int cpucount = 1;
#endif

	if (!strcmp(action, "add")) {
		if (cpucount > 1) {
			int cpumask = 0;
			cpumask = (1 << cpucount) - 1;
			writenet("queues/rx-0/rps_cpus", cpumask, interface);
			writenet("queues/rx-1/rps_cpus", cpumask, interface);
			writenet("queues/rx-2/rps_cpus", cpumask, interface);
			writenet("queues/rx-3/rps_cpus", cpumask, interface);
			writenet("queues/rx-4/rps_cpus", cpumask, interface);
			writenet("queues/rx-5/rps_cpus", cpumask, interface);
			writenet("queues/rx-6/rps_cpus", cpumask, interface);
			writenet("queues/rx-7/rps_cpus", cpumask, interface);
			writenet("queues/tx-0/xps_cpus", cpumask, interface);
			writenet("queues/tx-1/xps_cpus", cpumask, interface);
			writenet("queues/tx-2/xps_cpus", cpumask, interface);
			writenet("queues/tx-3/xps_cpus", cpumask, interface);
			writenet("queues/tx-4/xps_cpus", cpumask, interface);
			writenet("queues/tx-5/xps_cpus", cpumask, interface);
			writenet("queues/tx-6/xps_cpus", cpumask, interface);
			writenet("queues/tx-7/xps_cpus", cpumask, interface);
		}
	}
#ifdef HAVE_MADWIFI
	// sysprintf("echo \"Hotplug %s=%s\" > /dev/console\n",action,interface);
	if (strncmp(interface, "wlan", 4))
		return;

	// try to parse
	char ifname[32];

	bzero(ifname, 32);
	char *index = strrchr(interface, '.');

	if (!index) {
		if (!strcmp(action, "add")) {
			set_frame_compression(interface, interface);
		}
		return;
	}
	strncpy(ifname, index + 1, sizeof(ifname) - 1);
	if (strncmp(ifname, "sta", 3)) {
		return;
	}
	bzero(ifname, 32);
	strncpy(ifname, interface, index - interface);
	char bridged[32];

	sprintf(bridged, "%s_bridged", ifname);
	char tmp[256];
	if (!strcmp(action, "add")) {
		set_frame_compression(ifname, interface);
		eval("ifconfig", interface, "up");
		if (nvram_matchi(bridged, 1)) {
			br_add_interface(getBridge(ifname, tmp), interface);
#ifdef HAVE_VLANTAGGING
			void apply_bridgeif(char *ifname, char *realport);
			apply_bridgeif(ifname, interface);
#endif
		}
	}
	if (!strcmp(action, "remove")) {
		eval("ifconfig", interface, "down");
		if (nvram_matchi(bridged, 1))
			br_del_interface(getBridge(ifname, tmp), interface);
	}
	return;
#else

	if (strncmp(interface, "wds", 3))
		return;

	cprintf("action: %s\n", action);
#ifdef HAVE_BCMMODERN
	if (!strcmp(action, "add")) {
#else
	if (!strcmp(action, "register")) {
#endif
#ifdef HAVE_MICRO
		br_init();
#endif
		/*
		 * Bring up the interface and add to the bridge 
		 */
		ifconfig(interface, IFUP, NULL, NULL);
		sleep(2);

		/*
		 * Bridge WDS interfaces if lazywds active 
		 */

		if (!strncmp(interface, "wds", 3) && nvram_matchi("wl_lazywds", 1))
			br_add_interface("br0", interface); // eval ("brctl",
		// "addif", "br0",
		// interface);
		/*
		 * Notify NAS of adding the interface 
		 */
		sleep(5);
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
		notify_nas("lan", interface, "up");
#endif
		{
			char word[256];
			br_set_stp_state("br0", getBridgeSTP("br0", word));
		}
#ifdef HAVE_MICRO
		br_shutdown();
#endif
	}
	cprintf("config done()\n");
	return;
#endif
}

#ifdef BUFFALO_JP
#define DEFAULT_MTU 1454
#else
#define DEFAULT_MTU 1492
#endif

int init_mtu(char *wan_proto)
{
	int mtu = nvram_geti("wan_mtu");

	if (strcmp(wan_proto, "pppoe") == 0 || strcmp(wan_proto, "pppoe_dual") == 0) {
		if (nvram_matchi("mtu_enable", 0)) { // Auto
			nvram_seti("mtu_enable", 1);
			nvram_seti("wan_mtu", DEFAULT_MTU); // set max value

		} else { // Manual
			if (mtu > DEFAULT_MTU)
				nvram_seti("wan_mtu", DEFAULT_MTU);
			if (mtu < 576)
				nvram_seti("wan_mtu", 576);
		}
	} else if (strcmp(wan_proto, "pptp") == 0 || strcmp(wan_proto, "l2tp") == 0) { // 1200 < mtu < 1400 (1460)
		if (mtu > 1460)
			nvram_seti("wan_mtu", 1460);

		if (mtu < 1200)
			nvram_seti("wan_mtu", 1200);

	} else { // 576 < mtu < 1500
		if (nvram_matchi("mtu_enable", 0)) { // Auto
			nvram_seti("wan_mtu", 1500); // set max value
		} else { // Manual
			if (mtu < 576)
				nvram_seti("wan_mtu", 576);
		}
	}
	return 0;
}
