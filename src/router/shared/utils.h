#ifndef UTILS_H
#define UTILS_H

#ifdef CDEBUG
#include <shutils.h>
#include <malloc.h>
#include <cy_conf.h>
#endif

/* 2 byte router ID number; Eko 4.jul.06

X X X X X X X X   R R R P N N N N   = 0xXXXX
---------------   ----- - -------
 router num         |   |  gpio num (1111 = f = disable resetbutton)
                    |   |
                    |   |--- resetbutton polarity (0: normal, 1 inversed)
                    |
                    |-------- reserved for future use (maybe USB supp)
*/

// Linksys WRT54G, WRT54GS and WRT54GL all models except WRT54G v1.0, v1.1
#define ROUTER_WRT54G 0x0116

// Linksys WRT54G v1.0 and v1.1 (4702 cpu)
#define ROUTER_WRT54G1X 0x0216

// Linksys WRT55AG v1 (4702 cpu)
#define ROUTER_LINKSYS_WRT55AG 0x0316

// Asus WL-500G-Deluxe (5365 cpu), (fccid: Asus MSQWL500GD)
#define ROUTER_ASUS 0x0406

// Buffalo WBR-G54, WLA-G54 (4702 cpu)
#define ROUTER_BUFFALO_WBR54G 0x0504

// Buffalo WBR2-G54 (fccid: Buffalo FDI-04600142-0), WBR2-G54S and Buffalo WLA2-G54L
#define ROUTER_BUFFALO_WBR2G54S 0x0607
#define ROUTER_BUFFALO_WLA2G54L 0x0607	//should work like a WBR2

// Buffalo WLA2-G54C == WLI3-TX1-G54 (4712 cpu, no switch),(fccid: Buffalo FDI-09101669-0)
#define ROUTER_BUFFALO_WLA2G54C 0x0712  //gpio 2 is AOSS button, let it acts as reset for now.

// Buffalo WHR-G54S (fccid: Buffalo FDI-04600264-0) and WHR-HP-G54 (fccid: Buffalo FDI-09101577-0)
#define ROUTER_BUFFALO_WHRG54S 0x0804
#define ROUTER_BUFFALO_HP_WHRG54S 0x0804	//no differences between these models. so we leave it

// Buffalo WZR-RS-G54 (4704 cpu), WZR-G54, WZR-HP-G54 (4704 cpu, fccid: Buffalo FDI-09101457-0), WHR3-AG54
#define ROUTER_BUFFALO_WZRRSG54 0x0904

// Motorola WR850G v1 (4702 cpu)
#define ROUTER_MOTOROLA_V1 0x0a0f

// Motorola WR850G v2
#define ROUTER_MOTOROLA 0x0b0f

// RT210W generic and branded (fccid: Askey H8N-RT210W), (4702 cpu)
// Siemens se505 v1, Belkin F5D7230-4 v1000
#define ROUTER_RT210W 0x0c0f 

// RT480W generic and branded (fccid: Askey H8N-RT480W), (4712 cpu + ADM6996)
// Siemens se505 v2
#define ROUTER_SIEMENS 0x0d0f

// Microtik RouterBOARD 500
#define ROUTER_BOARD_500 0x0e0f

// NMN A/B/G Router Protoype (266 Mhz Xscale, dual minipci)
#define ROUTER_BOARD_XSCALE 0x0e01

// ??? 
#define ROUTER_BELKIN 0x0f0f

// Belkin F5D7230-4 v1444 (4712 cpu, 2MB flash) (fccid: Belkin K7S-F5D72304)
#define ROUTER_BELKIN_F5D7230 0x100f

// Microsoft MN-700 (4702 cpu), (fccid: Microsoft C3KMN700)
#define ROUTER_MICROSOFT_MN700 0x110f

// Buffalo WLA-G54C (4702 cpu)
#define ROUTER_BUFFALO_WLAG54C 0x120f

// Asus WL-500g Premium (4704 cpu + BCM5325E switch), (fccid MSQWL500GP)
#define ROUTER_ASUS_WL500G_PRE 0x130f

// Linksys WRTSL54GS (4704 cpu + BCM5325E switch), (fccid Q87-WTSLGS, same without USB: Q87-WRTH54GS) 
#define ROUTER_WRTSL54GS 0x1416


#define NVROUTER "DD_BOARD"


extern int getcpurev(void);
extern int check_vlan_support (void);
extern int startswith (char *source, char *cmp);
extern int getRouterBrand (void);
extern int diag_led (int type, int act);
extern int C_led (int i);
extern int get_single_ip (char *ipaddr, int which);
extern char *get_mac_from_ip (char *ip);
extern struct dns_lists *get_dns_list (void);
extern int dns_to_resolv (void);
extern char *get_wan_face (void);
extern int check_wan_link (int num);
extern char *get_complete_lan_ip (char *ip);
extern int get_int_len (int num);
extern int file_to_buf (char *path, char *buf, int len);
extern int buf_to_file (char *path, char *buf);
extern pid_t *find_pid_by_name (char *pidName);
extern int find_pid_by_ps (char *pidName);
extern int *find_all_pid_by_ps (char *pidName);
extern char *find_name_by_proc (int pid);
extern int get_ppp_pid (char *file);
extern long convert_ver (char *ver);
extern int check_flash (void);
extern int check_action (void);
extern int check_now_boot (void);
extern int check_hw_type (void);
extern int is_exist (char *filename);
extern void set_ip_forward (char c);
struct mtu_lists *get_mtu (char *proto);
extern void set_host_domain_name (void);

extern void encode (char *buf, int len);
extern void decode (char *buf, int len);


#ifdef CDEBUG
extern int mcoreleft (void);
extern int coreleft (void);
static void
cdebug (char *function)
{
  FILE *in = fopen ("/tmp/console.log", "a");
  fprintf (in, "free ram in %s = %d (malloc %d)\n", function, coreleft (),
	   mcoreleft ());
  fclose (in);
}

#else
#define cdebug(a)
#endif
extern int first_time (void);

extern int set_register_value (unsigned short port_addr,
			       unsigned short option_content);
extern unsigned long get_register_value (unsigned short id,
					 unsigned short num);
//extern int sys_netdev_ioctl(int family, int socket, char *if_name, int cmd, struct ifreq *ifr);

int ct_openlog (const char *ident, int option, int facility, char *log_name);
void ct_syslog (int level, int enable, const char *fmt, ...);
void ct_logger (int level, const char *fmt, ...);
struct wl_assoc_mac *get_wl_assoc_mac (int *c);

extern struct detect_wans *detect_protocol (char *wan_face, char *lan_face,
					    char *type);

enum
{ WL = 0,
  DIAG = 1,
  SES_LED1 = 2,
  SES_LED2 = 3,
  SES_BUTTON = 4,
  DMZ = 7
};

enum
{ START_LED, STOP_LED, MALFUNCTION_LED };

typedef enum
{ ACT_IDLE,
  ACT_TFTP_UPGRADE,
  ACT_WEB_UPGRADE,
#ifdef HAVE_HTTPS
  ACT_WEBS_UPGRADE,
#endif
  ACT_SW_RESTORE,
  ACT_HW_RESTORE,
  ACT_ERASE_NVRAM,
  ACT_NVRAM_COMMIT
} ACTION;

enum
{ UNKNOWN_BOOT = -1, PMON_BOOT, CFE_BOOT };

enum
{ BCM4702_CHIP, BCM4712_CHIP, BCM5325E_CHIP, BCM4704_BCM5325F_CHIP,
    BCM5352E_CHIP, NO_DEFINE_CHIP };

enum
{ FIRST, SECOND };

enum
{ SYSLOG_LOG = 1, SYSLOG_DEBUG, CONSOLE_ONLY, LOG_CONSOLE, DEBUG_CONSOLE };

#define ACTION(cmd)	buf_to_file(ACTION_FILE, cmd)

struct dns_lists
{
  int num_servers;
  char dns_server[4][16];
};

#define NOT_USING	0
#define USING		1

struct wl_assoc_mac
{
  char mac[18];
};

struct mtu_lists
{
  char *proto;			/* protocol */
  char *min;			/* min mtu */
  char *max;			/* max mtu */
};

struct detect_wans
{
  int proto;
  int count;
  char *name;
  char desc[1024];
};

#define	PROTO_DHCP	0
#define	PROTO_STATIC	1
#define	PROTO_PPPOE	2
#define	PROTO_PPTP	3
#define	PROTO_L2TP	4
#define	PROTO_HB	5
#define	PROTO_ERROR	-1

#define PPP_PSEUDO_IP	"10.64.64.64"
#define PPP_PSEUDO_NM	"255.255.255.255"
#define PPP_PSEUDO_GW	"10.112.112.112"

#define PING_TMP	"/tmp/ping.log"
#define TRACEROUTE_TMP	"/tmp/traceroute.log"
#define MAX_BUF_LEN	254

#define RESOLV_FILE	"/tmp/resolv.conf"
#define RESOLV_FORW	"/tmp/resolv.dnsmasq"
#define HOSTS_FILE	"/tmp/hosts"

#define LOG_FILE	"/var/log/mess"

#define ACTION_FILE	"/tmp/action"


#define split(word, wordlist, next, delim) \
	for (next = wordlist, \
	     strncpy(word, next, sizeof(word)), \
	     word[(next=strstr(next, delim)) ? strstr(word, delim) - word : sizeof(word) - 1] = '\0', \
	     next = next ? next + sizeof(delim) - 1 : NULL ; \
	     strlen(word); \
	     next = next ? : "", \
	     strncpy(word, next, sizeof(word)), \
	     word[(next=strstr(next, delim)) ? strstr(word, delim) - word : sizeof(word) - 1] = '\0', \
	     next = next ? next + sizeof(delim) - 1 : NULL)

#define STRUCT_LEN(name)    sizeof(name)/sizeof(name[0])

#define printHEX(str,len) { \
	int i; \
	for (i=0 ; i<len ; i++) { \
		printf("%02X ", (unsigned char)*(str+i)); \
		if(((i+1)%16) == 0) printf("- "); \
		if(((i+1)%32) == 0) printf("\n"); \
	} \
	printf("\n\n"); \
}


#define printASC(str,len) { \
	int i; \
	for (i=0 ; i<len ; i++) { \
		printf("%c", (unsigned char)*(str+i)); \
		if(((i+1)%16) == 0) printf("- "); \
		if(((i+1)%32) == 0) printf("\n"); \
	} \
	printf("\n\n"); \
}


int ishexit (char c);
int sv_valid_hwaddr (char *value);
int sv_valid_ipaddr (char *value);
int sv_valid_range (char *value, int low, int high);
int sv_valid_statics (char *value);
void get_network (char *ipaddr, char *netmask);
void get_broadcast (char *ipaddr, char *netmask);
int route_manip (int cmd, char *name, int metric, char *dst, char *gateway,
		 char *genmask);
int route_add (char *name, int metric, char *dst, char *gateway,
	       char *genmask);
int route_del (char *name, int metric, char *dst, char *gateway,
	       char *genmask);

#define MAX_WDS_DEVS 10
#endif
