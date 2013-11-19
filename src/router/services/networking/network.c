/*
 * network.c
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
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <sys/sysinfo.h>

typedef u_int64_t u64;
typedef u_int32_t u32;
typedef u_int16_t u16;
typedef u_int8_t u8;

typedef u_int64_t __u64;
typedef u_int32_t __u32;
typedef u_int16_t __u16;
typedef u_int8_t __u8;

#include <limits.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>
#include <error.h>
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
#include <linux/ethtool.h>
// #include <libbridge.h>

#include <bcmnvram.h>
#include <netconf.h>
#include <shutils.h>
#include <code_pattern.h>
#include <wlutils.h>
#include <utils.h>
#include <rc.h>
#include "ledcontrol.h"
#include <cy_conf.h>
#include <cymac.h>
#include <bcmutils.h>
#include <nvparse.h>
#include <etsockio.h>
#include <bcmparams.h>
#include <services.h>

extern int br_add_bridge(const char *brname);
extern int br_del_bridge(const char *brname);
extern int br_add_interface(const char *br, const char *dev);
extern int br_del_interface(const char *br, const char *dev);
extern int br_set_stp_state(const char *br, int stp_state);
void start_set_routes(void);

#define PTABLE_MAGIC 0xbadc0ded
#define PTABLE_SLT1 1
#define PTABLE_SLT2 2
#define PTABLE_ACKW 3
#define PTABLE_ADHM 4
#define PTABLE_END 0xffffffff

/*
 * phy types 
 */
#define	PHY_TYPE_A		0
#define	PHY_TYPE_B		1
#define	PHY_TYPE_G		2
#define	PHY_TYPE_NULL		0xf

#define WL_IOCTL(name, cmd, buf, len) (wl_ioctl((name), (cmd), (buf), (len)))

#define TXPWR_MAX 251
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

char *getMacAddr(char *ifname, char *mac)
{
	unsigned char hwbuff[16];
	int i = wl_hwaddr(ifname, hwbuff);

	if (i < 0)
		return NULL;
	sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", hwbuff[0], hwbuff[1], hwbuff[2], hwbuff[3], hwbuff[4], hwbuff[5]);
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
			if (val == 0xffffffff)	/* default */
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

void start_dhcpc(char *wan_ifname, char *pidfile, char *script, int fork)
{
	pid_t pid;
	char *wan_hostname = nvram_get("wan_hostname");
	char *vendorclass = nvram_get("dhcpc_vendorclass");
	char *requestip = nvram_get("dhcpc_requestip");
	int use_extra = 0;

	symlink("/sbin/rc", "/tmp/udhcpc");
	if (!script)
		script = "/tmp/udhcpc";
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

	char *dhcp_argv[] = { "udhcpc",
		"-i", wan_ifname,
		"-p", pidfile,
		"-s", script,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
		NULL, NULL,
	};

	int i = 7;

#ifdef HAVE_BUSYBOX_UDHCPC
	dhcp_argv[i++] = "-O";
	dhcp_argv[i++] = "routes";
	dhcp_argv[i++] = "-O";
	dhcp_argv[i++] = "msstaticroutes";
	dhcp_argv[i++] = "-O";
	dhcp_argv[i++] = "staticroutes";
#endif

	if (flags)
		dhcp_argv[i++] = flags;

	if (use_extra) {
		if (vendorclass != NULL && strlen(vendorclass) > 0) {
			dhcp_argv[i++] = "-V";
			dhcp_argv[i++] = vendorclass;
		}

		if (requestip != NULL && strlen(requestip) > 0) {
			dhcp_argv[i++] = "-r";
			dhcp_argv[i++] = requestip;
		}

		if (wan_hostname != NULL && strlen(wan_hostname) > 0) {
			dhcp_argv[i++] = "-H";
			dhcp_argv[i++] = wan_hostname;
		}
	}

	_evalpid(dhcp_argv, NULL, 0, &pid);

}

/*
 * Enable WET DHCP relay for ethernet clients 
 */
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
static int enable_dhcprelay(char *ifname)
{
	char name[80], *next;

	dprintf("%s\n", ifname);

	/*
	 * WET interface is meaningful only in bridged environment 
	 */
	if (strncmp(ifname, "br", 2) == 0) {
		foreach(name, nvram_safe_get("lan_ifnames"), next) {

			char mode[] = "wlXXXXXXXXXX_mode";
			int unit;

			/*
			 * make sure the interface is indeed of wl 
			 */
			if (wl_probe(name))
				continue;

			/*
			 * get the instance number of the wl i/f 
			 */
			wl_ioctl(name, WLC_GET_INSTANCE, &unit, sizeof(unit));
			snprintf(mode, sizeof(mode), "wl%d_mode", unit);

			/*
			 * enable DHCP relay, there should be only one WET i/f 
			 */
			if (nvram_match(mode, "wet")
			    || nvram_match(mode, "apstawet")) {
				uint32 ip;

				inet_aton(nvram_safe_get("lan_ipaddr"), (struct in_addr *)&ip);
				if (wl_iovar_setint(name, "wet_host_ipv4", ip))
					perror("wet_host_ipv4");
				break;
			}
		}
	}
	return 0;
}
#endif
static int wlconf_up(char *name)
{

	int phytype, gmode, val, ret;

#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880) || defined(HAVE_RT61)
	return -1;
#endif
	if (!strncmp(name, "vlan", 4))
		return -1;
	if (!strncmp(name, "br", 2))
		return -1;
#ifdef HAVE_ONLYCLIENT
	if (nvram_match("wl_mode", "ap")) {
		cprintf("this version does only support the client mode\n");
		nvram_set("wl_mode", "sta");
		nvram_commit();
	}
#endif
	int instance = get_wl_instance(name);
	
	if (instance == -1)
		return -1;	// no wireless device
	char prefix[16];
	sprintf(prefix,"wl%d",instance);
	if (nvram_nmatch("infra", "wl%d_mode", instance)) {
		nvram_nset("0", "wl%d_infra", instance);
	} else {
		nvram_nset("1", "wl%d_infra", instance);
	}
	eval("ifconfig", name, "up");
	if (has_2ghz(prefix) && has_ac(prefix)) {
		if (nvram_nmatch("1", "wl%d_turbo_qam", instance))
			eval("wl", "-i", name, "vht_features", "3");
		else
			eval("wl", "-i", name, "vht_features", "1");
	}

	ret = eval("wlconf", name, "up");
	/*
	 * eval("wl","radio","off"); eval("wl","atten","0","0","60");
	 * eval("wl","lrl","16"); eval("wl","srl","16");
	 * eval("wl","interference","0"); eval("wl","radio","on");
	 */
	gmode = atoi(nvram_nget("wl%d_gmode", instance));

	/*
	 * Get current phy type 
	 */
	WL_IOCTL(name, WLC_GET_PHYTYPE, &phytype, sizeof(phytype));

	// set preamble type for b cards
	if (phytype == PHY_TYPE_B || gmode == 0) {
		if (nvram_nmatch("long", "wl%d_plcphdr", instance))
			val = WLC_PLCP_LONG;
		else if (nvram_nmatch("short", "wl%d_plcphdr", instance))
			val = WLC_PLCP_SHORT;
		else
			val = WLC_PLCP_AUTO;
		WL_IOCTL(name, WLC_SET_PLCPHDR, &val, sizeof(val));
	}
	// adjust txpwr and txant
	val = atoi(nvram_nget("wl%d_txpwr", instance));
	if (val < 1 || val > TXPWR_MAX)
		val = TXPWR_DEFAULT;
	char pwr[8];
	sprintf(pwr, "%d", val);
#ifdef HAVE_TXPWRFIXED
	eval("wl", "-i", name, "txpwr1", "-m");
#else
	eval("wl", "-i", name, "txpwr1", "-m", "-o", pwr);
#endif
	/*
	 * Set txant 
	 */
	val = atoi(nvram_nget("wl%d_txant", instance));
	if (val < 0 || val > 3 || val == 2)
		val = 3;
	WL_IOCTL(name, WLC_SET_TXANT, &val, sizeof(val));

	/*
	 * if (nvram_match ("boardtype", "bcm94710dev")) { if (val == 0) val = 1;
	 * if (val == 1) val = 0; } 
	 */
	val = atoi(nvram_nget("wl%d_antdiv", instance));
	WL_IOCTL(name, WLC_SET_ANTDIV, &val, sizeof(val));

	/*
	 * search for "afterburner" string 
	 */
	char *afterburner = nvram_nget("wl%d_afterburner", instance);

	if (!strcmp(afterburner, "on"))
		eval("wl", "-i", name, "afterburner_override", "1");
	else if (!strcmp(afterburner, "off"))
		eval("wl", "-i", name, "afterburner_override", "0");
	else			// auto
		eval("wl", "-i", name, "afterburner_override", "-1");

	char *shortslot = nvram_nget("wl%d_shortslot", instance);

	if (!strcmp(shortslot, "long"))
		eval("wl", "-i", name, "shortslot_override", "0");
	else if (!strcmp(shortslot, "short"))
		eval("wl", "-i", name, "shortslot_override", "1");
	else			// auto
		eval("wl", "-i", name, "shortslot_override", "-1");

	// Set ACK Timing. Thx to Nbd
	char *v;
#if defined(HAVE_ACK)

	if ((v = nvram_nget("wl%d_distance", instance))) {
		rw_reg_t reg;
		uint32 shm;

		val = atoi(v);
		if (val == 0) {
			eval("wl", "-i", name, "noack", "1");
			// wlc_noack (0);
		} else {
			eval("wl", "-i", name, "noack", "0");
			// wlc_noack (1);
		}

		if (val) {
			val = 9 + (val / 150) + ((val % 150) ? 1 : 0);
			char strv[32];

			sprintf(strv, "%d", val);
			eval("wl", "-i", name, "acktiming", strv);
		}
	}
#endif

	/*
	 * if (nvram_match("wl0_mode","sta") || nvram_match("wl0_mode","infra"))
	 * { val = 0; WL_IOCTL(name, WLC_SET_WET, &val, sizeof(val)); if
	 * (nvram_match("wl_mode", "infra")){ val = 0; WL_IOCTL(name,
	 * WLC_SET_INFRA, &val, sizeof(val)); } else{ val = 1; WL_IOCTL(name,
	 * WLC_SET_INFRA, &val, sizeof(val)); } } 
	 */

	if (nvram_nmatch("infra", "wl%d_mode", instance)) {
		eval("wl", "-i", name, "infra", "0");
		eval("wl", "-i", name, "ssid", nvram_nget("wl%d_ssid", instance));
	}
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
	eval("wl", "-i", name, "vlan_mode", "0");
	char ifinst[32];

	sprintf(ifinst, "wl%d", instance);
	set_vifsmac(ifinst);

	char *next;
	char var[80];
	char *vifs = nvram_nget("%s_vifs", ifinst);
	if (vifs != NULL)
		foreach(var, vifs, next) {
		eval("ifconfig", var, "up");
		}
#endif
	eval("ifconfig", name, "up");
	return ret;
}

int isClient(void)
{
	if (getSTA())
		return 1;
	return 0;

}

void start_wlconf(void)
{
	int cnt = get_wl_instances();
	int c;

	for (c = 0; c < cnt; c++) {
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
		if (cnt > 1)
			eval("wl", "-i", get_wl_instance_name(c), "interference", "0");
#endif
		if (!nvram_nmatch("disabled", "wl%d_net_mode", c))
			wlconf_up(get_wl_instance_name(c));
	}
}

// #ifdef HAVE_PORTSETUP
#if defined(HAVE_RT2880) || defined(HAVE_RT61)
#define IFMAP(a) getRADev(a)
#else
#define IFMAP(a) (a)
#endif

static void do_portsetup(char *lan, char *ifname)
{
	char var[64];
	char var2[64];

	sprintf(var, "%s_bridged", IFMAP(ifname));
	if (nvram_default_match(var, "1", "1")) {
		br_add_interface(getBridge(IFMAP(ifname)), IFMAP(ifname));
	} else {
		ifconfig(ifname, IFUP, nvram_nget("%s_ipaddr", IFMAP(ifname)), nvram_nget("%s_netmask", ifname));
		sysprintf("/sbin/gratarp %s", ifname);
	}

}

// #endif

#define PORTSETUPWAN(a) if (strlen(a)>0 && strlen(nvram_safe_get ("wan_ifname2"))>0) \
	    { \
		strcpy(wan_ifname, nvram_safe_get ("wan_ifname2")); \
		nvram_set ("wan_ifnames", nvram_safe_get ("wan_ifname2"));\
	    } \
	  else \
	    { \
		strcpy(wan_ifname,a); \
		nvram_set ("wan_ifnames",a); \
	    }

void reset_hwaddr(char *ifname)
{
	struct ifreq ifr;
	char eabuf[32];
	int s;
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return;
	/*
	 * Get current LAN hardware address 
	 */

	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0) {
		nvram_set("lan_hwaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
		if (getRouterBrand() == ROUTER_DLINK_DIR320) {
			if (strlen(nvram_safe_get("et0macaddr")) == 12) {
				char wlmac[32];

				strcpy(wlmac, nvram_safe_get("wl0_hwaddr"));
				MAC_SUB(wlmac);
				nvram_set("et0macaddr", wlmac);
				nvram_unset("lan_hwaddr");
				nvram_unset("wan_hwaddr");
				// fis dlink quirk, by restarting system. utils.c will
				// automaticly assign the et0macaddr then
				nvram_commit();
				eval("event", "5", "1", "15");
			}
		}
		if (strlen(nvram_safe_get("et0macaddr")) == 0) {
			char *def = nvram_get("et0macaddr_safe");
			if (!def)
				def = nvram_safe_get("lan_hwaddr");
#ifdef HAVE_RB500
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_XSCALE
#ifndef HAVE_GATEWORX
			nvram_set("et0macaddr", def);
#endif
#endif
#ifdef HAVE_MAGICBOX
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_LAGUNA
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_VENTANA
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_RB600
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_FONERA
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_RT2880
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_LS2
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_LS5
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_SOLO51
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_WHRAG108
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_PB42
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_LSX
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_DANUBE
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_STORM
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_OPENRISC
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_ADM5120
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_TW6600
			nvram_set("et0macaddr", def);
#endif
#ifdef HAVE_CA8
			nvram_set("et0macaddr", def);
#endif
		}
	}
	close(s);
	// lock mac address on bridge if possible
	eval("ifconfig", ifname, "hw", "ether", nvram_safe_get("lan_hwaddr"));

}

#ifdef HAVE_3G
#define CANBRIDGE() (nvram_match( "wan_proto", "disabled" ) ||  nvram_match( "wan_proto", "3g" ) ||  nvram_match( "wan_proto", "iphone" ))
#else
#define CANBRIDGE() nvram_match( "wan_proto", "disabled" )
#endif

void start_lan(void)
{
	struct ifreq ifr;
	static unsigned char mac[20];
	int s;
	static char eabuf[32];
	static char lan_ifname[64];	//= strdup(nvram_safe_get("lan_ifname"));
	static char wan_ifname[64];	//= strdup(nvram_safe_get("wan_ifname"));
	static char lan_ifnames[128];	//= strdup(nvram_safe_get("lan_ifnames"));
	static char name[80];
	char *next, *svbuf;
	static char realname[80];
	static char wl_face[10];

	// don't let packages pass to iptables without ebtables loaded
	writeproc("/proc/sys/net/bridge/bridge-nf-call-arptables", "0");
	writeproc("/proc/sys/net/bridge/bridge-nf-call-ip6tables", "0");
	writeproc("/proc/sys/net/bridge/bridge-nf-call-iptables", "0");
	
	// fix list of active client in webif
	writeproc("/proc/sys/net/ipv4/neigh/default/gc_thresh1", "1");
	writeproc("/proc/sys/net/ipv4/neigh/default/gc_interval", "120");
	
	strcpy(lan_ifname, nvram_safe_get("lan_ifname"));
	strcpy(wan_ifname, nvram_safe_get("wan_ifname"));
	strcpy(lan_ifnames, nvram_safe_get("lan_ifnames"));

	if (strlen(nvram_safe_get("wan_default")) > 0) {
		PORTSETUPWAN(nvram_safe_get("wan_default"));	// setup
		// default
		// wan ports, 
		// or
		// reassign
		// wan if
		// required
		// by network 
		// setup

		/*
		 * add wan ifname to lan_ifnames if we use fullswitch 
		 */
		if (nvram_match("fullswitch", "1")
		    && (getSTA() || getWET()
			|| nvram_match("wan_proto", "disabled"))) {
			if (!nvram_match("fullswitch_set", "1")) {
				nvram_set("lan_default", lan_ifnames);
				nvram_set("fullswitch_set", "1");
			}
			sprintf(lan_ifnames, "%s %s", nvram_safe_get("lan_default"), nvram_safe_get("wan_default"));
			strcpy(wan_ifname, "");
		} else {
			if (nvram_match("fullswitch_set", "1")) {
				strcpy(lan_ifnames, nvram_safe_get("lan_default"));
				nvram_unset("lan_default");
				strcpy(wan_ifname, nvram_safe_get("wan_default"));
				nvram_unset("fullswitch_set");
			}
		}
	}

	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return;

#define nvram_setz(a,b) strcpy(a,b)
	nvram_setz(lan_ifname, "br0");
#ifdef HAVE_RB500
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 ath0 ath1 ath2 ath3 ath4 ath5");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 ath0 ath1 ath2 ath3 ath4 ath5");
		PORTSETUPWAN("eth0");
	}

	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
#endif

#ifdef HAVE_MAGICBOX
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0 ath1");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth1 eth1 ath0 ath1");
		PORTSETUPWAN("eth0");
	}

	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
	MAC_ADD(mac);
	ether_atoe(mac, ifr.ifr_hwaddr.sa_data);
	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCSIFHWADDR, &ifr);
#endif
#ifdef HAVE_UNIWIP
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 ath0 ath1");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 ath0 ath1");
		PORTSETUPWAN("eth0");
	}

	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
	MAC_ADD(mac);
#elif HAVE_WDR4900
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "vlan1 vlan2 ath0 ath1");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "vlan1 vlan2 ath0 ath1");
		PORTSETUPWAN("vlan2");
	}

	strncpy(ifr.ifr_name, "vlan1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
	MAC_ADD(mac);
	ether_atoe(mac, ifr.ifr_hwaddr.sa_data);
	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCSIFHWADDR, &ifr);
#elif HAVE_RB600
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 ath0 ath1 ath2 ath3 ath4 ath5 ath6 ath7");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 ath0 ath1 ath2 ath3 ath4 ath5 ath6 ath7");
		PORTSETUPWAN("eth0");
	}

	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
	MAC_ADD(mac);
	ether_atoe(mac, ifr.ifr_hwaddr.sa_data);
	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCSIFHWADDR, &ifr);
#endif
#if defined(HAVE_FONERA) && !defined(HAVE_DIR300) && !defined(HAVE_WRT54G2) && !defined(HAVE_MR3202A)  && !defined(HAVE_RTG32)
	if (getRouterBrand() == ROUTER_BOARD_FONERA2200) {
		if (getSTA() || getWET() || CANBRIDGE()) {
			nvram_setz(lan_ifnames, "vlan0 vlan1 ath0");
			PORTSETUPWAN("");
		} else {
			nvram_setz(lan_ifnames, "vlan1 vlan0 ath0");
			PORTSETUPWAN("vlan1");
		}
	} else {
		if (getSTA() || getWET()
		    || CANBRIDGE()) {
			nvram_setz(lan_ifnames, "eth0 ath0");
			PORTSETUPWAN("");
		} else {
			nvram_setz(lan_ifnames, "eth0 ath0");
			PORTSETUPWAN("eth0");
		}
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_WRT54G2
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "vlan1 vlan2 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "vlan1 vlan2 ath0");
		PORTSETUPWAN("vlan2");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_RTG32
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "vlan1 vlan2 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "vlan1 vlan2 ath0");
		PORTSETUPWAN("vlan2");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_DIR300
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "vlan1 vlan2 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "vlan1 vlan2 ath0");
		PORTSETUPWAN("vlan2");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_RS
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0 ath1 ath2");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0 ath1 ath2");
		PORTSETUPWAN("eth0");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WA901
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 ath0");
		PORTSETUPWAN("eth0");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WR941
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "vlan0 vlan1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "vlan0 vlan1 ath0");
		PORTSETUPWAN("vlan1");
	}
	strncpy(ifr.ifr_name, "vlan0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WR1043
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "vlan1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "vlan1 ath0");
		PORTSETUPWAN("vlan2");
	}
	strncpy(ifr.ifr_name, "vlan1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_AP83
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("eth1");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WZRG450
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "vlan1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "vlan1 ath0");
		PORTSETUPWAN("vlan2");
	}
#ifdef HAVE_SWCONFIG
	system("swconfig dev eth0 set reset 1");
	system("swconfig dev eth0 set enable_vlan 1");
	if (nvram_match("wan_proto", "disabled")
	    && nvram_match("fullswitch", "1")) {
		system("swconfig dev eth0 vlan 1 set ports \"0t 1 2 3 4 5\"");
	} else {
#ifdef HAVE_WZRG300NH2
		system("swconfig dev eth0 vlan 1 set ports \"0t 1 3 4 5\"");
		system("swconfig dev eth0 vlan 2 set ports \"0t 2\"");
#else
		system("swconfig dev eth0 vlan 1 set ports \"0t 2 3 4 5\"");
		system("swconfig dev eth0 vlan 2 set ports \"0t 1\"");
#endif
	}
	system("swconfig dev eth0 set apply");
#endif
	strncpy(ifr.ifr_name, "vlan1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WZRHPAG300NH
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("eth1");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_DIR632
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("eth1");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_AP94
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("eth1");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_HORNET
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
#ifdef HAVE_MAKSAT
		PORTSETUPWAN("eth0");
#elif HAVE_ONNET
		PORTSETUPWAN("eth0");
#else
		PORTSETUPWAN("eth1");
#endif
	}
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WNR2000
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("eth0");
	}
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WASP
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "vlan1 vlan2 ath0 ath1");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "vlan1 vlan2 ath0 ath1");
		PORTSETUPWAN("vlan2");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WDR2543
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "vlan1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "vlan1 ath0");
		PORTSETUPWAN("vlan2");
	}
	strncpy(ifr.ifr_name, "vlan1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WHRHPGN
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("eth0");
	}
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif CONFIG_JJAP93
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 ath0");
		PORTSETUPWAN("eth0");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_JJAP005
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("eth1");
	}
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_JJAP501
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("eth1");
	}
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_AC722
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("eth1");
	}
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_AC622
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("eth1");
	}
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_JA76PF
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("eth1");
	}
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_ALFAAP94
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0 ath1 ath2 ath3");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0 ath1 ath2 ath3");
		PORTSETUPWAN("eth1");
	}
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_JWAP003
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("eth1");
	}
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_DIR615E
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("eth0");
	}
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WA901v1
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth1 ath0");
		PORTSETUPWAN("eth1");
	}
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_CARAMBOLA
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "vlan1 vlan2 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "vlan1 vlan2 ath0");
		PORTSETUPWAN("vlan2");
	}
	strncpy(ifr.ifr_name, "vlan1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WR703
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth1 ath0");
		PORTSETUPWAN("eth1");
	}
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WA7510
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth1 ath0");
		PORTSETUPWAN("eth1");
	}
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WR741
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		PORTSETUPWAN("eth0");
	}
	strncpy(ifr.ifr_name, "eth1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_UBNTM
	int brand = getRouterBrand();
	int devnum = 2;
	switch (brand) {
	case ROUTER_BOARD_BS2M:
	case ROUTER_BOARD_UNIFI:
	case ROUTER_BOARD_BS5M:
	case ROUTER_BOARD_R2M:
	case ROUTER_BOARD_R5M:
		devnum = 1;
		break;
	case ROUTER_BOARD_NS2M:
	case ROUTER_BOARD_NS5M:
		devnum = 2;
		break;
	default:
		devnum = 2;
		break;
	}

	if (getSTA() || getWET() || CANBRIDGE()) {
		if (devnum == 2)
			nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		else
			nvram_setz(lan_ifnames, "eth0 ath0");
		PORTSETUPWAN("");
	} else {
		if (devnum == 2)
			nvram_setz(lan_ifnames, "eth0 eth1 ath0");
		else
			nvram_setz(lan_ifnames, "eth0 ath0");
		PORTSETUPWAN("eth0");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_WP546
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0 ath1");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0 ath1");
		PORTSETUPWAN("eth1");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_LSX
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 ath0");
		PORTSETUPWAN("eth0");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_DANUBE
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 ath0");
		stop_atm();
		start_atm();
		PORTSETUPWAN("nas0");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_RT2880
	int rb = getRouterBrand();
	if (rb == ROUTER_BOARD_ECB9750 || rb == ROUTER_BOARD_EAP9550 || rb == ROUTER_BOARD_TECHNAXX3G)	// lets load
	{
		if (getSTA() || getWET()
		    || CANBRIDGE()) {
			nvram_setz(lan_ifnames, "eth2 ra0");
			PORTSETUPWAN("");
		} else {
			nvram_setz(lan_ifnames, "eth2 ra0");
			PORTSETUPWAN("eth2");
		}
	} else {
		if (getSTA() || getWET()
		    || CANBRIDGE()) {
			nvram_setz(lan_ifnames, "vlan1 vlan2 ra0");
			PORTSETUPWAN("");
		} else {
			nvram_setz(lan_ifnames, "vlan1 vlan2 ra0");
			PORTSETUPWAN("vlan2");
		}
	}
	strncpy(ifr.ifr_name, "eth2", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_WBD222
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 eth2 ath0 ath1 ath2");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 eth2 ath0 ath1 ath2");
		PORTSETUPWAN("eth0");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#elif HAVE_STORM
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 ath0");
		PORTSETUPWAN("eth0");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_OPENRISC
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 ath0");
		PORTSETUPWAN("eth0");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_NORTHSTAR
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "vlan1 vlan2 eth1 eth2");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "vlan1 vlan2 eth1 eth2");
		PORTSETUPWAN("vlan2");
	}
	strncpy(ifr.ifr_name, "vlan1", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_LAGUNA
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 ath0 ath1 ath2 ath3");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 ath0 ath1 ath2 ath3");
		PORTSETUPWAN("eth0");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_VENTANA
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 ath0 ath1 ath2 ath3 ath4 ath5 ath6");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 ath0 ath1 ath2 ath3 ath4 ath5 ath6");
		PORTSETUPWAN("eth0");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_ADM5120

	if (getRouterBrand() == ROUTER_BOARD_WP54G || getRouterBrand() == ROUTER_BOARD_NP28G) {
		if (getSTA() || getWET()
		    || CANBRIDGE()) {
			nvram_setz(lan_ifnames, "eth0 eth1 ath0");
			PORTSETUPWAN("");
		} else {
			nvram_setz(lan_ifnames, "eth0 eth1 ath0");
			PORTSETUPWAN("eth1");
		}
	} else {

		if (getSTA() || getWET()
		    || CANBRIDGE()) {
			nvram_setz(lan_ifnames, "eth0 ath0");
			PORTSETUPWAN("");
		} else {
			nvram_setz(lan_ifnames, "eth0 ath0");
			PORTSETUPWAN("eth0");
		}
	}

	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_MR3202A
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "vlan1 vlan2 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "vlan1 vlan2 ath0");
		PORTSETUPWAN("vlan2");
	}
	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif

#if defined(HAVE_LS2) || defined(HAVE_SOLO51)
	if (getSTA() || getWET() || CANBRIDGE()) {
#if defined(HAVE_NS2) || defined(HAVE_BS2) || defined(HAVE_LC2) || defined(HAVE_BS2HP) || defined(HAVE_MS2) || defined(HAVE_PICO2) || defined(HAVE_PICO2HP)
		nvram_setz(lan_ifnames, "eth0 ath0");
		PORTSETUPWAN("");
#else
		nvram_setz(lan_ifnames, "vlan0 vlan2 ath0");
		PORTSETUPWAN("");
#endif
	} else {
#if defined(HAVE_NS2) || defined(HAVE_BS2) || defined(HAVE_LC2) || defined(HAVE_BS2HP) || defined(HAVE_MS2) || defined(HAVE_PICO2) || defined(HAVE_PICO2HP)
		nvram_setz(lan_ifnames, "eth0 ath0");
		PORTSETUPWAN("eth0");
#else
#ifdef HAVE_BWRG1000
		nvram_setz(lan_ifnames, "vlan0 vlan2 ath0");
		PORTSETUPWAN("vlan2");
#else
		nvram_setz(lan_ifnames, "vlan0 vlan2 ath0");
		PORTSETUPWAN("vlan0");
#endif
#endif
	}

	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_LS5
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 ath0");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 ath0");
		PORTSETUPWAN("eth0");
	}

	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_TW6600
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 ath0 ath1");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 ath0 ath1");
		PORTSETUPWAN("eth0");
	}

	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_PB42
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0 ath1");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0 ath1");
		PORTSETUPWAN("eth0");
	}

	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_WHRAG108
	if (getSTA() || getWET() || CANBRIDGE()) {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0 ath1");
		PORTSETUPWAN("");
	} else {
		nvram_setz(lan_ifnames, "eth0 eth1 ath0 ath1");
		PORTSETUPWAN("eth1");
	}

	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_CA8
	if (getSTA() || getWET() || CANBRIDGE()) {
		if (getRouterBrand() == ROUTER_BOARD_CA8PRO) {
			nvram_setz(lan_ifnames, "vlan0 vlan1 ath0");
			PORTSETUPWAN("");
		} else if (getRouterBrand() == ROUTER_BOARD_RCAA01) {
			nvram_setz(lan_ifnames, "vlan0 vlan1 ath0 ath1");
			PORTSETUPWAN("");
		} else if (getRouterBrand() == ROUTER_BOARD_RDAT81) {
			nvram_setz(lan_ifnames, "eth0 ath0 ath1");
			PORTSETUPWAN("");
		} else {
			nvram_setz(lan_ifnames, "eth0 ath0");
			PORTSETUPWAN("");
		}
	} else {
		if (getRouterBrand() == ROUTER_BOARD_CA8PRO) {
			nvram_setz(lan_ifnames, "vlan0 vlan1 ath0");
			PORTSETUPWAN("vlan1");
		} else if (getRouterBrand() == ROUTER_BOARD_RCAA01) {
			nvram_setz(lan_ifnames, "vlan0 vlan1 ath0 ath1");
			PORTSETUPWAN("vlan1");
		} else if (getRouterBrand() == ROUTER_BOARD_RDAT81) {
			nvram_setz(lan_ifnames, "eth0 ath0 ath1");
			PORTSETUPWAN("eth0");
		} else {
			nvram_setz(lan_ifnames, "eth0 ath0");
			PORTSETUPWAN("eth0");
		}
	}

	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	strcpy(mac, nvram_safe_get("et0macaddr"));
#endif
#ifdef HAVE_GATEWORX
	if (getSTA() || getWET() || CANBRIDGE()) {
		if (getRouterBrand() == ROUTER_BOARD_GATEWORX_SWAP) {
			if (nvram_match("intel_eth", "1"))
				nvram_setz(lan_ifnames, "ixp0 eth0 eth1 ath0 ath1 ath2 ath3 ofdm");
			else
				nvram_setz(lan_ifnames, "ixp0 ath0 ath1 ath2 ath3 ofdm");
			PORTSETUPWAN("");
		} else if (getRouterBrand() == ROUTER_BOARD_GATEWORX_GW2345) {
			if (nvram_match("intel_eth", "1"))
				nvram_setz(lan_ifnames, "ixp0 ixp1 eth0 eth1 ath0 ath1 ath2 ath3 ofdm");
			else
				nvram_setz(lan_ifnames, "ixp0 ixp1 ath0 ath1 ath2 ath3 ofdm");
			PORTSETUPWAN("");
		} else {
			if (nvram_match("intel_eth", "1"))
				nvram_setz(lan_ifnames, "ixp0 ixp1 eth0 eth1 ath0 ath1 ath2 ath3 ofdm");
			else
				nvram_setz(lan_ifnames, "ixp0 ixp1 ath0 ath1 ath2 ath3 ofdm");
			PORTSETUPWAN("");
		}
	} else {
		if (getRouterBrand() == ROUTER_BOARD_GATEWORX_SWAP) {
			if (nvram_match("intel_eth", "1"))
				nvram_setz(lan_ifnames, "eth0 eth1 ixp0 ath0 ath1 ath2 ath3 ofdm");
			else
				nvram_setz(lan_ifnames, "ixp0 ath0 ath1 ath2 ath3 ofdm");
			PORTSETUPWAN("ixp0");
		} else if (getRouterBrand() == ROUTER_BOARD_GATEWORX_GW2345) {
#ifdef HAVE_MI424WR
			if (nvram_match("intel_eth", "1"))
				nvram_setz(lan_ifnames, "eth0 eth1 ixp0 ixp1 ath0 ath1 ath2 ath3 ofdm");
			else
				nvram_setz(lan_ifnames, "ixp0 ixp1 ath0 ath1 ath2 ath3 ofdm");
			PORTSETUPWAN("ixp0");
#else
			if (nvram_match("intel_eth", "1"))
				nvram_setz(lan_ifnames, "eth0 eth1 ixp0 ixp1 ath0 ath1 ath2 ath3 ofdm");
			else
				nvram_setz(lan_ifnames, "ixp0 ixp1 ath0 ath1 ath2 ath3 ofdm");
			PORTSETUPWAN("ixp1");
#endif
		} else {
#ifdef HAVE_XIOCOM
			if (nvram_match("intel_eth", "1"))
				nvram_setz(lan_ifnames, "eth0 eth1 ixp0 ixp1 ath0 ath1 ath2 ath3 ofdm");
			else
				nvram_setz(lan_ifnames, "ixp0 ixp1 ath0 ath1 ath2 ath3 ofdm");

			PORTSETUPWAN("ixp0");

#else
			if (nvram_match("intel_eth", "1"))
				nvram_setz(lan_ifnames, "eth0 eth1 ixp0 ixp1 ath0 ath1 ath2 ath3 ofdm");
			else
				nvram_setz(lan_ifnames, "ixp0 ixp1 ath0 ath1 ath2 ath3 ofdm");

			PORTSETUPWAN("ixp1");
#endif
		}
	}
	strncpy(ifr.ifr_name, "ixp0", IFNAMSIZ);
	if (ioctl(s, SIOCGIFHWADDR, &ifr) != 0) {
		strncpy(ifr.ifr_name, "ixp0", IFNAMSIZ);
		ioctl(s, SIOCGIFHWADDR, &ifr);
	}
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
#endif
#ifdef HAVE_X86
	if (getSTA() || getWET()) {
#ifdef HAVE_NOWIFI
		nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10");
#else
		if (nvram_match("wifi_bonding", "1"))
			nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10 bond0");
		else
			nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10 ath0 ath1 ath2 ath3 ath4 ath5 ath6 ath7 ath8");

#endif
		PORTSETUPWAN("");
	} else if (CANBRIDGE()) {
#ifdef HAVE_NOWIFI
		nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10");
#else
		if (nvram_match("wifi_bonding", "1"))
			nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10 bond0");
		else
			nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10 ath0 ath1 ath2 ath3 ath4 ath5 ath6 ath7 ath8");
#endif
		PORTSETUPWAN("");
	} else {
#ifdef HAVE_NOWIFI
		nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10");
#else
		if (nvram_match("wifi_bonding", "1"))
			nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10 bond0");
		else
			nvram_setz(lan_ifnames, "eth0 eth1 eth2 eth3 eth4 eth5 eth6 eth7 eth8 eth9 eth10 ath0 ath1 ath2 ath3 ath4 ath5 ath6 ath7 ath8");
#endif
#ifdef HAVE_GW700
		PORTSETUPWAN("eth1");
#else
		PORTSETUPWAN("eth0");
#endif

	}

	strncpy(ifr.ifr_name, "eth0", IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_match("et0macaddr", ""))
		nvram_set("et0macaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
#endif

	if (!nvram_match("lan_ifname", lan_ifname)
	    || !nvram_match("wan_ifname", wan_ifname)
	    || !nvram_match("lan_ifnames", lan_ifnames)) {
		nvram_set("lan_ifname", lan_ifname);
		nvram_set("wan_ifname", wan_ifname);
		nvram_set("lan_ifnames", lan_ifnames);
		nvram_commit();
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
	diag_led(DIAG, STOP_LED);	// stop that blinking
	strcpy(wl_face, get_wdev());
#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880) || defined(HAVE_RT61)
#ifndef HAVE_NOWIFI
	deconfigure_wifi();
#endif
#else
	eval("wlconf", wl_face, "down");
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

	if (nvram_match("mac_clone_enable", "1") && nvram_invmatch("def_whwaddr", "00:00:00:00:00:00") && nvram_invmatch("def_whwaddr", "")) {
		ether_atoe(nvram_safe_get("def_whwaddr"), ifr.ifr_hwaddr.sa_data);

	} else {

		int instance = get_wl_instance(wl_face);
		getWirelessMac(mac, instance);

		ether_atoe(mac, ifr.ifr_hwaddr.sa_data);

		if (nvram_match("wl0_hwaddr", "") || !nvram_get("wl0_hwaddr")) {
			nvram_set("wl0_hwaddr", mac);
			nvram_commit();
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
	start_config_macs(wl_face);
#endif
	if (getSTA()) {
		unsigned char mac[20];

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
	char eadline[64];

	memset(eadline, 0, 64);
#endif

	if (strncmp(lan_ifname, "br0", 3) == 0) {
		br_add_bridge(lan_ifname);
		if (nvram_match("lan_stp", "0"))
			br_set_stp_state(lan_ifname, 0);
		else
			br_set_stp_state(lan_ifname, 1);
#ifdef HAVE_MICRO
		br_set_bridge_forward_delay(lan_ifname, 2);
#else
		br_set_bridge_forward_delay(lan_ifname, 2);
#endif
#ifdef HAVE_EAD
		eval("killall", "-9", "ead");
#endif
		foreach(name, lan_ifnames, next) {
			int ex = ifexists(name);
			if (!ex)
				continue;
#ifdef HAVE_EAD
#if defined(HAVE_RT2880) || defined(HAVE_MADWIFI) || defined(HAVE_RT61)
			if (strncmp(name, "ath", 3) && strncmp(name, "ra", 2))
#else
			if (wl_probe(name))
#endif
				sprintf(eadline, "%s%s %s ", eadline, "-d", name);
#endif
			if (nvram_match("wan_ifname", name))
				continue;
#if defined(HAVE_MADWIFI) && !defined(HAVE_RB500) && !defined(HAVE_XSCALE) && !defined(HAVE_LAGUNA) && !defined(HAVE_VENTANA)   && !defined(HAVE_MAGICBOX) && !defined(HAVE_RB600) && !defined(HAVE_FONERA) && !defined(HAVE_WHRAG108) && !defined(HAVE_X86) && !defined(HAVE_LS2) && !defined(HAVE_LS5) && !defined(HAVE_CA8) && !defined(HAVE_TW6600) && !defined(HAVE_PB42) && !defined(HAVE_LSX) && !defined(HAVE_DANUBE) && !defined(HAVE_STORM) && !defined(HAVE_OPENRISC) && !defined(HAVE_ADM5120) && !defined(HAVE_RT2880) && !defined(HAVE_SOLO51)
			if (!strcmp(name, "eth2")) {
				strcpy(realname, "ath0");
			} else
#endif
				strcpy(realname, name);

			cprintf("name=[%s] lan_ifname=[%s]\n", realname, lan_ifname);

			/*
			 * Bring up interface 
			 */
			if (ifconfig(realname, IFUP, "0.0.0.0", NULL))
				continue;

			// set proper mtu

			if (strncmp(realname, "ath", 3) != 0) {	// this is not an ethernet driver
				eval("ifconfig", realname, "down");	//fixup for some ethernet drivers
			}
			eval("ifconfig", realname, "mtu", getMTU(realname));
			eval("ifconfig", realname, "txqueuelen", getTXQ(realname));
			if (strncmp(realname, "ath", 3) != 0) {	// this is not an ethernet driver
				eval("ifconfig", realname, "up");	//fixup for some ethernet drivers
			}

			/*
			 * Set the logical bridge address to that of the first interface 
			 */

#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
			strncpy(ifr.ifr_name, lan_ifname, IFNAMSIZ);
			if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0 && (memcmp(ifr.ifr_hwaddr.sa_data, "\0\0\0\0\0\0", ETHER_ADDR_LEN) == 0 || (((ifr.ifr_hwaddr.sa_data[0] & 0x01) == 0)
																	   && ((ifr.ifr_hwaddr.sa_data[1] & 0x02) == 0x02)))
			    && strcmp(wl_face, realname) == 0) {
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
			/*
			 * If not a wl i/f then simply add it to the bridge 
			 */
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
			if (wlconf_up(name)) {
				// #ifdef HAVE_PORTSETUP
				do_portsetup(lan_ifname, name);
				// #else
				// br_add_interface (getBridge (name), name);
				// #endif
			} else {

				if (nvram_match("mac_clone_enable", "1") && nvram_invmatch("def_whwaddr", "00:00:00:00:00:00")
				    && nvram_invmatch("def_whwaddr", "")) {
					ether_atoe(nvram_safe_get("def_whwaddr"), ifr.ifr_hwaddr.sa_data);

				} else {
					int instance = get_wl_instance(name);
					getWirelessMac(mac, instance);

					ether_atoe(mac, ifr.ifr_hwaddr.sa_data);

					if (instance == -1)
						continue;	// no wireless device
					if (nvram_nmatch("", "wl%d_hwaddr", instance)
					    || !nvram_nget("wl%d_hwaddr", instance)) {
						nvram_nset(mac, "wl%d_hwaddr", instance);
						nvram_commit();
						ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
						strncpy(ifr.ifr_name, name, IFNAMSIZ);

						eval("wl", "-i", name, "down");
						if (ioctl(s, SIOCSIFHWADDR, &ifr) == -1)
							perror("Write wireless mac fail : ");
						else
							cprintf("Write wireless mac successfully\n");
						eval("wl", "-i", name, "up");
						start_config_macs(name);
					}
				}

#else
			cprintf("configure %s\n", name);
			if (strcmp(name, "wl0"))	// check if the interface is a
				// buffalo wireless
			{
				do_portsetup(lan_ifname, name);
			} else {

#endif
				/*
				 * get the instance number of the wl i/f 
				 */
				char wl_name[] = "wlXXXXXXXXXX_mode";
				int unit;

#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880) || defined(HAVE_RT61)
				unit = 0;
#else
				wl_ioctl(name, WLC_GET_INSTANCE, &unit, sizeof(unit));
#endif
				snprintf(wl_name, sizeof(wl_name), "wl%d_mode", unit);
				/*
				 * Do not attach the main wl i/f if in wds or client/adhoc 
				 */

				led_control(LED_BRIDGE, LED_OFF);
				if (nvram_match(wl_name, "wet")
				    || nvram_match(wl_name, "apstawet")) {
					ifconfig(name, IFUP | IFF_ALLMULTI, NULL, NULL);	// from 
					// up
					br_add_interface(getBridge(IFMAP(name)), name);
					led_control(LED_BRIDGE, LED_ON);
					/* Enable host DHCP relay */
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
					if (nvram_match("lan_dhcp", "1")) {
						wl_iovar_set(name, "wet_host_mac", ifr.ifr_hwaddr.sa_data, ETHER_ADDR_LEN);
					}
					/* Enable WET DHCP relay if requested */
					if (nvram_match("dhcp_relay", "1"))	// seems to fix some dhcp problems, also Netgear does it this way
					{
						enable_dhcprelay(lan_ifname);
					}
#endif
					do_mssid(name);
				}
#ifdef HAVE_WAVESAT
				if (nvram_match(wl_name, "bridge")) {
					ifconfig(name, IFUP | IFF_ALLMULTI, NULL, NULL);	// from 
					// up
					br_add_interface(getBridge(IFMAP(name)), name);
					led_control(LED_BRIDGE, LED_ON);
				}
#endif

				if (nvram_match(wl_name, "ap")) {

					do_portsetup(lan_ifname, name);
					// br_add_interface (getBridge (name), name); //eval
					// ("brctl", "addif", lan_ifname, name);
					do_mssid(name);
				}
				if (nvram_match(wl_name, "apsta")) {
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
					// eval ("wl", "ap", "0");
					eval("wl", "-i", name, "ap", "0");
					// eval ("wl", "infra", "1");
					eval("wl", "-i", name, "infra", "1");
					wl_ioctl(wl_name, WLC_SCAN, svbuf, sizeof(svbuf));
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
#ifndef HAVE_FON
					if (nvram_match("fon_enable", "0"))
						do_mssid(name);
#endif
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
					wl_ioctl(wl_name, WLC_SCAN, svbuf, sizeof(svbuf));
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
					wl_ioctl(name, WLC_SCAN, svbuf, sizeof(svbuf));
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
					do_mssid(name);
				}
#endif

			}

		}
	}
#ifdef HAVE_EAD
	if (strlen(eadline) > 0)
		sysprintf("ead %s -B", eadline);
#endif

	/*
	 * specific non-bridged lan i/f 
	 */
	if (strcmp(lan_ifname, "")) {	// FIXME
		/*
		 * Bring up interface 
		 */
		ifconfig(lan_ifname, IFUP, NULL, NULL);
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
			wl_ioctl(lan_ifname, WLC_GET_INSTANCE, &unit, sizeof(unit));
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
	reset_hwaddr(lan_ifname);
	ifconfig(lan_ifname, IFUP, nvram_safe_get("lan_ipaddr"), nvram_safe_get("lan_netmask"));
	sysprintf("/sbin/gratarp %s", lan_ifname);

	char staticlan[32];

	sprintf(staticlan, "%s:0", lan_ifname);
#if defined(HAVE_FONERA) || defined(HAVE_CA8) && !defined(HAVE_MR3202A)
	if (getRouterBrand() != ROUTER_BOARD_FONERA2200 && getRouterBrand() != ROUTER_BOARD_CA8PRO && getRouterBrand() != ROUTER_BOARD_RCAA01)
		if (nvram_match("ath0_mode", "sta")
		    || nvram_match("ath0_mode", "wdssta")
		    || nvram_match("ath0_mode", "wet")
		    || CANBRIDGE()) {
#endif

			eval("ifconfig", "eth0:0", "down");
			// add fallback ip
			eval("ifconfig", staticlan, "169.254.255.1", "netmask", "255.255.0.0");

#if defined(HAVE_FONERA) || defined(HAVE_CA8) && !defined(HAVE_MR3202A)
		} else
			eval("ifconfig", staticlan, "0.0.0.0", "down");
#endif
	close(s);

#if defined(HAVE_MADWIFI) || defined(HAVE_RT2880) || defined(HAVE_RT61)

#if defined(HAVE_RT2880) || defined(HAVE_RT61)
#define getWifi(a) a
#define getWDSSTA() NULL
#endif
#ifndef HAVE_NOWIFI
	if (nvram_match("mac_clone_enable", "1") && nvram_invmatch("def_hwaddr", "00:00:00:00:00:00") && nvram_invmatch("def_hwaddr", "")) {
#ifdef HAVE_MADWIFI
		char *wifi = "wifi0";
#else
		char *wifi = "ra0";
#endif
		eval("ifconfig", wifi, "down");
		eval("ifconfig", wifi, "hw", "ether", nvram_safe_get("def_whwaddr"));
//              eval("ifconfig", wifi, "up");
	}
	if (nvram_match("mac_clone_enable", "1") && nvram_invmatch("def_whwaddr", "00:00:00:00:00:00") && nvram_invmatch("def_whwaddr", "")) {
#ifdef HAVE_MADWIFI
		char *wifi = "wifi0";
#else
		char *wifi = "ra0";
#endif
		eval("ifconfig", wifi, "down");
		eval("ifconfig", wifi, "hw", "ether", nvram_safe_get("def_whwaddr"));
//              eval("ifconfig", wifi, "up");
	}
	configure_wifi();
#endif
#endif
#ifdef HAVE_WAVESAT
	configure_wimax();
#endif

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

		sprintf(br1enable, "ath%d_br1_enable", c);
		sprintf(br1ipaddr, "ath%d_br1_ipaddr", c);
		sprintf(br1netmask, "ath%d_br1_netmask", c);
#else
		char br1enable[32];
		char br1ipaddr[32];
		char br1netmask[32];

		sprintf(br1enable, "wl%d_br1_enable", c);
		sprintf(br1ipaddr, "wl%d_br1_ipaddr", c);
		sprintf(br1netmask, "wl%d_br1_netmask", c);
#endif
		if (nvram_get(br1enable) == NULL)
			nvram_set(br1enable, "0");
		if (nvram_get(br1ipaddr) == NULL)
			nvram_set(br1ipaddr, "0.0.0.0");
		if (nvram_get(br1netmask) == NULL)
			nvram_set(br1netmask, "255.255.255.0");
		if (nvram_match(br1enable, "1")) {
			ifconfig("br1", 0, 0, 0);

			// eval ("ifconfig", "br1", "down");
			br_del_bridge("br1");
			br_add_bridge("br1");

			if (nvram_match("lan_stp", "0"))
				br_set_stp_state("br1", 0);	// eval ("brctl", "stp",
			// "br1", "off");
			else
				br_set_stp_state("br1", 1);	// eval ("brctl", "stp",
			// "br1", "off");
			br_set_bridge_forward_delay("br1", 2);

			/*
			 * Bring up and configure br1 interface 
			 */
			if (nvram_invmatch(br1ipaddr, "0.0.0.0")) {
				ifconfig("br1", IFUP, nvram_safe_get(br1ipaddr), nvram_safe_get(br1netmask));

				if (nvram_match("lan_stp", "0"))
					br_set_stp_state("br1", 0);	// eval ("brctl",
				// "stp", "br1",
				// "off");
				else
					br_set_stp_state("br1", 1);	// eval ("brctl",
				// "stp", "br1",
				// "off");

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

			sprintf(wdsvarname, "ath%d_wds%d_enable", c, s);
			sprintf(wdsdevname, "ath%d_wds%d_if", c, s);
			sprintf(br1enable, "ath%d_br1_enable", c);
			if (nvram_get(wdsvarname) == NULL)
				nvram_set(wdsvarname, "0");
#else
			char br1enable[32];

			sprintf(wdsvarname, "wl%d_wds%d_enable", c, s);
			sprintf(wdsdevname, "wl%d_wds%d_if", c, s);
			sprintf(br1enable, "wl%d_br1_enable", c);
			if (nvram_get(wdsvarname) == NULL)
				nvram_set(wdsvarname, "0");
#endif
			dev = nvram_safe_get(wdsdevname);
			if (strlen(dev) == 0)
				continue;
#ifdef HAVE_RT2880
			dev = getWDSDev(dev);
#endif
			ifconfig(dev, 0, 0, 0);

			// eval ("ifconfig", dev, "down");
			if (nvram_match(wdsvarname, "1")) {
				char *wdsip;
				char *wdsnm;
				char wdsbc[32] = { 0 };
#ifdef HAVE_MADWIFI
				wdsip = nvram_nget("ath%d_wds%d_ipaddr", c, s);
				wdsnm = nvram_nget("ath%d_wds%d_netmask", c, s);
#else
				wdsip = nvram_nget("wl%d_wds%d_ipaddr", c, s);
				wdsnm = nvram_nget("wl%d_wds%d_netmask", c, s);
#endif

				snprintf(wdsbc, 31, "%s", wdsip);
				get_broadcast(wdsbc, wdsnm);
				eval("ifconfig", dev, wdsip, "broadcast", wdsbc, "netmask", wdsnm, "up");
			} else if (nvram_match(wdsvarname, "2")
				   && nvram_match(br1enable, "1")) {
				eval("ifconfig", dev, "up");
				sleep(1);
				br_add_interface("br1", dev);
			} else if (nvram_match(wdsvarname, "3")) {
				ifconfig(dev, IFUP, 0, 0);
				sleep(1);
				br_add_interface(getBridge(dev), dev);
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
				memset(&info, 0, sizeof(info));
				info.cmd = ETHTOOL_GDRVINFO;
				ifr.ifr_data = (caddr_t) & info;
				if (ioctl(s, SIOCETHTOOL, &ifr) >= 0) {
					/*
					 * currently only need to set QoS to et devices 
					 */
					if (!strncmp(info.driver, "et", 2)) {
						ifr.ifr_data = (caddr_t) & qos;
						ioctl(s, SIOCSETCQOS, &ifr);
					}
				}
				ifr.ifr_data = ifrdata;
			}
		}
		close(s);
	}
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
	}
#endif
	/*
	 * Disable wireless will cause diag led blink, so we want to stop it. 
	 */
	if (check_hw_type() == BCM4712_CHIP) {
		diag_led(DIAG, STOP_LED);
		/*
		 * Light or go out the DMZ led even if there is no wan ip. 
		 */
		if (nvram_invmatch("dmz_ipaddr", "")
		    && nvram_invmatch("dmz_ipaddr", "0"))
			diag_led(DMZ, START_LED);
		else
			diag_led(DMZ, STOP_LED);
	}

	if (nvram_match("lan_stp", "0"))
		br_set_stp_state("br0", 0);
	else
		br_set_stp_state("br0", 1);

	// eval ("rm", "/tmp/hosts");
	addHost("localhost", "127.0.0.1");
	if (strlen(nvram_safe_get("wan_hostname")) > 0)
		addHost(nvram_safe_get("wan_hostname"), nvram_safe_get("lan_ipaddr"));
	else if (strlen(nvram_safe_get("router_name")) > 0)
		addHost(nvram_safe_get("router_name"), nvram_safe_get("lan_ipaddr"));
#ifdef HAVE_MICRO
	br_shutdown();
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
	br_del_interface(lan_ifname, "wl0.1");
	ifconfig("wl0.1", 0, NULL, NULL);
	br_del_interface(lan_ifname, "wl0.2");
	ifconfig("wl0.2", 0, NULL, NULL);
	br_del_interface(lan_ifname, "wl0.3");
	ifconfig("wl0.3", 0, NULL, NULL);
	br_del_interface(lan_ifname, "wl1.1");
	ifconfig("wl1.1", 0, NULL, NULL);
	br_del_interface(lan_ifname, "wl1.2");
	ifconfig("wl1.2", 0, NULL, NULL);
	br_del_interface(lan_ifname, "wl1.3");
	ifconfig("wl1.3", 0, NULL, NULL);
#endif
	/*
	 * Bring down bridged interfaces 
	 */
	if (strncmp(lan_ifname, "br", 2) == 0) {
		foreach(name, nvram_safe_get("lan_ifnames"), next) {
			if (nvram_match("wan_ifname", name))
				continue;
			if (!ifexists(name))
				continue;
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
			eval("wlconf", name, "down");
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
		eval("wlconf", lan_ifname, "down");
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

	foreach(name, nvram_safe_get("wan_ifnames"), next)
	    if (ifname && !strcmp(ifname, name))
		return 1;

	if (getSTA() && !strcmp(getSTA(), ifname))
		return 1;

	return 0;
}

void start_force_to_dial(void);

static void vdsl_fuckup(char *ifname)
{
	int s;
	struct ifreq ifr;
	char mac[32];
	char eabuf[32];
	eval("ifconfig", ifname, "down");
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return;
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ioctl(s, SIOCGIFHWADDR, &ifr);
	if (nvram_get("wan_hwaddr"))
		strcpy(mac, nvram_safe_get("wan_hwaddr"));
	else
		strcpy(mac, ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
	MAC_SUB(mac);
	ether_atoe(mac, ifr.ifr_hwaddr.sa_data);
	ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ);
	ioctl(s, SIOCSIFHWADDR, &ifr);
	close(s);
	eval("ifconfig", ifname, "up");
}

void start_wan(int status)
{
	FILE *fp;
	char *wan_ifname = get_wan_face();
	char *wan_proto = nvram_safe_get("wan_proto");
	int s;
	struct ifreq ifr;
	if (isClient()) {
		char *ifn = getSTA();
		int count = 10;
		while ((count--) > 0)	// wait until wan is available (10 sek max)
		{
			if (ifexists(ifn)) {
				break;
			}
			sleep(1);
		}
	}
	rmmod("n_hdlc");

	eval("ifconfig", nvram_safe_get("wan_ifname"), "allmulti", "promisc");

	// wan test mode
	if (nvram_match("wan_testmode", "1")) {
		status = 0;	// avoid redialing
		fprintf(stderr, "[SERVICE WAN] testmode\n");
	}
#ifdef HAVE_PPPOE
#ifdef HAVE_RB500
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : nvram_safe_get("wan_ifname");
#else

#ifdef HAVE_XSCALE
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "ixp1";
#elif HAVE_MAGICBOX
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_NORTHSTAR
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "vlan2";
#elif HAVE_LAGUNA
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_VENTANA
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_UNIWIP
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_WDR4900
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "vlan2";
#elif HAVE_RB600
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_WRT54G2
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "vlan1";
#elif HAVE_RTG32
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "vlan1";
#elif HAVE_DIR300
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "vlan2";
#elif HAVE_BWRG1000
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "vlan2";
#elif HAVE_ECB9750
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth2";
#elif HAVE_TECHNAXX3G
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth2";
#elif HAVE_EAP9550
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth2";
#elif HAVE_RT2880
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "vlan2";
#elif HAVE_MR3202A
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "vlan2";
#elif HAVE_FONERA
	char *pppoe_wan_ifname = NULL;

	if (getRouterBrand() == ROUTER_BOARD_FONERA2200)
		pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname", "") ? nvram_safe_get("pppoe_wan_ifname") : "vlan1";
	else
		pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname", "") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_NS2
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_BS2
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_PICO2
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_PICO2HP
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_MS2
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_BS2HP
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_LC2
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_LS2
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : nvram_safe_get("wan_ifname");
#elif HAVE_SOLO51
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : nvram_safe_get("wan_ifname");
#elif HAVE_WDR2543
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "vlan2";
#elif HAVE_WA901
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_WR941
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "vlan1";
#elif HAVE_WA901v1
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth1";
#elif HAVE_CARAMBOLA
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "vlan2";
#elif HAVE_WR703
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth1";
#elif HAVE_WA7510
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth1";
#elif HAVE_WR741
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_WR1043
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "vlan2";
#elif HAVE_WZRG450
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "vlan2";
#elif HAVE_DIR632
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth1";
#elif HAVE_AP83
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth1";
#elif HAVE_AP94
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth1";
#elif HAVE_WNR2000
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_WASP
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "vlan2";
#elif HAVE_WHRHPGN
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_JA76PF
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth1";
#elif HAVE_ALFAAP94
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth1";
#elif HAVE_JWAP003
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth1";
#elif HAVE_WP546
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth1";
#elif HAVE_LSX
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_DANUBE
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "nas0";
#elif HAVE_STORM
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_OPENRISC
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_ADM5120
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_LS5
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_WHRAG108
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth1";
#elif HAVE_PB42
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_TW6600
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_RDAT81
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_RCAA01
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "vlan1";
#elif HAVE_CA8PRO
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "vlan1";
#elif HAVE_CA8
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "eth0";
#elif HAVE_X86
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : nvram_safe_get("wan_ifname");
#else
	char *pppoe_wan_ifname = nvram_invmatch("pppoe_wan_ifname",
						"") ? nvram_safe_get("pppoe_wan_ifname") : "vlan1";
#endif
#ifdef HAVE_MULTICAST
	if (!nvram_match("dtag_vlan8", "1") || nvram_match("wan_vdsl", "0"))
		stop_igmp_proxy();
#endif
#ifdef HAVE_UDPXY
	if (!nvram_match("udpxy_enable", "1"))
		stop_udpxy();
#endif

#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
	if (getWET()) {
		dns_to_resolv();
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
#ifdef HAVE_PPPOE
	if (nvram_match("wan_proto", "pppoe"))
		strncpy(ifr.ifr_name, pppoe_wan_ifname, IFNAMSIZ);
	else
#endif
#ifdef HAVE_L2TP
	if (nvram_match("wan_proto", "l2tp"))
		strncpy(ifr.ifr_name, pppoe_wan_ifname, IFNAMSIZ);
	else
#endif
#ifdef HAVE_PPTP
	if (nvram_match("wan_proto", "pptp"))
		strncpy(ifr.ifr_name, pppoe_wan_ifname, IFNAMSIZ);
	else
#endif
		strncpy(ifr.ifr_name, wan_ifname, IFNAMSIZ);

	/*
	 * Set WAN hardware address before bringing interface up 
	 */
	memset(ifr.ifr_hwaddr.sa_data, 0, ETHER_ADDR_LEN);

	ifconfig(wan_ifname, 0, NULL, NULL);
#if defined(HAVE_FONERA) || defined(HAVE_CA8) && !defined(HAVE_MR3202A)
	if (getRouterBrand() != ROUTER_BOARD_FONERA2200 && getRouterBrand() != ROUTER_BOARD_CA8PRO) {
		char staticlan[32];

		sprintf(staticlan, "%s:0", wan_ifname);
		if (!nvram_match("ath0_mode", "sta")
		    && !nvram_match("ath0_mode", "wdssta")
		    && !nvram_match("ath0_mode", "wet")
		    && !CANBRIDGE()) {
			eval("ifconfig", "br0:0", "down");
			eval("ifconfig", staticlan, "169.254.255.1", "netmask", "255.255.0.0");
		} else
			eval("ifconfig", staticlan, "0.0.0.0", "down");
	}
#endif

	// fprintf(stderr,"%s %s\n", wan_ifname, wan_proto);
	char *wlifname = getSTA();

	if (!wlifname) {
		wlifname = getWET();
	}

	unsigned char mac[20];

	if (nvram_match("mac_clone_enable", "1") && nvram_invmatch("def_hwaddr", "00:00:00:00:00:00") && nvram_invmatch("def_hwaddr", "")) {
		ether_atoe(nvram_safe_get("def_hwaddr"), ifr.ifr_hwaddr.sa_data);
	} else {

		if (wlifname && (!strcmp(wan_ifname, wlifname) || nvram_match("wan_proto", "l2tp") || nvram_match("wan_proto", "pppoe") || nvram_match("wan_proto", "pptp")))	// sta mode
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

	if (memcmp(ifr.ifr_hwaddr.sa_data, "\0\0\0\0\0\0", ETHER_ADDR_LEN)) {
		ifr.ifr_hwaddr.sa_family = ARPHRD_ETHER;
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)

		if (wlifname && !strcmp(wan_ifname, wlifname))
			eval("wl", "-i", wan_ifname, "down");
		else if (strcmp(wan_ifname, "br0"))
			eval("ifconfig", wan_ifname, "down");

		ioctl(s, SIOCSIFHWADDR, &ifr);
#else
		if (!wlifname && strcmp(wan_ifname, "br0")) {
			eval("ifconfig", wan_ifname, "down");
			ioctl(s, SIOCSIFHWADDR, &ifr);
		}
#endif
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
		if (wlifname && !strcmp(wan_ifname, wlifname)) {
			eval("wl", "-i", wan_ifname, "up");
			start_config_macs(wan_ifname);
		}
#endif
		cprintf("Write WAN mac successfully\n");
	} else
		perror("Write WAN mac fail : \n");
	// fprintf(stderr,"%s %s\n", wan_ifname, wan_proto);

	/*
	 * Set MTU 
	 */
	init_mtu(wan_proto);	// add by honor 2002/12/27
	// fprintf(stderr,"%s %s\n", wan_ifname, wan_proto);

	// Set our Interface to the right MTU
#ifdef HAVE_PPPOE
	if (nvram_match("wan_proto", "pppoe")) {
		ifr.ifr_mtu = atoi(getMTU(wan_ifname));	// default ethernet frame size
	} else
#endif
#ifdef HAVE_PPTP
	if (nvram_match("wan_proto", "pptp")) {
		ifr.ifr_mtu = atoi(getMTU(wan_ifname));	// default ethernet frame size
	} else
#endif
#ifdef HAVE_L2TP
	if (nvram_match("wan_proto", "l2tp")) {
		ifr.ifr_mtu = atoi(getMTU(wan_ifname));	// default ethernet frame size
	} else
#endif
	{
		int mtu = atoi(nvram_safe_get("wan_mtu"));
		if (mtu == 1500)
			mtu = atoi(getMTU(wan_ifname));
		ifr.ifr_mtu = mtu;
	}
	// fprintf(stderr,"set mtu for %s to %d\n",ifr.ifr_name,ifr.ifr_mtu);
	ioctl(s, SIOCSIFMTU, &ifr);
	eval("ifconfig", wan_ifname, "txqueuelen", getTXQ(wan_ifname));

	if (strcmp(wan_proto, "disabled") == 0) {
		start_wan_done(wan_ifname);
		return;
	}

	/*
	 * Bring up WAN interface 
	 */
#ifdef HAVE_PPPOE
	/*
	 * AhMan March 19 2005 
	 */
	/*
	 * ice-man March 19 2005 
	 */
	/*
	 * pppoe_wan interface must be up in order to use any pppoe client 
	 */
	if (strcmp(wan_proto, "pppoe") == 0)
		ifconfig(pppoe_wan_ifname, IFUP, NULL, NULL);
	else
#endif
#ifdef HAVE_L2TP
	if (strcmp(wan_proto, "l2tp") == 0)
		ifconfig(pppoe_wan_ifname, IFUP, NULL, NULL);
	else
#endif
	{
		ifconfig(wan_ifname, IFUP, NULL, NULL);
	}
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
		if (!nvram_match("usb_enable", "1")) {
			nvram_set("usb_enable", "1");	//  simply enable it, otherwise 3g might not work
			nvram_commit();
			start_drivers();
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
		char *controldevice = get3GControlDevice();
		int timeout = 5;
#ifdef HAVE_UQMI
		if (controldevice && !strcmp(controldevice, "qmi")) {
			/* disconnect network */
			int clientid = 0;
			FILE *fp = fopen("/tmp/qmi-clientid", "rb");
			if (fp) {
				fscanf(fp, "%d", &clientid);
				fclose(fp);
				sysprintf("uqmi -d /dev/cdc-wdm0 --set-client-id wds,%d --release-client-id wds", clientid);
			}
			clientid = 0;
			if (nvram_match("wan_conmode", "6"))
				sysprintf("uqmi -d /dev/cdc-wdm0 --set-network-modes lte");
//              if (nvram_match("wan_conmode","5")) //unsupported and useless. i dont know what that means
//                  sysprintf("qmicli -d /dev/cdc-wdm0 --nas-set-network-mode=LTE");
			if (nvram_match("wan_conmode", "4"))
				sysprintf("uqmi -d /dev/cdc-wdm0 --set-network-modes gsm,umts");
			if (nvram_match("wan_conmode", "3"))
				sysprintf("uqmi -d /dev/cdc-wdm0 --set-network-modes umts,gsm");
			if (nvram_match("wan_conmode", "2"))
				sysprintf("uqmi -d /dev/cdc-wdm0 --set-network-modes gsm");
			if (nvram_match("wan_conmode", "1"))
				sysprintf("uqmi -d /dev/cdc-wdm0 --set-network-modes umts");
			if (nvram_match("wan_conmode", "0"))
				sysprintf("uqmi -d /dev/cdc-wdm0 --set-network-modes all");

			//set pin
			sysprintf("uqmi -d /dev/cdc-wdm0 --verify-pin1 %s", nvram_safe_get("wan_pin"));
			//set apn and dial
			fp = popen("/usr/sbin/uqmi -d /dev/cdc-wdm0 --get-client-id wds", "rb");
			fscanf(fp, "%d", &clientid);
			pclose(fp);
			fp = fopen("/tmp/qmi-clientid", "wb");
			fprintf(fp, "%d", clientid);
			fclose(fp);

			fp = fopen("/tmp/qmi-connect.sh", "wb");
			fprintf(fp, "#!/bin/sh\n");
			fprintf(fp, "REG=0\n");
			fprintf(fp, "COUNT=3\n");
			fprintf(fp, "while [ $REG = 0 ]\n");
			fprintf(fp, "do\n");
			fprintf(fp, "if [ $COUNT = 0 ]\n");
			fprintf(fp, "then\n");
			fprintf(fp, " exit\n");
			fprintf(fp, "fi\n");
			fprintf(fp, "CLIENTID=`cat /tmp/qmi-clientid`\n");
			fprintf(fp, "REG=`uqmi -d /dev/cdc-wdm0 --set-client-id wds,${CLIENTID} --keep-client-id wds --get-serving-system|grep registered|wc -l`\n");
			fprintf(fp, "if [ $REG = 0 ]\n");
			fprintf(fp, "then\n");
			fprintf(fp, "echo \"not yet registered to the network (${COUNT})\" | logger -t wan_dial\n");
			fprintf(fp, "fi\n");
			fprintf(fp, "COUNT=$(($COUNT - 1))\n");
			fprintf(fp, "sleep 5\n");
			fprintf(fp, "done\n");
			if (strlen(nvram_safe_get("ppp_username")) > 0 && strlen(nvram_safe_get("ppp_passwd")) > 0) {
				fprintf(fp, "uqmi -d /dev/cdc-wdm0 --set-client-id wds,${CLIENTID} --start-network %s --auth-type both --username %s --password %s --keep-client-id wds\n",
					nvram_safe_get("wan_apn"), nvram_safe_get("ppp_username"), nvram_safe_get("ppp_passwd"));
			} else {
				fprintf(fp, "uqmi -d /dev/cdc-wdm0 --set-client-id wds,${CLIENTID} --start-network %s --auth-type both --keep-client-id wds\n", nvram_safe_get("wan_apn"));
			}
			fprintf(fp, "ifconfig wwan0 up\n");
			fprintf(fp, "ln -s /sbin/rc /tmp/udhcpc\n");
			fprintf(fp, "udhcpc -i wwan0 -p /var/run/udhcpc.pid -s /tmp/udhcpc\n");
			fclose(fp);
			chmod("/tmp/qmi-connect.sh", 0700);
			sysprintf("/tmp/qmi-connect.sh &");
			if (status != REDIAL) {
				start_redial();
			}

		} else
#elif HAVE_LIBQMI
		if (controldevice && !strcmp(controldevice, "qmi")) {
			if (nvram_match("wan_conmode", "6"))
				sysprintf("qmicli -d /dev/cdc-wdm0 --nas-set-network-mode=LTE");
//              if (nvram_match("wan_conmode","5")) //unsupported and useless. i dont know what that means
//                  sysprintf("qmicli -d /dev/cdc-wdm0 --nas-set-network-mode=LTE");
			if (nvram_match("wan_conmode", "4"))
				sysprintf("qmicli -d /dev/cdc-wdm0 --nas-set-network-mode=GSMUMTS");
			if (nvram_match("wan_conmode", "3"))
				sysprintf("qmicli -d /dev/cdc-wdm0 --nas-set-network-mode=UMTSGSM");
			if (nvram_match("wan_conmode", "2"))
				sysprintf("qmicli -d /dev/cdc-wdm0 --nas-set-network-mode=GSM");
			if (nvram_match("wan_conmode", "1"))
				sysprintf("qmicli -d /dev/cdc-wdm0 --nas-set-network-mode=UMTS");
			if (nvram_match("wan_conmode", "0"))
				sysprintf("qmicli -d /dev/cdc-wdm0 --nas-set-network-mode=ANY");

			//set pin
			sysprintf("qmicli -d /dev/cdc-wdm0 --dms-uim-verify-pin=PIN,%s", nvram_safe_get("wan_pin"));
			//set apn and dial
			FILE *fp = fopen("/tmp/qmi-network.conf", "wb");
			fprintf(fp, "APN=%s", nvram_safe_get("wan_apn"));
			if (strlen(nvram_safe_get("ppp_username")) > 0 && strlen(nvram_safe_get("ppp_passwd")) > 0) {
				fprintf(fp, ",BOTH,%s,%s\n", nvram_safe_get("ppp_username"), nvram_safe_get("ppp_passwd"));
			} else {
				fprintf(fp, "\n");
			}
			fclose(fp);

			eval("qmi-network", "/dev/cdc-wdm0", "stop");	//release it before
			eval("qmi-network", "/dev/cdc-wdm0", "start");
			eval("ifconfig", "wwan0", "up");
			start_dhcpc("wwan0", NULL, NULL, 1);
			if (status != REDIAL) {
				start_redial();
			}

		} else
#endif
		if (controldevice && !strcmp(controldevice, "hso")) {

		} else {

			/* init PIN */
			if (strlen(nvram_safe_get("wan_pin")))
				sysprintf("export COMGTPIN=%s;comgt PIN -d %s", nvram_safe_get("wan_pin"), controldevice);
			// set netmode, even if it is auto, should be set every time, the stick might save it
			// some sticks, don't save it ;-)
			if (strlen(nvram_safe_get("3gnmvariant"))) {
				int netmode;
				int netmodetoggle;
				netmode = atoi(nvram_default_get("wan_conmode", "0"));
				if (netmode == 5) {
					if (strlen(nvram_safe_get("3gnetmodetoggle"))) {
						netmodetoggle = atoi(nvram_safe_get("3gnetmodetoggle"));
						if (netmodetoggle == 1) {
							// 2g
							netmode = 2;
							nvram_set("3gnetmodetoggle", "0");
						} else {
							// auto
							netmode = 0;
							nvram_set("3gnetmodetoggle", "1");
						}
					} else {
						// auto
						netmode = 0;
						nvram_set("3gnetmodetoggle", "1");
					}
				}
				sysprintf("export COMGNMVARIANT=%s;export COMGTNM=%d;comgt -d %s -s /etc/comgt/netmode.comgt >/tmp/comgt-netmode.out", nvram_safe_get("3gnmvariant"), netmode, controldevice);
				printf("3g setting netmode with variant %s to mode %d\n", nvram_safe_get("3gnmvariant"), netmode);
			}
			// Wait for device to attach to the provider network
			int retcgatt = 0;
			retcgatt = sysprintf("comgt CGATT -d %s >/tmp/comgt-cgatt.out 2>&1", controldevice);
			// if (retcgatt == 0) 
			// {
			// nvram_set("3g_fastdial", "1");
			// return (5);
			// }
			if (strlen(nvram_safe_get("wan_apn")))
				if (!nvram_match("wan_dial", "2"))
					sysprintf("export COMGTAPN=\"%s\";comgt APN -d %s", nvram_safe_get("wan_apn"), controldevice);
			// Lets open option file and enter all the parameters.
			char *username = nvram_safe_get("ppp_username");
			char *passwd = nvram_safe_get("ppp_passwd");
			if (strlen(username) && strlen(passwd)) {
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
			if (strlen(username) && strlen(passwd)) {
				fprintf(fp, "chap-secrets /tmp/ppp/chap-secrets\n");
				fprintf(fp, "pap-secrets /tmp/ppp/pap-secrets\n");
			}

			fprintf(fp, "defaultroute\n");
			fprintf(fp, "usepeerdns\n");
			fprintf(fp, "noipdefault\n");
			fprintf(fp, "noauth\n");
			fprintf(fp, "ipcp-max-failure 30\n");
			if (nvram_match("mtu_enable", "1")) {
				if (atoi(nvram_safe_get("wan_mtu")) > 0) {
					fprintf(fp, "mtu %s\n", nvram_safe_get("wan_mtu"));
					fprintf(fp, "mru %s\n", nvram_safe_get("wan_mtu"));
				}

			}
			fprintf(fp, "crtscts\n");
			fprintf(fp, "460800\n");
//      fprintf(fp,"connect \"/usr/sbin/comgt DIAL -d %s\"\n",controldevice);
			char *dial = "*99***1#";
			if (nvram_match("wan_dial", "0"))
				dial = "ATD*99***1#";
			if (nvram_match("wan_dial", "1"))
				dial = "ATD*99#";
			if (nvram_match("wan_dial", "2"))
				dial = "ATDT#777";
			if (nvram_match("wan_dial", "3"))
				dial = "ATD*99***3#";
			if (nvram_match("wan_dial", "4"))
				dial = "ATD*99***2#";
			if (nvram_match("wan_dial", "5"))
				dial = "ATD*99***4#";
			fprintf(fp, "connect \"COMGTDIAL='%s' /usr/sbin/comgt DIAL -d %s >/tmp/comgt.out 2>&1\"\n", dial, nvram_safe_get("3gdata"));
			if (strlen(username))
				fprintf(fp, "user '%s'\n", username);
			if (strlen(passwd))
				fprintf(fp, "password '%s'\n", passwd);
			fprintf(fp, "%s\n", nvram_safe_get("3gdata"));

			fclose(fp);

			eval("pppd", "file", "/tmp/ppp/options.pppoe");

			/*
			 * Pretend that the WAN interface is up 
			 */
			if (nvram_match("ppp_demand", "1")) {
				/*
				 * Wait for ppp0 to be created 
				 */
				while (ifconfig("ppp0", IFUP, NULL, NULL)
				       && timeout--)
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
				while (ioctl(s, SIOCGIFDSTADDR, &ifr)
				       && timeout--) {
					perror("ppp0");
					printf("Wait ppp inteface to init (2) ...\n");
					sleep(1);
				}
				char *peer = inet_ntop(AF_INET,
						       &sin_addr(&ifr.ifr_dstaddr),
						       client,
						       16);

				nvram_set("wan_gateway", peer);

				start_wan_done("ppp0");

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

	} else
#endif
#ifdef HAVE_PPPOE
	if ((strcmp(wan_proto, "pppoe") == 0)) {
		char username[80], passwd[80];
		char idletime[20], retry_num[20];

		snprintf(idletime, sizeof(idletime), "%d", atoi(nvram_safe_get("ppp_idletime")) * 60);
		snprintf(retry_num, sizeof(retry_num), "%d", (atoi(nvram_safe_get("ppp_redialperiod")) / 5) - 1);

		snprintf(username, sizeof(username), "%s", nvram_safe_get("ppp_username"));
		snprintf(passwd, sizeof(passwd), "%s", nvram_safe_get("ppp_passwd"));

		mkdir("/tmp/ppp", 0777);
		int timeout = 5;

		// Lets open option file and enter all the parameters.
		fp = fopen("/tmp/ppp/options.pppoe", "w");
		// rp-pppoe kernelmode plugin
#if defined(HAVE_ADM5120) && !defined(HAVE_WP54G) && !defined(HAVE_NP28G)
		fprintf(fp, "plugin /lib/rp-pppoe.so\n");
#else
		fprintf(fp, "plugin /usr/lib/rp-pppoe.so\n");
#endif
		if (nvram_invmatch("pppoe_service", ""))
			fprintf(fp, " rp_pppoe_service %s", nvram_safe_get("pppoe_service"));
		fprintf(fp, "\n");
		char vlannic[32];
		char tvnic[32];

		if (!strncmp(pppoe_wan_ifname, "vlan", 4)) {
			if (nvram_match("wan_vdsl", "1")) {
				char *ifn = enable_dtag_vlan(1);

				if (nvram_match("dtag_vlan8", "1")) {
					sprintf(vlannic, "%s.0008", ifn);
					if (!ifexists(vlannic)) {
						eval("vconfig", "set_name_type", "DEV_PLUS_VID");
						eval("vconfig", "add", ifn, "8");
						eval("ifconfig", vlannic, "up");
					}
					nvram_set("tvnicfrom", vlannic);
					symlink("/sbin/rc", "/tmp/udhcpc_tv");
					start_dhcpc(vlannic, "/var/run/udhcpc_tv.pid", "/tmp/udhcpc_tv", 1);
				}
				sprintf(vlannic, "%s.0007", ifn);
				if (!ifexists(vlannic)) {
					eval("vconfig", "set_name_type", "DEV_PLUS_VID");
					eval("vconfig", "add", ifn, "7");
					eval("ifconfig", vlannic, "up");
				}

				fprintf(fp, "nic-%s\n", vlannic);
				vdsl_fuckup(vlannic);	/* work around for DTAG DSLAMS */
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
			if (nvram_match("wan_vdsl", "1"))	// Deutsche Telekom
				// VDSL2 Vlan 7 Tag
			{
				if (nvram_match("dtag_vlan8", "1")) {
					sprintf(vlannic, "%s.0008", pppoe_wan_ifname);
					if (!ifexists(vlannic)) {
						eval("vconfig", "set_name_type", "DEV_PLUS_VID");
						eval("vconfig", "add", pppoe_wan_ifname, "8");
						eval("ifconfig", vlannic, "up");
					}
					nvram_set("tvnicfrom", vlannic);
					symlink("/sbin/rc", "/tmp/udhcpc_tv");
					start_dhcpc(vlannic, "/var/run/udhcpc_tv.pid", "/tmp/udhcpc_tv", 1);
				}
				sprintf(vlannic, "%s.0007", pppoe_wan_ifname);
				if (!ifexists(vlannic)) {
					eval("vconfig", "set_name_type", "DEV_PLUS_VID");
					eval("vconfig", "add", pppoe_wan_ifname, "7");
					eval("ifconfig", vlannic, "up");
				}
				fprintf(fp, "nic-%s\n", vlannic);
				vdsl_fuckup(vlannic);	/* work around for DTAG DSLAMS */
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
		if (nvram_match("ppp_compression", "1")) {
			fprintf(fp, "mppc\n");
		} else {
			fprintf(fp, "noccp\n");
			fprintf(fp, "nomppc\n");
		}
		fprintf(fp, "noipdefault\n" "noauth\n" "defaultroute\n" "noaccomp\n" "nobsdcomp\n" "nodeflate\n"
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
		if (nvram_match("ppp_mlppp", "1"))
			fprintf(fp, "mp\n");
		fprintf(fp, "usepeerdns\nuser '%s'\n" "password '%s'\n", username, passwd);

		// This is a tricky one. When used it could improve speed of PPPoE
		// but not all ISP's can support it.
		// default-asyncmap escapes all control characters. By using asyncmap 
		// 0 PPPD will not escape any control characters
		// Not all ISP's can handle this. By default use default-asyncmap
		// and if ppp_asyncmap=1 do not escape
		if (nvram_match("ppp_asyncmap", "1"))
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
		if (nvram_match("mtu_enable", "1")) {
			if (atoi(nvram_safe_get("wan_mtu")) > 0) {
				fprintf(fp, "mtu %s\n", nvram_safe_get("wan_mtu"));
				fprintf(fp, "mru %s\n", nvram_safe_get("wan_mtu"));
			}

		} else {
			// If MRU set to Auto we still allow custom MTU/MRU settings for
			// expirienced users
			if (nvram_invmatch("pppoe_ppp_mtu", ""))
				if (atoi(nvram_safe_get("pppoe_ppp_mtu")) > 0)
					fprintf(fp, "mtu %s\n", nvram_safe_get("pppoe_ppp_mtu"));
			if (nvram_invmatch("pppoe_ppp_mru", ""))
				if (atoi(nvram_safe_get("pppoe_ppp_mru")) > 0)
					fprintf(fp, "mru %s\n", nvram_safe_get("pppoe_ppp_mru"));
		}

		// Allow runtime debugging
		if (nvram_match("ppp_debug", "1"))
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
		if (nvram_match("ppp_demand", "1"))
			fprintf(fp, "demand\n"
				"idle %s\n"
				"10.112.112.112:10.112.112.113\n" "lcp-echo-interval %d\n" "lcp-echo-failure 10\n" "ipcp-accept-remote\n" "ipcp-accept-local\n" "connect true\n" "ktune\n", idletime, atoi(idletime) * 2);
		else
			fprintf(fp, "persist\n" "lcp-echo-interval 3\n" "lcp-echo-failure 20\n");
#ifdef HAVE_IPV6
		if (nvram_match("ipv6_enable", "1"))
			fprintf(fp, "ipv6 ,\n");
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
		eval("pppd", "file", "/tmp/ppp/options.pppoe");

		// This is horrible.
		// What if pppoe recconects with ppp1?

		/*
		 * Pretend that the WAN interface is up 
		 */
		if (nvram_match("ppp_demand", "1")) {
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
			char *peer = inet_ntop(AF_INET, &sin_addr(&ifr.ifr_dstaddr),
					       client,
					       16);

			nvram_set("wan_gateway", peer);

			start_wan_done("ppp0");

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
#ifdef HAVE_MODEMBRIDGE
	if ((strcmp(wan_proto, "bridge") == 0)) {
		stop_atm();
		start_atm();
		br_add_interface("br0", "nas0");
		start_wan_done("nas0");
	} else
#endif
#ifdef HAVE_PPPOATM
	if ((strcmp(wan_proto, "pppoa") == 0)) {
		char username[80], passwd[80];
		char idletime[20], retry_num[20];
		stop_atm();
		start_atm();

		snprintf(idletime, sizeof(idletime), "%d", atoi(nvram_safe_get("ppp_idletime")) * 60);
		snprintf(retry_num, sizeof(retry_num), "%d", (atoi(nvram_safe_get("ppp_redialperiod")) / 5) - 1);

		snprintf(username, sizeof(username), "%s", nvram_safe_get("ppp_username"));
		snprintf(passwd, sizeof(passwd), "%s", nvram_safe_get("ppp_passwd"));

		mkdir("/tmp/ppp", 0777);
		int timeout = 5;

		// Lets open option file and enter all the parameters.
		fp = fopen("/tmp/ppp/options.pppoa", "w");

		fprintf(fp, "plugin /usr/lib/pppoatm.so %s.%s %s", nvram_safe_get("vpi"), nvram_safe_get("vci"), nvram_match("atm_encaps", "0") ? "llc-encaps" : "vc-encaps");
		fprintf(fp, "\n");

		// Those are default options we use + user/passwd
		// By using user/password options we dont have to deal with chap/pap
		// secrets files.
		if (nvram_match("ppp_compression", "1")) {
			fprintf(fp, "mppc\n");
		} else {
			fprintf(fp, "noccp\n");
			fprintf(fp, "nomppc\n");
		}
		fprintf(fp, "noipdefault\n" "noauth\n" "defaultroute\n" "noaccomp\n" "nobsdcomp\n" "nodeflate\n" "nopcomp\n");
		if (nvram_invmatch("ppp_mppe", ""))
			fprintf(fp, "%s\n", nvram_safe_get("ppp_mppe"));
		else
			fprintf(fp, "nomppe\n");
		if (nvram_match("ppp_mlppp", "1"))
			fprintf(fp, "mp\n");
		fprintf(fp, "usepeerdns\nuser '%s'\n" "password '%s'\n", username, passwd);

		if (nvram_match("ppp_asyncmap", "1"))
			fprintf(fp, "asyncmap 0\n");
		else
			fprintf(fp, "default-asyncmap\n");

		if (nvram_match("mtu_enable", "1")) {
			if (atoi(nvram_safe_get("wan_mtu")) > 0) {
				fprintf(fp, "mtu %s\n", nvram_safe_get("wan_mtu"));
				fprintf(fp, "mru %s\n", nvram_safe_get("wan_mtu"));
			}

		} else {
			// If MRU set to Auto we still allow custom MTU/MRU settings for
			// expirienced users
			if (nvram_invmatch("pppoe_ppp_mtu", ""))
				if (atoi(nvram_safe_get("pppoe_ppp_mtu")) > 0)
					fprintf(fp, "mtu %s\n", nvram_safe_get("pppoe_ppp_mtu"));
			if (nvram_invmatch("pppoe_ppp_mru", ""))
				if (atoi(nvram_safe_get("pppoe_ppp_mru")) > 0)
					fprintf(fp, "mru %s\n", nvram_safe_get("pppoe_ppp_mru"));
		}

		if (nvram_match("ppp_debug", "1"))
			fprintf(fp, "debug\n");

		if (nvram_match("ppp_demand", "1"))
			fprintf(fp, "demand\n"
				"idle %s\n"
				"10.112.112.112:10.112.112.113\n" "lcp-echo-interval %d\n" "lcp-echo-failure 10\n" "ipcp-accept-remote\n" "ipcp-accept-local\n" "connect true\n" "ktune\n", idletime, atoi(idletime) * 2);
		else
			fprintf(fp, "persist\n" "lcp-echo-interval 3\n" "lcp-echo-failure 20\n");

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
		eval("pppd", "file", "/tmp/ppp/options.pppoa");

		/*
		 * Pretend that the WAN interface is up 
		 */
		if (nvram_match("ppp_demand", "1")) {
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
			char *peer = inet_ntop(AF_INET, &sin_addr(&ifr.ifr_dstaddr),
					       client,
					       16);

			nvram_set("wan_gateway", peer);

			start_wan_done("ppp0");

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
	if (strcmp(wan_proto, "dhcp") == 0) {
		start_dhcpc(wan_ifname, NULL, NULL, 1);
	}
#ifdef HAVE_IPETH
	else if (strcmp(wan_proto, "iphone") == 0) {
		if (!nvram_match("usb_enable", "1")) {
			nvram_set("usb_enable", "1");	//  simply enable it, otherwise 3g might not work
			nvram_commit();
			start_drivers();
		}
		insmod("ipheth");
		stop_process("ipheth-loop", "IPhone Pairing daemon");
		stop_process("usbmuxd", "IPhone Mux daemon");
		stop_process("udhcpc", "DHCP Client");
		eval("usbmuxd");
		eval("ipheth-pair");
		eval("ifconfig", "iph0", "up");
		start_dhcpc("iph0", NULL, NULL, 1);
		if (status != REDIAL) {
			start_redial();
		}
	}
#endif
#ifdef HAVE_PPTP
	else if (strcmp(wan_proto, "pptp") == 0) {
		if (nvram_match("pptp_iptv", "1"))
			nvram_set("tvnicfrom", nvram_safe_get("wan_iface"));
		else
			nvram_unset("tvnicfrom");

		start_pptp(status);
	}
#endif
#ifdef HAVE_L2TP
	else if (strcmp(wan_proto, "l2tp") == 0) {
		if (nvram_match("l2tp_iptv", "1"))
			nvram_set("tvnicfrom", nvram_safe_get("wan_iface"));
		else
			nvram_unset("tvnicfrom");

		if (nvram_match("l2tp_use_dhcp", "1")) {
			nvram_set("wan_get_dns", "");
			char *l2tp_ifname = nvram_safe_get("wan_ifname");
			if (isClient()) {
				l2tp_ifname = getSTA();
			}

			start_dhcpc(l2tp_ifname, NULL, NULL, 1);
		} else {
			start_l2tp(status);
		}
	}
#endif
#ifdef HAVE_HEARTBEAT
	else if (strcmp(wan_proto, "heartbeat") == 0) {
		start_dhcpc(wan_ifname, NULL, NULL, 1);
	}
#endif
	else {
		ifconfig(wan_ifname, IFUP, nvram_safe_get("wan_ipaddr"), nvram_safe_get("wan_netmask"));
		start_wan_done(wan_ifname);
		sysprintf("/sbin/gratarp %s", wan_ifname);
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
		if (ioctl(s, SIOCGIFHWADDR, &ifr) == 0) {
			char eabuf[32];

			nvram_set("wan_hwaddr", ether_etoa(ifr.ifr_hwaddr.sa_data, eabuf));
			// fprintf(stderr,"write wan addr
			// %s\n",nvram_safe_get("wan_hwaddr"));
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
	if ((check_hw_type() == BCM4712_CHIP)
	    || (check_hw_type() == BCM5325E_CHIP)) {
		// Barry will put disable WLAN here
		if (nvram_match("wl_gmode", "-1")) {
			diag_led(WL, STOP_LED);
#if 0
			eval("wlled", "0 0 1");
			eval("wlled", "0 1 1");
#endif
		}
		diag_led(DIAG, STOP_LED);
	}
	/*
	 * Light or go out the DMZ led even if there is no wan ip. 
	 */
	if (nvram_match("dmz_enable", "1") && nvram_invmatch("dmz_ipaddr", "")
	    && nvram_invmatch("dmz_ipaddr", "0"))
		diag_led(DMZ, START_LED);
	else
		diag_led(DMZ, STOP_LED);

	cprintf("%s %s\n", nvram_safe_get("wan_ipaddr"), nvram_safe_get("wan_netmask"));

	if (nvram_match("wan_proto", "l2tp")) {
		/*
		 * Delete all default routes 
		 */
		while (route_del(get_wan_face(), 0, NULL, NULL, NULL) == 0) ;
	}
	cprintf("wep handling\n");
	cprintf("disable stp if needed\n");
	if (nvram_match("lan_stp", "0")) {
#ifdef HAVE_MICRO
		br_init();
#endif

		br_set_stp_state("br0", 0);

#ifdef HAVE_MICRO
		br_shutdown();
#endif

	} else {
#ifdef HAVE_MICRO
		br_init();
#endif

		br_set_stp_state("br0", 1);
#ifdef HAVE_MICRO
		br_shutdown();
#endif

	}

	cprintf("done()()()\n");
}

void start_wan_boot(void)
{
	start_wan(BOOT);
}

void start_wan_redial(void)
{
	start_wan(REDIAL);
}

void start_wan_service(void)
{
	stop_process_monitor();
	stop_ddns();
	cprintf("start process monitor\n");
	start_process_monitor();
	cprintf("start ddns\n");
	start_ddns();
}

void start_wan_done(char *wan_ifname)
{
	if (nvram_match("wan_testmode", "1")) {
		fprintf(stderr, "[WAN IF] testmode: skipping wan_done\n");
		return;
	}

	cprintf("%s %s\n", wan_ifname, nvram_safe_get("wan_proto"));

	if (nvram_match("wan_proto", "l2tp")) {
		/*
		 * Delete all default routes 
		 */
		while (route_del(nvram_safe_get("wan_ifname"), 0, NULL, NULL, NULL) == 0) ;
	}

	/*
	 * Delete all default routes 
	 */
	while (route_del(wan_ifname, 0, NULL, NULL, NULL) == 0) ;

	if ((nvram_match("wan_proto", "pppoe")) && check_wan_link(1)) {
		while (route_del(nvram_safe_get("wan_ifname_1"), 0, NULL, NULL, NULL) == 0) ;
	}
#ifdef HAVE_PPPOATM
	if ((nvram_match("wan_proto", "pppoa")) && check_wan_link(1)) {
		while (route_del(nvram_safe_get("wan_ifname_1"), 0, NULL, NULL, NULL) == 0) ;
	}
#endif

	if (nvram_invmatch("wan_proto", "disabled")) {
		int timeout = 5;

		/*
		 * Set default route to gateway if specified 
		 */
		char *gateway = nvram_match("wan_proto",
					    "pptp") ? nvram_safe_get("pptp_get_ip") : nvram_safe_get("wan_gateway");
		if (strcmp(gateway, "0.0.0.0")) {
//                      route_add(wan_ifname, 0, gateway, NULL,"255.255.255.255");

			while (route_add(wan_ifname, 0, "0.0.0.0", gateway, "0.0.0.0")
			       && timeout--) {
				if ((nvram_match("wan_proto", "pppoe")
				     || nvram_match("wan_proto", "pppoa"))
				    && nvram_match("ppp_demand", "1")) {
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
		if (nvram_match("l2tp_use_dhcp", "1"))
			route_add(nvram_safe_get("wan_ifname"), 0, nvram_safe_get("l2tp_server_ip"), nvram_safe_get("wan_gateway_buf"), "255.255.255.255");	// fixed 
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
#ifdef HAVE_UDHCPD
	stop_udhcpd();
	start_udhcpd();
#endif
	cprintf("restart dns proxy\n");
	/*
	 * Restart DNS proxy stop_dnsmasq (); start_dnsmasq (); 
	 */
	cprintf("start firewall\n");
	/*
	 * Start firewall 
	 */
	start_firewall();

	/*
	 * Set additional wan static routes if need 
	 */

	start_set_routes();
	cprintf("routes done\n");
	if (nvram_match("wan_proto", "pppoe")
	    || nvram_match("wan_proto", "pptp")
	    || nvram_match("wan_proto", "pppoa")
	    || nvram_match("wan_proto", "l2tp")
	    || nvram_match("wan_proto", "3g")) {
		if (nvram_match("ppp_demand", "1")) {	// ntp and ddns will trigger DOD, so we must
			// stop them when wan is unavaile.
			if (f_exists("/tmp/ppp/link")) {
				start_wan_service();
			}
		} else {
			start_wan_service();
		}
	} else {
		start_wan_service();
	}

#ifdef HAVE_UPNP
	stop_upnp();
#endif
#ifdef HAVE_BIRD
	stop_zebra();
#endif
	// stop_cron ();
	stop_wshaper();
	cprintf("start zebra\n");
#ifdef HAVE_BIRD
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
	start_wshaper();
	start_wland();
	if (nvram_match("wan_proto", "pptp")) {

		if (nvram_invmatch("pptp_customipup", "")) {

			// We not going to assume that /tmp/ppp is created..
			mkdir("/tmp/ppp", 0700);

			// Create our custom pptp ipup script and change its attributes
			writenvram("pptp_customipup", "/tmp/ppp/sh_pptp_customipup");
			chmod("/tmp/ppp/sh_pptp_customipup", 0744);

			// Execute our custom ipup script
			system2("/tmp/ppp/sh_pptp_customipup");

		}
	}
	cprintf("std on\n");
#ifdef HAVE_MICRO
	br_init();
#endif

	if (nvram_match("lan_stp", "0"))
		br_set_stp_state(nvram_safe_get("lan_ifname"), 0);
	else
		br_set_stp_state(nvram_safe_get("lan_ifname"), 1);
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
			eval("ledtool", "5");	// blink 5 times the 3 time interval 
	}
	/*
	 * end 
	 */

	cprintf("running custom DD-WRT ipup scripts\n");
	runStartup("/etc/config", ".ipup");
#ifdef HAVE_REGISTER
	if (isregistered_real())
#endif
	{
#ifdef HAVE_RB500
		runStartup("/usr/local/etc/config", ".ipup");
#else
		runStartup("/jffs/etc/config", ".ipup");
		runStartup("/mmc/etc/config", ".ipup");
#endif
	}
	cprintf("trigger gpio");

	led_control(LED_CONNECTED, LED_OFF);
	if (!nvram_match("wan_proto", "disabled")) {
		led_control(LED_CONNECTED, LED_ON);
		dd_syslog(LOG_INFO, "WAN is up. IP: %s\n", get_wan_ipaddr());
	}

	float sys_uptime;
	FILE *up;

	up = fopen("/proc/uptime", "r");
	fscanf(up, "%f", &sys_uptime);
	fclose(up);

	up = fopen("/tmp/.wanuptime", "w");
	fprintf(up, "%f", sys_uptime);
	fclose(up);

	cprintf("done\n");

	nvram_set("wan_iface", nvram_safe_get("wan_ifname"));

#ifdef HAVE_OPENVPN
	cprintf("starting openvpn\n");
	stop_openvpn_wandone();
	start_openvpn();
	cprintf("done\n");

#endif
#ifdef HAVE_OPENVPN
	stop_openvpnserverwan();
	start_openvpnserverwan();
#endif
#ifdef HAVE_DHCPFORWARD
	stop_dhcpfwd();
	start_dhcpfwd();
#endif
	nvram_set("wanup", "1");
#ifdef HAVE_MILKFISH
	if (nvram_match("milkfish_enabled", "1")) {
		cprintf("starting milkfish netup script\n");
		eval("/etc/config/milkfish.netup");
	}
#endif
#ifdef HAVE_SPUTNIK_APD
	stop_sputnik();
	start_sputnik();
#endif

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
	if (!nvram_match("dtag_vlan8", "1") || nvram_match("wan_vdsl", "0"))
		stop_igmp_proxy();
	start_igmp_proxy();
#endif
	cprintf("ready\n");
#ifdef HAVE_UDPXY
	if (!nvram_match("udpxy_enable", "1"))
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
}

void stop_wan(void)
{
	char *wan_ifname = get_wan_face();

	nvram_set("wanup", "0");

	led_control(LED_CONNECTED, LED_OFF);
	unlink("/tmp/.wanuptime");

	cprintf("%s %s\n", wan_ifname, nvram_safe_get("wan_proto"));
#ifdef HAVE_OPENVPN
	stop_openvpnserverwan();
	stop_openvpn_wandone();
#endif
#ifdef HAVE_DHCPFORWARD
	stop_dhcpfwd();
#endif
	/*
	 * Stop firewall 
	 */
	stop_firewall();
	/*
	 * Kill any WAN client daemons or callbacks 
	 */
#ifdef HAVE_PPPOE
	stop_pppoe();
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
	eval("ifconfig", wan_ifname, "down");	// to allow for MAC clone to
	// take effect
#ifdef HAVE_PPP
#endif

	cprintf("done\n");
}

void start_set_routes(void)
{
	char word[80], *tmp;
	char *ipaddr, *netmask, *gateway, *metric, *ifname;

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
	if (strcmp(defgateway, "0.0.0.0")
	    && !nvram_match("wan_proto", "disabled")) {
		eval("route", "del", "default");
		eval("route", "add", "default", "gw", defgateway);
	}
	foreach(word, nvram_safe_get("static_route"), tmp) {
		netmask = word;
		ipaddr = strsep(&netmask, ":");
		if (!ipaddr || !netmask)
			continue;
		gateway = netmask;
		netmask = strsep(&gateway, ":");
		if (!netmask || !gateway)
			continue;
		metric = gateway;
		gateway = strsep(&metric, ":");
		if (!gateway || !metric)
			continue;
		ifname = metric;
		metric = strsep(&ifname, ":");
		if (!metric || !ifname)
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

	if (f_exists("/tmp/tvrouting"))
		system("sh /tmp/tvrouting");
	if (f_exists("/tmp/udhcpstaticroutes"))
		system("sh /tmp/udhcpstaticroutes");
}

#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880)  && !defined(HAVE_RT61)
static int notify_nas(char *type, char *ifname, char *action)
{
	char *argv[] = { "nas4not", type, ifname, action,
		NULL,		/* role */
		NULL,		/* crypto */
		NULL,		/* auth */
		NULL,		/* passphrase */
		NULL,		/* ssid */
		NULL
	};
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
	wl_ioctl(ifname, WLC_GET_INSTANCE, &unit, sizeof(unit));
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

		if (get_wds_wsec(unit, i, mac, role, crypto, auth, ssid, pass)
		    && ether_atoe(mac, ea)
		    && !bcmp(ea, remote, ETHER_ADDR_LEN)) {
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
/*
 * static int notify_nas(char *type, char *ifname, char *action) { char
 * *argv[] = {"nas4not", type, ifname, action, NULL, NULL, NULL, NULL,
 * NULL, NULL}; char *str = NULL; int retries = 10; char tmp[100], prefix[]
 * = "wlXXXXXXXXXX_"; int unit; char remote[ETHER_ADDR_LEN]; char ssid[48],
 * pass[80], auth[16], crypto[16], role[8]; int i;
 * 
 * wl_ioctl(ifname, WLC_GET_INSTANCE, &unit, sizeof(unit)); snprintf(prefix,
 * sizeof(prefix), "wl%d_", unit); #ifdef WPA2_WMM if
 * (nvram_match(strcat_r(prefix, "akm", tmp), "") &&
 * nvram_match(strcat_r(prefix, "auth_mode", tmp), "none")) #else if
 * (nvram_match(strcat_r(prefix, "auth_mode", tmp), "open") ||
 * nvram_match(strcat_r(prefix, "auth_mode", tmp), "shared")) #endif return
 * 0;
 * 
 * wl_ioctl(ifname, WLC_WDS_GET_REMOTE_HWADDR, remote, ETHER_ADDR_LEN); for
 * (i = 0; i < MAX_NVPARSE; i ++) { char mac[ETHER_ADDR_STR_LEN]; uint8
 * ea[ETHER_ADDR_LEN];
 * 
 * if (get_wds_wsec(unit, i, mac, role, crypto, auth, ssid, pass) &&
 * ether_atoe(mac, ea) && !bcmp(ea, remote, ETHER_ADDR_LEN)) { argv[4] =
 * role; argv[5] = crypto; argv[6] = auth; argv[7] = pass; argv[8] = ssid;
 * break; } }
 * 
 * if (i == MAX_NVPARSE) {
 * 
 * argv[4] = "auto";
 * 
 * argv[5] = nvram_safe_get(strcat_r(prefix, "crypto", tmp));
 * 
 * #ifdef WPA2_WMM argv[6] = nvram_safe_get(strcat_r(prefix, "akm", tmp));
 * #else argv[6] = nvram_safe_get(strcat_r(prefix, "auth_mode", tmp)); #endif
 * 
 * argv[7] = nvram_safe_get(strcat_r(prefix, "wpa_psk", tmp));
 * 
 * argv[8] = nvram_safe_get(strcat_r(prefix, "ssid", tmp)); }
 * 
 * while (retries -- > 0 && !(str = file2str("/tmp/nas.lan.pid"))) sleep(1);
 * if (str) { int pid; free(str); return _eval(argv, ">/dev/console", 0,
 * &pid); } return -1;
 * 
 * } 
 */
void start_hotplug_net(void)
{
#ifdef HAVE_MADWIFI
	char *interface, *action;

	interface = getenv("INTERFACE");
	if (!interface)
		return;
	action = getenv("ACTION");
	if (!action)
		return;
	// sysprintf("echo \"Hotplug %s=%s\" > /dev/console\n",action,interface);
	if (strncmp(interface, "ath", 3))
		return;

	// try to parse
	char ifname[32];

	memset(ifname, 0, 32);
	int index = indexof(interface, '.');

	if (index == -1)
		return;
	strncpy(ifname, interface + index + 1, strlen(interface) - (index + 1));
	if (strncmp(ifname, "sta", 3)) {
		return;
	}
	char nr[32];

	memset(nr, 0, 32);
	strcpy(nr, ((unsigned char *)&ifname[0]) + 3);
	memset(ifname, 0, 32);
	strncpy(ifname, interface, index);
	char bridged[32];

	sprintf(bridged, "%s_bridged", ifname);

	if (!strcmp(action, "add")) {

		eval("ifconfig", interface, "up");
		if (nvram_match(bridged, "1"))
			br_add_interface(getBridge(ifname), interface);
	}
	if (!strcmp(action, "remove")) {
		eval("ifconfig", interface, "down");
		if (nvram_match(bridged, "1"))
			br_del_interface(getBridge(ifname), interface);
	}
	return;
#else

	// char *lan_ifname = nvram_safe_get("lan_ifname");
	char *interface, *action;

	if (!(interface = getenv("INTERFACE"))
	    || !(action = getenv("ACTION")))
		return;

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

		if (!strncmp(interface, "wds", 3)
		    && nvram_match("wl_lazywds", "1"))
			br_add_interface("br0", interface);	// eval ("brctl",
		// "addif", "br0",
		// interface);
		/*
		 * Notify NAS of adding the interface 
		 */
		sleep(5);
#if !defined(HAVE_MADWIFI) && !defined(HAVE_RT2880) && !defined(HAVE_RT61)
		notify_nas("lan", interface, "up");
#endif
		if (nvram_match("lan_stp", "0"))
			br_set_stp_state("br0", 0);

		else
			br_set_stp_state("br0", 1);
#ifdef HAVE_MICRO
		br_shutdown();
#endif

	}
	cprintf("config done()\n");
	return;
#endif
}

int init_mtu(char *wan_proto)
{
	if (strcmp(wan_proto, "pppoe") == 0) {	// 576 < mtu < 1454(linksys japan) |
		// 1492(other)
		if (nvram_match("mtu_enable", "0")) {	// Auto
			nvram_set("mtu_enable", "1");
#ifdef BUFFALO_JP
			nvram_set("wan_mtu", "1454");	// set max value
#else
			nvram_set("wan_mtu", "1492");	// set max value
#endif

		} else {	// Manual
#ifdef BUFFALO_JP
			if (atoi(nvram_safe_get("wan_mtu")) > 1454) {
				nvram_set("wan_mtu", "1454");
			}
#else
			if (atoi(nvram_safe_get("wan_mtu")) > 1492) {
				nvram_set("wan_mtu", "1492");
			}
#endif
			if (atoi(nvram_safe_get("wan_mtu")) < 576) {
				nvram_set("wan_mtu", "576");
			}
		}
	} else if (strcmp(wan_proto, "pptp") == 0 || strcmp(wan_proto, "l2tp") == 0) {	// 1200 < mtu < 1400 (1460)
//      if( nvram_match( "mtu_enable", "0" ) )
//      {                       // Auto
//          nvram_set( "mtu_enable", "1" );
//          nvram_set( "wan_mtu", "1460" );     // set max value (linksys
//                                              // request to set to 1460)                                              // 2003/06/23
//      }
//      else
		{		// Manual
			if (atoi(nvram_safe_get("wan_mtu")) > 1460) {
				nvram_set("wan_mtu", "1460");	// set max value (linksys
				// request to set to 1460)
				// 2003/06/23
			}
			if (atoi(nvram_safe_get("wan_mtu")) < 1200) {
				nvram_set("wan_mtu", "1200");
			}
		}
	} else {		// 576 < mtu < 1500
		if (nvram_match("mtu_enable", "0")) {	// Auto
			nvram_set("wan_mtu", "1500");	// set max value
		} else {	// Manual
			if (atoi(nvram_safe_get("wan_mtu")) < 576) {
				nvram_set("wan_mtu", "576");
			}
		}
	}
	return 0;
}
