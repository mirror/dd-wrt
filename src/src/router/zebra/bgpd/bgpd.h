/* BGP message definition header.
 * Copyright (C) 1996, 97, 98, 99, 2000 Kunihiro Ishiguro
 *
 * This file is part of GNU Zebra.
 *
 * GNU Zebra is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2, or (at your option) any
 * later version.
 *
 * GNU Zebra is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with GNU Zebra; see the file COPYING.  If not, write to the Free
 * Software Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.  
 */

#ifndef _ZEBRA_BGPD_H
#define _ZEBRA_BGPD_H

#include "sockunion.h"

/* Typedef BGP specific types.  */
typedef u_int16_t as_t;
typedef u_int16_t bgp_size_t;

/* BGP master for system wide configurations and variables.  */
struct bgp_master
{
  u_char flags;
#define BGP_DO_NOT_INSTALL_TO_FIB      (1 << 0)

  /* BGP start time.  */
  time_t start_time;

  /* BGP program name.  */
  char *progname;

  /* BGP thread master.  */
  struct thread_master *master;
};

/* BGP instance structure.  */
struct bgp 
{
  /* AS number of this BGP instance.  */
  as_t as;

  /* Name of this BGP instance.  */
  char *name;

  /* BGP configuration.  */
  u_int16_t config;
#define BGP_CONFIG_ROUTER_ID              (1 << 0)
#define BGP_CONFIG_CLUSTER_ID             (1 << 1)
#define BGP_CONFIG_CONFEDERATION          (1 << 2)
#define BGP_CONFIG_ALWAYS_COMPARE_MED     (1 << 3)
#define BGP_CONFIG_DETERMINISTIC_MED      (1 << 4)
#define BGP_CONFIG_MED_MISSING_AS_WORST   (1 << 5)
#define BGP_CONFIG_MED_CONFED             (1 << 6)
#define BGP_CONFIG_NO_DEFAULT_IPV4        (1 << 7)
#define BGP_CONFIG_NO_CLIENT_TO_CLIENT    (1 << 8)
#define BGP_CONFIG_ENFORCE_FIRST_AS       (1 << 9)
#define BGP_CONFIG_COMPARE_ROUTER_ID      (1 << 10)
#define BGP_CONFIG_ASPATH_IGNORE          (1 << 11)
#define BGP_CONFIG_DAMPENING              (1 << 12)
#define BGP_CONFIG_IMPORT_CHECK           (1 << 13)

  /* BGP identifier.  */
  struct in_addr id;

  /* BGP route reflector cluster ID.  */
  struct in_addr cluster;

  /* BGP Confederation Information.  */
  as_t confederation_id;
  int confederation_peers_cnt;
  as_t *confederation_peers;

  /* BGP peer.  */
  struct list *peer_group;

  /* BGP peer-conf.  */
  struct list *peer_conf;

  /* Static route configuration.  */
  struct route_table *route[AFI_MAX][SAFI_MAX];

  /* Aggregate address configuration.  */
  struct route_table *aggregate[AFI_MAX][SAFI_MAX];

  /* BGP Routing information base.  */
  struct route_table *rib[AFI_MAX][SAFI_MAX];

  /* BGP redistribute configuration. */
  u_char redist[AFI_MAX][ZEBRA_ROUTE_MAX];

  /* BGP redistribute route-map.  */
  struct
  {
    char *name;
    struct route_map *map;
  } rmap[AFI_MAX][ZEBRA_ROUTE_MAX];

  /* BGP distance configuration.  */
  u_char distance_ebgp;
  u_char distance_ibgp;
  u_char distance_local;
  
  /* BGP default local-preference.  */
  u_int32_t default_local_pref;

  /* BGP default timer.  */
  u_int32_t default_holdtime;
  u_int32_t default_keepalive;
};

/* BGP filter structure. */
struct bgp_filter
{
  /* Distribute-list.  */
  struct 
  {
    char *name;
    struct access_list *alist;
  } dlist[FILTER_MAX];

  /* Prefix-list.  */
  struct
  {
    char *name;
    struct prefix_list *plist;
  } plist[FILTER_MAX];

  /* Filter-list.  */
  struct
  {
    char *name;
    struct as_list *aslist;
  } aslist[FILTER_MAX];

  /* Route-map.  */
  struct
  {
    char *name;
    struct route_map *map;
  } map[FILTER_MAX];
};

/* BGP peer-group support. */
struct peer_group
{
  /* Name of the peer-group. */
  char *name;
  
  /* Address family configuration. */
  int afi;
  int safi;

  /* AS configuration. */
  as_t as;

  /* Peer-group client list. */
  struct list *peer_conf;
};

/* BGP peer configuration. */
struct peer_conf
{
  /* Pointer to BGP structure. */
  struct bgp *bgp;

  /* Pointer to peer. */
  struct peer *peer;

  /* Pointer to peer-group. */
  struct peer_group *group;

  /* Address Family Configuration. */
  u_char afc[AFI_MAX][SAFI_MAX];

  /* Prefix count. */
  unsigned long pcount[AFI_MAX][SAFI_MAX];

  /* Max prefix count. */
  unsigned long pmax[AFI_MAX][SAFI_MAX];
  u_char pmax_warning[AFI_MAX][SAFI_MAX];

  /* Filter structure. */
  struct bgp_filter filter[AFI_MAX][SAFI_MAX];
};

/* Next hop self address. */
struct bgp_nexthop
{
  struct interface *ifp;
  struct in_addr v4;
#ifdef HAVE_IPV6
  struct in6_addr v6_global;
  struct in6_addr v6_local;
#endif /* HAVE_IPV6 */  
};

/* Store connection information. */
struct peer_connection
{
  /* Peer is on the connected link. */
  int shared;

  /* Socket's remote and local address. */
  union sockunion remote;
  union sockunion local;

  /* For next-hop-self detemination. */
  struct interface *ifp;
  struct in_addr v4;
#ifdef HAVE_IPV6
  struct in6_addr v6_global;
  struct in6_addr v6_local;
#endif /* HAVE_IPV6 */  
};

/* Update source configuration. */
struct peer_source
{
  unsigned int ifindex;
  char *ifname;
  char *update_if;
  union sockunion *update_source;
};

/* BGP Notify message format. */
struct bgp_notify 
{
  u_char code;
  u_char subcode;
  char *data;
  bgp_size_t length;
};

/* BGP neighbor structure. */
struct peer
{
  /* Peer's remote AS number. */
  as_t as;			

  /* Peer's local AS number. */
  as_t local_as;

  /* Remote router ID. */
  struct in_addr remote_id;

  /* Local router ID. */
  struct in_addr local_id;

  /* Packet receive and send buffer. */
  struct stream *ibuf;
  struct stream_fifo *obuf;

  /* Status of the peer. */
  int status;
  int ostatus;

  /* Peer information */
  int fd;			/* File descriptor */
  int ttl;			/* TTL of TCP connection to the peer. */
  char *desc;			/* Description of the peer. */
  unsigned short port;          /* Destination port for peer */
  char *host;			/* Printable address of the peer. */
  union sockunion su;		/* Sockunion address of the peer. */
  time_t uptime;		/* Last Up/Down time */
  time_t readtime;		/* Last read time */
  safi_t translate_update;       
  
  unsigned int ifindex;		/* ifindex of the BGP connection. */
  char *ifname;			/* bind interface name. */
  char *update_if;
  union sockunion *update_source;
  struct zlog *log;
  u_char version;		/* Peer BGP version. */

  union sockunion *su_local;	/* Sockunion of local address.  */
  union sockunion *su_remote;	/* Sockunion of remote address.  */
  int shared_network;		/* Is this peer shared same network. */
  struct bgp_nexthop nexthop;	/* Nexthop */

  /* Peer address family configuration. */
  u_char afc[AFI_MAX][SAFI_MAX];
  u_char afc_nego[AFI_MAX][SAFI_MAX];
  u_char afc_adv[AFI_MAX][SAFI_MAX];
  u_char afc_recv[AFI_MAX][SAFI_MAX];

  /* Route refresh capability. */
  u_char refresh_adv;
  u_char refresh_nego_old;
  u_char refresh_nego_new;

  /* Global configuration flags. */
  u_int32_t flags;
#define PEER_FLAG_PASSIVE                   (1 << 0) /* passive mode */
#define PEER_FLAG_SHUTDOWN                  (1 << 1) /* shutdown */
#define PEER_FLAG_DEFAULT_ORIGINATE         (1 << 2) /* default-originate */
#define PEER_FLAG_DONT_CAPABILITY           (1 << 3) /* dont-capability */
#define PEER_FLAG_OVERRIDE_CAPABILITY       (1 << 4) /* override-capability */
#define PEER_FLAG_STRICT_CAP_MATCH          (1 << 5) /* strict-capability-match */
#define PEER_FLAG_CAPABILITY_ROUTE_REFRESH  (1 << 6) /* route-refresh */
#define PEER_FLAG_TRANSPARENT_AS            (1 << 7) /* transparent-as */
#define PEER_FLAG_TRANSPARENT_NEXTHOP       (1 << 8) /* transparent-next-hop */
#define PEER_FLAG_IGNORE_LINK_LOCAL_NEXTHOP (1 << 9) /* ignore-link-local-nexthop */

  /* Per AF configuration flags. */
  u_int32_t af_flags[AFI_MAX][SAFI_MAX];
#define PEER_FLAG_SEND_COMMUNITY            (1 << 0) /* send-community */
#define PEER_FLAG_SEND_EXT_COMMUNITY        (1 << 1) /* send-community ext. */
#define PEER_FLAG_NEXTHOP_SELF              (1 << 2) /* next-hop-self */
#define PEER_FLAG_REFLECTOR_CLIENT          (1 << 3) /* reflector-client */
#define PEER_FLAG_RSERVER_CLIENT            (1 << 4) /* route-server-client */
#define PEER_FLAG_SOFT_RECONFIG             (1 << 5) /* soft-reconfiguration */

  /* Peer status flags. */
  u_int16_t sflags;
#define PEER_STATUS_ACCEPT_PEER	      (1 << 0) /* accept peer */
#define PEER_STATUS_PREFIX_OVERFLOW   (1 << 1) /* prefix-overflow */
#define PEER_STATUS_CAPABILITY_OPEN   (1 << 2) /* capability open send */
#define PEER_STATUS_HAVE_ACCEPT       (1 << 3) /* accept peer's parent */

  /* Default attribute value for the peer. */
  u_int32_t config;
#define PEER_CONFIG_TIMER             (1 << 0) /* keepalive & holdtime */
#define PEER_CONFIG_CONNECT           (1 << 1) /* connect */
  u_int32_t weight;
  u_int32_t holdtime;
  u_int32_t keepalive;
  u_int32_t connect;

  /* Global timer */
  u_int32_t global_holdtime;
  u_int32_t global_keepalive;

  /* Timer values. */
  u_int32_t v_start;
  u_int32_t v_connect;
  u_int32_t v_holdtime;
  u_int32_t v_keepalive;
  u_int32_t v_asorig;
  u_int32_t v_routeadv;

  /* Threads. */
  struct thread *t_read;
  struct thread *t_write;
  struct thread *t_start;
  struct thread *t_connect;
  struct thread *t_holdtime;
  struct thread *t_keepalive;
  struct thread *t_asorig;
  struct thread *t_routeadv;

  /* Statistics field */
  u_int32_t open_in;		/* Open message input count */
  u_int32_t open_out;		/* Open message output count */
  u_int32_t update_in;		/* Update message input count */
  u_int32_t update_out;		/* Update message ouput count */
  time_t update_time;		/* Update message received time. */
  u_int32_t keepalive_in;	/* Keepalive input count */
  u_int32_t keepalive_out;	/* Keepalive output count */
  u_int32_t notify_in;		/* Notify input count */
  u_int32_t notify_out;		/* Notify output count */
  u_int32_t refresh_in;		/* Route Refresh input count */
  u_int32_t refresh_out;	/* Route Refresh output count */

  /* BGP state count */
  u_int32_t established;	/* Established */
  u_int32_t dropped;		/* Dropped */

  /* Adj-RIBs-In.  */
  struct route_table *adj_in[AFI_MAX][SAFI_MAX];
  struct route_table *adj_out[AFI_MAX][SAFI_MAX];

  /* Linked peer configuration. */
  struct list *conf;

  /* Notify data. */
  struct bgp_notify notify;

  /* Whole packet size to be read. */
  unsigned long packet_size;
};

/* This structure's member directly points incoming packet data
   stream. */
struct bgp_nlri
{
  /* AFI.  */
  afi_t afi;

  /* SAFI.  */
  safi_t safi;

  /* Pointer to NLRI byte stream.  */
  u_char *nlri;

  /* Length of whole NLRI.  */
  bgp_size_t length;
};

/* BGP version.  Zebra bgpd supports BGP-4 and it's various
   extensions.  */
#define BGP_VERSION_4		           4
#define BGP_VERSION_MP_4_DRAFT_00         40

/* Default BGP port number.  */
#define BGP_PORT_DEFAULT                 179

/* BGP message header and packet size.  */
#define BGP_MARKER_SIZE		          16
#define BGP_HEADER_SIZE		          19
#define BGP_MAX_PACKET_SIZE             4096

/* BGP minimum message size.  */
#define BGP_MSG_OPEN_MIN_SIZE              (BGP_HEADER_SIZE + 10)
#define BGP_MSG_UPDATE_MIN_SIZE            (BGP_HEADER_SIZE + 4)
#define BGP_MSG_NOTIFY_MIN_SIZE            (BGP_HEADER_SIZE + 2)
#define BGP_MSG_KEEPALIVE_MIN_SIZE         (BGP_HEADER_SIZE + 0)
#define BGP_MSG_ROUTE_REFRESH_MIN_SIZE     (BGP_HEADER_SIZE + 4)

/* BGP message types.  */
#define	BGP_MSG_OPEN		           1
#define	BGP_MSG_UPDATE		           2
#define	BGP_MSG_NOTIFY		           3
#define	BGP_MSG_KEEPALIVE	           4
#define BGP_MSG_ROUTE_REFRESH_01           5
#define BGP_MSG_ROUTE_REFRESH	         128

/* BGP open optional parameter.  */
#define BGP_OPEN_OPT_AUTH                  1
#define BGP_OPEN_OPT_CAP                   2

/* BGP attribute type codes.  */
#define BGP_ATTR_ORIGIN                    1
#define BGP_ATTR_AS_PATH                   2
#define BGP_ATTR_NEXT_HOP                  3
#define BGP_ATTR_MULTI_EXIT_DISC           4
#define BGP_ATTR_LOCAL_PREF                5
#define BGP_ATTR_ATOMIC_AGGREGATE          6
#define BGP_ATTR_AGGREGATOR                7
#define BGP_ATTR_COMMUNITIES               8
#define BGP_ATTR_ORIGINATOR_ID             9
#define BGP_ATTR_CLUSTER_LIST             10
#define BGP_ATTR_DPA                      11
#define BGP_ATTR_ADVERTISER               12
#define BGP_ATTR_RCID_PATH                13
#define BGP_ATTR_MP_REACH_NLRI            14
#define BGP_ATTR_MP_UNREACH_NLRI          15
#define BGP_ATTR_EXT_COMMUNITIES          16

/* BGP update origin.  */
#define BGP_ORIGIN_IGP                     0
#define BGP_ORIGIN_EGP                     1
#define BGP_ORIGIN_INCOMPLETE              2

/* BGP notify message codes.  */
#define BGP_NOTIFY_HEADER_ERR              1
#define BGP_NOTIFY_OPEN_ERR                2
#define BGP_NOTIFY_UPDATE_ERR              3
#define BGP_NOTIFY_HOLD_ERR                4
#define BGP_NOTIFY_FSM_ERR                 5
#define BGP_NOTIFY_CEASE                   6
#define BGP_NOTIFY_MAX	                   7

/* BGP_NOTIFY_HEADER_ERR sub codes.  */
#define BGP_NOTIFY_HEADER_NOT_SYNC         1
#define BGP_NOTIFY_HEADER_BAD_MESLEN       2
#define BGP_NOTIFY_HEADER_BAD_MESTYPE      3
#define BGP_NOTIFY_HEADER_MAX              4

/* BGP_NOTIFY_OPEN_ERR sub codes.  */
#define BGP_NOTIFY_OPEN_UNSUP_VERSION      1
#define BGP_NOTIFY_OPEN_BAD_PEER_AS        2
#define BGP_NOTIFY_OPEN_BAD_BGP_IDENT      3
#define BGP_NOTIFY_OPEN_UNSUP_PARAM        4
#define BGP_NOTIFY_OPEN_AUTH_FAILURE       5
#define BGP_NOTIFY_OPEN_UNACEP_HOLDTIME    6
#define BGP_NOTIFY_OPEN_UNSUP_CAPBL        7
#define BGP_NOTIFY_OPEN_MAX                8

/* BGP_NOTIFY_UPDATE_ERR sub codes.  */
#define BGP_NOTIFY_UPDATE_MAL_ATTR         1
#define BGP_NOTIFY_UPDATE_UNREC_ATTR       2
#define BGP_NOTIFY_UPDATE_MISS_ATTR        3
#define BGP_NOTIFY_UPDATE_ATTR_FLAG_ERR    4
#define BGP_NOTIFY_UPDATE_ATTR_LENG_ERR    5
#define BGP_NOTIFY_UPDATE_INVAL_ORIGIN     6
#define BGP_NOTIFY_UPDATE_AS_ROUTE_LOOP    7
#define BGP_NOTIFY_UPDATE_INVAL_NEXT_HOP   8
#define BGP_NOTIFY_UPDATE_OPT_ATTR_ERR     9
#define BGP_NOTIFY_UPDATE_INVAL_NETWORK   10
#define BGP_NOTIFY_UPDATE_MAL_AS_PATH     11
#define BGP_NOTIFY_UPDATE_MAX             12

/* BGP finite state machine status.  */
#define Idle                               1
#define Connect                            2
#define Active                             3
#define OpenSent                           4
#define OpenConfirm                        5
#define Established                        6
#define BGP_STATUS_MAX                     7

/* BGP finite state machine events.  */
#define BGP_Start                          1
#define BGP_Stop                           2
#define TCP_connection_open                3
#define TCP_connection_closed              4
#define TCP_connection_open_failed         5
#define TCP_fatal_error                    6
#define ConnectRetry_timer_expired         7
#define Hold_Timer_expired                 8
#define KeepAlive_timer_expired            9
#define Receive_OPEN_message              10
#define Receive_KEEPALIVE_message         11
#define Receive_UPDATE_message            12
#define Receive_NOTIFICATION_message      13
#define BGP_EVENTS_MAX                    14

/* Time in second to start bgp connection. */
#define BGP_INIT_START_TIMER               5
#define BGP_ERROR_START_TIMER             30
#define BGP_DEFAULT_HOLDTIME             180
#define BGP_DEFAULT_KEEPALIVE             60 
#define BGP_DEFAULT_ASORIGINATE           15
#define BGP_DEFAULT_ROUTEADV              30
#define BGP_CLEAR_CONNECT_RETRY           20
#define BGP_DEFAULT_CONNECT_RETRY        120

/* BGP default local preference.  */
#define BGP_DEFAULT_LOCAL_PREF           100

/* SAFI which used in open capability negotiation.  */
#define BGP_SAFI_VPNV4                   128

/* Default max TTL.  */
#define TTL_MAX                          255

/* Default configuration file name for bgpd.  */
#define BGP_VTY_PORT                    2605
#define BGP_VTYSH_PATH          "/tmp/.bgpd"
#define BGP_DEFAULT_CONFIG       "bgpd.conf"

/* Check AS path loop when we send NLRI.  */
/* #define BGP_SEND_ASPATH_CHECK */

/* IBGP/EBGP identifier.  We also have a CONFED peer, which is to say,
   a peer who's AS is part of our Confederation.  */
enum
{
  BGP_PEER_IBGP,
  BGP_PEER_EBGP,
  BGP_PEER_INTERNAL,
  BGP_PEER_CONFED
};

/* Macros. */
#define BGP_INPUT(P)         ((P)->ibuf)
#define BGP_INPUT_PNT(P)     (STREAM_PNT(BGP_INPUT(P)))

/* Macro to check BGP information is alive or not.  */
#define BGP_INFO_HOLDDOWN(BI)                         \
  (! CHECK_FLAG ((BI)->flags, BGP_INFO_VALID)         \
   || CHECK_FLAG ((BI)->flags, BGP_INFO_HISTORY)      \
   || CHECK_FLAG ((BI)->flags, BGP_INFO_DAMPED))

/* Count prefix size from mask length */
#define PSIZE(a) (((a) + 7) / (8))

/* Prototypes. */
void bgp_init ();
void zebra_init ();
void bgp_terminate (void);
void bgp_reset (void);
void bgp_route_map_init ();
int peer_sort (struct peer *peer);
void bgp_filter_init ();
void bgp_zclient_reset ();
void bgp_snmp_init ();
struct peer *peer_lookup_by_su (union sockunion *);
struct peer *peer_lookup_from_bgp (struct bgp *bgp, char *addr);
struct peer *peer_lookup_by_host (char *host);
struct peer *peer_new (void);
void event_add (struct peer *peer, int event);
void bgp_clear(struct peer *peer, int error);
void peer_delete_all ();
void peer_delete (struct peer *peer);
void bgp_open_recv (struct peer *peer, u_int16_t size);
void bgp_notify_print (struct peer *, struct bgp_notify *, char *);
int bgp_nexthop_set (union sockunion *, union sockunion *, 
		     struct bgp_nexthop *, struct peer *);
int bgp_confederation_peers_check(struct bgp *, as_t);
struct bgp *bgp_get_default ();
struct bgp *bgp_lookup_by_name (char *);
void bgp_config_write_family_header (struct vty *, afi_t, safi_t, int *);
struct peer *peer_lookup_with_open (union sockunion *, as_t, struct in_addr *,
				    int *);
struct peer *peer_create_accept ();
int peer_active (struct peer *);
char *peer_uptime (time_t, char *, size_t);
void bgpTrapEstablished (struct peer *);
void bgpTrapBackwardTransition (struct peer *);

extern struct message bgp_status_msg[];
extern int bgp_status_msg_max;

/* All BGP instance. */
extern struct list *bgp_list;

/* All peer instance.  This linked list is rarely used.  Usually
   bgp_list is used to walk down peer's list.  */
extern struct list *peer_list;

extern time_t bgp_start_time;

extern int no_kernel_mode;

extern char *progname;

extern struct thread_master *master;

/* Using MPLS_VPN */
extern struct cmd_element exit_address_family_cmd;
extern struct cmd_element neighbor_send_community_cmd;
extern struct cmd_element neighbor_send_community_type_cmd;
extern struct cmd_element neighbor_nexthop_self_cmd;
extern struct cmd_element no_neighbor_nexthop_self_cmd;
extern struct cmd_element no_neighbor_send_community_cmd;
extern struct cmd_element no_neighbor_send_community_type_cmd;
extern struct cmd_element neighbor_route_reflector_client_cmd;
extern struct cmd_element no_neighbor_route_reflector_client_cmd;
extern struct cmd_element neighbor_route_server_client_cmd;
extern struct cmd_element no_neighbor_route_server_client_cmd;
extern struct cmd_element neighbor_distribute_list_cmd;
extern struct cmd_element no_neighbor_distribute_list_cmd;
extern struct cmd_element neighbor_prefix_list_cmd;
extern struct cmd_element no_neighbor_prefix_list_cmd;
extern struct cmd_element neighbor_filter_list_cmd;
extern struct cmd_element no_neighbor_filter_list_cmd;
extern struct cmd_element neighbor_route_map_cmd;
extern struct cmd_element no_neighbor_route_map_cmd;
extern struct cmd_element neighbor_maximum_prefix_cmd;
extern struct cmd_element neighbor_maximum_prefix_warning_cmd;
extern struct cmd_element no_neighbor_maximum_prefix_cmd;
extern struct cmd_element no_neighbor_maximum_prefix_val_cmd;
extern struct cmd_element no_neighbor_maximum_prefix_val2_cmd;
#endif /* _ZEBRA_BGPD_H */
