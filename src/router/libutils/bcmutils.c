// #define CDEBUG 

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
#include <fcntl.h>
#include <netinet/in.h>
#include <stdarg.h>
#include <sys/ioctl.h>
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
#include <cymac.h>
#include <broadcom.h>
#include <md5.h>

#define SIOCGMIIREG	0x8948	/* Read MII PHY register.  */
#define SIOCSMIIREG	0x8949	/* Write MII PHY register.  */

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
		if (cnt == 3) {
			getc(fp);
			char cpurev[32];
			int i = 0;

			for (i = 0; i < 32; i++) {
				cpurev[i] = getc(fp);
				if (cpurev[i] == '\n')
					break;
			}
			cpurev[i] = 0;
			fclose(fp);
			if (strstr(cpurev, "BCM4710 V0.0"))	// BCM4702, BCM4710
				// (old 125 MHz)
				return 0;
			if (strstr(cpurev, "BCM3302 V0.6"))	// BCM4704
				return 6;
			if (strstr(cpurev, "BCM3302 V0.7"))	// BCM4712, BCM5365
				return 7;
			if (strstr(cpurev, "BCM3302 V0.8"))	// BCM5350, BCM5352
				return 8;
			if (strstr(cpurev, "BCM3302 V2.9"))	// BCM5354
				return 29;
			if (strstr(cpurev, "BCM3302 V1.10"))	// BCM4785 (BCM3302 V1.10)
				return 110;
			if (strstr(cpurev, "MIPS 74K V4.9"))	// BCM4716B0 (Broadcom BCMB83A chip rev 0)
				return 50;	// BCM5356B0 (Broadcom BCM5356 chip rev 1) 
			if (strstr(cpurev, "MIPS 74K V4.0"))	// BCM4716B0 (Broadcom BCMB83A chip rev 0)
				return 51;	// BCM5356B0 (Broadcom BCM5356 chip rev 1) 
			if (strstr(cpurev, "MIPS 74Kc V4.9"))	// BCM4716B0 (Broadcom BCMB83A chip rev 0) //newer kernels 
				return 50;	// BCM5356B0 (Broadcom BCM5356 chip rev 1) 
			if (strstr(cpurev, "MIPS 74Kc V4.0"))	// BCM4716B0 (Broadcom BCMB83A chip rev 0) //newer kernels 
				return 51;	// BCM5356B0 (Broadcom BCM5356 chip rev 1) 
			return -1;
		}
	}
	fclose(fp);
	return -1;
}

int cpu_plltype(void)
{
#ifdef HAVE_NORTHSTAR
	return 9;
#endif
#if defined(HAVE_BUFFALO) || defined(BUFFALO_JP)
	if (nvram_match("DD_BOARD", "Buffalo WHR-G54S") ||	//
	    nvram_match("DD_BOARD", "Buffalo WHR-HP-G54") ||	//
	    nvram_match("DD_BOARD", "Buffalo AS-A100") ||	//
	    nvram_match("DD_BOARD", "Buffalo WHR-HP-G54DD"))	//
		return 0;
#endif

	int cpurev = getcpurev();
	int cputype = check_hw_type();

	if (cpurev == 0)	// BCM4702, BCM4710 (old 125 MHz)
		return 0;
	if (cpurev == 6)	// BCM4704
		return 0;
	if (cpurev == 7 && cputype == BCM5365_CHIP)	// BCM5365 only
		// supports fixed 200 
		// MHz
		return 0;
	if (cpurev == 7 && cputype != BCM5365_CHIP)	// BCM4712
		return 4;
	if (cpurev == 8 && cputype == BCM5350_CHIP)	// BCM5350
		return 3;
	if (cpurev == 8 && cputype != BCM5350_CHIP)	// BCM5352
		return 7;
	if (cpurev == 29)	// BCM5354, only supports fixed 240 MHz
		return 0;
	if (cpurev == 110)	// BCM4705
		return 2;
	if (cpurev == 50)	// BCM4706
		return 8;
	if (cpurev == 51)	// BCM4706
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
		strncpy(occurrence, &occurrence[namelen + 1 /* space */ ],
			strlen(&occurrence[namelen + 1 /* space */ ]) +
			1 /* terminate */ );
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

	if (listsize <= listlen + namelen + 1 /* space */  + 1 /* NULL */ )
		return EMSGSIZE;

	/* add a space if the list isn't empty and it doesn't already have space */
	if (list[0] != 0 && list[listlen - 1] != ' ') {
		list[listlen++] = 0x20;
	}

	strncpy(&list[listlen], name, namelen + 1 /* terminate */ );

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

char *substring(int start, int stop, const char *src, char *dst)
{
	sprintf(dst, "%.*s", stop - start, src + start);

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

	if (year > 1900)	// 1900 was not a leap year
		year -= 1900;
	ix = ((year - 21) % 28) + vx + (month > 2);	// take care of
	// February
	tx = (ix + (ix / 4)) % 7 + day;	// take care of leap year

	return (tx % 7);

}

#if defined(HAVE_RT2880) || defined(HAVE_RT61)

int has_mimo(char *prefix)
{
	return 1;
}

int has_ac(char *prefix)
{
	return 0;
}
#else

int has_mimo(char *prefix)
{
	char mimo[32];
	sprintf(mimo, "%s_phytypes", prefix);
	char *phy = nvram_safe_get(mimo);
	if (contains(phy, 'n') || contains(phy, 'h') || contains(phy, 's') || contains(phy, 'v'))
		return 1;
	else
		return 0;
}

int has_ac(char *prefix)
{
#ifdef HAVE_ATH10K
	return is_ath10k(prefix);
#elif HAVE_ATH9K
	return 0;
#else
	char mimo[32];
	sprintf(mimo, "%s_phytypes", prefix);
	char *phy = nvram_safe_get(mimo);
	if (contains(phy, 'v'))
		return 1;
	else
		return 0;
#endif
}
#endif


int has_qtn(char *prefix)
{
#ifdef HAVE_QTN
	if (!strcmp(prefix,"wl1"))
	    return 1;
#endif
    return 0;
}
char *get_mac_from_ip(char *ip)
{
	FILE *fp;
	char line[100];
	char ipa[50];		// ip address
	char hwa[50];		// HW address / MAC
	char mask[50];		// ntemask 
	char dev[50];		// interface
	int type;		// HW type
	int flags;		// flags
	static char mac[20];

	if ((fp = fopen("/proc/net/arp", "r")) == NULL)
		return NULL;

	// Bypass header -- read until newline 
	if (fgets(line, sizeof(line), fp) != (char *)NULL) {
		// Read the ARP cache entries.
		// IP address HW type Flags HW address Mask Device
		// 192.168.1.1 0x1 0x2 00:90:4C:21:00:2A * eth0
		for (; fgets(line, sizeof(line), fp);) {
			if (sscanf(line, "%s 0x%x 0x%x %100s %100s %100s\n", ipa, &type, &flags, hwa, mask, dev) != 6)
				continue;
			// cprintf("ip1=[%s] ip2=[%s] mac=[%s] (flags & ATF_COM)=%d\n",
			// ip, ipa, hwa, (flags & ATF_COM));
			if (strcmp(ip, ipa))
				continue;
			// if (!(flags & ATF_COM)) { //ATF_COM = 0x02 completed entry (ha 
			// valid)
			strcpy(mac, hwa);
			fclose(fp);
			return mac;
			// }
		}
	}

	fclose(fp);
	return "";
}

struct dns_lists *get_dns_list(void)
{
	char list[254];
	char *next, word[254];
	struct dns_lists *dns_list = NULL;
	int i, match = 0, altdns_index = 1;

	dns_list = (struct dns_lists *)malloc(sizeof(struct dns_lists));
	memset(dns_list, 0, sizeof(struct dns_lists));

	dns_list->num_servers = 0;

	// nvram_safe_get("wan_dns") ==> Set by user
	// nvram_safe_get("wan_get_dns") ==> Get from DHCP, PPPoE or PPTP
	// The nvram_safe_get("wan_dns") priority is higher than
	// nvram_safe_get("wan_get_dns")
	snprintf(list, sizeof(list), "%s %s %s", nvram_safe_get("sv_localdns"), nvram_safe_get("wan_dns"), nvram_safe_get("wan_get_dns"));
	foreach(word, list, next) {
		if (strcmp(word, "0.0.0.0") && strcmp(word, "")) {
			match = 0;
			for (i = 0; i < dns_list->num_servers; i++) {	// Skip same DNS
				if (!strcmp(dns_list->dns_server[i], word))
					match = 1;
			}
			if (!match) {
				snprintf(dns_list->dns_server[dns_list->num_servers], sizeof(dns_list->dns_server[dns_list->num_servers]), "%s", word);
				dns_list->num_servers++;
			}
		}
		if (dns_list->num_servers == 3)
			break;	// We only need 3 DNS entries
	}

	/*
	 * if < 3 DNS servers found, try to insert alternates 
	 */
	while (dns_list->num_servers < 3 && altdns_index <= 3) {
		char altdnsvar[32] = {
			0
		};

		snprintf(altdnsvar, 31, "altdns%d", altdns_index);

		if (strlen(nvram_safe_get(altdnsvar)) > 0) {
			snprintf(dns_list->dns_server[dns_list->num_servers], sizeof(dns_list->dns_server[dns_list->num_servers]), "%s", nvram_safe_get(altdnsvar));
			dns_list->num_servers++;
		}
		altdns_index++;
	}
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
	if (nvram_invmatch("wan_get_domain", "")) {
		fprintf(fp_w, "search %s\n", nvram_safe_get("wan_get_domain"));
	} else if (nvram_invmatch("wan_domain", "")) {
		fprintf(fp_w, "search %s\n", nvram_safe_get("wan_domain"));
	}
	if (nvram_invmatch("lan_domain", "")) {
		fprintf(fp_w, "search %s\n", nvram_safe_get("lan_domain"));
	}
	if (nvram_match("dnsmasq_enable", "1")) {
		fprintf(fp_w, "nameserver %s\n", nvram_get("lan_ipaddr"));
		fclose(fp_w);
		if (!(fp_w = fopen(RESOLV_FORW, "w"))) {
			perror(RESOLV_FORW);
			return errno;
		}
	}

	dns_list = get_dns_list();

	for (i = 0; i < dns_list->num_servers; i++)
		fprintf(fp_w, "nameserver %s\n", dns_list->dns_server[i]);

	/*
	 * Put a pseudo DNS IP to trigger Connect On Demand 
	 */
	if (dns_list->num_servers == 0 && (nvram_match("wan_proto", "pppoe")
					   || nvram_match("wan_proto", "pptp")
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
	    ) && nvram_match("ppp_demand", "1"))
		fprintf(fp_w, "nameserver 1.1.1.1\n");

	fclose(fp_w);
	if (dns_list)
		free(dns_list);

	eval("touch", "/tmp/hosts");

	return 1;
}

/*
 * Example: lan_ipaddr = 192.168.1.1 get_dns_ip("lan_ipaddr", 1); produces
 * "168" 
 */
int get_single_ip(char *ipaddr, int which)
{
	static int ip[4] = {
		0, 0, 0, 0
	};
	int ret;

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

	dir = opendir("/proc");

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
			pidList[i++] = strtol(next->d_name, NULL, 0);
		}
	}

	if (pidList)
		pidList[i] = 0;
	else {
		pidList = realloc(pidList, sizeof(pid_t));
		pidList[0] = -1;
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

/*
 * To avoid user to download old image that is not support intel flash to new 
 * hardware with intel flash. 
 */
int check_flash(void)
{
	// The V2 image can support intel flash completely, so we don't want to
	// check.
	if (check_hw_type() == BCM4712_CHIP)
		return FALSE;

	// The V1.X some images cann't support intel flash, so we want to avoid
	// user to downgrade.
	if (nvram_match("skip_amd_check", "1")) {
		if (strstr(nvram_safe_get("flash_type"), "Intel")
		    && nvram_invmatch("skip_intel_check", "1"))
			return TRUE;
		else
			return FALSE;
	} else			// Cann't downgrade to old firmware version,
		// no matter AMD or Intel flash
		return TRUE;
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
	else if (btype == 0x456 && getRouterBrand() == ROUTER_BELKIN_F5D7231)	// stupid 
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
	else if (btype == 0x478 && strstr(vlan0, "5*"))	/* WRT300NV1.1 */
		return BCM4705L_BCM5325E_EWC_CHIP;
	else if ((btype == 0x478 && (strstr(vlan0, "8*") || strstr(vlan1, "8*"))) || nvram_match("boot_hw_model", "WRT350N") || nvram_match("boot_hw_model", "WRT610N"))	/* WRT350N 
																						 */
		return BCM4705_BCM5397_EWC_CHIP;
	else if (btype == 0x489 || nvram_match("boot_hw_model", "WRT310N"))	/* WRT310N, 
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
	else if ((btype == 0x0101 || btype == 0x0446) && (boardflags & BFL_ENETADM))	// 0x446 
		// is 
		// wap54g 
		// v2
		return BCM4712_CHIP;
	else if (btype == 0x0472 && !(boardflags & BFL_ENETADM))
		return BCM4704_BCM5325F_EWC_CHIP;
	else
		return NO_DEFINE_CHIP;
}

int ct_openlog(const char *ident, int option, int facility, char *log_name)
{
	int level = atoi(nvram_safe_get(log_name));

	switch (level) {
	case CONSOLE_ONLY:
		break;
	}
	return level;
}

void ct_syslog(int level, int enable, const char *fmt, ...)
{
	char buf[1000];
	va_list args;

	va_start(args, fmt);
	vsnprintf(buf, sizeof(buf), fmt, args);
	va_end(args);

	switch (enable) {
	case CONSOLE_ONLY:
		cprintf("[%d] %s\n", getpid(), buf);	// print to console
		break;
	}
}

void ct_logger(int level, const char *fmt, ...)
{
}

static char *device_name[] = {
	"eth0", "qos0"
};

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
	dest[size] = 0;		/* Make sure the string is null terminated */
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
		if ((s = sockets_open(family, family == AF_PACKET ? SOCK_PACKET : SOCK_DGRAM, family == AF_PACKET ? htons(ETH_P_ALL) : 0)) < 0) {
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

struct wl_assoc_mac *get_wl_assoc_mac(int instance, int *c)
{
	FILE *fp;
	struct wl_assoc_mac *wlmac = NULL;
	int count;
	char line[80];
	char list[2][20];
	char checkif[12];
	char assoccmd[32];

	wlmac = NULL;
	count = *c = 0;

	int ifcnt = 4;
	int i;
	int gotit = 0;

	// fprintf(stderr,"assoclist\n");

	for (i = 0; i < ifcnt; i++) {
		if (i == 0)
			strcpy(checkif, get_wl_instance_name(instance));
		else
			sprintf(checkif, "wl%d.%d", instance, i);
		if (!ifexists(checkif))
			break;

		sprintf(assoccmd, "wl -i %s assoclist", checkif);

		if ((fp = popen(assoccmd, "r"))) {
			gotit = 1;
			while (fgets(line, sizeof(line), fp) != NULL) {
				strcpy(list[0], "");
				strcpy(list[1], "");

				if (sscanf(line, "%s %s", list[0], list[1]) != 2)	// assoclist 
					// 00:11:22:33:44:55
					continue;
				if (strcmp(list[0], "assoclist"))
					continue;

				wlmac = realloc(wlmac, sizeof(struct wl_assoc_mac) * (count + 1));

				memset(&wlmac[count], 0, sizeof(struct wl_assoc_mac));
				strncpy(wlmac[count].mac, list[1], sizeof(wlmac[0].mac));
				count++;
			}

			pclose(fp);
		}
	}

	if (gotit) {
		// cprintf("Count of wl assoclist mac is %d\n", count);
		*c = count;
		return wlmac;
	} else
		return NULL;
}

struct mtu_lists mtu_list[] = {
#ifdef BUFFALO_JP
	{
	 "pppoe", "576", "1454"},
#else
	{
	 "pppoe", "576", "1492"},
#endif
	{
	 "pppoe_dual", "576", "1492"},
	{
	 "pppoa", "576", "10000"},
	{
	 "pptp", "576", "1460"},
	{
	 "l2tp", "576", "1460"},
	{
	 "dhcp", "576", "10000"},
	{
	 "iphone", "576", "1500"},
	{
	 "static", "576", "10000"},
	{
	 "heartbeat", "576", "1500"},
	{
	 "3g", "576", "1500"},
	{
	 "default", "576", "10000"},	// The value must be at last
};

struct mtu_lists *get_mtu(char *proto)
{
	struct mtu_lists *v = NULL;

	for (v = mtu_list; v < &mtu_list[STRUCT_LEN(mtu_list)]; v++) {
		if (!strcmp(proto, v->proto))
			return v;
	}
	return v;		// Use default settings
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
	if (strlen(wan_hostname) > 0)
		sethostname(wan_hostname, strlen(wan_hostname));

	/*
	 * Allow you to use getdomainname to get Domain Name 
	 */
	if (strlen(wan_domain) > 0 && strlen(wan_domain) <= 64)	// no 
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

	memset(&ifr, 0, sizeof(struct ifreq));

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
		if (wds_list == (void *)0 || strlen(wds_list) <= 0)
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

		if (!sv_valid_hwaddr(hwaddr) || !sv_valid_ipaddr(ip)
		    || !sv_valid_ipaddr(netmask)) {
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
	char ip[16] = {
		0
	}, mac[18] = {
	0}, hostname[255] = {
	0}, *p = value;

	if (NULL == value)
		return FALSE;

	do {
		while (isspace(*p++) && p - value < strlen(value)) ;

		if (p - value >= strlen(value))
			return FALSE;

		if (sscanf(p, "%15s%17s%254s", ip, mac, hostname) < 3)
			return FALSE;

		if (!sv_valid_ipaddr(ip) || !sv_valid_hwaddr(mac)
		    || strlen(hostname) <= 0)
			return FALSE;

	}
	while ((p = strpbrk(p, "\n\r")) && p - value < strlen(value));

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

int get_net(char *netmask)
{
	if (!netmask)
		return -1;
	unsigned int mask[4];
	sscanf(netmask, "%d.%d.%d.%d", &mask[0], &mask[1], &mask[2], &mask[3]);
	unsigned int value = 0;
	value |= mask[0] << 24;
	value |= mask[1] << 16;
	value |= mask[2] << 8;
	value |= mask[3];
	unsigned int base = 0;
	unsigned int i;
	for (i = 0; i < 32; i++) {
		if ((value & (1 << i)) == (1 << i))
			base++;
	}
	return base;
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
	memset(&rt, 0, sizeof(rt));
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

      err:close(s);
	perror(name);
	return errno;
}

int route_add(char *name, int metric, char *dst, char *gateway, char *genmask)
{
	if (nvram_match("console_debug", "1"))
		fprintf(stderr, "route_add: if:%s dst:%s gw: %s mask: %s \n", name, dst, gateway, genmask);
	return route_manip(SIOCADDRT, name, metric, dst, gateway, genmask);
}

int route_del(char *name, int metric, char *dst, char *gateway, char *genmask)
{
	if (nvram_match("console_debug", "1"))
		fprintf(stderr, "route_del: if:%s dst:%s gw: %s mask: %s \n", name, dst, gateway, genmask);
	return route_manip(SIOCDELRT, name, metric, dst, gateway, genmask);
}

// #endif

void getIfLists(char *eths, int size)
{
	char eths2[256];

	memset(eths, 0, size);
	memset(eths2, 0, 256);
#ifdef HAVE_XSCALE
	getIfList(eths, "ixp");
	getIfList(eths2, "eth");
	sprintf(eths, "%s %s", eths, eths2);
#else
	getIfList(eths, "eth");
#endif
	memset(eths2, 0, 256);
	getIfList(eths2, "vlan");
	sprintf(eths, "%s %s", eths, eths2);
#ifdef HAVE_MADWIFI
	memset(eths2, 0, 256);
	getIfList(eths2, "ath");
	sprintf(eths, "%s %s", eths, eths2);
#elif defined(HAVE_RT2880) || defined(HAVE_RT61)
	memset(eths2, 0, 256);
	getIfList(eths2, "ra");
	sprintf(eths, "%s %s", eths, eths2);

	memset(eths2, 0, 256);
	getIfList(eths2, "apcli");
	sprintf(eths, "%s %s", eths, eths2);

	memset(eths2, 0, 256);
	getIfList(eths2, "wds");
	sprintf(eths, "%s %s", eths, eths2);
#else
	memset(eths2, 0, 256);
	getIfList(eths2, "wl");
	sprintf(eths, "%s %s", eths, eths2);
#endif
	memset(eths2, 0, 256);
	getIfList(eths2, "br");
	sprintf(eths, "%s %s", eths, eths2);

	memset(eths2, 0, 256);
	getIfList(eths2, "oet");
	sprintf(eths, "%s %s", eths, eths2);
#ifdef HAVE_WAVESAT
	memset(eths2, 0, 256);
	getIfList(eths2, "ofdm");
	sprintf(eths, "%s %s", eths, eths2);
#endif

}

int contains(const char *string, char value)
{
	if (string == NULL)
		return 0;
	int len = strlen(string);
	int i;

	for (i = 0; i < len; i++) {
		if (string[i] == value)
			return 1;
	}
	return 0;
}

int haswifi(void)
{
	int count = 0;
#ifdef HAVE_NOWIFI
	return 0;
#elif defined(HAVE_ATH9K) || defined(HAVE_MADWIFI) || defined(HAVE_MADWIFI_MIMO)
	count += getdevicecount();
	return (count);
#else
	return 1;
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
	int ip[4] = {
		0, 0, 0, 0
	};

	sscanf(nmask, "%d.%d.%d.%d", &ip[0], &ip[1], &ip[2], &ip[3]);

	int n = 8;

	for (--n; n >= 0; --n)	// test all 4 bytes in one pass
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
	char name[80], *next;
	int ifcount = 0;

	if (nvram_match("wan_proto", "disabled"))
		return 0;
	if (nvram_match("block_multicast", "0")) {
		ifcount++;
	}
	char *lan_ifnames = nvram_safe_get("lan_ifnames");

	foreach(name, lan_ifnames, next) {
		if (nvram_nmatch("1", "%s_multicast", name)
		    && nvram_nmatch("0", "%s_bridged", name)) {
			ifcount++;
		}
	}
	return ifcount;
}

static int sockaddr_to_dotted(struct sockaddr *saddr, char *buf)
{
	buf[0] = '\0';
	if (saddr->sa_family == AF_INET) {
		inet_ntop(AF_INET, &((struct sockaddr_in *)saddr)->sin_addr, buf, 128);
		return 0;
	}
	if (saddr->sa_family == AF_INET6) {
		inet_ntop(AF_INET6, &((struct sockaddr_in6 *)saddr)->sin6_addr, buf, 128);
		return 0;
	}
	return -1;
}

static int sockaddr_to_dotted_n(char *sin_addr, char *buf)
{
	inet_ntop(AF_INET, sin_addr, buf, 128);
	return 0;
}

#define DIE_ON_ERROR AI_CANONNAME

void getIPFromName(char *name, char *ip)
{
	int count = 5;
	while (count--) {
		struct addrinfo *result = NULL;
		int rc;
		struct addrinfo hint;
		struct hostent *hp = gethostbyname(name);
		if (hp != NULL) {
			sockaddr_to_dotted_n(hp->h_addr_list[0], ip);
			if (strcmp(ip, "0.0.0.0"))
				break;
		}
		res_init();
		memset(&hint, 0, sizeof(hint));
		hint.ai_family = AF_INET;
		hint.ai_socktype = SOCK_STREAM;
		hint.ai_flags = DIE_ON_ERROR;
		rc = getaddrinfo(name, NULL, &hint, &result);
		if (!result)	// give it a second try
			rc = getaddrinfo(name, NULL, &hint, &result);

		if (result) {
			sockaddr_to_dotted(result->ai_addr, ip);
			freeaddrinfo(result);
		} else {
			struct hostent *hp = gethostbyname(name);
			if (hp != NULL) {
				sockaddr_to_dotted_n(hp->h_addr_list[0], ip);
			} else
				sprintf(ip, "0.0.0.0");
		}
		if (strcmp(ip, "0.0.0.0"))
			break;
		sleep(1);
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
			return 0;	// null
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

char *get_filter_services(void)
{

	l7filters *filters = filters_list;
	char temp[128] = "";
	char *proto[] = { "l7", "p2p", "dpi" };
	char *services = NULL;

	while (filters->name)	// add l7 and p2p filters
	{
		sprintf(temp, "$NAME:%03d:%s$PROT:%03d:%s$PORT:003:0:0<&nbsp;>", strlen(filters->name), filters->name, filters->protocol == 0 ? 2 : 3, proto[filters->protocol]);
		if (!services) {
			services = malloc(strlen(temp) + 1);
			services[0] = 0;
		} else
			services = realloc(services, strlen(services) + strlen(temp) + 1);
		strcat(services, temp);
		filters++;
	}
	services = realloc(services, strlen(services) + strlen(nvram_safe_get("filter_services")) + 1);
	strcat(services, nvram_safe_get("filter_services"));	// this is
	// user
	// defined
	// filters
	services = realloc(services, strlen(services) + strlen(nvram_safe_get("filter_services_1")) + 1);
	strcat(services, nvram_safe_get("filter_services_1"));

	return services;
}

int endswith(char *str, char *cmp)
{
	int cmp_len, str_len, i;

	if (cmp == NULL)
		return 0;
	if (str == NULL)
		return 0;
	cmp_len = strlen(cmp);
	str_len = strlen(str);
	if (cmp_len > str_len)
		return (0);
	for (i = 0; i < cmp_len; i++) {
		if (str[(str_len - 1) - i] != cmp[(cmp_len - 1) - i])
			return (0);
	}
	return (1);
}

int searchfor(FILE * fp, char *str, int scansize)
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

void addAction(char *action)
{
	char *actionstack = "";
	char *next;
	char service[80];
	if (action == NULL || strlen(action) == 0)
		return;
	char *services = nvram_safe_get("action_service");

	foreach(service, services, next) {
		if (!strcmp(service, action)) {
			return;
		}
	}
	if (strlen(services) > 0) {
		actionstack = safe_malloc(strlen(services) + strlen(action) + 2);
		memset(actionstack, 0, strlen(services) + strlen(action) + 2);
		strcpy(actionstack, action);
		strcat(actionstack, " ");
		strcat(actionstack, nvram_safe_get("action_service"));
		nvram_set("action_service", actionstack);
		free(actionstack);
	} else {
		nvram_set("action_service", action);
	}
	nvram_commit();

}

#ifdef NVRAM_SPACE_256
#define NVRAMSPACE NVRAM_SPACE_256
#else
#define NVRAMSPACE NVRAM_SPACE
#endif

int nvram_used(int *space)
{
	char *name, buf[NVRAMSPACE];

	*space = NVRAMSPACE;

	nvram_getall(buf, sizeof(buf));

	name = buf;

	while (*name) {
		name += strlen(name) + 1;
	}

	return (sizeof(struct nvram_header) + (long)name - (long)buf);

}
