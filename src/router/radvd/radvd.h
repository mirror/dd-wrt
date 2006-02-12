/*
 *   $Id: radvd.h,v 1.7 2001/11/14 19:58:11 lutchann Exp $
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
 *   may request it from <lutchann@litech.org>.
 *
 */

#ifndef RADV_H
#define RADV_H

#include <config.h>
#include <includes.h>
#include <defaults.h>

#define CONTACT_EMAIL	"Nathan Lutchansky <lutchann@litech.org>"

/* for log.c */
#define	L_NONE		0
#define L_SYSLOG	1
#define L_STDERR	2
#define L_LOGFILE	3

#define LOG_TIME_FORMAT "%b %d %H:%M:%S"

struct timer_lst {
	struct timeval		expires;
	void			(*handler)(void *);
	void *			data;
	struct timer_lst	*next;
	struct timer_lst	*prev;	
};

struct AdvPrefix;

#define HWADDR_MAX 16

struct Interface {
	char			Name[IFNAMSIZ];	/* interface name */

	struct in6_addr		if_addr;
	int			if_index;

	uint8_t			if_hwaddr[HWADDR_MAX];
	int			if_hwaddr_len;
	int			if_prefix_len;
	int			if_maxmtu;

	int			AdvSendAdvert;
	double			MaxRtrAdvInterval;
	double			MinRtrAdvInterval;
	int			AdvManagedFlag;
	int			AdvOtherConfigFlag;
	int			AdvLinkMTU;
	int			AdvReachableTime;
	int			AdvRetransTimer;
	int			AdvCurHopLimit;
	int			AdvDefaultLifetime;
	int			AdvSourceLLAddress;
	int			UnicastOnly;

	/* Mobile IPv6 extensions */
	int			AdvIntervalOpt;
	int			AdvHomeAgentInfo;
	int			AdvHomeAgentFlag;
	int16_t			HomeAgentPreference;
	uint16_t		HomeAgentLifetime;

	struct AdvPrefix	*AdvPrefixList;
	struct timer_lst	tm;
	unsigned long		last_multicast;
	struct Interface	*next;

};

struct AdvPrefix {
	struct in6_addr		Prefix;
	int			PrefixLen;
	
	int			AdvOnLinkFlag;
	int			AdvAutonomousFlag;
	uint32_t		AdvValidLifetime;
	uint32_t		AdvPreferredLifetime;

	/* Mobile IPv6 extensions */
        int                     AdvRouterAddr;

	/* 6to4 extensions */
	char			if6to4[IFNAMSIZ];
	int			enabled;

	struct AdvPrefix	*next;
};

/* Mobile IPv6 extensions */

struct AdvInterval {
	uint8_t			type;
	uint8_t			length;
	uint16_t		reserved;
	uint32_t		adv_ival;
};

struct HomeAgentInfo {
	uint8_t			type;
	uint8_t			length;
	uint16_t		reserved;
	int16_t			preference;
	uint16_t		lifetime;
};	


/* gram.y */
int yyparse(void);

/* scanner.l */
int yylex(void);

/* timer.c */
void set_timer(struct timer_lst *tm, double);
void clear_timer(struct timer_lst *tm);
void init_timer(struct timer_lst *, void (*)(void *), void *); 

/* log.c */
int log_open(int, char *, char*, int);
int log(int, char *, ...);
int dlog(int, int, char *, ...);
int log_close(void);
int log_reopen(void);
void set_debuglevel(int);
int get_debuglevel(void);

/* device.c */
int setup_deviceinfo(int, struct Interface *);
int check_device(int, struct Interface *);
int setup_linklocal_addr(int, struct Interface *);
int setup_allrouters_membership(int, struct Interface *);
int check_allrouters_membership(int, struct Interface *);
int get_v4addr(const char *, unsigned int *);

/* interface.c */
void iface_init_defaults(struct Interface *);
void prefix_init_defaults(struct AdvPrefix *);
int check_iface(struct Interface *);

/* socket.c */
int open_icmpv6_socket(void);

/* send.c */
void send_ra(int, struct Interface *iface, struct in6_addr *dest);

/* process.c */
void process(int sock, struct Interface *, unsigned char *, int,
	struct sockaddr_in6 *, struct in6_pktinfo *, int);

/* recv.c */
int recv_rs_ra(int, unsigned char *, struct sockaddr_in6 *, struct in6_pktinfo **, int *);

/* util.c */
void mdelay(int);
double rand_between(double, double);
void print_addr(struct in6_addr *, char *);

#endif
