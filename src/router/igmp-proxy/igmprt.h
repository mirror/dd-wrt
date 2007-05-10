#ifndef __IGMPRT_H__
#define __IGMPRT_H__
#include "util.h"

#define	IGMP_TIMER_SCALE	10
#define IGMP_DEF_RV		2
#define IGMP_DEF_QI		125
#ifdef IGMP_SEND_THE_SECOND_QUERY_PACKET
#define IGMP_DEF_FIRST_QI		30
#endif	/*  */
#define IGMP_DEF_QRI		10
#define IGMP_GMI		((IGMP_DEF_RV * IGMP_DEF_QI) + (IGMP_DEF_QRI * IGMP_TIMER_SCALE))
#define IGMP_OQPI		((IGMP_DEF_RV * IGMP_DEF_QI) + (IGMP_DEF_QRI * IGMP_TIMER_SCALE)/2)
#define IGMP_DEF_LMQI		1
#define IGMP_DEF_LMQC		IGMP_DEF_RV
  
#define UPSTREAM_INTERFACE	1
#define DOWNSTREAM_INTERFACE	2
/* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
/* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
#define BOTHSTREAM_INTERFACE    3
  
#define MAX_NUMBER_GROUPS	128
#define MAX_NUMBER_QUERIES	100
#define	MAX_BUFFER_SIZE		1600
#define MAXCTRLSIZE		(sizeof(struct cmsghdr) +sizeof(struct in_pktinfo))
  
#define htonl(x)		__bswap_32 (x)
#define LOCAL_MCAST(x)		(((x) & htonl(0xFFFFFF00)) == htonl(0xE0000000))
#define show_usage()		usage(argv[0])
  
/* IGMP router config */ 
  typedef struct _igmp_cfg_t
{
  
    /* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
    /* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
  char upstream_ifname[IFNAMSIZ];
   char bothstream_ifaceName[2][IFNAMSIZ];
   int igmp_oqp;		/* other querier present timer */
   int igmp_qi;		/* query interval */
   int igmp_qri;		/* query response interval */
   int igmp_gmi;		/* group membership interval */
   int igmp_lmqi;		/* last member query interval */
   int igmp_lmqc;		/* last member query count */
 } igmp_cfg_t;

/* IGMP query format */ 
  typedef struct _igmpq_t
{
  set_t links;
  int type;
   int max_resp_time;
   unsigned int group_addr;
   
#ifdef IGMP_SEND_THE_SECOND_QUERY_PACKET
  int bLeave;
   
#endif	/*  */
} igmpq_t;

/* IGMP group type */ 
  typedef struct _igmp_group_t
{
  set_t links;
  igmp_cfg_t * config;
  unsigned int group_addr;
   int group_timer;
   int v1_host_timer;
   int rtx_timer;
   int state;
   igmpq_t * query_list;
 } igmp_group_t;

/* IGMP interface type */ 
  typedef struct _igmp_interface_t
{
  set_t links;
  int mode;
   int socket;
   char name[IFNAMSIZ];
   int if_index;
   struct in_addr ip_addr;
   short int flags;
   unsigned int vif_index;
   int is_querier;
   int general_query_timer;
   int querier_present_timer;
   igmp_group_t * group_database;
 } igmp_interface_t;

/* IGMP router type */ 
  typedef struct _igmp_router_t
{
  igmp_interface_t * network_if_list;
  igmp_cfg_t config;
  igmp_interface_t * upstream_interface;
  
    /* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
    /* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
    //Cached entry to the network interface that is confgiured as BOTHSTREAM.
  igmp_interface_t * ptr_bothstream_interface[2];
  int igmp_socket;
   igmp_group_t * free_group_list;
   igmpq_t * free_query_list;
 } igmp_router_t;

/* IGMP group states */ 
  enum
{ IGMP_NO_MEMBERS_PRESENT, IGMP_MEMBERS_PRESENT, IGMP_V1_MEMBERS_PRESENT,
    IGMP_CHECKING_MEMBERSHIP 
};

/* Prototype declarations */ 
/* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
/* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
//igmp_group_t* igmp_group_timer_expire(igmp_router_t*,igmp_interface_t*,igmp_group_t*);
//igmp_group_t* igmp_rtx_timer_expire(igmp_router_t*,igmp_interface_t*,igmp_group_t*);
  igmp_group_t * igmp_group_timer_expire (igmp_router_t *, igmp_interface_t *,
					  igmp_group_t *, int);
igmp_group_t * igmp_rtx_timer_expire (igmp_router_t *, igmp_interface_t *,
				       igmp_group_t *, int);
igmp_group_t * igmprt_group_lookup (igmp_interface_t *, unsigned int);
int igmprt_init (igmp_router_t *);
void igmprt_stop (igmp_router_t *);
void igmprt_kill (void);
void igmprt_tick (void);
void igmp_interface_cleanup (igmp_router_t *, igmp_interface_t *);
void igmp_timer_expire (igmp_router_t *);
void igmp_v1_host_timer_expire (igmp_router_t *, igmp_interface_t *,
				 igmp_group_t *);
void igmprt_send_query (igmp_router_t *, igmp_interface_t *, unsigned char,
			 unsigned int);
void igmprt_received_general_query (igmp_router_t *, igmp_interface_t *,
				     struct iphdr *);
void igmprt_received_v1_report (igmp_router_t *, igmp_interface_t *,
				 struct igmphdr *);
void igmprt_received_v2_report (igmp_router_t *, igmp_interface_t *,
				 struct igmphdr *);
void igmprt_received_leave (igmp_router_t *, igmp_interface_t *,
			     struct igmphdr *);
void igmp_read_data (igmp_router_t *);
void igmp_add_membership (igmp_router_t *, igmp_group_t *);

/* ******* zg 2007-01-04 porting WAG54Gv3 Source code ******* */ 
/* ******* To fix cdrouter v3.4 item 295(cdrouter_mcast_11) bug ******* */ 
//void igmp_drop_membership(igmp_router_t*,igmp_group_t*);
void igmp_drop_membership (igmp_router_t *, igmp_group_t *,
			   igmp_interface_t *, int);
void usage (char *);
void log (int, char *, ...);

#ifdef IGMP_BLOCK_SOME_SPEC_ADDRS
int is_spec_addr (unsigned int group);

#endif	/*  */
  
#endif	/*  */
