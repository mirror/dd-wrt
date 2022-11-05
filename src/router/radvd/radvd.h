/*
 *
 *   Authors:
 *    Pedro Roque		<roque@di.fc.ul.pt>
 *    Lars Fenneberg		<lf@elemental.net>
 *
 *   This software is Copyright 1996,1997 by the above mentioned author(s),
 *   All Rights Reserved.
 *
 *   The license which is distributed with this software in the file COPYRIGHT
 *   applies to this software. If your distribution is missing this file, you
 *   may request it from <reubenhwk@gmail.com>.
 *
 */

#pragma once

#include "config.h"
#include "defaults.h"
#include "includes.h"
#include "log.h"

#define CONTACT_EMAIL "Reuben Hawkins <reubenhwk@gmail.com>"

extern int sock;

extern int disableigmp6check;

#define min(a, b) (((a) < (b)) ? (a) : (b))

struct AdvPrefix;
struct Clients;

#define HWADDR_MAX 16
#define USER_HZ 100

struct safe_buffer {
	int should_free;
	size_t allocated;
	size_t used;
	unsigned char *buffer;
};

#define SAFE_BUFFER_INIT                                                                                                         \
	(struct safe_buffer) { .should_free = 0, .allocated = 0, .used = 0, .buffer = 0 }

struct safe_buffer_list {
	struct safe_buffer *sb;
	struct safe_buffer_list *next;
};

struct Interface {
	struct Interface *next;

	int IgnoreIfMissing;
	int AdvSendAdvert;
	double MaxRtrAdvInterval;
	double MinRtrAdvInterval;
	double MinDelayBetweenRAs;
	int AdvSourceLLAddress;
	int UnicastOnly;
	int AdvRASolicitedUnicast;
	struct Clients *ClientList;

	struct state_info {
		int ready;   /* Info whether this interface has been initialized successfully */
		int changed; /* Info whether this interface's settings have changed */
		int cease_adv;
		uint32_t racount;
	} state_info;

	struct properties {
		char name[IFNAMSIZ]; /* interface name */
		unsigned int if_index;
		struct in6_addr if_addr;   /* the first link local addr */
		struct in6_addr *if_addrs; /* all the addrs */
		int addrs_count;
		struct in6_addr *if_addr_rasrc; /* selected AdvRASrcAddress or NULL */
		uint32_t max_ra_option_size;
	} props;

	struct ra_header_info {
		int AdvManagedFlag;
		int AdvOtherConfigFlag;
		uint8_t AdvCurHopLimit;
		int AdvHomeAgentFlag;
		int32_t AdvDefaultLifetime; /* XXX: really uint16_t but we need to use -1 */
		int AdvDefaultPreference;
		uint32_t AdvReachableTime;
		uint32_t AdvRetransTimer;
	} ra_header_info;

	struct times {
		struct timespec last_multicast;
		struct timespec next_multicast;
		struct timespec last_ra_time;
	} times;

	struct AdvPrefix *AdvPrefixList;
	struct AdvRoute *AdvRouteList;
	struct AdvRDNSS *AdvRDNSSList;
	struct AdvDNSSL *AdvDNSSLList;

	uint32_t AdvLinkMTU; /* XXX: sllao also has an if_maxmtu value...Why? */
	uint32_t AdvRAMTU;   /* MTU used for RA */

	struct sllao {
		uint8_t if_hwaddr[HWADDR_MAX];
		int if_hwaddr_len;
		int if_prefix_len;
		int if_maxmtu;
	} sllao;

	struct mipv6 {
		/* Mobile IPv6 extensions */
		int AdvIntervalOpt;
		int AdvHomeAgentInfo;

		uint16_t HomeAgentPreference;
		int32_t HomeAgentLifetime; /* XXX: really uint16_t but we need to use -1 */

		/* NEMO extensions */
		int AdvMobRtrSupportFlag;
	} mipv6;

	struct AdvLowpanCo *AdvLowpanCoList;
	struct AdvAbro *AdvAbroList;

	struct AdvRASrcAddress *AdvRASrcAddressList;

	int lineno; /* On what line in the config file was this iface defined? */
};

struct Clients {
	struct in6_addr Address;
	struct Clients *next;
};

struct AdvPrefix {
	struct in6_addr Prefix;
	uint8_t PrefixLen;

	int AdvOnLinkFlag;
	int AdvAutonomousFlag;
	uint32_t AdvValidLifetime;
	uint32_t AdvPreferredLifetime;
	int DeprecatePrefixFlag;
	int DecrementLifetimesFlag;

	uint32_t curr_validlft;
	uint32_t curr_preferredlft;

	/* Mobile IPv6 extensions */
	int AdvRouterAddr;

	/* 6to4 etc. extensions */
	char if6to4[IFNAMSIZ];

	/* Select prefixes from this interface. */
	char if6[IFNAMSIZ];

	struct AdvPrefix *next;
};

/* More-Specific Routes extensions */

struct AdvRoute {
	struct in6_addr Prefix;
	uint8_t PrefixLen;

	int AdvRoutePreference;
	uint32_t AdvRouteLifetime;
	int RemoveRouteFlag;

	struct AdvRoute *next;
};

/* Options for DNS configuration */

struct AdvRDNSS {
	int AdvRDNSSNumber;
	uint32_t AdvRDNSSLifetime;
	int FlushRDNSSFlag;
	struct in6_addr AdvRDNSSAddr1;
	struct in6_addr AdvRDNSSAddr2;
	struct in6_addr AdvRDNSSAddr3;

	struct AdvRDNSS *next;
};

struct AdvDNSSL {
	uint32_t AdvDNSSLLifetime;

	int AdvDNSSLNumber;
	int FlushDNSSLFlag;
	char **AdvDNSSLSuffixes;

	struct AdvDNSSL *next;
};

/* Options for 6lopan configuration */

struct AdvLowpanCo {
	uint8_t ContextLength;
	uint8_t ContextCompressionFlag;
	uint8_t AdvContextID;
	uint16_t AdvLifeTime;
	struct in6_addr AdvContextPrefix;

	struct AdvLowpanCo *next;
};

struct AdvAbro {
	uint16_t Version[2];
	uint16_t ValidLifeTime;
	struct in6_addr LBRaddress;

	struct AdvAbro *next;
};

struct AdvRASrcAddress {
	struct in6_addr address;

	struct AdvRASrcAddress *next;
};

/* Mobile IPv6 extensions */

struct AdvInterval {
	uint8_t type;
	uint8_t length;
	uint16_t reserved;
	uint32_t adv_ival;
};

struct HomeAgentInfo {
	uint8_t type;
	uint8_t length;
	uint16_t flags_reserved;
	uint16_t preference;
	uint16_t lifetime;
};

/* Uclibc : include/netinet/icmpv6.h - Added by Bhadram*/
#define ND_OPT_ARO 33
#define ND_OPT_6CO 34
#define ND_OPT_ABRO 35

struct nd_opt_abro {
	uint8_t nd_opt_abro_type;
	uint8_t nd_opt_abro_len;
	uint16_t nd_opt_abro_ver_low;
	uint16_t nd_opt_abro_ver_high;
	uint16_t nd_opt_abro_valid_lifetime;
	struct in6_addr nd_opt_abro_6lbr_address;
};

struct nd_opt_6co {
	uint8_t nd_opt_6co_type;
	uint8_t nd_opt_6co_len;
	uint8_t nd_opt_6co_context_len;
	uint8_t nd_opt_6co_res_c_cid; /* [ res=3-bits | c=1-bit | cid=4-bits ] */
	uint16_t nd_opt_6co_reserved;
	uint16_t nd_opt_6co_valid_lifetime;
	struct in6_addr nd_opt_6co_con_prefix;
}; /*Added by Bhadram */

/* gram.y */
struct Interface *readin_config(char const *fname);

/* radvd.c */

/* timer.c */
int expired(struct Interface const *iface);
int64_t timespecdiff(struct timespec const *a, struct timespec const *b);
struct timespec next_timespec(double next);
uint64_t next_time_msec(struct Interface const *iface);

/* device.c */
int check_device(int sock, struct Interface *);
int check_ip6_forwarding(void);
int check_ip6_iface_forwarding(const char *iface);
int get_v4addr(const char *, unsigned int *);
int set_interface_curhlim(const char *, uint8_t);
int set_interface_linkmtu(const char *, uint32_t);
int set_interface_reachtime(const char *, uint32_t);
int set_interface_retranstimer(const char *, uint32_t);
int setup_allrouters_membership(int sock, struct Interface *);
int setup_iface_addrs(struct Interface *);
int update_device_index(struct Interface *iface);
int update_device_info(int sock, struct Interface *);
int get_iface_addrs(char const *name, struct in6_addr *if_addr, /* the first link local addr */
		    struct in6_addr **if_addrs			/* all the addrs */
		    );

/* interface.c */
int check_iface(struct Interface *);
int setup_iface(int sock, struct Interface *iface);
struct Interface *find_iface_by_index(struct Interface *iface, int index);
struct Interface *find_iface_by_name(struct Interface *iface, const char *name);
struct Interface *find_iface_by_time(struct Interface *iface_list);
void dnssl_init_defaults(struct AdvDNSSL *, struct Interface *);
void for_each_iface(struct Interface *ifaces, void (*foo)(struct Interface *iface, void *), void *data);
void free_ifaces(struct Interface *ifaces);
void iface_init_defaults(struct Interface *);
void prefix_init_defaults(struct AdvPrefix *);
void rdnss_init_defaults(struct AdvRDNSS *, struct Interface *);
void reschedule_iface(struct Interface *iface, double next);
void route_init_defaults(struct AdvRoute *, struct Interface *);
void touch_iface(struct Interface *iface);

/* socket.c */
int open_icmpv6_socket(void);

/* send.c */
int send_ra_forall(int sock, struct Interface *iface, struct in6_addr *dest);

/* process.c */
void process(int sock, struct Interface *, unsigned char *, int, struct sockaddr_in6 *, struct in6_pktinfo *, int);

/* recv.c */
int recv_rs_ra(int sock, unsigned char *, struct sockaddr_in6 *, struct in6_pktinfo **, int *, unsigned char *);

/* util.c */
int countbits(int b);
int count_mask(struct sockaddr_in6 *m);
struct in6_addr get_prefix6(struct in6_addr const *addr, struct in6_addr const *mask);
char *strdupf(char const *format, ...) __attribute__((format(printf, 1, 2)));
double rand_between(double, double);
int check_dnssl_presence(struct AdvDNSSL *, const char *);
int check_rdnss_presence(struct AdvRDNSS *, struct in6_addr *);
void safe_buffer_resize(struct safe_buffer *sb, size_t new_capacity);
size_t safe_buffer_append(struct safe_buffer *sb, void const *m, size_t count);
size_t safe_buffer_pad(struct safe_buffer *sb, size_t count);
ssize_t readn(int fd, void *buf, size_t count);
ssize_t writen(int fd, const void *buf, size_t count);
struct safe_buffer *new_safe_buffer(void);
void addrtostr(struct in6_addr const *, char *, size_t);
void safe_buffer_free(struct safe_buffer *sb);
struct safe_buffer_list *new_safe_buffer_list(void);
void safe_buffer_list_free(struct safe_buffer_list *sbl);
struct safe_buffer_list *safe_buffer_list_append(struct safe_buffer_list *sbl);
void safe_buffer_list_to_safe_buffer(struct safe_buffer_list *sbl, struct safe_buffer *sb);

/* privsep.c */
int privsep_interface_curhlim(const char *iface, uint32_t hlim);
int privsep_interface_linkmtu(const char *iface, uint32_t mtu);
int privsep_interface_reachtime(const char *iface, uint32_t rtime);
int privsep_interface_retranstimer(const char *iface, uint32_t rettimer);
void privsep_init(int);
void privsep_set_write_fd(int);

/*
 * compat hacks in case libc and kernel get out of sync:
 *
 * glibc 2.4 and uClibc 0.9.29 introduce IPV6_RECVPKTINFO etc. and change IPV6_PKTINFO
 * This is only supported in Linux kernel >= 2.6.14
 *
 * This is only an approximation because the kernel version that libc was compiled against
 * could be older or newer than the one being run.  But this should not be a problem --
 * we just keep using the old kernel interface.
 *
 * these are placed here because they're needed in all of socket.c, recv.c and send.c
 */
#ifdef __linux__
#if defined IPV6_RECVHOPLIMIT || defined IPV6_RECVPKTINFO
#include <linux/version.h>
#if LINUX_VERSION_CODE < KERNEL_VERSION(2, 6, 14)
#if defined IPV6_RECVHOPLIMIT && defined IPV6_2292HOPLIMIT
#undef IPV6_RECVHOPLIMIT
#define IPV6_RECVHOPLIMIT IPV6_2292HOPLIMIT
#endif
#if defined IPV6_RECVPKTINFO && defined IPV6_2292PKTINFO
#undef IPV6_RECVPKTINFO
#undef IPV6_PKTINFO
#define IPV6_RECVPKTINFO IPV6_2292PKTINFO
#define IPV6_PKTINFO IPV6_2292PKTINFO
#endif
#endif
#endif
#endif
