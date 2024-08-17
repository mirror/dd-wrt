// #define CDEBUG
#include <sys/sysinfo.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <net/if.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <syslog.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/sysinfo.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <sys/ioctl.h>
#undef _LINUX_KERNEL_H
#include <sys/sysinfo.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <resolv.h>
#include <signal.h>

#include <utils.h>
#include <wlutils.h>
#include <bcmnvram.h>
#include <shutils.h>
#include <cy_conf.h>
#include <code_pattern.h>
#include <bcmdevs.h>
#include <net/route.h>
#include <cy_conf.h>
#include <linux/if_ether.h>
// #include <linux/mii.h>
#include <linux/sockios.h>
#include <broadcom.h>
#include <md5.h>

#define SIOCGMIIREG 0x8948 /* Read MII PHY register.  */
#define SIOCSMIIREG 0x8949 /* Write MII PHY register.  */

struct mii_ioctl_data {
	unsigned short phy_id;
	unsigned short reg_num;
	unsigned short val_in;
	unsigned short val_out;
};

int getcpurev(void)
{
	FILE *fp = fopen("/proc/cpuinfo", "rb");

	if (fp == NULL) {
		return -1;
	}
	int cnt = 0;
	int b = 0;

	while (b != EOF) {
		b = getc(fp);
		if (b == ':')
			cnt++;
		if (cnt == 1) {
			getc(fp);
			char cpumodel[65];
			int i = 0;
			for (i = 0; i < 64; i++) {
				cpumodel[i] = getc(fp);
				if (cpumodel[i] == '\n')
					break;
				if (cpumodel[i] == ':') {
					cnt++;
					break;
				}
			}
			cpumodel[i] = 0;
			if (strstr(cpumodel, "BCM5357")) {
				fclose(fp);
				return 51;
			}
		}
		if (cnt == 3) {
			getc(fp);
			char cpurev[33];
			int i = 0;

			for (i = 0; i < 32; i++) {
				cpurev[i] = getc(fp);
				if (cpurev[i] == '\n')
					break;
			}
			cpurev[i] = 0;
			fclose(fp);
			if (strstr(cpurev, "BCM4710 V0.0") || strstr(cpurev,
								     "BMIPS3300 V0.0")) // BCM4702, BCM4710
				// (old 125 MHz)
				return 0;
			if (strstr(cpurev, "BCM3302 V0.6") || strstr(cpurev, "BMIPS3300 V0.6")) // BCM4704
				return 6;
			if (strstr(cpurev, "BCM3302 V0.7") || strstr(cpurev,
								     "BMIPS3300 V0.7")) // BCM4712, BCM5365
				return 7;
			if (strstr(cpurev, "BCM3302 V0.8") || strstr(cpurev,
								     "BMIPS3300 V0.8")) // BCM5350, BCM5352
				return 8;
			if (strstr(cpurev, "BCM3302 V2.9") || strstr(cpurev, "BMIPS3300 V2.9")) // BCM5354
				return 29;
			if (strstr(cpurev, "BCM3302 V1.10") || strstr(cpurev,
								      "BMIPS3300 V1.10")) // BCM4785 (BCM3302 V1.10)
				return 110;
			if (strstr(cpurev,
				   "MIPS 74K V4.9")) // BCM4716B0 (Broadcom BCMB83A chip rev 0)
				return 50; // BCM5356B0 (Broadcom BCM5356 chip rev 1)
			if (strstr(cpurev,
				   "MIPS 74K V4.0")) // BCM4716B0 (Broadcom BCMB83A chip rev 0)
				return 51; // BCM5356B0 (Broadcom BCM5356 chip rev 1)
			if (strstr(cpurev,
				   "MIPS 74Kc V4.9")) // BCM4716B0 (Broadcom BCMB83A chip rev 0) //newer kernels
				return 50; // BCM5356B0 (Broadcom BCM5356 chip rev 1)
			if (strstr(cpurev,
				   "MIPS 74Kc V4.0")) // BCM4716B0 (Broadcom BCMB83A chip rev 0) //newer kernels
				return 51; // BCM5356B0 (Broadcom BCM5356 chip rev 1)
			return -1;
		}
	}
	fclose(fp);
	return -1;
}

int cpu_plltype(void)
{
#ifdef HAVE_NORTHSTAR
	FILE *fp = fopen("/proc/bcm_chipinfo", "rb");
	if (!fp) {
		return 9;
	}
	int chipid;
	int chiprevision;
	int packageoption;
	fscanf(fp, "%*s %X\n", &chipid);
	fscanf(fp, "%*s %X\n", &chiprevision);
	fscanf(fp, "%*s %X\n", &packageoption);
	fclose(fp);
	if (chipid == 53573)
		return 11;
	if (packageoption == 2) {
		if (chipid == 53030)
			return 10; // 600 / 800 / 1000
		else
			return 9; // 600 / 800
	}
	if (packageoption == 0) {
		if (chipid == 53030)
			return 8; // 600 / 800 / 1000 / 1200 / 1400
		else
			return 7; // 600 / 800 / 1000
	}
	return 9;
#endif
#if defined(HAVE_BUFFALO) || defined(BUFFALO_JP)
	if (nvram_match("DD_BOARD", "Buffalo WHR-G54S") || //
	    nvram_match("DD_BOARD", "Buffalo WHR-HP-G54") || //
	    nvram_match("DD_BOARD", "Buffalo AS-A100") || //
	    nvram_match("DD_BOARD", "Buffalo WHR-HP-G54DD")) //
		return 0;
#endif

	int cpurev = getcpurev();
	int cputype = check_hw_type();

	if (cpurev == 0) // BCM4702, BCM4710 (old 125 MHz)
		return 0;
	if (cpurev == 6) // BCM4704
		return 0;
	if (cpurev == 7 && cputype == BCM5365_CHIP) // BCM5365 only
		// supports fixed 200
		// MHz
		return 0;
	if (cpurev == 7 && cputype != BCM5365_CHIP) // BCM4712
		return 4;
	if (cpurev == 8 && cputype == BCM5350_CHIP) // BCM5350
		return 3;
	if (cpurev == 8 && cputype != BCM5350_CHIP) // BCM5352
		return 7;
	if (cpurev == 29) // BCM5354, only supports fixed 240 MHz
		return 0;
	if (cpurev == 110) // BCM4705
		return 2;
	if (cpurev == 50) // BCM4706
		return 8;
	if (cpurev == 51) // BCM4706
		return 10;

	return 0;
}

/* In the space-separated/null-terminated list(haystack), try to
 * locate the string "needle"
 */
char *find_in_list(const char *haystack, const char *needle)
{
	const char *ptr = haystack;
	int needle_len = 0;
	int haystack_len = 0;
	int len = 0;

	if (!haystack || !needle || !*haystack || !*needle)
		return NULL;

	needle_len = strlen(needle);
	haystack_len = strlen(haystack);

	while (*ptr != 0 && ptr < &haystack[haystack_len]) {
		/* consume leading spaces */
		ptr += strspn(ptr, " ");

		/* what's the length of the next word */
		len = strcspn(ptr, " ");

		if ((needle_len == len) && (!strncmp(needle, ptr, len)))
			return (char *)ptr;

		ptr += len;
	}
	return NULL;
}

/**
 *	remove_from_list
 *	Remove the specified word from the list.

 *	@param name word to be removed from the list
 *	@param list Space separated list to modify
 *	@param listsize Max size the list can occupy

 *	@return	error code
 */
int remove_from_list(const char *name, char *list, int listsize)
{
	int listlen = 0;
	int namelen = 0;
	char *occurrence = list;

	if (!list || !name || (listsize <= 0))
		return EINVAL;

	listlen = strlen(list);
	namelen = strlen(name);

	occurrence = find_in_list(occurrence, name);

	if (!occurrence)
		return EINVAL;

	/* last item in list? */
	if (occurrence[namelen] == 0) {
		/* only item in list? */
		if (occurrence != list)
			occurrence--;
		occurrence[0] = 0;
	} else if (occurrence[namelen] == ' ') {
		strncpy(occurrence, &occurrence[namelen + 1 /* space */],
			strlen(&occurrence[namelen + 1 /* space */]) + 1 /* terminate */);
	}

	return 0;
}

/**
 *		add_to_list
 *	Add the specified interface(string) to the list as long as
 *	it will fit in the space left in the list.

 *	NOTE: If item is already in list, it won't be added again.

 *	@param name Name of interface to be added to the list
 *	@param list List to modify
 *	@param listsize Max size the list can occupy

 *	@return	error code
 */
int add_to_list(const char *name, char *list, int listsize)
{
	int listlen = 0;
	int namelen = 0;

	if (!list || !name || (listsize <= 0))
		return EINVAL;

	listlen = strlen(list);
	namelen = strlen(name);

	/* is the item already in the list? */
	if (find_in_list(list, name))
		return 0;

	if (listsize <= listlen + namelen + 1 /* space */ + 1 /* NULL */)
		return EMSGSIZE;

	/* add a space if the list isn't empty and it doesn't already have space */
	if (list[0] != 0 && list[listlen - 1] != ' ') {
		list[listlen++] = 0x20;
	}

	strncpy(&list[listlen], name, namelen + 1 /* terminate */);

	return 0;
}

int count_occurences(char *source, int cmp)
{
	int i, cnt = 0;
	int len = strlen(source);

	for (i = 0; i < len; i++) {
		if (source[i] == cmp)
			cnt++;
	}
	return cnt;
}

int pos_nthoccurence(char *source, int cmp, int which)
{
	int i, cnt = 0;
	int len = strlen(source);

	for (i = 0; i < len; i++) {
		if (source[i] == cmp)
			cnt++;
		if (cnt == which)
			return i;
	}
	return -1;
}

char *substring(int start, int stop, const char *src, char *dst, size_t len)
{
	snprintf(dst, len, "%.*s", stop - start, src + start);

	return dst;
}

/*
 * given month, day, year, returns day of week, eg. Monday = 0 etc.  
 */
int weekday(int month, int day, int year)
{
	int ix, tx, vx = 0;

	switch (month) {
	case 2:
	case 6:
		vx = 0;
		break;
	case 8:
		vx = 4;
		break;
	case 10:
		vx = 8;
		break;
	case 9:
	case 12:
		vx = 12;
		break;
	case 3:
	case 11:
		vx = 16;
		break;
	case 1:
	case 5:
		vx = 20;
		break;
	case 4:
	case 7:
		vx = 24;
		break;
	}

	if (year > 1900) // 1900 was not a leap year
		year -= 1900;
	ix = ((year - 21) % 28) + vx + (month > 2); // take care of
	// February
	tx = (ix + (ix / 4)) % 7 + day; // take care of leap year

	return (tx % 7);
}

const char *getdefaultconfig(char *service, char *path, size_t len, char *configname)
{
	sprintf(path, "/jffs/etc/%s", configname);
	FILE *fp = fopen(path, "r"); //test if custom config is available
	if (fp != NULL) {
		if (service)
			registerCustom(service);
		fclose(fp);
	} else {
		sprintf(path, "/tmp/%s", configname);
	}
	return path;
}

char *get_ipfrominterface(char *ifname, char *ip)
{
	int fd;
	struct ifreq ifr;
	struct ifreq ifrnm;

	fd = socket(AF_INET, SOCK_DGRAM, 0);
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name, ifname, IFNAMSIZ - 1);
	if (ioctl(fd, SIOCGIFADDR, &ifr) < 0)
		return "255.255.255.255/32";
	memcpy(&ifrnm, &ifr, sizeof(ifr));
	if (ioctl(fd, SIOCGIFNETMASK, &ifrnm) < 0)
		return "255.255.255.255/32";
	close(fd);
	inet_addr_to_cidr(((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr, ((struct sockaddr_in *)&ifrnm.ifr_netmask)->sin_addr,
			  ip);
	return ip;
}

char *get_mac_from_ip(char *mac, char *ip)
{
	FILE *fp;
	char line[256];
	char ipa[128]; // ip address
	char hwa[50]; // HW address / MAC
	char mask[50]; // ntemask
	char dev[50]; // interface
	int type; // HW type
	int flags; // flags

	if ((fp = fopen("/proc/net/arp", "rb")) == NULL)
		return NULL;
	char ipcopy[128];
	strlcpy(ipcopy, ip, sizeof(ipcopy));
	char *check = strrchr(ipcopy, ':');
	int ipv6 = 0;
	if (check) {
		ipv6 = strncmp(ipcopy, "::ffff", 6);
		check++;
	} else
		check = ipcopy;

	// Bypass header -- read until newline
	if (fgets(line, sizeof(line) - 1, fp) != NULL) {
		// Read the ARP cache entries.
		// IP address HW type Flags HW address Mask Device
		// 192.168.1.1 0x1 0x2 00:90:4C:21:00:2A * eth0
		while (fgets(line, sizeof(line) - 1, fp)) {
			if (sscanf(line, "%s 0x%x 0x%x %49s %49s %49s\n", ipa, &type, &flags, hwa, mask, dev) != 6)
				continue;
			if (strcmp(ipcopy, ipa))
				continue;
			strncpy(mac, hwa, 17);
			fclose(fp);
			return mac;
		}
	}
	fclose(fp);

	if (ipv6) {
		fp = popen("ip -6 neigh", "rb");
		if (fp) {
			while (fgets(line, sizeof(line) - 1, fp)) {
				if (sscanf(line, "%s %*s %*s %*s %s %*s %*s %*s %*s %*s %*s %*s\n", ipa, hwa) != 2)
					continue;
				if (strcmp(ip, ipa))
					continue;
				strncpy(mac, hwa, 17);
				pclose(fp);
				return mac;
			}
			pclose(fp);
		}
	}
	return NULL;
}

static void add_dnslist_internal(struct dns_lists *dns_list, char *dns, int custom, int ipv6)
{
	int i;
	if (strcmp(dns, "0.0.0.0") && strcmp(dns, "")) {
		int match = 0;
		for (i = 0; i < dns_list->num_servers; i++) { // Skip same DNS
			if (!strcmp(dns_list->dns_server[i].ip, dns))
				match = 1;
		}
		if (!match) {
			dns_list->dns_server = (struct dns_entry *)realloc(dns_list->dns_server,
									   sizeof(struct dns_entry) * (dns_list->num_servers + 1));
			dns_list->dns_server[dns_list->num_servers].ip = strdup(dns);
			dns_list->dns_server[dns_list->num_servers].type = custom;
			dns_list->dns_server[dns_list->num_servers].ipv6 = ipv6;
			dns_list->num_servers++;
		}
	}
}

static void add_dnslist(struct dns_lists *dns_list, char *dns, int custom, int ipv6)
{
	char *next, word[128];
	if (!dns)
		return;
	foreach(word, dns, next)
	{
		add_dnslist_internal(dns_list, word, custom, ipv6);
	}
}

void free_dns_list(struct dns_lists *dns_list)
{
	int i;
	if (!dns_list)
		return;
	for (i = 0; i < dns_list->num_servers; i++) {
		free(dns_list->dns_server[i].ip);
	}
	free(dns_list->dns_server);

	free(dns_list);
}

struct dns_entry *get_dns_entry(struct dns_lists *dns_list, int idx)
{
	if (!dns_list || idx > (dns_list->num_servers - 1))
		return NULL;
	return &dns_list->dns_server[idx];
}

struct dns_lists *get_dns_list(int v6)
{
	struct dns_lists *dns_list = NULL;
	int altdns_index = 1;

	dns_list = (struct dns_lists *)malloc(sizeof(struct dns_lists));
	bzero(dns_list, sizeof(struct dns_lists));
	char *sv_localdns = nvram_safe_get("sv_localdns");
	char *wan_dns = nvram_safe_get("wan_dns");
	char *wan_get_dns = nvram_safe_get("wan_get_dns");
	char *openvpn_get_dns = nvram_safe_get("openvpn_get_dns");
	char *wg_get_dns = nvram_safe_get("wg_get_dns");
	if (v6 != 2) {
		if (*openvpn_get_dns) {
			add_dnslist(dns_list, openvpn_get_dns, 0, 0);
		}
		if (*wg_get_dns) {
			add_dnslist(dns_list, wg_get_dns, 0, 0);
		}
		/*
		 * if < 3 DNS servers found, try to insert alternates 
		 */
		while (altdns_index <= 3) {
			char altdnsvar[32] = { 0 };

			snprintf(altdnsvar, 31, "altdns%d", altdns_index);

			if (*(nvram_safe_get(altdnsvar))) {
				add_dnslist(dns_list, nvram_safe_get(altdnsvar), 1, 0);
			}
			altdns_index++;
		}
		if (*sv_localdns)
			add_dnslist(dns_list, sv_localdns, 0, 0);
		//egc if DNS server from WG or OpenVPN exist do not add existing DNS server from Static DNS to stop DNS leak
		if (*wan_dns && !*wg_get_dns && !*openvpn_get_dns) {
			add_dnslist(dns_list, wan_dns, 0, 0);
		}
		if (!nvram_match("ignore_wan_dns", "1") || nvram_match("wan_proto", "static")) {
			if (*wan_get_dns) {
				add_dnslist(dns_list, wan_get_dns, 0, 0);
			}
		}
	}
#ifdef HAVE_IPV6
	if (v6 && nvram_matchi("ipv6_enable", 1)) {
		char *a1 = nvram_safe_get("ipv6_dns1");
		char *a2 = nvram_safe_get("ipv6_dns2");
		if (*a1)
			add_dnslist(dns_list, a1, 1, 1);
		if (*a2)
			add_dnslist(dns_list, a2, 1, 1);

		if (!nvram_match("ignore_wan_dns", "1") || nvram_match("wan_proto", "static")) {
			char *next, *wordlist = nvram_safe_get("ipv6_get_dns");
			char word[64];
			foreach(word, wordlist, next)
			{
				add_dnslist(dns_list, word, 0, 1);
			}
		}
	}
#endif

	return dns_list;
}

int dns_to_resolv(void)
{
	FILE *fp_w;
	struct dns_lists *dns_list = NULL;
	int i = 0;

	/*
	 * Save DNS to resolv.conf 
	 */
	if (!(fp_w = fopen(RESOLV_FILE, "w"))) {
		perror(RESOLV_FILE);
		return errno;
	}
	if (nvram_invmatch("wan_get_domain", "") && nvram_matchi("ignore_wan_dns", 0)) {
		fprintf(fp_w, "search %s\n", nvram_safe_get("wan_get_domain"));
	} else if (nvram_invmatch("wan_domain", "")) {
		fprintf(fp_w, "search %s\n", nvram_safe_get("wan_domain"));
	}
	if (nvram_invmatch("lan_domain", "")) {
		fprintf(fp_w, "search %s\n", nvram_safe_get("lan_domain"));
	}
	if (nvram_matchi("dnsmasq_enable", 1) && nvram_matchi("dns_dnsmasq", 1)) {
		fprintf(fp_w, "nameserver %s\n", nvram_safe_get("lan_ipaddr"));
		fclose(fp_w);
		if (!(fp_w = fopen(RESOLV_FORW, "w"))) {
			perror(RESOLV_FORW);
			return errno;
		}
	}

	dns_list = get_dns_list(1);

	for (i = 0; i < dns_list->num_servers; i++)
		fprintf(fp_w, "nameserver %s\n", dns_list->dns_server[i].ip);

	/*
	 * Put a pseudo DNS IP to trigger Connect On Demand 
	 */
	if (dns_list->num_servers == 0 &&
	    (nvram_match("wan_proto", "pppoe") || nvram_match("wan_proto", "pptp")
#ifdef HAVE_PPPOEDUAL
	     || nvram_match("wan_proto", "pppoe_dual")
#endif
#ifdef HAVE_PPPOATM
	     || nvram_match("wan_proto", "pppoa")
#endif
#ifdef HAVE_L2TP
	     || nvram_match("wan_proto", "l2tp")
#endif
#ifdef HAVE_3G
	     || nvram_match("wan_proto", "3g")
#endif
		     ) &&
	    nvram_matchi("ppp_demand", 1))
		fprintf(fp_w, "nameserver 1.1.1.1\n");

	fclose(fp_w);
	free_dns_list(dns_list);

	eval("touch", "/tmp/hosts");

	return 1;
}

/*
 * Example: lan_ipaddr = 192.168.1.1 get_dns_ip("lan_ipaddr", 1); produces
 * "168" 
 */
int get_single_ip(char *ipaddr, int which)
{
	static int ip[5] = { 0, 0, 0, 0, 0 };
	int ret;
	if (which == 4)
		ret = sscanf(ipaddr, "%d.%d.%d.%d/%d", &ip[0], &ip[1], &ip[2], &ip[3], &ip[4]);
	else
		ret = sscanf(ipaddr, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
	return ip[which];
}

char *get_complete_ip(char *from, char *to)
{
	static char ipaddr[20];

	int i[4];

	if (!from || !to)
		return "0.0.0.0";

	if (sscanf(from, "%d.%d.%d.%d", &i[0], &i[1], &i[2], &i[3]) != 4)
		return "0.0.0.0";

	snprintf(ipaddr, sizeof(ipaddr), "%d.%d.%d.%s", i[0], i[1], i[2], to);

	return ipaddr;
}

char *get_complete_lan_ip(char *ip)
{
	static char ipaddr[20];

	int i[4];

	if (sscanf(nvram_safe_get("lan_ipaddr"), "%d.%d.%d.%d", &i[0], &i[1], &i[2], &i[3]) != 4)
		return "0.0.0.0";

	snprintf(ipaddr, sizeof(ipaddr), "%d.%d.%d.%s", i[0], i[1], i[2], ip);

	return ipaddr;
}

/*
 * =====================================================================================
 * by tallest
 * ===================================================================================== 
 */

int get_int_len(int num)
{
	char buf[80];

	snprintf(buf, sizeof(buf), "%d", num);

	return strlen(buf);
}

#define READ_BUF_SIZE 254
/*
 * from busybox find_pid_by_name.c 
 */
pid_t *find_pid_by_name(char *pidName)
{
	DIR *dir;
	struct dirent *next;
	pid_t *pidList = NULL;
	int i = 0;

	if ((dir = opendir("/proc")) != NULL) {
		while ((next = readdir(dir)) != NULL) {
			FILE *status;
			char filename[READ_BUF_SIZE];
			char buffer[READ_BUF_SIZE];
			char name[READ_BUF_SIZE];

			/*
			 * Must skip ".." since that is outside /proc 
			 */
			if (strcmp(next->d_name, "..") == 0)
				continue;

			/*
			 * If it isn't a number, we don't want it 
			 */
			if (!isdigit(*next->d_name))
				continue;

			sprintf(filename, "/proc/%s/status", next->d_name);
			if (!(status = fopen(filename, "r"))) {
				continue;
			}
			if (fgets(buffer, READ_BUF_SIZE - 1, status) == NULL) {
				fclose(status);
				continue;
			}
			fclose(status);

			/*
			 * Buffer should contain a string like "Name: binary_name" 
			 */
			sscanf(buffer, "%*s %s", name);
			// printf("buffer=[%s] name=[%s]\n",buffer,name);
			if (strcmp(name, pidName) == 0) {
				pidList = realloc(pidList, sizeof(pid_t) * (i + 2));
				if (pidList)
					pidList[i++] = strtol(next->d_name, NULL, 0);
			}
		}

		if (pidList)
			pidList[i] = 0;
		else {
			pidList = realloc(pidList, sizeof(pid_t));
			if (pidList)
				pidList[0] = -1;
		}
	}
	return pidList;
}

/*
 * Find first process pid with same name from ps command 
 */
int find_pid_by_ps(char *pidName)
{
	FILE *fp;
	int pid = -1;
	char line[254];

	if ((fp = popen("ps", "r"))) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			if (strstr(line, pidName)) {
				sscanf(line, "%d", &pid);
				printf("%s pid is %d\n", pidName, pid);
				break;
			}
		}
		pclose(fp);
	}

	return pid;
}

/*
 * Find all process pid with same name from ps command 
 */
int *find_all_pid_by_ps(char *pidName)
{
	FILE *fp;
	int pid = -1;
	char line[254];
	int *pidList = NULL;
	int i = 0;

	cprintf("Search for %s\n", pidName);
	if ((fp = popen("ps", "r"))) {
		while (fgets(line, sizeof(line), fp) != NULL) {
			if (strstr(line, pidName)) {
				sscanf(line, "%d", &pid);
				cprintf("%s pid is %d\n", pidName, pid);
				pidList = realloc(pidList, sizeof(int) * (i + 2));
				pidList[i++] = pid;
			}
		}
		pclose(fp);
	}
	if (pidList)
		pidList[i] = 0;
	else {
		pidList = realloc(pidList, sizeof(int));
		pidList[0] = -1;
	}
	cprintf("Search done...\n");

	return pidList;
}

void encode(char *buf, int len)
{
	int i;
	char ch;

	for (i = 0; i < len; i++) {
		ch = (buf[i] & 0x03) << 6;
		buf[i] = (buf[i] >> 2);
		buf[i] &= 0x3f;
		buf[i] |= ch;
		buf[i] = ~buf[i];
	}
}

void decode(char *buf, int len)
{
	int i;
	char ch;

	for (i = 0; i < len; i++) {
		ch = (buf[i] & 0xC0) >> 6;
		buf[i] = (buf[i] << 2) | ch;
		buf[i] = ~buf[i];
	}
}

/*
 * v1.41.7 => 014107 v1.2 => 0102 
 */
int convert_ver(char *ver)
{
	char buf[10];
	int v[3];
	int ret;

	ret = sscanf(ver, "v%d.%d.%d", &v[0], &v[1], &v[2]);

	if (ret == 2) {
		snprintf(buf, sizeof(buf), "%02d%02d", v[0], v[1]);
		return atoi(buf);
	} else if (ret == 3) {
		snprintf(buf, sizeof(buf), "%02d%02d%02d", v[0], v[1], v[2]);
		return atoi(buf);
	} else
		return -1;
}

int check_now_boot(void)
{
	char *ver = nvram_safe_get("pmon_ver");
	char *cfe = nvram_safe_get("CFEver");

	// for 4712
	// The boot_ver value is lower v2.0 (no included)
	if (!strncmp(ver, "PMON", 4)) {
		cprintf("The boot is PMON\n");
		return PMON_BOOT;
	}
	// for 4712
	// The boot_ver value is higher v2.0 (included)
	else if (!strncmp(ver, "CFE", 3)) {
		cprintf("The boot is CFE\n");
		return CFE_BOOT;
	} else if (!strncmp(ver, "2", 1)) {
		cprintf("The boot is CFE %s\n", ver);
		return CFE_BOOT;
	} else if (!strncmp(cfe, "MotoWR", 6)) {
		cprintf("The boot is Motorola CFE\n");
		return CFE_BOOT;
	} else {
		cprintf("The boot is UNKNOWN\n");
		return UNKNOWN_BOOT;
	}
}

int check_hw_type(void)
{
	char *boardtype = nvram_safe_get("boardtype");
	uint boardflags = strtoul(nvram_safe_get("boardflags"), NULL, 0);
	uint btype = strtoul(boardtype, NULL, 0);
	char *vlan0 = nvram_safe_get("vlan0ports");
	char *vlan1 = nvram_safe_get("vlan1ports");

	if (!strncmp(boardtype, "bcm94710", 8))
		return BCM4702_CHIP;
	else if (btype == 0x0708 && !(boardflags & BFL_ENETADM))
		return BCM5325E_CHIP;
	else if (btype == 0x456 && getRouterBrand() == ROUTER_BELKIN_F5D7231) // stupid
		// Belkin!
		return BCM5352E_CHIP;
	else if (btype == 0x456)
		return BCM5350_CHIP;
	else if (!strncmp(boardtype, "bcm95365", 8))
		return BCM5365_CHIP;
	else if (btype == 0x048e)
		return BCM5354G_CHIP;
	else if (btype == 0x042f && !(boardflags & BFL_ENETADM))
		return BCM4704_BCM5325F_CHIP;
	else if (btype == 0x478 && strstr(vlan0, "5*")) /* WRT300NV1.1 */
		return BCM4705L_BCM5325E_EWC_CHIP;
	else if ((btype == 0x478 && (strstr(vlan0, "8*") || strstr(vlan1, "8*"))) || nvram_match("boot_hw_model", "WRT350N") ||
		 nvram_match("boot_hw_model", "WRT610N")) /* WRT350N 
																						 */
		return BCM4705_BCM5397_EWC_CHIP;
	else if (btype == 0x489 || nvram_match("boot_hw_model", "WRT310N")) /* WRT310N, 
										 * temporal 
										 * boardtype 
										 * 0x478, 
										 * wait 
										 * for 
										 * braodcom's 
										 * txt 
										 * file 
										 */
		return BCM4705G_BCM5395S_EWC_CHIP;
	else if (btype == 0x0467)
		return BCM5352E_CHIP;
	else if (btype == 0x0101 && !(boardflags & BFL_ENETADM))
		return BCM4712_BCM5325E_CHIP;
	else if ((btype == 0x0101 || btype == 0x0446) && (boardflags & BFL_ENETADM)) // 0x446
		// is
		// wap54g
		// v2
		return BCM4712_CHIP;
	else if (btype == 0x0472 && !(boardflags & BFL_ENETADM))
		return BCM4704_BCM5325F_EWC_CHIP;
	else
		return NO_DEFINE_CHIP;
}

static char *device_name[] = { "eth0", "qos0" };

char *get_device_name(void)
{
	int i;

	switch (check_hw_type()) {
	case BCM5325E_CHIP:
	case BCM5350_CHIP:
	case BCM5365_CHIP:
	case BCM4704_BCM5325F_CHIP:
	case BCM4704_BCM5325F_EWC_CHIP:
	case BCM5352E_CHIP:
	case BCM5354G_CHIP:
	case BCM4712_BCM5325E_CHIP:
	case BCM4705L_BCM5325E_EWC_CHIP:
		i = 0;
		break;
	case BCM4702_CHIP:
	case BCM4712_CHIP:
	default:
		i = 1;
		break;
	}

	return device_name[i];
}

char *strncpyz(char *dest, char const *src, size_t size)
{
	if (!size--)
		return dest;
	strncpy(dest, src, size);
	dest[size] = 0; /* Make sure the string is null terminated */
	return dest;
}

static int sockets_open(int domain, int type, int protocol)
{
	int fd = socket(domain, type, protocol);

	if (fd < 0)
		cprintf("sockets_open: no usable address was found.\n");
	return fd;
}

int sys_netdev_ioctl(int family, int socket, char *if_name, int cmd, struct ifreq *ifr)
{
	int rc, s;

	if ((s = socket) < 0) {
		if ((s = sockets_open(family, family == AF_PACKET ? SOCK_PACKET : SOCK_DGRAM,
				      family == AF_PACKET ? htons(ETH_P_ALL) : 0)) < 0) {
			cprintf("sys_netdev_ioctl: failed\n");
			return -1;
		}
	}
	strncpyz(ifr->ifr_name, if_name, IFNAMSIZ);
	rc = ioctl(s, cmd, ifr);
	if (socket < 0)
		close(s);
	return rc;
}

int set_register_value(unsigned short port_addr, unsigned short option_content)
{
	struct ifreq ifr;
	struct mii_ioctl_data stats;

	stats.phy_id = port_addr;
	stats.val_in = option_content;

	ifr.ifr_data = (void *)&stats;

	if (sys_netdev_ioctl(AF_INET, -1, get_device_name(), SIOCSMIIREG, &ifr) < 0)
		return -1;

	return 0;
}

unsigned int get_register_value(unsigned short id, unsigned short num)
{
	struct ifreq ifr;
	struct mii_ioctl_data stats;

	stats.phy_id = id;
	stats.reg_num = num;
	stats.val_in = 0;
	stats.val_out = 0;

	ifr.ifr_data = (void *)&stats;

	sys_netdev_ioctl(AF_INET, -1, get_device_name(), SIOCGMIIREG, &ifr);

	return ((stats.val_in << 16) | stats.val_out);
}

struct mtu_lists mtu_list[] = {
#ifdef BUFFALO_JP
	{ "pppoe", "576", "1454" },
#else
	{ "pppoe", "576", "1492" },
#endif
	{ "pppoe_dual", "576", "1492" }, { "pppoa", "576", "16320" },  { "pptp", "576", "1460" },
	{ "l2tp", "576", "1460" },	 { "dhcp", "576", "16320" },   { "dhcp_auth", "576", "16320" },
	{ "iphone", "576", "1500" },	 { "android", "576", "1500" }, { "static", "576", "16320" },
	{ "heartbeat", "576", "1500" },	 { "3g", "576", "1500" },      { "default", "576", "16320" }, // The value must be at last
};

struct mtu_lists *get_mtu(char *proto)
{
	struct mtu_lists *v = NULL;

	for (v = mtu_list; v < &mtu_list[STRUCT_LEN(mtu_list)]; v++) {
		if (!strcmp(proto, v->proto))
			return v;
	}
	return v; // Use default settings
}

void set_host_domain_name(void)
{
	char *wan_hostname = nvram_safe_get("wan_hostname");
	char *wan_domain = nvram_safe_get("wan_domain");

	/*
	 * Allow you to use gethostname to get Host Name 
	 */
	/*
	 * If wan_hostname is blank then we do nothing, we leave to what it was
	 * set at boot 
	 */
	if (*wan_hostname)
		sethostname(wan_hostname, strlen(wan_hostname));

	/*
	 * Allow you to use getdomainname to get Domain Name 
	 */
	if (*wan_domain && strlen(wan_domain) <= 64) // no
		// more
		// than
		// 64
		setdomainname(wan_domain, strlen(wan_domain));
	else {
		char *wan_get_domain = nvram_safe_get("wan_get_domain");

		setdomainname(wan_get_domain, strlen(wan_get_domain));
	}
}

int first_time(void)
{
	struct sysinfo info;

	sysinfo(&info);
	if (info.uptime < 20L)
		return 1;
	return 0;
}

#ifdef CDEBUG
int coreleft(void)
{
	struct sysinfo info;

	sysinfo(&info);
	return info.freeram;
}

int mcoreleft(void)
{
	struct mallinfo minfo;

	minfo = mallinfo();
	return minfo.uordblks;
	// int uordblks; /* total allocated space */
}
#endif

#define sin_addr(s) (((struct sockaddr_in *)(s))->sin_addr)

// #define WDS_DEBUG 1
#undef WDS_DEBUG
#ifdef WDS_DEBUG
FILE *fp;
#endif

int wds_dev_config(int dev, int up)
{
	char wds_var[32] = "";
	char wds_enable_var[32] = "";
	char wds_dev[32] = "";
	char *wds = (void *)0;
	char wds_gw_var[32] = "";
	char *gw = (void *)0;
	int s = -1;
	struct ifreq ifr;

#ifdef WDS_DEBUG
	fp = fopen("/tmp/.wds_debug.log", "a");
#endif

	bzero(&ifr, sizeof(struct ifreq));

	snprintf(wds_var, 31, "wl_wds%d", dev);
	snprintf(wds_enable_var, 31, "%s_enable", wds_var);

	if ((wds = nvram_safe_get(wds_enable_var)) == NULL || strcmp(wds, "0") == 0)
		return -1;
	snprintf(wds_dev, 31, "wds0.%d", dev + 1);
	snprintf(ifr.ifr_name, IFNAMSIZ, wds_dev);
#ifdef WDS_DEBUG
	fprintf(fp, "opening kernelsocket\n");
#endif
	if ((s = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) < 0)
		return -1;

	if (up) {
		char wds_hwaddr_var[32] = "";
		char wds_ip_var[32] = "";
		char wds_netmask_var[32] = "";
		char *wds_list = (void *)0;
		char *hwaddr = (void *)0;
		char *ip = (void *)0;
		char *netmask = (void *)0;

#ifdef WDS_DEBUG
		fprintf(fp, "running up\n");
#endif

		wds_list = nvram_safe_get("wl0_wds");
		if (wds_list == (void *)0 || !*wds_list)
			return 0;

		snprintf(wds_hwaddr_var, 31, "%s_hwaddr", wds_var);
		snprintf(wds_ip_var, 31, "%s_ipaddr", wds_var);
		snprintf(wds_netmask_var, 31, "%s_netmask", wds_var);

		hwaddr = nvram_safe_get(wds_hwaddr_var);
		ip = nvram_safe_get(wds_ip_var);
		netmask = nvram_safe_get(wds_netmask_var);

		if (!strstr(wds_list, hwaddr)) {
			close(s);
			return -1;
		}
#ifdef WDS_DEBUG
		fprintf(fp, "checking validity\n");
#endif

		if (!sv_valid_hwaddr(hwaddr) || !sv_valid_ipaddr(ip) || !sv_valid_ipaddr(netmask)) {
			close(s);
			return -1;
		}
#ifdef WDS_DEBUG
		fprintf(fp, "valid mac %s ip %s nm %s\n", hwaddr, ip, netmask);
#endif

		sysprintf("ifconfig %s down", wds_dev);

		sysprintf("ifconfig %s %s netmask %s up", wds_dev, ip, netmask);

		snprintf(wds_gw_var, 31, "%s_gw", wds_var);
		gw = nvram_safe_get(wds_gw_var);
		if (strcmp(gw, "0.0.0.0") != 0) {
			get_network(ip, netmask);
			route_del(wds_dev, 0, ip, gw, netmask);
			route_add(wds_dev, 0, ip, gw, netmask);
		}

	} else {
#ifdef WDS_DEBUG
		fprintf(fp, "running down\n");
#endif
		sysprintf("ifconfig %s down", wds_dev);
	}

#ifdef WDS_DEBUG
	fprintf(fp, "running ioctl\n");
	fclose(fp);
#endif

	close(s);

	return 0;
}

int sv_valid_range(char *value, int low, int high)
{
	if (!isdigit(value[0]) || atoi(value) < low || atoi(value) > high)
		return FALSE;
	else
		return TRUE;
}

int sv_valid_statics(char *value)
{
	char ip[16] = { 0 }, mac[18] = { 0 }, hostname[255] = { 0 }, *p = value;

	if (NULL == value)
		return FALSE;

	do {
		while (isspace(*p++) && p - value < strlen(value))
			;

		if (p - value >= strlen(value))
			return FALSE;

		if (sscanf(p, "%15s%17s%254s", ip, mac, hostname) < 3)
			return FALSE;

		if (!sv_valid_ipaddr(ip) || !sv_valid_hwaddr(mac) || !*hostname)
			return FALSE;

	} while ((p = strpbrk(p, "\n\r")) && p - value < strlen(value));

	return TRUE;
}

/*
 * Example: legal_ipaddr("192.168.1.1"); return true;
 * legal_ipaddr("192.168.1.1111"); return false; 
 */
int sv_valid_ipaddr(char *value)
{
	struct in_addr ipaddr;
	int ip[4], ret = 0;

	ret = sscanf(value, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);

	if (ret != 4 || !inet_aton(value, &ipaddr))
		return FALSE;
	else
		return TRUE;
}

// note - network address returned in ipaddr
void get_network(char *ipaddr, char *netmask)
{
	int ip[4], mask[4];

	if (!ipaddr || !netmask)
		return;

	sscanf(ipaddr, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);
	sscanf(netmask, "%d.%d.%d.%d", &mask[0], &mask[1], &mask[2], &mask[3]);

	ip[0] &= mask[0];
	ip[1] &= mask[1];
	ip[2] &= mask[2];
	ip[3] &= mask[3];

	sprintf(ipaddr, "%d.%d.%d.%d", ip[0], ip[1], ip[2], ip[3]);
#ifdef WDS_DEBUG
	fprintf(fp, "get_network return %s\n", ipaddr);
#endif
}

/*
 * note: copied from Broadcom code and put in shared via this file 
 */

int route_manip(int cmd, char *name, int metric, char *dst, char *gateway, char *genmask)
{
	int s;
	struct rtentry rt;

	// dprintf("cmd=[%d] name=[%s] ipaddr=[%s] netmask=[%s] gateway=[%s]
	// metric=[%d]\n",cmd,name,dst,genmask,gateway,metric);

	/*
	 * Open a raw socket to the kernel 
	 */
	if ((s = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
		goto err;

	/*
	 * Fill in rtentry 
	 */
	bzero(&rt, sizeof(rt));
	if (dst)
		inet_aton(dst, &sin_addr(&rt.rt_dst));
	if (gateway)
		inet_aton(gateway, &sin_addr(&rt.rt_gateway));
	if (genmask)
		inet_aton(genmask, &sin_addr(&rt.rt_genmask));
	rt.rt_metric = metric;
	rt.rt_flags = RTF_UP;
	if (sin_addr(&rt.rt_gateway).s_addr)
		rt.rt_flags |= RTF_GATEWAY;
	if (sin_addr(&rt.rt_genmask).s_addr == INADDR_BROADCAST)
		rt.rt_flags |= RTF_HOST;
	rt.rt_dev = name;

	/*
	 * Force address family to AF_INET 
	 */
	rt.rt_dst.sa_family = AF_INET;
	rt.rt_gateway.sa_family = AF_INET;
	rt.rt_genmask.sa_family = AF_INET;

	if (ioctl(s, cmd, &rt) < 0)
		goto err;

	close(s);
	return 0;

err:
	close(s);
	perror(name);
	return errno;
}

int route_add(char *name, int metric, char *dst, char *gateway, char *genmask)
{
	dd_debug(DEBUG_CONSOLE, "route_add: if:%s dst:%s gw: %s mask: %s \n", name, dst, gateway, genmask);
	return route_manip(SIOCADDRT, name, metric, dst, gateway, genmask);
}

int route_del(char *name, int metric, char *dst, char *gateway, char *genmask)
{
	dd_debug(DEBUG_CONSOLE, "route_del: if:%s dst:%s gw: %s mask: %s \n", name, dst, gateway, genmask);
	return route_manip(SIOCDELRT, name, metric, dst, gateway, genmask);
}

// #endif

void getIfLists(char *eths, int size)
{
	char eths2[256];

	bzero(eths, size);
	bzero(eths2, 256);
#ifdef HAVE_XSCALE
	getIfList(eths, "ixp");
	getIfList(eths2, "eth");
	strcat(eths, " ");
	strcat(eths, eths2);
#else
	getIfList(eths, "eth");
#endif
	bzero(eths2, 256);
	getIfList(eths2, "vlan");
	strcat(eths, " ");
	strcat(eths, eths2);
#ifdef HAVE_MADWIFI
	bzero(eths2, 256);
	getIfList(eths2, "wlan");
	strcat(eths, " ");
	strcat(eths, eths2);
#elif defined(HAVE_RT2880) || defined(HAVE_RT61)
	bzero(eths2, 256);
	getIfList(eths2, "ra");
	strcat(eths, " ");
	strcat(eths, eths2);

	bzero(eths2, 256);
	getIfList(eths2, "apcli");
	strcat(eths, " ");
	strcat(eths, eths2);
	bzero(eths2, 256);
	getIfList(eths2, "wds");
	strcat(eths, " ");
	strcat(eths, eths2);
#else
	bzero(eths2, 256);
	getIfList(eths2, "wl");
	strcat(eths, " ");
	strcat(eths, eths2);
#endif
	bzero(eths2, 256);
	getIfList(eths2, "br");
	strcat(eths, " ");
	strcat(eths, eths2);

	bzero(eths2, 256);
	getIfList(eths2, "oet");
	strcat(eths, " ");
	strcat(eths, eths2);

	bzero(eths2, 256);
	getIfList(eths2, "vxlan");
	strcat(eths, " ");
	strcat(eths, eths2);
#ifdef HAVE_WAVESAT
	bzero(eths2, 256);
	getIfList(eths2, "ofdm");
	strcat(eths, " ");
	strcat(eths, eths2);
#endif
}

static uint32_t str_to_addr(const char *addr)
{
	uint32_t split[4];
	uint32_t ip;

	sscanf(addr, "%d.%d.%d.%d", &split[0], &split[1], &split[2], &split[3]);

	ip = (split[0] << 24) | (split[1] << 16) | (split[2] << 8) | (split[3]);

	return htonl(ip);
}

void getHostName(char *buf, char *ip)
{
	struct hostent *host;
	struct in_addr addr;

	res_init();
	addr.s_addr = str_to_addr(ip);
	host = gethostbyaddr((char *)&addr, 4, AF_INET);
	if (!host || !host->h_name)
		strcpy(buf, "unknown");
	else
		strcpy(buf, host->h_name);
}

void getinterfacelist(const char *ifprefix, char *buffer)
{
	int count = getifcount(ifprefix);
	int i;

	for (i = 0; i < count; i++) {
		char ifname[32];

		sprintf(ifname, "%s%d", ifprefix, i);
		strcat(buffer, ifname);
		if (i < count - 1)
			strcat(buffer, " ");
	}
}

int softkill(char *name)
{
	killall(name, SIGKILL);
	return 0;
}

int getmask(char *nmask)
{
	int loopmask = 0;
	int ip[4] = { 0, 0, 0, 0 };

	sscanf(nmask, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);

	int n = 8;

	for (--n; n >= 0; --n) // test all 4 bytes in one pass
	{
		if (ip[0] & 1 << n)
			loopmask++;
		if (ip[1] & 1 << n)
			loopmask++;
		if (ip[2] & 1 << n)
			loopmask++;
		if (ip[3] & 1 << n)
			loopmask++;
	}
	return loopmask;
}

int doMultiCast(void)
{
	char wan_if_buffer[33];
	char name[80], *next;
	int ifcount = 0;

	if (nvram_match("wan_proto", "disabled"))
		return 0;
	if (nvram_matchi("block_multicast", 0)) {
		ifcount++;
	}

	char ifnames[256];

	getIfLists(ifnames, 256);
	foreach(name, ifnames, next)
	{
		if (strcmp(safe_get_wan_face(wan_if_buffer), name) && strcmp(nvram_safe_get("lan_ifname"), name) &&
		    strcmp(nvram_safe_get("tvnicfrom"), name)) {
			if ((nvram_nmatch("0", "%s_bridged", name) || isbridge(name)) && nvram_nmatch("1", "%s_multicast", name)) {
				ifcount++;
			}
		}
	}
	return ifcount;
}

static int sockaddr_to_dotted(struct sockaddr *saddr, char *buf, size_t len)
{
	buf[0] = '\0';
	if (saddr->sa_family == AF_INET) {
		inet_ntop(AF_INET, &((struct sockaddr_in *)saddr)->sin_addr, buf, len);
		return 0;
	}
	if (saddr->sa_family == AF_INET6) {
		inet_ntop(AF_INET6, &((struct sockaddr_in6 *)saddr)->sin6_addr, buf, len);
		return 0;
	}
	return -1;
}

static int sockaddr_to_dotted_n(char *sin_addr, char *buf, size_t len)
{
	inet_ntop(AF_INET, sin_addr, buf, len);
	return 0;
}

#define DIE_ON_ERROR AI_CANONNAME

void getIPFromName(char *name, char *ip, size_t len)
{
	int count = 5;
	while (count--) {
		struct addrinfo *result = NULL;
		int rc;
		struct addrinfo hint;
		struct hostent *hp = gethostbyname(name);
		if (hp != NULL) {
			sockaddr_to_dotted_n(hp->h_addr_list[0], ip, len);
			if (strcmp(ip, "0.0.0.0"))
				break;
		}
		res_init();
		bzero(&hint, sizeof(hint));
		hint.ai_family = AF_INET;
		hint.ai_socktype = SOCK_STREAM;
		hint.ai_flags = DIE_ON_ERROR;
		rc = getaddrinfo(name, NULL, &hint, &result);
		if (!result) // give it a second try
			rc = getaddrinfo(name, NULL, &hint, &result);

		if (result) {
			sockaddr_to_dotted(result->ai_addr, ip, len);
			freeaddrinfo(result);
		} else {
			struct hostent *hp = gethostbyname(name);
			if (hp != NULL) {
				sockaddr_to_dotted_n(hp->h_addr_list[0], ip, len);
			} else
				snprintf(ip, len, "0.0.0.0");
		}
		if (strcmp(ip, "0.0.0.0"))
			break;
		struct timespec tim, tim2;
		tim.tv_sec = 1;
		tim.tv_nsec = 0;
		nanosleep(&tim, &tim2);
	}
}

/*
 * Example: legal_ip_netmask("192.168.1.1","255.255.255.0","192.168.1.100");
 * return true;
 * legal_ip_netmask("192.168.1.1","255.255.255.0","192.168.2.100"); return
 * false; 
 */
int legal_ip_netmask(char *sip, char *smask, char *dip)
{
	struct in_addr ipaddr, netaddr, netmask;
	int tag;

	inet_aton(nvram_safe_get(sip), &netaddr);
	inet_aton(nvram_safe_get(smask), &netmask);
	inet_aton(dip, &ipaddr);

	netaddr.s_addr &= netmask.s_addr;

	if (netaddr.s_addr != (ipaddr.s_addr & netmask.s_addr))
		tag = FALSE;
	else
		tag = TRUE;

	return tag;
}

/*
 * Example: ISDIGIT("", 0); return true; ISDIGIT("", 1); return false;
 * ISDIGIT("123", 1); return true; 
 */
int ISDIGIT(char *value, int flag)
{
	int i, tag = TRUE;

	if (!strcmp(value, "")) {
		if (flag)
			return 0; // null
		else
			return 1;
	}

	for (i = 0; *(value + i); i++) {
		if (!isdigit(*(value + i))) {
			tag = FALSE;
			break;
		}
	}
	return tag;
}

void rep(char *in, char from, char to)
{
	int i;
	int slen = strlen(in);

	for (i = 0; i < slen; i++)
		if (in[i] == from)
			in[i] = to;
}

#include "l7protocols.h"

int get_risk_by_name(char *name)
{
	l7filters *filters = filters_list;
	while (filters->name) {
		if (!strcmp(filters->name, name))
			return filters->level;
		filters++;
	}
	return 0;
}

char *get_dep_by_name(char *name)
{
	l7filters *filters = filters_list;
	while (filters->name) {
		if (!strcmp(filters->name, name))
			return filters->matchdep;
		filters++;
	}
	return 0;
}

l7filters *get_raw_filters(void)
{
	return filters_list;
}

char *get_filter_services(void)
{
	l7filters *filters = filters_list;
	char temp[128] = "";
	char *proto[] = { "l7", "p2p", "dpi", "risk" };
	char *services = NULL;

	while (filters->name) // add l7 and p2p filters
	{
#ifdef HAVE_OPENDPI
		if (filters->protocol == L7_ONLY) {
		    filters++;
		    continue;
		}
#endif
		sprintf(temp, "$NAME:%03d:%s$PROT:%03d:%s$PORT:003:0:0<&nbsp;>", strlen(filters->name), filters->name,
			strlen(proto[filters->protocol]), proto[filters->protocol]);
		if (!services) {
			services = malloc(strlen(temp) + 1);
			services[0] = 0;
		} else
			services = realloc(services, strlen(services) + strlen(temp) + 1);
		strcat(services, temp);
		filters++;
	}
	if (services)
		services = realloc(services, strlen(services) + strlen(nvram_safe_get("filter_services")) + 1);
	else
		services = malloc(strlen(nvram_safe_get("filter_services")) + 1);

	strcat(services, nvram_safe_get("filter_services")); // this is
	// user
	// defined
	// filters
	if (services)
		services = realloc(services, strlen(services) + strlen(nvram_safe_get("filter_services_1")) + 1);
	else
		services = malloc(strlen(nvram_safe_get("filter_services_1")) + 1);
	strcat(services, nvram_safe_get("filter_services_1"));

	return services;
}

void free_filters(filters *filter)
{
	if (!filter)
		return;
	int count = 0;
	while (filter[count].name != NULL) {
		free(filter[count].name);
		filter[count].name = NULL;
		count++;
	}
	free(filter);
}

filters *get_filters_list(void)
{
	char word[1024], *next;
	char protocol[64], ports[64];
	char delim[] = "<&nbsp;>";
	char *services;
	// services = nvram_safe_get("filter_services");
	services = get_filter_services();
	int count = 0;
	split(word, services, next, delim)
	{
		count++;
	}
	filters *s_filters = (filters *)malloc(sizeof(filters) * count + 1);
	count = 0;
	split(word, services, next, delim)
	{
		int len = 0;
		char *name, *prot, *port;
		int from = 0, to = 0;
		s_filters[count].name = NULL;
		if ((name = strstr(word, "$NAME:")) == NULL || (prot = strstr(word, "$PROT:")) == NULL ||
		    (port = strstr(word, "$PORT:")) == NULL)
			continue;

		/*
		 * $NAME 
		 */
		if (sscanf(name, "$NAME:%3d:", &len) != 1) {
			free(services);
			free_filters(s_filters);
			return NULL;
		}

		strncpy(name, name + sizeof("$NAME:nnn:") - 1, len);
		name[len] = '\0';
		s_filters[count].name = strdup(name);

		/*
		 * $PROT 
		 */
		if (sscanf(prot, "$PROT:%3d:", &len) != 1) {
			free(services);
			free_filters(s_filters);
			return NULL;
		}

		strncpy(protocol, prot + sizeof("$PROT:nnn:") - 1, len);
		protocol[len] = '\0';
		if (!strcmp(protocol, "tcp"))
			s_filters[count].proto = 1;
		if (!strcmp(protocol, "udp"))
			s_filters[count].proto = 2;
		if (!strcmp(protocol, "both"))
			s_filters[count].proto = 3;
		if (!strcmp(protocol, "l7"))
			s_filters[count].proto = 4;
		if (!strcmp(protocol, "dpi"))
			s_filters[count].proto = 5;
		if (!strcmp(protocol, "p2p"))
			s_filters[count].proto = 6;
		if (!strcmp(protocol, "risk"))
			s_filters[count].proto = 7;

		/*
		 * $PORT 
		 */
		if (sscanf(port, "$PORT:%3d:", &len) != 1) {
			free(services);
			free_filters(s_filters);
			return NULL;
		}

		strncpy(ports, port + sizeof("$PORT:nnn:") - 1, len);
		ports[len] = '\0';

		if (sscanf(ports, "%d:%d", &from, &to) != 2) {
			free(services);
			free_filters(s_filters);
			return NULL;
		}
		s_filters[count].portfrom = from;
		s_filters[count].portto = to;
		count++;
	}
	s_filters[count].name = NULL;
	free(services);
	return s_filters;
}

/*
 * Example data = $NAME:111$STAT:222 find_pattern(data, strlen(data),
 * "$NAME:", sizeof("$NAME:"), '$', &offset, &len); produces : ret = 1,
 * offset=6, len=3, buf = 111 find_pattern(data, strlen(data), "$IP:",
 * sizeof("$IP:"), '$', &offset, &len); produces : ret = 0 
 */
/*
 * Return 1 for match, 0 for accept, -1 for partial. 
 */
int find_pattern(const char *data, size_t dlen, const char *pattern, size_t plen, char term, unsigned int *numoff,
		 unsigned int *numlen)
{
	size_t i, j, k;

	// DEBUGP("find_pattern `%s': dlen = %u\n", pattern, dlen);
	if (dlen == 0)
		return 0;

	if (dlen <= plen) {
		/*
		 * Short packet: try for partial? 
		 */
		if (strncmp(data, pattern, dlen) == 0)
			return -1;
		else
			return 0;
	}

	for (i = 0; i <= (dlen - plen); i++) {
		if (memcmp(data + i, pattern, plen) != 0)
			continue;

		/*
		 * patten match !! 
		 */
		*numoff = i + plen;
		for (j = *numoff, k = 0; data[j] != term; j++, k++)
			if (j > dlen)
				return -1; /* no terminal char */

		*numlen = k;
		return 1;
	}

	return 0;
}

/*
 * Example data = $NAME:111$STAT:222 find_match_pattern(name, strlen(name),
 * data, "$NAME", ""); produces : ret = 1, name = 111 find_match_pattern(ip, 
 * strlen(ip), data, "$IP", "0.0.0.0"); produces : ret = 0, ip = 0.0.0.0 
 */
int find_match_pattern(char *name, size_t mlen, const char *data, const char *pattern, char *def)
{
	int ret = 0;
	unsigned int offset, len, length;

	ret = find_pattern(data, strlen(data), pattern, strlen(pattern), '$', &offset, &len);
	// printf("ret=[%d] offset=[%d] len=[%d]\n", ret, offset,len);

	if (ret == 1 && len > 0) {
		length = len > mlen ? mlen : len;
		strncpy(name, data + offset, length);
		name[length] = '\0';
	} else
		snprintf(name, mlen, "%s", def); // not found and set to
	// default value

	return ret;
}

int endswith(char *str, char *cmp)
{
	if (!str || !cmp)
		return 0;
	int diff = strlen(str) - strlen(cmp);
	return diff >= 0 && 0 == strcmp(&str[diff], cmp);
}

int searchfor(FILE *fp, char *str, int scansize)
{
	char *buffer = safe_malloc(scansize);
	int len = fread(buffer, scansize, 1, fp);
	int i;

	for (i = 0; i < len - strlen(str); i++) {
		if (memcmp(buffer + i, str, strlen(str)) == 0) {
			fseek(fp, i + strlen(str), SEEK_SET);
			free(buffer);
			return 0;
		}
	}
	free(buffer);
	return -1;
}

static void addactions(char *nv, char *action)
{
	char *actionstack = "";
	char *next;
	char service[80];
	if (action == NULL || !*action)
		return;
	char *services = nvram_safe_get(nv);

	foreach(service, services, next)
	{
		if (!strcmp(service, action)) {
			return;
		}
	}
	if (*services) {
		asprintf(&actionstack, "%s %s", action, services);
		nvram_set(nv, actionstack);
		free(actionstack);
	} else {
		nvram_set(nv, action);
	}
}

void addAction(char *action)
{
	addactions("action_service", action);
}

void registerCustom(char *action)
{
	addactions("custom_configs", action);
}
