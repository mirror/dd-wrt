/*
 *	BIRD -- Multi-Threaded Routing Toolkit (MRT) Protocol
 *
 *	(c) 2017--2018 Ondrej Zajicek <santiago@crfreenet.org>
 *	(c) 2017--2018 CZ.NIC z.s.p.o.
 *
 *	Can be freely distributed and used under the terms of the GNU GPL.
 */

#ifndef _BIRD_MRT_H_
#define _BIRD_MRT_H_

#include "nest/bird.h"
#include "nest/protocol.h"
#include "lib/lists.h"
#include "nest/route.h"
#include "lib/event.h"
#include "lib/hash.h"


struct mrt_config {
  struct proto_config c;

  struct rtable_config *table_cf;
  const char *table_expr;
  struct filter *filter;
  const char *filename;
  uint period;
  int always_add_path;
};

struct mrt_proto {
  struct proto p;
  timer *timer;
  event *event;

  struct mrt_target *file;
  struct mrt_table_dump_state *table_dump;
};

struct mrt_dump_data {
  const char *table_expr;
  struct rtable *table_ptr;
  struct filter *filter;
  char *filename;
};

struct mrt_peer_entry {
  u32 index;
  u32 peer_id;
  u32 peer_as;
  ip_addr peer_ip;
  struct mrt_peer_entry *next;
};

struct mrt_table_dump_state {
  struct mrt_proto *proto;		/* Protocol for regular MRT dumps (or NULL) */
  struct cli *cli;			/* CLI for irregular MRT dumps (or NULL) */
  struct config *config;		/* Config valid during start of dump, locked */

					/* Configuration information */
  const char *table_expr;		/* Wildcard for table name (or NULL) */
  struct rtable *table_ptr;		/* Explicit table (or NULL) */
  struct filter *filter;		/* Optional filter */
  const char *filename;			/* Filename pattern */
  int always_add_path;			/* Always use *_ADDPATH message subtypes */

  /* Allocated by mrt_table_dump_init() */
  pool *pool;				/* Pool for table dump */
  linpool *linpool;			/* Temporary linear pool */
  linpool *peer_lp;			/* Linear pool for peer entries in peer_hash */
  buffer buf;				/* Buffer for MRT messages */

  HASH(struct mrt_peer_entry) peer_hash; /* Hash for peers to find the index */

  struct rtable *table;			/* Processed table, NULL initially */
  struct fib_iterator fit;		/* Iterator in processed table */
  int table_open;			/* Whether iterator is linked */

  int add_path;				/* Current message subtype is *_ADDPATH */
  int want_add_path;			/* Want *_ADDPATH message later */
  int max;				/* Decreasing counter of dumped routes */
  u32 seqnum;				/* MRT message sequence number */
  bird_clock_t time_offset;		/* Time offset between monotonic and real time */

  u16 peer_count;			/* Number of peers */
  u32 peer_count_offset;		/* Buffer offset to store peer_count later */
  u16 entry_count;			/* Number of RIB Entries */
  u32 entry_count_offset;		/* Buffer offset to store entry_count later */

  struct rfile *file;			/* tracking for mrt table dump file */
  int fd;
};

struct mrt_bgp_data {
  uint peer_as;
  uint local_as;
  uint index;
  uint af;
  ip_addr peer_ip;
  ip_addr local_ip;
  byte *message;
  uint msg_len;
  uint old_state;
  uint new_state;
  u8 as4;
  u8 add_path;
};


#define MRT_HDR_LENGTH		12	/* MRT Timestamp + MRT Type + MRT Subtype + MRT Load Length */
#define MRT_PEER_TYPE_32BIT_ASN	2	/* MRT Table Dump: Peer Index Table: Peer Type: Use 32bit ASN */
#define MRT_PEER_TYPE_IPV6	1	/* MRT Table Dump: Peer Index Table: Peer Type: Use IPv6 IP Address */

#define MRT_ATTR_BUFFER_SIZE	65536

/* MRT Types */
#define MRT_TABLE_DUMP_V2 	13
#define MRT_BGP4MP		16

/* MRT Table Dump v2 Subtypes */
#define MRT_PEER_INDEX_TABLE		1
#define MRT_RIB_IPV4_UNICAST		2
#define MRT_RIB_IPV4_MULTICAST		3
#define MRT_RIB_IPV6_UNICAST		4
#define MRT_RIB_IPV6_MULTICAST 		5
#define MRT_RIB_GENERIC			6
#define MRT_RIB_IPV4_UNICAST_ADDPATH	8
#define MRT_RIB_IPV4_MULTICAST_ADDPATH	9
#define MRT_RIB_IPV6_UNICAST_ADDPATH	10
#define MRT_RIB_IPV6_MULTICAST_ADDPATH 	11
#define MRT_RIB_GENERIC_ADDPATH		12

/* MRT BGP4MP Subtypes */
#define MRT_BGP4MP_MESSAGE		1
#define MRT_BGP4MP_MESSAGE_AS4		4
#define MRT_BGP4MP_STATE_CHANGE_AS4	5
#define MRT_BGP4MP_MESSAGE_LOCAL	6
#define MRT_BGP4MP_MESSAGE_AS4_LOCAL	7
#define MRT_BGP4MP_MESSAGE_ADDPATH	8
#define MRT_BGP4MP_MESSAGE_AS4_ADDPATH	9
#define MRT_BGP4MP_MESSAGE_LOCAL_ADDPATH	10
#define MRT_BGP4MP_MESSAGE_AS4_LOCAL_ADDPATH	11


#ifdef CONFIG_MRT
void mrt_dump_cmd(struct mrt_dump_data *d);
void mrt_dump_bgp_message(struct mrt_bgp_data *d);
void mrt_dump_bgp_state_change(struct mrt_bgp_data *d);
void mrt_check_config(struct proto_config *C);
#else
static inline void mrt_dump_bgp_message(struct mrt_bgp_data *d UNUSED) { }
static inline void mrt_dump_bgp_state_change(struct mrt_bgp_data *d UNUSED) { }
#endif

#endif	/* _BIRD_MRT_H_ */
